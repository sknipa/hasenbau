/**
   Receive OSC messages at NodeMCU from any OSC speaking sender.
   Case: Switch an LED (connected to NodeMCU) on or off via Smartphone
   Created by Fabian Fiess in November 2016
   Inspired by Oscuino Library Examples, Make Magazine 12/2015, https://trippylighting.com/teensy-arduino-ect/touchosc-and-arduino-oscuino/
*/

#include <ESP8266WiFi.h>                // ESP8266 WiFi
#include <WiFiUdp.h>
#include <OSCBundle.h>                  // for receiving OSC messages

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>               // OLED Display
#include <Adafruit_SSD1306.h>           // OLED Display Grafik



#include <ESPDMX.h>                     // DMX

#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2



#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


char ssid[] = "VVAirNet2";                 // your network SSID (name)
char pass[] = "hans200770";              // your network password

// Button Input + LED Output
const int ledPin = 14;                 // D5 pin at NodeMCU
//const int boardLed = 14;      // Builtin LED

WiFiUDP Udp;                           // A UDP instance to let us send and receive packets over UDP
const unsigned int localPort = 8000;   // local port to listen for UDP packets at the NodeMCU (another device must send OSC messages to this port)
const unsigned int destPort = 9000;    // remote port of the target device where the NodeMCU sends OSC to

unsigned int ledState = 1;             // LOW means led is *on*

DMXESPSerial DMX;

uint8_t Red[]   =   {255, 255, 0,   0,  176, 173, 135, 135, 0  , 30 , 65 , 25 , 0  , 0  , 60 , 46 , 34 , 0  , 107, 245, 250, 238, 255, 255, 255, 255, 184, 255, 178, 255, 255, 208, 218, 128};
uint8_t Green[] = {255, 0,   255, 0,  224, 216, 206, 206, 191, 144, 105, 25 , 250, 255, 179, 139, 139, 128, 142, 245, 250, 232, 255, 215, 165, 140, 134, 160, 34 , 0  , 20 , 32 , 112, 0};
uint8_t Blue[]  =  {255, 0,   0,   255, 230, 230, 230, 235, 255, 255, 225, 112, 154, 127, 113, 87 , 34 , 0  , 35 , 220, 210, 170, 0  , 0  , 0  , 0  , 11 , 122, 34, 255, 147, 144, 214, 128};

int channel = 0;

void setup() {
  Serial.begin(115200);

  // Specify a static IP address for NodeMCU
  // If you erase this line, your ESP8266 will get a dynamic IP address
  //WiFi.config(IPAddress(192,168,0,123),IPAddress(192,168,0,1), IPAddress(255,255,255,0));

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());

  // LED Output
  //pinMode(boardLed, OUTPUT);
  pinMode(ledPin, OUTPUT);
  //digitalWrite(boardLed, ledState);   // turn *off* led
  digitalWrite(ledPin, ledState);   // turn *off* led

  delay(200);




  // OLED Display ##########################################################################
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  display.drawPixel(10, 10, WHITE);
  // draw Text
  //
  // text display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("TouchOSC->DMX");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Connected to");
  display.println(ssid);
  display.println(WiFi.localIP());

  // Show the display buffer on the hardware.
  // NOTE: You _must_ call display after making any drawing commands
  // to make them visible on the display hardware!
  display.display();        // Display AN
  delay(2000);
  display.clearDisplay();   // Display löschen
  display.display();        // Display AUS

  DMX.init();               // initialization for first 32 addresses by default
  //DMX.init(512);          // initialization for complete bus
  delay(200);

}

void loop() {

  OSCMessage msgIN;
  int size;
  if ((size = Udp.parsePacket()) > 0) {
    while (size--)
      msgIN.fill(Udp.read());
    if (!msgIN.hasError()) {
      msgIN.route("/1/toggleLED", toggleOnOff);
    }
    // LED PAR Nr. 1
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader1", slideFader1); // R
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader2", slideFader2); // G
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader3", slideFader3); // B
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader4", slideFader4); // Fader
    }
    // LED PAR Nr. 2
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader6", slideFader6); // R
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader7", slideFader7); // G
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader8", slideFader8); // B
    }
    if (!msgIN.hasError()) {
      msgIN.route("/1/fader9", slideFader9); // Fader
    }

 }
  DMX.update();
}

void toggleOnOff(OSCMessage &msg, int addrOffset) {
  ledState = (boolean) msg.getFloat(0);

  //digitalWrite(boardLed, (ledState + 1) % 2);   // Onboard LED works the wrong direction (1 = 0 bzw. 0 = 1)
  digitalWrite(ledPin, ledState);               // External LED

  if (ledState) {
    Serial.println("LED on");
    colorMe();

    delay(2);
  }
  else {
    Serial.println("LED off");
    dmxOff();
  }
  ledState = !ledState;     // toggle the state from HIGH to LOW to HIGH to LOW ...
}

void slideFader1(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(1, data);
  Serial.print("slideFader Channel 1: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader2(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(2, data);
  Serial.print("slideFader Channel 2: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader3(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(3, data);
  Serial.print("slideFader Channel 3: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader4(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(4, data);
  Serial.print("slideFader Channel 4: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader6(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(6, data);
  Serial.print("slideFader Channel 6: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader7(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(7, data);
  Serial.print("slideFader Channel 7: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader8(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(8, data);
  Serial.print("slideFader Channel 8: ");
  Serial.print("/");
  Serial.println(data);
}
void slideFader9(OSCMessage &msg, int addrOffset) {
  int data = (int) msg.getFloat(0);
  DMX.write(9, data);
  Serial.print("slideFader Channel 9: ");
  Serial.print("/");
  Serial.println(data);
}

void colorMe() {
  int Color = random(0, 33);
  DMX.write(1, Red[Color]);
  DMX.write(2, Green[Color]);
  DMX.write(3, Blue[Color]);
  DMX.write(4, 255);

  DMX.write(6, Red[Color]);
  DMX.write(7, Green[Color]);
  DMX.write(8, Blue[Color]);
  DMX.write(9, 255);


  Serial.println("Color Me!");
  Serial.print(Red[Color]);
  Serial.print("/");
  Serial.print(Green[Color]);
  Serial.print("/");
  Serial.println(Blue[Color]);
}

void dmxOff() {
  DMX.write(4, 0);
  DMX.write(9, 0);
}





