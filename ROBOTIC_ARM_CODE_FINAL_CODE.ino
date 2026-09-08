/*
  Dual Joystick - 4 Servo Proportional Controller
  ------------------------------------------------
  Hardware:
    - Arduino Uno + Sensor Shield V5.0
    - Joystick 1: X -> A0, Y -> A1
    - Joystick 2: X -> A2, Y -> A3
    - Servo 1 -> D3   (driven by Joystick 1 X)
    - Servo 2 -> D5   (driven by Joystick 1 Y)
    - Servo 3 -> D6   (driven by Joystick 2 X)
    - Servo 4 -> D9   (driven by Joystick 2 Y)

  Features:
    - Exponential Moving Average (EMA) filtering to remove noise
    - Configurable dead zone around joystick center
    - Movement threshold to avoid micro-jitters reaching the servo
    - Per-axis calibrated center value (doesn't assume exactly 512)
    - Serial Monitor debug output of raw/filtered readings
*/

#include <Servo.h>

// ------------------- PIN DEFINITIONS -------------------
// Joystick analog input pins
const int JOY1_X_PIN = A0;
const int JOY1_Y_PIN = A1;
const int JOY2_X_PIN = A2;
const int JOY2_Y_PIN = A3;

// Servo digital PWM output pins
const int SERVO1_PIN = 3;
const int SERVO2_PIN = 5;
const int SERVO3_PIN = 6;
const int SERVO4_PIN = 9;

// ------------------- CALIBRATION CONSTANTS -------------------

// Joystick center values (ideal is 512, but real joysticks vary).
// If your servo doesn't rest at 90 degrees when the stick is released,
// print the raw values via Serial Monitor and adjust these to match
// what YOUR joystick reads at rest.
int joy1XCenter = 512;
int joy1YCenter = 512;
int joy2XCenter = 512;
int joy2YCenter = 512;

// Dead zone: +/- this many raw ADC counts around center are treated as "centered".
// Increase this if your servo still creeps/jitters near center.
const int DEAD_ZONE = 25;

// EMA filter strength (0.0 - 1.0).
// Smaller = smoother but slower to respond. Larger = faster but noisier.
const float FILTER_ALPHA = 0.05;

// Servo angle limits
const int SERVO_MIN_ANGLE = 30;
const int SERVO_MAX_ANGLE = 150;
const int SERVO_CENTER_ANGLE = 90;

// Minimum angle change (degrees) required before we actually move a servo.
// This prevents constant tiny writes caused by residual noise.
const int SERVO_UPDATE_THRESHOLD = 2;

// How often to print debug info to Serial Monitor (milliseconds)
const unsigned long DEBUG_PRINT_INTERVAL = 1500;

// ------------------- SERVO OBJECTS -------------------
Servo servo1, servo2, servo3, servo4;

// ------------------- STATE VARIABLES -------------------
// Filtered joystick readings (start at center to avoid startup jump)
float filteredJoy1X = 512;
float filteredJoy1Y = 512;
float filteredJoy2X = 512;
float filteredJoy2Y = 512;

// Last angle actually sent to each servo
int lastServo1Angle = SERVO_CENTER_ANGLE;
int lastServo2Angle = SERVO_CENTER_ANGLE;
int lastServo3Angle = SERVO_CENTER_ANGLE;
int lastServo4Angle = SERVO_CENTER_ANGLE;

unsigned long lastDebugPrint = 0;

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(9600);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);

  // Move all servos to center position at startup
  servo1.write(SERVO_CENTER_ANGLE);
  servo2.write(SERVO_CENTER_ANGLE);
  servo3.write(SERVO_CENTER_ANGLE);
  servo4.write(SERVO_CENTER_ANGLE);

  Serial.println("Dual Joystick 4-Servo Controller Started");
  Serial.println("Raw / Filtered / Angle values will be printed below:");
}

