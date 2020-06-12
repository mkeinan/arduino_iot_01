

// A Trial to control the vehicle using Bluetooth.



#include <arduino.h>
#include <softwareserial.h>

// Possible vehicle control commands:
#define STOP 's'
#define MOVE_FORWARD 'w'
#define MOVE_BACKWARD 'x'
#define TURN_LEFT 'a'
#define TURN_RIGHT 'd'
#define GO_TO_BLACK_LINE 'g'

// Bluetooth pins
// #define LED_BUILTIN 13
#define BLUETOOTH_RX 13
#define BLUETOOTH_TX 12

const int IR_SENSOR_LEFT = A1;
const int IR_SENSOR_RIGHT = A0;
const int IR_THRESHOLD = 850;  // Totally ampirical value => might have to change
bool black_line_detected = false;

// Bluetooth software object
SoftwareSerial BT(BLUETOOTH_TX, BLUETOOTH_RX);
// BT input char:
char received_chr; // stores incoming character from other device

// car pins:
int Enable_A = 6;
int In_1 = 7;
int In_2 = 5;
int In_3 = 4;
int In_4 = 2;
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

void vehicle_go_to_black_line(){
  // TODO: note that if the speed is 0 or too low we will get stuck!

  int val_right = analogRead(IR_SENSOR_RIGHT);
  int val_left = analogRead(IR_SENSOR_LEFT);

  bool black_line_detected_right = val_right > IR_THRESHOLD;
  bool black_line_detected_left = val_left > IR_THRESHOLD;

  // int left_track_enable = speed;
  // int right_track_enable = speed;
  //
  // bool left_trigger = false;  // true means stop immediately when black line detected.
  // bool right_trigger = false;  // true means stop immediately when black line detected.

  vehicle_move_forward(); // setting the correct direction of movement

  // first of all move out of a possible current black line:
  while (black_line_detected_right || black_line_detected_left){
    Serial.println("-D- I'm starting to move from a black line");
    if (black_line_detected_right){
      analogWrite(Enable_B, speed);
    } else {
      analogWrite(Enable_B, 0);
    }
    if (black_line_detected_left){
      analogWrite(Enable_A, speed);
    } else {
      analogWrite(Enable_A, 0);
    }

    // measure again:
    val_right = analogRead(IR_SENSOR_RIGHT);
    val_left = analogRead(IR_SENSOR_LEFT);
    black_line_detected_right = val_right > IR_THRESHOLD;
    black_line_detected_left = val_left > IR_THRESHOLD;
  }

  Serial.println("-D- exited from current black line");

  while (!(black_line_detected_right && black_line_detected_left)){
    if (black_line_detected_right){
      analogWrite(Enable_B, 0);
    } else {
      analogWrite(Enable_B, speed);
    }
    if (black_line_detected_left){
      analogWrite(Enable_A, 0);
    } else {
      analogWrite(Enable_A, speed);
    }

    val_right = analogRead(IR_SENSOR_RIGHT);
    val_left = analogRead(IR_SENSOR_LEFT);
    black_line_detected_right = val_right > IR_THRESHOLD;
    black_line_detected_left = val_left > IR_THRESHOLD;
  }

  Serial.println("-D- reached next black line");
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

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT, INPUT);
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

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

    else if (received_chr == GO_TO_BLACK_LINE){
      vehicle_go_to_black_line();
      BT.println("Going to black line...");
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

  // delay(300);
  //
  // int val_right = analogRead(IR_SENSOR_RIGHT);
  // int val_left = analogRead(IR_SENSOR_LEFT);
  //
  // bool black_line_detected_right = val_right > IR_THRESHOLD;
  // bool black_line_detected_left = val_left > IR_THRESHOLD;
  //
  // if (black_line_detected_right) {
  //   Serial.println("RIGHT: Black line / I see nothing");
  // } else {
  //   Serial.println("RIGHT: Floor / IR reflecting object detected");
  // }
  // // print out the value that was read:
  // Serial.println(val_right);
  //
  // if (black_line_detected_left) {
  //   Serial.println("LEFT: Black line / I see nothing");
  // } else {
  //   Serial.println("LEFT: Floor / IR reflecting object detected");
  // }
  // // print out the value that was read:
  // Serial.println(val_left);
}
