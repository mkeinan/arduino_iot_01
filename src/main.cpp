

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
#define DEG_90_RIGHT 'e'
#define DEG_90_LEFT 'q'

// Bluetooth pins
// #define LED_BUILTIN 13
#define BLUETOOTH_RX 13
#define BLUETOOTH_TX 12

// ultrasonic sensor pins
#define TRIG 10
#define ECHO 11

const int IR_SENSOR_LEFT = A1;
const int IR_SENSOR_RIGHT = A0;
const int IR_THRESHOLD = 650;  // Totally ampirical value => might have to change
bool black_line_detected = false;

const int OBSTACLE_DIST_THRESHOLD = 18;

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
bool in_rest = true;

#define DEG_90_DELAY 880  // in milliseconds - completely ampirical value


// measure the distance (in cm) using the ultrasonic light sensor:
long measure_distance(){
   digitalWrite(TRIG, LOW);
   delayMicroseconds(5);
   digitalWrite(TRIG, HIGH);
   delayMicroseconds(10);
   digitalWrite(TRIG, LOW);
   long duration = pulseIn(ECHO, HIGH);
   long dist_in_cm = (duration * 0.0343) / 2;
   return dist_in_cm;
}

// check if there's an obstacle right ahead:
bool is_obstacle_in_front(){
  long dist = measure_distance();
  BT.print("-D- measured distance to object in front: ");
  BT.print(dist);
  BT.println(" [cm]");
  return dist < OBSTACLE_DIST_THRESHOLD;
}

// some vehicle utility functions:
void vehicle_stop(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, 0);
  analogWrite(Enable_B, 0);
  in_rest = true;
}

void vehicle_move_forward(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, LOW);
  digitalWrite(In_4, HIGH);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
  in_rest = false;
}

void vehicle_move_backward(){
  digitalWrite(In_1, LOW);
  digitalWrite(In_2, HIGH);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
  in_rest = false;
}

void vehicle_turn_right(){
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
  in_rest = false;
}

void vehicle_turn_left(){
  digitalWrite(In_1, LOW);
  digitalWrite(In_2, HIGH);
  digitalWrite(In_3, LOW);
  digitalWrite(In_4, HIGH);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
  in_rest = false;
}

void vehicle_turn_90_deg_left(){
  vehicle_turn_left();
  delay(DEG_90_DELAY);
  vehicle_stop();
}

void vehicle_turn_90_deg_right(){
  vehicle_turn_right();
  delay(DEG_90_DELAY);
  vehicle_stop();
}

void vehicle_change_speed(int new_speed){
  speed = new_speed;
  if (!in_rest) {
    analogWrite(Enable_A, speed);
    analogWrite(Enable_B, speed);
  }
}

void check_black_line(){
  int val_right = analogRead(IR_SENSOR_RIGHT);
  int val_left = analogRead(IR_SENSOR_LEFT);

  bool black_line_detected_right = val_right > IR_THRESHOLD;
  bool black_line_detected_left = val_left > IR_THRESHOLD;

  if (black_line_detected_right) {
    Serial.println("RIGHT: Black line / I see nothing");
    BT.println("RIGHT: Black line / I see nothing");
  } else {
    Serial.println("RIGHT: Floor / IR reflecting object detected");
    BT.println("RIGHT: Floor / IR reflecting object detected");
  }
  // print out the value that was read:
  Serial.println(val_right);
  BT.println(val_right);

  if (black_line_detected_left) {
    Serial.println("LEFT: Black line / I see nothing");
    BT.println("LEFT: Black line / I see nothing");
  } else {
    Serial.println("LEFT: Floor / IR reflecting object detected");
    BT.println("LEFT: Floor / IR reflecting object detected");
  }
  // print out the value that was read:
  Serial.println(val_left);
  BT.println(val_left);
}

void vehicle_go_to_black_line(){

  // don't go if you see an obstacle:
  if (is_obstacle_in_front()){
    Serial.println("-D- can't move forward, obstacle is in the way");
    BT.println("-D can't move forward, obstacle is in the way");
    vehicle_stop();
    return;
  }

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

  // // first of all move out of a possible current black line:
  // while (black_line_detected_right || black_line_detected_left){
  //   Serial.println("-D- I'm starting to move from a black line");
  //   BT.println("-D- I'm starting to move from a black line");
  //   if (black_line_detected_right){
  //     analogWrite(Enable_B, speed);
  //   } else {
  //     analogWrite(Enable_B, 0);
  //   }
  //   if (black_line_detected_left){
  //     analogWrite(Enable_A, speed);
  //   } else {
  //     analogWrite(Enable_A, 0);
  //   }
  //
  //   // measure again:
  //   val_right = analogRead(IR_SENSOR_RIGHT);
  //   val_left = analogRead(IR_SENSOR_LEFT);
  //   black_line_detected_right = val_right > IR_THRESHOLD;
  //   black_line_detected_left = val_left > IR_THRESHOLD;
  // }
  //
  // Serial.println("-D- exited from current black line");
  // BT.println("-D- exited from current black line");

  bool left_was_stopped = false;
  bool right_was_stopped = false;

  while (!(left_was_stopped || right_was_stopped)){
    if (!right_was_stopped){
      if (black_line_detected_right){
        right_was_stopped = true;
        analogWrite(Enable_B, 0);
      }
    }
    if (!left_was_stopped){
      if (black_line_detected_left){
        left_was_stopped = true;
        analogWrite(Enable_A, 0);
      }
    }

    val_right = analogRead(IR_SENSOR_RIGHT);
    val_left = analogRead(IR_SENSOR_LEFT);
    black_line_detected_right = val_right > IR_THRESHOLD;
    black_line_detected_left = val_left > IR_THRESHOLD;
  }

  vehicle_stop();

  Serial.println("-D- reached next black line");
  BT.println("-D- reached next black line");
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
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // initialize the vehicle in "STOP" mode:
  vehicle_stop();

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT, INPUT);
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // init the bluetooth module
  BT.begin(9600);
  // Send test message to other device
  BT.println("setup(): Hello from Arduino");
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

    else if (received_chr == DEG_90_RIGHT){
      vehicle_turn_90_deg_right();
      BT.println("Turned 90 degrees right");
    }

    else if (received_chr == DEG_90_LEFT){
      vehicle_turn_90_deg_left();
      BT.println("Turned 90 degrees left");
    }

    else if (received_chr == GO_TO_BLACK_LINE){
      vehicle_go_to_black_line();
      BT.println("Going to black line...");
    }

    else if (received_chr == 'b') {
      check_black_line();
      BT.println("-D- checked black line");
    }

    else if (received_chr == 't'){
      long dist = measure_distance();
      BT.print("-D- measured distance: ");
      BT.print(dist);
      BT.println(" [cm]");
    }

    else if (received_chr == 'y'){
      bool is_obstacle = is_obstacle_in_front();
      if (is_obstacle){
        BT.println("obstacle detected!");
      } else {
        BT.println("path is clear");
      }
    }

    else if (isDigit(received_chr)) {
      String digit_str = "";
      digit_str += (char)received_chr;
      int digit_int = digit_str.toInt();
      vehicle_change_speed(digit_int * 28);  // 9 * 28 = 252 < 255 which is ok
      BT.println("Changing speed");
    }
    else {
      BT.println("Error: unrecognized command... stopping the vehicle");
      vehicle_stop();
    }
  }

  // delay(300);
  //

}
