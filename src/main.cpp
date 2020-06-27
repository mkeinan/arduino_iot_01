

// A Trial to control the vehicle using Bluetooth.



#include <arduino.h>
#include <softwareserial.h>

// Possible vehicle control commands:
#define STOP 's'
#define MOVE_FORWARD 'w'
#define MOVE_BACKWARD 'x'
#define TURN_LEFT 'a'
#define TURN_RIGHT 'd'
#define GO_TO_BLACK_LINE 'G'
#define DEG_90_RIGHT 'R'
#define DEG_90_LEFT 'L'

// TCS230 or TCS3200 pins wiring to Arduino
#define S0 A4
#define S1 A2
#define S2 A3
#define S3 A5
#define colorSensor 9

#define RED_HIGH_FREQ 100
#define RED_LOW_FREQ 21
#define GREEN_HIGH_FREQ 90
#define GREEN_LOW_FREQ 30
#define BLUE_HIGH_FREQ 90
#define BLUE_LOW_FREQ 30

#define RED 555
#define GREEN 666
#define BLUE 777
#define FLOOR 999

#define FLOOR_OR_COLOR_THRESHOLD 0.6

// Stores frequency read by the photodiodes
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// Stores the red. green and blue colors
int redColor = 0;
int greenColor = 0;
int blueColor = 0;

int lastColor = RED;

// Bluetooth pins
// #define LED_BUILTIN 13
#define BLUETOOTH_RX 13
#define BLUETOOTH_TX 12

// ultrasonic sensor pins
#define TRIG 10
#define ECHO 11

const int IR_SENSOR_LEFT = A1;
const int IR_SENSOR_RIGHT = A0;
const int IR_THRESHOLD = 250;  // Totally ampirical value => might have to change
bool black_line_detected = false;

const int OBSTACLE_DIST_THRESHOLD = 20;  // in centimeters

const int LED_PIN = 8;

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
#define HIGH_SPEED 160
#define LOW_SPEED 110
int speed = LOW_SPEED;  // just an initial value between 0 and 255
bool in_rest = true;

#define DELAY_BETWEEN_COMMANDS 3500
#define DEG_90_DELAY 530  // in milliseconds - completely ampirical value
#define REVERSE_BACKOFF_DELAY 250
#define FORWARD_KICKOFF_DELAY 350


int detect_color(){
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  // Reading the output frequency
  redFrequency = pulseIn(colorSensor, LOW);
  // Remaping the value of the RED (R) frequency from 0 to 255
  redColor = map(redFrequency, RED_LOW_FREQ, RED_HIGH_FREQ, 255, 0);

   // Printing the RED (R) value
  Serial.print("R = ");
  Serial.print(redColor);

  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);

  // Reading the output frequency
  greenFrequency = pulseIn(colorSensor, LOW);
  // Remaping the value of the GREEN (G) frequency from 0 to 255
  greenColor = map(greenFrequency, GREEN_LOW_FREQ, GREEN_HIGH_FREQ, 255, 0);

  // Printing the GREEN (G) value
  Serial.print(" G = ");
  Serial.print(greenColor);

  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);

  // Reading the output frequency
  blueFrequency = pulseIn(colorSensor, LOW);
  // Remaping the value of the BLUE (B) frequency from 0 to 255
  blueColor = map(blueFrequency, BLUE_LOW_FREQ, BLUE_HIGH_FREQ, 255, 0);

  // Printing the BLUE (B) value
  Serial.print(" B = ");
  Serial.println(blueColor);

  bool is_color = false;
  is_color = is_color || ((double)redColor / greenColor < FLOOR_OR_COLOR_THRESHOLD);
  is_color = is_color || ((double)redColor / blueColor < FLOOR_OR_COLOR_THRESHOLD);
  is_color = is_color || ((double)greenColor / redColor < FLOOR_OR_COLOR_THRESHOLD);
  is_color = is_color || ((double)greenColor / blueColor < FLOOR_OR_COLOR_THRESHOLD);
  is_color = is_color || ((double)blueColor / redColor < FLOOR_OR_COLOR_THRESHOLD);
  is_color = is_color || ((double)blueColor / greenColor < FLOOR_OR_COLOR_THRESHOLD);
  if (!is_color){
    Serial.println("detect_color() => FLOOR");
    return FLOOR;
  } else {
    if (redColor > greenColor){
      Serial.println("detect_color() => RED");
      return RED;
    } else {
      if (redColor < blueColor){
        Serial.println("detect_color() => BLUE");
        return BLUE;
      } else {
        Serial.println("detect_color() => GREEN");
        return GREEN;  // this should not happen...
      }
    }
  }

  // // Checks the current detected color and prints
  // // a message in the serial monitor
  // if(redColor > greenColor && redColor > blueColor){
  //     Serial.println(" - RED detected!");
  // }
  // if(greenColor > redColor && greenColor > blueColor){
  //   Serial.println(" - GREEN detected!");
  // }
  // if(blueColor > redColor && blueColor > greenColor){
  //   Serial.println(" - BLUE detected!");
  // }
}


