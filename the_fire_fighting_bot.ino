#include <Wire.h>
#include <Servo.h>
#include <Adafruit_MLX90614.h>
#include <SoftwareSerial.h>

Adafruit_MLX90614 mlx;
Servo myservo;
SoftwareSerial bluetooth(12, 13); // RX, TX

#define Left 8
#define Right 9
#define Forward 10
#define LM1 2
#define LM2 3
#define RM1 4
#define RM2 5
#define pump 6

const int toggleSwitchPin = 7;  // Pin for the toggle switch
bool manualControl = false;

void setup() {
  pinMode(Left, INPUT);
  pinMode(Right, INPUT);
  pinMode(Forward, INPUT);
  pinMode(LM1, OUTPUT);
  pinMode(LM2, OUTPUT);
  pinMode(RM1, OUTPUT);
  pinMode(RM2, OUTPUT);
  pinMode(pump, OUTPUT);

  pinMode(toggleSwitchPin, INPUT_PULLUP);  // Use internal pull-up resistor

  mlx.begin();
  myservo.attach(11);
  myservo.write(90);

  bluetooth.begin(9600);
}

void sweepServo() {
  for (int pos = 50; pos <= 130; pos += 1) {
    myservo.write(pos);
    delay(10);
  }
  for (int pos = 130; pos >= 50; pos -= 1) {
    myservo.write(pos);
    delay(10);
  }
}

void put_off_fire()
{
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, LOW);
  digitalWrite(RM2, LOW);
  
  digitalWrite(pump, HIGH);
  {
  sweepServo();  // Sweep the servo while extinguishing the fire
  }
  digitalWrite(pump, LOW);
  myservo.write(90);  // Center the servo
}

void automaticControl() 
{
  int leftSensor = digitalRead(Left);
  int rightSensor = digitalRead(Right);
  int forwardSensor = digitalRead(Forward);

  if (leftSensor && rightSensor && forwardSensor) {
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, HIGH);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, HIGH);
  } else {
    // Stop and adjust movement based on sensor inputs
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, LOW);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, LOW);

    if (leftSensor == LOW) {
      // Turn left
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, LOW);
      digitalWrite(RM1, HIGH);
      digitalWrite(RM2, HIGH);
    }

    if (rightSensor == LOW) {
      // Turn right
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, HIGH);
      digitalWrite(RM1, HIGH);
      digitalWrite(RM2, LOW);
    }
  }
}

void manualControlViaBluetooth() {
  if (bluetooth.available() > 0) {
    char command = bluetooth.read();
    handleBluetoothCommand(command);
  }
}

void handleBluetoothCommand(char command) {
  switch (command) {
    case 'F':
      // Move forward
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, LOW);
      digitalWrite(RM1, HIGH);
      digitalWrite(RM2, LOW);
      break;
    case 'B':
      // Move backward
      digitalWrite(LM1, LOW);
      digitalWrite(LM2, HIGH);
      digitalWrite(RM1, LOW);
      digitalWrite(RM2, HIGH);
      break;
    case 'L':
      // Turn left
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, LOW);
      digitalWrite(RM1, LOW);
      digitalWrite(RM2, HIGH);
      break;
    case 'R':
      // Turn right
      digitalWrite(LM1, LOW);
      digitalWrite(LM2, HIGH);
      digitalWrite(RM1, HIGH);
      digitalWrite(RM2, LOW);
      break;
    case 'X':
      put_off_fire();
      break;
    case 'S':
      // Stop
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, HIGH);
      digitalWrite(RM1, HIGH);
      digitalWrite(RM2, HIGH);
      break;
    default:
      break;
  }
}

void loop() {
  // Read the state of the toggle switch
  bool switchState = digitalRead(toggleSwitchPin);

  if (!switchState) {
    // Automatic control mode
    automaticControl();
    
    // Read temperature only in automatic mode
    double temp = mlx.readObjectTempC();

    if (temp > 37.0) {
      put_off_fire();
    }
  } else {
    // Manual control mode
    manualControlViaBluetooth();
  }
}
