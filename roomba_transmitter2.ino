/*
        DIY Arduino based RC Transmitter
  by Dejan Nedelkovski, www.HowToMechatronics.com
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>


// Define the digital inputs
#define jB1 1  // Joystick button 1
#define jB2 0  // Joystick button 2
#define t1 7   // Toggle switch 1
#define t2 4   // Toggle switch 1
#define b1 8   // Button 1
#define b2 9   // Button 2
#define b3 2   // Button 3
#define b4 3   // Button 4

int c = 0;
unsigned long lefttimelimit = 0;
unsigned long righttimelimit = 0;
unsigned long currentTime = 0;  


RF24 radio(9, 10);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001"; // Address

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte left;
  byte right;
  byte pan;
  byte tilt;
  byte button1;
  byte button2;
  byte button3;
  byte button4;
};

Data_Package data; //Create a variable with the above structure

void setup() {
  Serial.begin(9600);
  
  // Define the radio communication
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  
  // Activate the Arduino internal pull-up resistors
  pinMode(jB1, INPUT_PULLUP);
  pinMode(jB2, INPUT_PULLUP);
  pinMode(t1, INPUT_PULLUP);
  pinMode(t2, INPUT_PULLUP);
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP);
  pinMode(b4, INPUT_PULLUP);
  
  // Set initial default values
  data.left = 127;
  data.right = 127;
  data.pan = 90;
  data.tilt = 127;
  data.button1 = 0;
  data.button2 = 1;
  data.button3 = 1;
  data.button4 = 1;

}
void loop() {
  // Read all analog inputs and map them to one Byte value
  currentTime = millis();
  while (Serial.available()>0){
    delay(10);
    char incomingByte = Serial.read();
    if (incomingByte == 'R') {
      //read ascii integer value and set right motor to it.
      data.right = Serial.parseInt();
      incomingByte = Serial.peek();
      if (incomingByte == ',') { //comma seperated time value
        Serial.read();//flush the period
        righttimelimit = currentTime + Serial.parseInt(); //add time limit to motor motion
      } else {
        righttimelimit = currentTime + 500; //default safety is half a second
      }
    }
    if (incomingByte == 'L') {
      data.left = Serial.parseInt();
      incomingByte = Serial.peek();
      if (incomingByte == ',') { //comma seperated time value
        Serial.read();//flush the period
        lefttimelimit = currentTime + Serial.parseInt(); //add time limit to motor motion
      } else {
        lefttimelimit = currentTime + 500; //default safety is half a second
      }
    }
    if (incomingByte == 'C') {//camera pan
      data.pan = map(Serial.parseInt(),0,100,0,100);
      //data.pan = -500;
    }
    if (incomingByte == 'B') {//button 1
      data.button1 = Serial.parseInt();
    }
  }
  //check for time limits
  if (currentTime > righttimelimit) {
    data.right = 127; //default
  }
  if (currentTime > lefttimelimit) {
    data.left = 127; //default
  }

  
  //data.button1 = digitalRead(b1);
  //data.button2 = digitalRead(b2);
  //data.button3 = digitalRead(b3);
  //data.button4 = digitalRead(b4);
  
  // Send the whole data from the structure to the receiver
  radio.write(&data, sizeof(Data_Package));
}
