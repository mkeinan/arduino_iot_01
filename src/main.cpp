

// A Trial to control the vehicle using Bluetooth.



#include <arduino.h>
#include <softwareserial.h>

// Possible vehicle control commands:
#define STOP 's'
#define MOVE_FORWARD 'w'
#define MOVE_BACKWARD 'x'
#define TURN_LEFT 'a'
#define TURN_RIGHT 'd'

// Bluetooth pins
#define LED_BUILTIN 13
#define BLUETOOTH_RX 12
#define BLUETOOTH_TX 11

// Bluetooth software object
SoftwareSerial BT(BLUETOOTH_RX, BLUETOOTH_TX);
// BT input char:
char received_chr; // stores incoming character from other device

// car pins:
int Enable_A = 5;
int In_1 = 7;
int In_2 = 8;
int In_3 = 2;
int In_4 = 4;
int Enable_B = 3;

// car speed:
int speed = 153;  // just an initial value between 0 and 255


// some vehicle utility functions:
void vehicle_stop(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, 0);
  analogWrite(Enable_B, 0);
}

void vehicle_move_forward(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
}

void vehicle_move_backward(){
  digitalWrite(In_1, LOW);
  digitalWrite(In_2, HIGH);
  digitalWrite(In_3, LOW);
  digitalWrite(In_4, HIGH);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
}

void vehicle_turn_right(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, LOW);
  digitalWrite(In_4, HIGH);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
}

void vehicle_turn_left(){
  digitalWrite(In_1, LOW);
  digitalWrite(In_2, HIGH);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
}

void vehicle_change_speed(int new_speed){
  speed = new_speed;
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
}


// Arduino Setup
void setup()
{
  // initialize all pins on the L298N H-Bridge as outputs:
  pinMode(Enable_A, OUTPUT);
  pinMode(In_1, OUTPUT);
  pinMode(In_2, OUTPUT);
  pinMode(In_3, OUTPUT);
  pinMode(In_4, OUTPUT);
  pinMode(Enable_B, OUTPUT);

  // initialize the vehicle in "STOP" mode:
  vehicle_stop();

  // init the bluetooth module
  BT.begin(9600);
  // Send test message to other device
  BT.println("Hello from Arduino");
}



void loop()
{
  if (BT.available())
  // if text arrived in from BT serial...
  {
    received_chr = BT.read();
    if (received_chr == STOP) {
      vehicle_stop();
      BT.println("Stop");
    }
    else if (received_chr == MOVE_FORWARD) {
      vehicle_move_forward();
      BT.println("Forward");
    }
    else if (received_chr == MOVE_BACKWARD) {
      vehicle_move_backward();
      BT.println("Reverse");
    }
    else if (received_chr == TURN_RIGHT) {
      vehicle_turn_right();
      BT.println("Turn Right");
    }
    else if (received_chr == TURN_LEFT) {
      vehicle_turn_left();
      BT.println("Turn Left");
    }
    else if (received_chr == '0') {
      vehicle_change_speed(0);
      BT.println("Changing speed");
    }
    else if (received_chr == '1') {
      vehicle_change_speed(51);
      BT.println("Changing speed");
    }
    else if (received_chr == '2') {
      vehicle_change_speed(102);
      BT.println("Changing speed");
    }
    else if (received_chr == '3') {
      vehicle_change_speed(153);
      BT.println("Changing speed");
    }
    else if (received_chr == '4') {
      vehicle_change_speed(204);
      BT.println("Changing speed");
    }
    else if (received_chr == '5') {
      vehicle_change_speed(255);
      BT.println("Changing speed");
    }
    else {
      BT.println("Error: unrecognized command... stopping the vehicle");
      vehicle_stop();
    }
  }
}