// ------------------- MAIN LOOP -------------------
void loop() {
  // Read raw joystick values
  int raw1X = analogRead(JOY1_X_PIN);
  int raw1Y = analogRead(JOY1_Y_PIN);
  int raw2X = analogRead(JOY2_X_PIN);
  int raw2Y = analogRead(JOY2_Y_PIN);

  // Apply EMA filter to smooth out noise
  filteredJoy1X = applyEMA(filteredJoy1X, raw1X);
  filteredJoy1Y = applyEMA(filteredJoy1Y, raw1Y);
  filteredJoy2X = applyEMA(filteredJoy2X, raw2X);
  filteredJoy2Y = applyEMA(filteredJoy2Y, raw2Y);

  // Convert filtered readings into target servo angles
  int targetAngle1 = joystickToAngle(filteredJoy1X, joy1XCenter);
  int targetAngle2 = joystickToAngle(filteredJoy1Y, joy1YCenter);
  int targetAngle3 = joystickToAngle(filteredJoy2X, joy2XCenter);
  int targetAngle4 = joystickToAngle(filteredJoy2Y, joy2YCenter);

  // Only update each servo if the change is meaningful (reduces jitter, gives smooth motion)
  updateServoIfNeeded(servo1, lastServo1Angle, targetAngle1);
  updateServoIfNeeded(servo2, lastServo2Angle, targetAngle2);
  updateServoIfNeeded(servo3, lastServo3Angle, targetAngle3);
  updateServoIfNeeded(servo4, lastServo4Angle, targetAngle4);

  // Print debug info periodically (not every loop, to keep Serial readable)
  if (millis() - lastDebugPrint >= DEBUG_PRINT_INTERVAL) {
    lastDebugPrint = millis();
    printDebugInfo(raw1X, filteredJoy1X, lastServo1Angle,
                   raw1Y, filteredJoy1Y, lastServo2Angle,
                   raw2X, filteredJoy2X, lastServo3Angle,
                   raw2Y, filteredJoy2Y, lastServo4Angle);
  }
}

// ------------------- HELPER FUNCTIONS -------------------

// Exponential Moving Average filter: smooths a noisy raw reading over time
float applyEMA(float previousFiltered, int newRawValue) {
  return previousFiltered + FILTER_ALPHA * (newRawValue - previousFiltered);
}

// Converts a filtered joystick reading into a servo angle,
// applying the dead zone around that joystick's calibrated center.
int joystickToAngle(float filteredValue, int centerValue) {
  int offset = (int)(filteredValue - centerValue);

  // Inside the dead zone -> treat as perfectly centered
  if (abs(offset) <= DEAD_ZONE) {
    return SERVO_CENTER_ANGLE;
  }

  // Outside the dead zone: map the remaining range proportionally to servo angle.
  // We map from the edge of the dead zone (not from raw 0/1023) so there's no
  // sudden jump in servo angle right as you leave the dead zone.
  if (offset > 0) {
    // Joystick pushed above center
    int mappedAngle = map(offset, DEAD_ZONE, 512, SERVO_CENTER_ANGLE, SERVO_MAX_ANGLE);
    return constrain(mappedAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  } else {
    // Joystick pushed below center
    int mappedAngle = map(offset, -512, -DEAD_ZONE, SERVO_MIN_ANGLE, SERVO_CENTER_ANGLE);
    return constrain(mappedAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  }
}

// Only writes to the servo (and updates lastAngle) if the change is
// bigger than SERVO_UPDATE_THRESHOLD. This eliminates jitter from tiny,
// meaningless fluctuations and also naturally smooths motion.
void updateServoIfNeeded(Servo &servoObj, int &lastAngle, int targetAngle) {
  if (abs(targetAngle - lastAngle) >= SERVO_UPDATE_THRESHOLD) {
    servoObj.write(targetAngle);
    lastAngle = targetAngle;
  }
}

// Prints raw ADC value, filtered value, and final servo angle for all 4 axes
void printDebugInfo(int raw1X, float filt1X, int angle1,
                     int raw1Y, float filt1Y, int angle2,
                     int raw2X, float filt2X, int angle3,
                     int raw2Y, float filt2Y, int angle4) {
  Serial.print("J1X raw:"); Serial.print(raw1X);
  Serial.print(" filt:"); Serial.print(filt1X, 1);
  Serial.print(" ang:"); Serial.print(angle1);

  Serial.print(" | J1Y raw:"); Serial.print(raw1Y);
  Serial.print(" filt:"); Serial.print(filt1Y, 1);
  Serial.print(" ang:"); Serial.print(angle2);

  Serial.print(" | J2X raw:"); Serial.print(raw2X);
  Serial.print(" filt:"); Serial.print(filt2X, 1);
  Serial.print(" ang:"); Serial.print(angle3);

  Serial.print(" | J2Y raw:"); Serial.print(raw2Y);
  Serial.print(" filt:"); Serial.print(filt2Y, 1);
  Serial.print(" ang:"); Serial.println(angle4);
}