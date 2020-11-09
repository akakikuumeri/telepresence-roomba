/*
    DIY Arduino based RC Transmitter Project
              == Receiver Code ==

  by Dejan Nedelkovski, www.HowToMechatronics.com
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Include the AccelStepper library:
#include <AccelStepper.h>

// Motor pin definitions:
#define motorPin1  A1      // IN1 on the ULN2003 driver
#define motorPin2  A2     // IN2 on the ULN2003 driver
#define motorPin3  A3     // IN3 on the ULN2003 driver
#define motorPin4  A4     // IN4 on the ULN2003 driver
#define limitSwitch A5

// Define the AccelStepper interface type; 4 wire motor in half step mode:
#define MotorInterfaceType 8

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

RF24 radio(9, 10);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

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
int oldpan = 0;

//Motor Pins
int EN_A = 2;      //Enable pin for first motor
int IN1 = 3;       //control pin for first motor
int IN2 = 4;       //control pin for first motor
int IN3 = 5;        //control pin for second motor
int IN4 = 6;        //control pin for second motor
int EN_B = 7;      //Enable pin for second motor
int motor_speed = 0;

Data_Package data; //Create a variable with the above structure

void setup() {
  //Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); //  Set the module as receiver
  resetData();

   //Initializing the motor pins as output
  pinMode(EN_A, OUTPUT);
  pinMode(IN1, OUTPUT);  
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);  
  pinMode(IN4, OUTPUT);
  pinMode(EN_B, OUTPUT);

  pinMode(A0, OUTPUT);
  //pinMode(A1, OUTPUT);

  //stepper motor:
  // Set the maximum steps per second:
  stepper.setMaxSpeed(500);
  // Set the maximum acceleration in steps per second^2:
  stepper.setAcceleration(1000);

  pinMode(limitSwitch, INPUT_PULLUP);//init limit switch for camera gimbal

  // Set target position:
  stepper.moveTo(10000); //positive is down in this robot. Move way down
  // Run to position with set speed and acceleration:
  while (digitalRead(limitSwitch) == HIGH) {
    stepper.run();//run until hitting limit switch
  }
  //then set current position to 1024. 0 is now straight ahead
  stepper.setCurrentPosition(1024);
  // Set target position:
  stepper.moveTo(0);
  // Run to position with set speed and acceleration:
  stepper.runToPosition();

}
void loop() {
  // Check whether there is data to be received
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone has a throttle up and we lose connection, it can keep flying unless we reset the values
  }

  //control motors:
  
  if (data.right < 127){     //Rotating the right motor backwards
    motor_speed = map(data.right, 126, 0, 0, 255);   //Mapping the values to 0-255 to move the motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(EN_A, motor_speed);
  }
  else if (data.right > 127) {
    motor_speed = map(data.right, 128, 255, 0, 255);   //Mapping the values to 0-255 to move the motor
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(EN_A, motor_speed);
  } else {
    //Disable motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  if (data.left < 127){     //Rotating the left motor backwards
    motor_speed = map(data.left, 126, 0, 0, 255);   //Mapping the values to 0-255 to move the motor
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(EN_B, motor_speed);
  }
  else if (data.left > 127) {
    motor_speed = map(data.left, 128, 255, 0, 255);   //Mapping the values to 0-255 to move the motor
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(EN_B, motor_speed);
  } else {
    //Disable motor
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
  
  //control button output
  if (data.button1 == 1) {
    digitalWrite(A0, HIGH);
  } else {
    digitalWrite(A0, LOW);
  }

  //control pan servo output. data.pan contains 1024 to -1024
  int pantarget = map(data.pan, 0, 100, 1024, -1024);
  if (pantarget != data.pan) {//if needs updating
    stepper.moveTo(pantarget); //this uses some time to calculate accel and such, so better only do if neccesary
  }
  stepper.run();//always keep running
  
  // Print the data in the Serial Monitor
  /*Serial.print("left: ");
  Serial.print(data.left);
  Serial.print("; right: ");
  Serial.print(data.right);
  Serial.print("; button1: ");
  Serial.print(data.button1);
  Serial.print("; pan: ");
  Serial.println(data.pan); */
}

void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.left = 127;
  data.right = 127;
  data.pan = 50;
  data.tilt = 127;
  data.button1 = 0;
  data.button2 = 1;
  data.button3 = 1;
  data.button4 = 1;
}
