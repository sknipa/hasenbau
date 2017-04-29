

/*


This sketch is a combination of two other sketches:
1.
Plot DTH11 data on thingspeak.com using an ESP8266
April 11 2015
Author: Jeroen Beemster
Website: www.arduinesp.com
2.
Example sketch: adafruit BMP 085
Sensor api BMP180
*/ 


//library DHT22
#include <DHT.h>


//library esp
#include <ESP8266WiFi.h>

//library bmp180
#include <Wire.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BMP085_U.h>

//Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);


// replace with your channel’s thingspeak API key,
String apiKey = "BT03YLMSOHXKGJXQ";
const char* ssid = "VVAirNet2";
const char* password = "hans200770";                             //fill in your wifi password

const char* server = "api.thingspeak.com";
#define DHTPIN 2 // what pin we’re connected to
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

int sensorPin = A0;    // input for LDR and rain sensor
int enable1 = 15;      // enable reading LDR
int enable2 = 13;      // enable reading Rain sensor


int sensorValue1 = 0;  // variable to store the value coming from sensor LDR
int sensorValue2 = 0;  // variable to store the value coming from sensor Rain sensor


//--------------------------setup-------------------------
void setup() {

// declare the enable and ledPin as an OUTPUT:
pinMode(enable1, OUTPUT);
pinMode(enable2, OUTPUT);

  
Serial.begin(115200);
delay(10);

dht.begin();

WiFi.begin(ssid, password);

Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
Serial.print("..........");
Serial.println();
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);

}
Serial.println("WiFi connected");
Serial.println();

}


void loop() {
//--------------------------DHT22/DHT11-------------------------

float h = dht.readHumidity();
float t = dht.readTemperature();

if (isnan(h) || isnan(t)) {
Serial.println("Failed to read from DHT sensor!");
return;
}


Serial.print("Temperature:      ");
Serial.print(t);
Serial.print(" degrees Celcius ");
Serial.println();

Serial.print("Humidity:         ");
Serial.print(h);
Serial.print("%");
Serial.println();

//--- extra---- you can measure dew point with the temperature and the humidity

double gamma = log(h/100) + ((17.62*t) / (243.5+t));
double dp = 243.5*gamma / (17.62-gamma);

Serial.print("Dew point:        ");
Serial.print(dp);
Serial.print(" degrees Celcius ");
Serial.println();

//--------------------------BMP180------------------------

if(!bmp.begin()) {
Serial.print("Failed to read from BMP sensor!!");
while(1);
}

sensors_event_t event;
bmp.getEvent(&event);

Serial.print("Pressure:         ");
Serial.print(event.pressure);
Serial.println(" hPa");

float temperature;
bmp.getTemperature(&temperature);
Serial.print("Temperature:      ");
Serial.print(temperature);
Serial.println(" degrees Celcius ");

//--- extra----you can measure the altitude with the temperature and the air pressure

float seaLevelPressure = 1015;
Serial.print("Altitude:         "); 
Serial.print(bmp.pressureToAltitude(seaLevelPressure,event.pressure)); 
Serial.println(" m");

//--------------------------LDR-------------------------

digitalWrite(enable1, HIGH); 
sensorValue1 = analogRead(sensorPin);
sensorValue1 = constrain(sensorValue1, 300, 850); 
sensorValue1 = map(sensorValue1, 300, 850, 0, 1023); 
Serial.print("Light intensity:  ");
Serial.println(sensorValue1);
digitalWrite(enable1, LOW);
delay(100);

//--------------------------Rain Sensor-------------------------

digitalWrite(enable2, HIGH); 

delay(500);
sensorValue2 = analogRead(sensorPin);
sensorValue2 = constrain(sensorValue2, 150, 440); 
sensorValue2 = map(sensorValue2, 150, 440, 1023, 0); 

Serial.print("Rain value:       ");
Serial.println(sensorValue2);
Serial.println();
delay(100);

digitalWrite(enable2, LOW);

//--------------------------thingspeak-------------------------

if (client.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
String postStr = apiKey;
postStr +="&field1=";
postStr += String(t);
postStr +="&field2=";
postStr += String(h);
postStr +="&field3=";
postStr += String(dp);
postStr +="&field4=";
postStr += String(event.pressure);
postStr +="&field5=";
postStr += String(temperature);
postStr +="&field6=";
postStr += String(sensorValue1);
postStr +="&field7=";
postStr += String(sensorValue2);
postStr +="&field8=";
postStr += String(bmp.pressureToAltitude(seaLevelPressure,event.pressure));
postStr += "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n";

client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n\n\n\n\n\n\n");
client.print(postStr);



}
client.stop();


// thingspeak needs minimum 15 sec delay between updates
delay(20000);