// measure the distance (in cm) using the ultrasonic light sensor:
long measure_distance(){
  Serial.println("-D- measuring distance...");
   digitalWrite(TRIG, LOW);
   delayMicroseconds(5);
   digitalWrite(TRIG, HIGH);
   delayMicroseconds(10);
   digitalWrite(TRIG, LOW);
   long duration = pulseIn(ECHO, HIGH);
   long dist_in_cm = (duration * 0.0343) / 2;
   Serial.print("measure_distance(): measured ");
   Serial.println(dist_in_cm);
   return dist_in_cm;
}

// check if there's an obstacle right ahead:
bool is_obstacle_in_front(){
  long dist = measure_distance();
  // BT.print("-D- measured distance to object in front: ");
  // BT.print(dist);
  // BT.println(" [cm]");
  bool ans = dist < OBSTACLE_DIST_THRESHOLD;
  Serial.print("is_obstacle_in_front(): measured ");
  Serial.print(dist);
  Serial.print(" centimeters to object in front");
  Serial.print("  =>> ");
  if (ans){
    Serial.println("OBSTACLE DETECTED!");
  } else {
    Serial.println("PATH IS CLEAR");
  }
  return ans;
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
  // digitalWrite(In_1, HIGH);
  // digitalWrite(In_2, LOW);
  // digitalWrite(In_3, LOW);
  // digitalWrite(In_4, HIGH);
  digitalWrite(In_1, LOW);
  digitalWrite(In_2, HIGH);
  digitalWrite(In_3, HIGH);
  digitalWrite(In_4, LOW);
  analogWrite(Enable_A, speed);
  analogWrite(Enable_B, speed);
  in_rest = false;
}

void vehicle_move_backward(){
  // digitalWrite(In_1, LOW);
  // digitalWrite(In_2, HIGH);
  // digitalWrite(In_3, HIGH);
  // digitalWrite(In_4, LOW);
  digitalWrite(In_1, HIGH);
  digitalWrite(In_2, LOW);
  digitalWrite(In_3, LOW);
  digitalWrite(In_4, HIGH);
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
  // stop the rotation when the appropriate color is reached:
  int nextColor = RED;
  if (lastColor == RED){
    nextColor = BLUE;
  }
  int curColor = detect_color();
  while (curColor != nextColor){
    curColor = detect_color();
  }
  delay(30);
  vehicle_stop();
  // small correction in case the rotation was too strong:
  // go to the floor (past the color):
  curColor = detect_color();
  vehicle_turn_left();
  while (curColor == nextColor){
    curColor = detect_color();
  }
  vehicle_stop();
  // go from the floor to the color:
  curColor = detect_color();
  vehicle_turn_right();
  while (curColor == FLOOR){
    curColor = detect_color();
  }
  vehicle_stop();
  lastColor = curColor;
}

