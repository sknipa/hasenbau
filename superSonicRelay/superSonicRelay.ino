int pingPin = 5; 
int inPin = 6;
int relayPin = 2;
const int cycles =20;
#define DELAY 50
int ledBar[8] = {
  4, 7, 8, 9, 10, 11, 12, 13};
const int sensorMin = 0;
const int sensorMax = 100; 


void setup() { 
  Serial.begin(9600);
  for(int j=0; j<8; j++) { 
    pinMode(ledBar[j], OUTPUT);
    digitalWrite(ledBar[j], LOW);
  }
  for(int k=0; k<8; k++) {
    digitalWrite(ledBar[k], HIGH);
    delay(100);
    digitalWrite(ledBar[k], LOW);
    delay(20);
  }


} 

void loop() { 


  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters: 

  long duration, inches, cm, resultCm; 

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds. 
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse: 

  pinMode(pingPin, OUTPUT); 
  pinMode(relayPin, OUTPUT);

  digitalWrite(pingPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(pingPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(pingPin, LOW); 

  // The same pin is used to read the signal from the PING))): a HIGH 
  // pulse whose duration is the time (in microseconds) from the sending 
  // of the ping to the reception of its echo off of an object. 

  pinMode(inPin, INPUT); 

  duration = pulseIn(inPin, HIGH); 

  // convert the time into a distance 
  inches = microsecondsToInches(duration); 
  cm = microsecondsToCentimeters(duration);

  resultCm = 0;

  for(int i = 0; i < cycles; i++) {
    resultCm += cm;
    delay(DELAY);
  }
  resultCm /= cycles;

  Serial.print(resultCm, DEC); 
  Serial.print(" cm");
  int range = map(resultCm, sensorMin, sensorMax, 0, 7);
  Serial.print(" / ");
  Serial.print(range);
  Serial.println( " Range");
  switch (range) {
  case 7:
    digitalWrite(ledBar[0], HIGH);
        for(int l = 1; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }

    break;
  case 6:
    for(int l = 0; l<2; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 2; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 5:
    for(int l = 0; l<3; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 3; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 4:
    for(int l = 0; l<4; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 4; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 3:
    for(int l = 0; l<5; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 5; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 2:
    for(int l = 0; l<6; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 6; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 1:
    for(int l = 0; l<7; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    for(int l = 7; l<8; l++) 
    {
      digitalWrite(ledBar[l], LOW);
    }
    break;
  case 0 :
    for(int l = 0; l<8; l++) 
    {
      digitalWrite(ledBar[l], HIGH);
    }
    break;
  }
  delay(1);




  if (resultCm <= 40) {
    Serial.println(" ALARM");
    digitalWrite(relayPin, HIGH);
    delay(1000);
  }
  else {
    Serial.println(" OK");
    digitalWrite(relayPin, LOW); 
  }
} 

long microsecondsToInches(long microseconds) 
{ 
  // According to Parallax's datasheet for the PING))), there are 
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per 
  // second). This gives the distance travelled by the ping, outbound 
  // and return, so we divide by 2 to get the distance of the obstacle. 
  return microseconds / 74 / 2; 
} 
long microsecondsToCentimeters(long microseconds) 
{ 
  // The speed of sound is 340 m/s or 29 microseconds per centimeter. 
  // The ping travels out and back, so to find the distance of the 
  // object we take half of the distance travelled. 
  return microseconds / 29 / 2; 

}








