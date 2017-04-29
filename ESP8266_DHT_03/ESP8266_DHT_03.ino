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
#include "OLED.h"
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

#define OLED_address  0x3c

int dust = 13;
float dustConcentration;

// Declare OLED display
// display(SDA, SCL);
// SDA and SCL are the GPIO pins of ESP8266 that are connected to respective pins of display.
OLED display(4, 5);


DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

void setup() {
  pinMode (dust, OUTPUT);
  digitalWrite (dust, HIGH);
  pinMode (A0, INPUT);



  Serial.begin(9600);



  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }


  Serial.begin(115200);
  delay(10);
  dht.begin();

  Serial.println("OLED test!");
  display.begin();

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
  delayMicroseconds(50);
  float adcValue = analogRead(A0);
  digitalWrite(dust, HIGH);
  float adcVoltage = adcValue * (5.0 / 1024);
  if (adcVoltage < 0.27)
    dustConcentration = 0;
  else
    dustConcentration = 1000 * (0.185 * adcVoltage - 0.05);




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
    postStr += "&field7=";
    postStr += String(dustConcentration);
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

    Serial.print("Temperatur: ");
    Serial.print(t);
    Serial.print(" Relative Luftfeuchtigkeit: ");
    Serial.print(h);
    Serial.print(" Luftdruck: ");
    Serial.print(p);
    Serial.print(" Temperatur_BMP180: ");
    Serial.print(t2);
    Serial.print(" RSSI: ");
    Serial.print(rssi);
    Serial.print(" Taupunkt: ");
    Serial.print(dp);
    Serial.print(" Feinstaub: ");
    Serial.print(dustConcentration);
    Serial.print(" adcVoltage: ");
    Serial.print(adcVoltage);
    Serial.print(" adcValue: ");
    Serial.print(adcValue);
    Serial.println(" send to Thingspeak");

    // OLED messages
    display.print("*** HASENBAU ***", 0);
    
    String ssidString = String(ssid);
    int length = ssidString.length();
    char ssid[length];
    ssidString.toCharArray(ssid, ssidString.length() + 1);
    display.print(ssid, 1);

    String temperatureString = "Temp.:   " + String(t) + " C ";
    length = temperatureString.length();
    char temperature[length];
    temperatureString.toCharArray(temperature, temperatureString.length() + 1);
    display.print(temperature, 2);

    String humidityString = "rel. LF: " + String(h) + " %";
    length = humidityString.length();
    char humidity[length];
    humidityString.toCharArray(humidity, humidityString.length() + 1);
    display.print(humidity, 3);

    String rssiString = String(rssi) + " dB";
    length = rssiString.length();
    char rssi2[length];
    rssiString.toCharArray(rssi2, rssiString.length() + 1);
    display.print(rssi2, 1, 10);

    String pressureString = "Druck: " + String(p) + " hPa";
    length = pressureString.length();
    char pressure[length];
    pressureString.toCharArray(pressure, pressureString.length() + 1);
    display.print(pressure, 4);
    
    String dewPointString = "Taupunkt: " + String(dp) + " C";
    length = dewPointString.length();
    char dewPoint[length];
    dewPointString.toCharArray(dewPoint, dewPointString.length() + 1);
    display.print(dewPoint, 5);





  }
  client.stop();

  Serial.println("Waiting…");
  // thingspeak needs minimum 15 sec delay between updates
  delay(15000);
}