void vehicle_turn_90_deg_right(){
  vehicle_turn_right();
  // stop the rotation when the appropriate color is reached:
  int nextColor = RED;
  if (lastColor == RED){
    nextColor = BLUE;
  }
  int curColor = detect_color();
  while (curColor != nextColor){
    curColor = detect_color();
  }
  delay(30);
  vehicle_stop();
  // small correction in case the rotation was too strong:
  // go to the floor (past the color):
  curColor = detect_color();
  vehicle_turn_right();
  while (curColor == nextColor){
    curColor = detect_color();
  }
  vehicle_stop();
  // go from the floor to the color:
  curColor = detect_color();
  vehicle_turn_left();
  while (curColor == FLOOR){
    curColor = detect_color();
  }
  vehicle_stop();
  lastColor = curColor;
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

void check_right_black_line_sensor(){
  int val_right = analogRead(IR_SENSOR_RIGHT);
  bool black_line_detected_right = val_right > IR_THRESHOLD;
  Serial.print("right sensor = ");
  Serial.print(val_right);
  if (black_line_detected_right) {
    Serial.println("  => Black line / nothing");
  } else {
    Serial.println("  => Floor / IR reflecting object");
  }
}

void check_left_black_line_sensor(){
  int val_left = analogRead(IR_SENSOR_LEFT);
  bool black_line_detected_left = val_left > IR_THRESHOLD;
  Serial.print("left sensor = ");
  Serial.print(val_left);
  if (black_line_detected_left) {
    Serial.println("  => Black line / nothing");
  } else {
    Serial.println("  => Floor / IR reflecting object");
  }
}

void align_on_black_line(){
  int val_right = analogRead(IR_SENSOR_RIGHT);
  int val_left = analogRead(IR_SENSOR_LEFT);

  bool black_line_detected_right = val_right > IR_THRESHOLD;
  bool black_line_detected_left = val_left > IR_THRESHOLD;

  int before = 0;

  while (true){
    if (black_line_detected_right && black_line_detected_left){
      vehicle_stop();
      vehicle_change_speed(LOW_SPEED);
      vehicle_stop();
      return;
    }

    if (black_line_detected_right && !black_line_detected_left){
      if (before != 1) {
        before = 1;
        vehicle_change_speed(HIGH_SPEED);
        vehicle_turn_right();
        continue;
      }
    }

    if (!black_line_detected_right && black_line_detected_left){
      if (before != 2) {
        before = 2;
        vehicle_change_speed(HIGH_SPEED);
        vehicle_turn_left();
        continue;
      }
    }

    if (!black_line_detected_right && !black_line_detected_left){
      if (before != 3) {
        before = 3;
        vehicle_change_speed(LOW_SPEED);
        vehicle_move_forward();
        continue;
      }
    }

    val_right = analogRead(IR_SENSOR_RIGHT);
    val_left = analogRead(IR_SENSOR_LEFT);

    black_line_detected_right = val_right > IR_THRESHOLD;
    black_line_detected_left = val_left > IR_THRESHOLD;
  }
}

void vehicle_go_to_black_line(){

  // don't go if you see an obstacle:
  if (is_obstacle_in_front()){  // TODO: remove debug statement
    Serial.println("-W- can't move forward, obstacle is in the way");
    // BT.println("-D can't move forward, obstacle is in the way");
    vehicle_stop();
    BT.print("1");   // OBSTACLE
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
        Serial.println("-D- black line detected on the right");
        right_was_stopped = true;
        analogWrite(Enable_B, 0);
      }
    }
    if (!left_was_stopped){
      if (black_line_detected_left){
        Serial.println("-D- black line detected on the left");
        left_was_stopped = true;
        analogWrite(Enable_A, 0);
      }
    }

    val_right = analogRead(IR_SENSOR_RIGHT);
    val_left = analogRead(IR_SENSOR_LEFT);
    black_line_detected_right = val_right > IR_THRESHOLD;
    black_line_detected_left = val_left > IR_THRESHOLD;

    align_on_black_line();
  }

  vehicle_stop();   // stop both tracks

  Serial.println("-I- reached next black line");
  BT.print("0");
  // BT.println("-D- reached next black line");
}


