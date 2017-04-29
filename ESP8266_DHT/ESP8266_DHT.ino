// www.arduinesp.com
//
// Plot DTH11 data on thingspeak.com using an ESP8266
// April 11 2015
// Author: Jeroen Beemster
// Website: www.arduinesp.com

#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;



// replace with your channel’s thingspeak API key,
String apiKey = "BT03YLMSOHXKGJXQ";
const char* ssid = "VVAirNet2";
const char* password = "hans200770";

const char* server = "api.thingspeak.com";
#define DHTPIN 2 // what pin we’re connected to
#define DHTTYPE DHT22

int dust = D0;
pinMode (dust, OUTPUT);


DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

void setup() {

  digitalWrite (dust, HIGH);

  Serial.begin(9600);
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }


  Serial.begin(115200);
  delay(10);
  dht.begin();

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

void loop() {

  digitalWrite(dust, LOW);
  delay(50);
  long adcValue = analogRead(A0);
  digitalWrite(dust, HIGH);
  long adcVoltage = adcValue * (5.0 / 1024);
  if (adcVoltage < 0.583)
    long dustConcentration = 0;
  else
    long dustConcentration = 6 * adcVoltage / 35 - 0.1;




  float h = dht.readHumidity();
  float t = dht.readTemperature();

  double gamma = log(h / 100) + ((17.62 * t) / (243.5 + t));
  double dp = 243.5 * gamma / (17.62 - gamma);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  if (!bmp.begin()) {
    Serial.print("Failed to read from BMP sensor!!");
    while (1);
  }
  float p = bmp.readPressure() / 100;
  float t2 = bmp.readTemperature();

  if (client.connect(server, 80)) { // "184.106.153.149" or api.thingspeak.com
    long rssi = WiFi.RSSI();

    //--- extra---- you can measure dew point with the temperature and the humidity



    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(p);
    postStr += "&field4=";
    postStr += String(t2);
    postStr += "&field5=";
    postStr += String(rssi);
    postStr += "&field6=";
    postStr += String(dp);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius Humidity: ");
    Serial.print(h);
    Serial.print("Pressure: ");
    Serial.print(p);
    Serial.print("Temperatur_BMP180: ");
    Serial.print(t2);
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.print("Dew point: ");
    Serial.print(dp);
    Serial.println("% send to Thingspeak");
  }
  client.stop();

  Serial.println("Waiting…");
  // thingspeak needs minimum 15 sec delay between updates
  delay(15000);
}

