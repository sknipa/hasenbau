
// Plot DTH11 data on thingspeak.com and BLYNK using an ESP8266
// March 25 2017
// Author: Johannes Stein
// Website: www.arduinesp.com
// HASENBAU 01

#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include "OLED.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;


WiFiClient client;

// TIMER
#include <SimpleTimer.h>
SimpleTimer timer;

// BLYNK
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
char auth[] = "44ef566040104bfab007adedc7673755";
const char* ssid = "VVAirNet2";
const char* password = "hans200770";

// replace with your channel’s thingspeak API key,
const char* server = "api.thingspeak.com";
String apiKey = "BT03YLMSOHXKGJXQ";

#define DHTPIN 2 // what pin we’re connected to
#define DHTTYPE DHT22
#define OLED_address  0x3c

int dust = 13;
float dustConcentration;
float t;
float h;
long rssi;
double dp;
float p;
float t2;


// Declare OLED display
// display(SDA, SCL);
// SDA and SCL are the GPIO pins of ESP8266 that are connected to respective pins of display.
OLED display(4, 5);

DHT dht(DHTPIN, DHTTYPE);

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
  display.begin();
  Blynk.begin(auth, ssid, password);

  dht.begin();

  timer.setInterval(5000L, getDhtData);
  timer.setInterval(5000L, sendUptime);
  timer.setInterval(15000L, sendDataTS);
}

void loop()
{

  timer.run(); // Initiates SimpleTimer
  Blynk.run();

}

/***************************************************
 * Get data (DHT and ESP8266)
 **************************************************/
void getDhtData(void)
{
  
  digitalWrite(dust, LOW);
  delayMicroseconds(50);
  float adcValue = analogRead(A0);
  digitalWrite(dust, HIGH);
  float adcVoltage = adcValue * (5.0 / 1024);
  if (adcVoltage < 0.27)
    dustConcentration = 0;
  else
    dustConcentration = 1000 * (0.185 * adcVoltage - 0.05);
  
  
  float tempIni = t;
  float humIni = h;
  long rssiIni = rssi;
  double dpIni = dp;
  float pIni = p;
  float t2Ini = t2;
  float dustIni = dustConcentration;

  t = dht.readTemperature();
  h = dht.readHumidity();
  rssi = WiFi.RSSI();
  double gamma = log(h / 100) + ((17.62 * t) / (243.5 + t));
  dp = 243.5 * gamma / (17.62 - gamma);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    t = tempIni;
    h = humIni;
    rssi = rssiIni;
    dp = dpIni;
    p = pIni;
    t2 = t2Ini;
    dustConcentration = dustIni;
    return;
  }

  if (!bmp.begin()) {
    Serial.print("Failed to read from BMP sensor!!");
    while (1);
  }
  p = bmp.readPressure() / 100;
  t2 = bmp.readTemperature();

  display.print("*** HASENBAU 02", 0);

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

/***************************************************
 * Send DHT data to Blynk
 **************************************************/
void sendUptime()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(10, t);                     // virtual pin V10
  Blynk.virtualWrite(11, h);                     // virtual pin V11
  Blynk.virtualWrite(12, rssi);                  // virtual pin V12
  Blynk.virtualWrite(13, dp);                    // virtual pin V13
  Blynk.virtualWrite(14, p);                     // virtual pin V14
  Blynk.virtualWrite(15, t2);                    // virtual pin V15
  Blynk.virtualWrite(16, dustConcentration);     // virtual pin V16
}

/*********************************************
* Send to Thingspeak
**********************************************/
void sendDataTS(void )
{
  if (client.connect(server, 80)) { // "184.106.153.149" or api.thingspeak.com

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

    Serial.println("Waiting…");
    // thingspeak needs minimum 15 sec delay between updates
    delay(1000);
  }
  client.stop();
}
