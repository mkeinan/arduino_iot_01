/*

#include <arduino.h>
#include <softwareserial.h>

#define LED_BUILTIN 13
#define BLUETOOTH_RX 10
#define BLUETOOTH_TX 11

SoftwareSerial BT(BLUETOOTH_RX, BLUETOOTH_TX);
// creates a "virtual" serial port/UART
// connect BT module RX to D10
// connect BT module TX to D11
// connect BT Vcc to 5V, GND to GND

void setup()
{
  // set digital pin to control as an output
  pinMode(LED_BUILTIN, OUTPUT);
  // set the data rate for the SoftwareSerial port
  BT.begin(9600);
  // Send test message to other device
  BT.println("Hello from Arduino");
}
char received_chr; // stores incoming character from other device
void loop()
{
  if (BT.available())
  // if text arrived in from BT serial...
  {
    received_chr = BT.read();
    if (received_chr == '1')
    {
      digitalWrite(LED_BUILTIN, HIGH);
      BT.println("LED on");
    }
    if (received_chr == '2')
    {
      digitalWrite(LED_BUILTIN, LOW);
      BT.println("LED off");
    }
    if (received_chr == '?')
    {
      BT.println("Send '1' to turn LED on");
      BT.println("Send '2' to turn LED off");
    }
    // you can add more "if" statements with other characters to add more commands
  }
}

*/