void vehicle_go_two_black_lines_forward(){
  align_on_black_line();

  if (is_obstacle_in_front()){  // TODO: remove debug statement
    Serial.println("-W- can't move forward, obstacle is in the way");
    // BT.println("-D can't move forward, obstacle is in the way");
    vehicle_move_backward();
    delay(REVERSE_BACKOFF_DELAY);
    vehicle_stop();
    BT.print("1");   // OBSTACLE
    return;
  }

  vehicle_move_forward();
  delay(FORWARD_KICKOFF_DELAY);
  align_on_black_line();
  vehicle_move_backward();
  delay(REVERSE_BACKOFF_DELAY);
  vehicle_stop();
  BT.print("0");   // SUCCESS
}


void turn_on_leds(){
   digitalWrite(LED_PIN, HIGH);
}

void turn_off_leds(){
   digitalWrite(LED_PIN, LOW);
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
  pinMode(LED_PIN, OUTPUT);

  // initialize the vehicle in "STOP" mode:
  vehicle_stop();

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT, INPUT);

  // color sensor related output pins:
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Setting the colorSensor as an input
  pinMode(colorSensor, INPUT);

  // Setting frequency scaling of colors to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // reset the lastColor variable:
  lastColor = RED;

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // init the bluetooth module
  BT.begin(9600);
  // Send test message to other device
  Serial.println("setup(): Hello from Arduino");
}



void loop()
{

  // check_right_black_line_sensor();
  // check_left_black_line_sensor();
  // delay(200);
  // return;


  if (BT.available())
  // if text arrived in from BT serial...
  {
    received_chr = BT.read();
    Serial.print("Received char = ");
    Serial.println(received_chr);
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
      vehicle_change_speed(HIGH_SPEED);
      vehicle_turn_right();
      vehicle_change_speed(LOW_SPEED);
      BT.println("Turn Right");
    }
    else if (received_chr == TURN_LEFT) {
      vehicle_change_speed(HIGH_SPEED);
      vehicle_turn_left();
      vehicle_change_speed(LOW_SPEED);
      BT.println("Turn Left");
    }

    else if (received_chr == DEG_90_RIGHT){
      vehicle_change_speed(HIGH_SPEED);
      vehicle_turn_90_deg_right();
      // BT.println("Turned 90 degrees right");
      vehicle_change_speed(LOW_SPEED);
      BT.print("0");  // SUCCESS
    }

    else if (received_chr == DEG_90_LEFT){
      vehicle_change_speed(HIGH_SPEED);
      vehicle_turn_90_deg_left();
      // BT.println("Turned 90 degrees left");
      vehicle_change_speed(LOW_SPEED);
      BT.print("0");  // SUCCESS
    }

    else if (received_chr == GO_TO_BLACK_LINE){
      // vehicle_go_to_black_line();
      vehicle_go_two_black_lines_forward();
      // BT.println("Going to black line...");
      // BT.println("y");
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

    else if (received_chr == 'c'){
      int ans = detect_color();
      if (ans == FLOOR){
        BT.println("FLOOR");
      }
      if (ans == RED){
        BT.println("RED");
      }
      if (ans == GREEN){
        BT.println("GREEN");
      }
      if (ans == BLUE){
        BT.println("BLUE");
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

 turn_on_leds();
 delay(DELAY_BETWEEN_COMMANDS);  // TODO: remove or decrease, this is just for debug purpose
 turn_off_leds();
  // delay(300);
  //

}
