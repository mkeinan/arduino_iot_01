
/*
This program prints the value of the input from the TCRT5000 IR sensor.
An example of serial communication.
*/
#include <arduino.h>
#include <softwareserial.h>

// analog pin of potentiometer:
const int IR_SENSOR = A1;
const int IR_THRESHOLD = 150;  // Totally ampirical value => might have to change
bool black_line_detected = false;

void setup() {
  pinMode(IR_SENSOR, INPUT);
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

void loop() {
  // read the potentiometer:
  int val = analogRead(IR_SENSOR);

  black_line_detected = val > 150;
  if (black_line_detected) {
    Serial.println("Black line / I see nothing");
  } else {
    Serial.println("Floor / IR reflecting object detected");
  }

  // print out the value that was read:
  Serial.println(val);

  delay(200);  // delay in between reads
}
