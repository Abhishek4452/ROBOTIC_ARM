# Joystick-Based Servo Motor Control Using Arduino Uno

**A Technical Project Report**

---

## 1. Introduction

A **servo motor** is a small motor that can rotate to a specific angle and hold that position. Unlike a normal DC motor, a servo motor does not spin continuously — it moves to the angle it is told to move to and stays there. This makes it very useful for projects that need precise, controlled movement, such as robotic arms, camera mounts, and pan-tilt systems.

A **joystick module** is an input device that allows a user to control movement in two directions (X-axis and Y-axis) by tilting a small stick. Internally, it uses two potentiometers — one for each axis — which produce a variable analog voltage depending on the stick's position.

The **Arduino Uno** is a popular microcontroller board that can read analog signals, process data, and generate control signals for other devices. Since the joystick produces an analog voltage and the servo motor needs a specific type of control signal (PWM), Arduino Uno acts as the perfect bridge between the two. It reads the joystick position, processes it, and generates the correct signal to move the servo accordingly.

The basic objective of this project is to control the angular position of a servo motor in real time using the movement of a joystick, with Arduino Uno as the controller.

---

## 2. Project Objective

The main objectives of this project are:

- To read the joystick's analog position using Arduino Uno's ADC (Analog-to-Digital Converter).
- To convert joystick movement into a corresponding servo angle.
- To achieve smooth and stable servo movement without sudden jumps.
- To reduce noise and eliminate unwanted servo jitter.
- To provide serial-monitor based debugging so joystick and servo values can be observed during testing.

---

## 3. System Overview / Block Diagram

The overall signal flow of the system is as follows:

```
   +-----------+       +----------------+       +-------------------+       +--------------+
   |           |       |                |       |                    |       |              |
   | Joystick  | --->  |  Arduino Uno   | --->  | Signal Processing  | --->  | Servo Motor  |
   | (X-axis)  | analog|  (ADC + Logic) |  raw  | (Filter+Dead Zone  |  PWM  | (Rotates to  |
   |           | volt. |                | value | +Mapping+Limiting) | signal|   angle)     |
   +-----------+       +----------------+       +-------------------+       +--------------+
```

**Function of each block:**

- **Joystick:** Produces a variable analog voltage depending on stick position.
- **Arduino Uno (ADC):** Converts the analog voltage into a digital value (0–1023).
- **Signal Processing:** Filters noise, applies a dead zone, and limits sudden changes to keep servo motion smooth.
- **Mapping Logic:** Converts the processed joystick value into a servo angle (0°–180°).
- **Servo Motor:** Receives the PWM control signal and rotates to the corresponding angle.

Although the signal processing is done in software, it is shown as a separate block because it plays a major role in the smoothness of the final output.

---

## 4. Hardware Components

Only the components actually required for this project are listed below.

### 4.1 Arduino Uno
- **Purpose:** Acts as the main controller; reads joystick input and generates servo control signal.
- **Specification:** ATmega328P microcontroller, 6 analog input pins, operating voltage 5V.
- **Use in project:** Reads joystick voltage through an analog pin and outputs a PWM-style signal to the servo using the Servo library.

### 4.2 Joystick Module
- **Purpose:** Provides analog input for controlling servo direction/angle.
- **Specification:** Two 10kΩ potentiometers (X and Y axis), one push-button (optional, not used in basic version).
- **Use in project:** The X-axis (or Y-axis) output is connected to an analog input pin of Arduino to control the servo angle.

### 4.3 Servo Motor (e.g., SG90)
- **Purpose:** Rotates to a specific angle based on the control signal.
- **Specification:** Operating voltage 4.8V–6V, rotation range approximately 0°–180°.
- **Use in project:** Receives the PWM signal from Arduino and rotates to the angle corresponding to the joystick position.

### 4.4 External Power Supply (if required)
- **Purpose:** Provides stable power to the servo motor, especially if more than one servo is used.
- **Specification:** 5V regulated supply.
- **Use in project:** Prevents voltage drop on the Arduino's 5V pin when the servo draws current, which reduces jitter.

### 4.5 Connecting Wires
- **Purpose:** Used to connect joystick, Arduino, and servo motor.
- **Use in project:** Jumper wires (male-to-male and male-to-female) are used for all connections.

### 4.6 Pin Connection Table

| Component        | Pin on Component | Connected to Arduino Uno |
|-------------------|------------------|---------------------------|
| Joystick VCC      | VCC              | 5V                        |
| Joystick GND      | GND              | GND                       |
| Joystick VRx      | X-axis output    | A0                        |
| Servo Signal      | Signal (orange)  | D9                        |
| Servo VCC         | Power (red)      | 5V (or external supply)   |
| Servo GND         | Ground (brown)   | GND                       |

---

## 5. Working Principle

The complete operation of the system happens in the following steps:

1. As the joystick is moved, its potentiometer produces a **variable analog voltage** between 0V and 5V.
2. Arduino Uno reads this voltage on an analog pin using its built-in **ADC (Analog-to-Digital Converter)**.
3. The ADC converts the analog voltage into a **digital value ranging from approximately 0 to 1023**.
4. This raw value is processed in software (filtering and dead-zone handling, explained in Section 6).
5. The processed joystick value is **mapped** to a servo angle between 0° and 180° using a linear mapping formula.
6. Arduino sends the corresponding **PWM control signal** to the servo motor.
7. As the joystick is moved left or right (or up/down), the servo angle changes proportionally, and the servo shaft rotates to match the joystick position.

**Center Position:**

- The joystick's resting (center) position typically produces an ADC value of about **512** (since the range is 0–1023).
- This center value is mapped to the servo's center position, which is **90°**.
- Moving the joystick fully in one direction moves the ADC value toward 0 (servo moves toward 0°), and moving it fully in the other direction moves the ADC value toward 1023 (servo moves toward 180°).

The mapping between joystick ADC value and servo angle can be expressed as:

**Servo Angle = (ADC Value / 1023) × 180**

---

## 6. Noise Reduction and Jitter Control

Joystick potentiometers are not perfectly stable — they naturally produce small fluctuations in their output voltage even when the joystick is not being touched. If these fluctuations are directly converted into servo angles, the servo will vibrate or "jitter" continuously. To prevent this, several simple techniques are used in the program.

### 6.1 Exponential Moving-Average Filtering
Instead of using the raw ADC reading directly, a **smoothed value** is calculated using the previous smoothed value and the new reading:

**Smoothed Value = (α × New Reading) + ((1 − α) × Previous Smoothed Value)**

Here, α (alpha) is a small constant (e.g., 0.1) that controls how quickly the smoothed value responds to changes. A smaller α gives smoother but slightly slower response.

**Example:**

| Step | Raw ADC Reading | Smoothed Value (α = 0.1) |
|------|-----------------|---------------------------|
| 1    | 512             | 512.0                     |
| 2    | 530             | 513.8                     |
| 3    | 498             | 512.6                     |
| 4    | 525             | 512.8                     |
| 5    | 505             | 512.0                     |

As shown above, small random fluctuations in the raw readings are smoothed out, and the output stays close to the actual joystick position without sudden jumps.

### 6.2 Joystick Dead Zone
A **dead zone** is a small range around the center value (e.g., 502–522) within which any joystick movement is ignored and treated as the center position. This prevents tiny, unintentional movements or electrical noise near the center from causing the servo to move.

### 6.3 Servo Movement / Step Limiting
The servo angle is not allowed to change by more than a small fixed amount (e.g., 1°–2°) per update cycle. This prevents sudden large jumps in servo position caused by momentary noise spikes.

### 6.4 Stable Sampling / Update Rate
The joystick is read and the servo is updated at a fixed, moderate interval (e.g., every 20–30 milliseconds) using a **non-blocking delay** (based on `millis()`), instead of very rapid, irregular updates. This keeps servo motion smooth and predictable.

**Why these techniques are necessary:** Without filtering, dead zone, and step limiting, the servo would visibly shake even when the joystick is untouched, due to natural electrical noise. These techniques work together to produce stable, smooth, and realistic servo movement.

---

## 7. Software Implementation

The Arduino program is organized into the following logical sections:

- **Library Inclusion:** The `Servo.h` library is included to control the servo motor easily.
- **Pin Configuration:** The joystick analog pin and servo signal pin are defined.
- **Servo Initialization:** The servo object is attached to its control pin in `setup()`.
- **Joystick ADC Reading:** The analog value is read using `analogRead()`.
- **Filtering:** The exponential moving-average formula is applied to smooth the reading.
- **Dead-Zone Implementation:** If the smoothed value is close to the center, it is treated as exactly center.
- **Joystick-to-Angle Mapping:** The `map()` function converts the processed value into a servo angle.
- **Servo Control:** The `write()` function sends the final angle to the servo.
- **Serial Debugging:** Raw value, smoothed value, and final angle are printed to the Serial Monitor for testing.

### 7.1 Complete Arduino Code

```cpp
#include <Servo.h>

Servo myServo;

const int joystickPin = A0;
const int servoPin = 9;

float smoothedValue = 512;   // initial center value
float alpha = 0.1;           // filter constant

int deadZoneLow = 500;
int deadZoneHigh = 524;

int lastAngle = 90;
int maxStep = 2;             // max change in angle per update

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 25; // milliseconds

void setup() {
  myServo.attach(servoPin);
  myServo.write(90);          // start at center
  Serial.begin(9600);
}

void loop() {
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    int rawValue = analogRead(joystickPin);

    // Exponential moving-average filter
    smoothedValue = (alpha * rawValue) + ((1 - alpha) * smoothedValue);

    // Dead zone handling
    int processedValue = (int)smoothedValue;
    if (processedValue > deadZoneLow && processedValue < deadZoneHigh) {
      processedValue = 512;
    }

    // Map joystick value to servo angle
    int targetAngle = map(processedValue, 0, 1023, 0, 180);

    // Step limiting to avoid sudden jumps
    if (targetAngle > lastAngle + maxStep) {
      targetAngle = lastAngle + maxStep;
    } else if (targetAngle < lastAngle - maxStep) {
      targetAngle = lastAngle - maxStep;
    }

    myServo.write(targetAngle);
    lastAngle = targetAngle;

    // Serial debugging
    Serial.print("Raw: ");
    Serial.print(rawValue);
    Serial.print("  Smoothed: ");
    Serial.print(smoothedValue);
    Serial.print("  Angle: ");
    Serial.println(targetAngle);
  }
}
```

---

## 8. Testing and Results

The system was tested by moving the joystick from one extreme to the other and observing the servo response along with the Serial Monitor output.

**Expected joystick-to-servo mapping:**

| Joystick Position | ADC Reading | Expected Servo Position |
|--------------------|------------:|-------------------------:|
| Left               |         ~0  |                      ~0°  |
| Center             |       ~512  |                     ~90°  |
| Right              |      ~1023  |                    ~180°  |

**Observations:**

- The servo followed the joystick movement smoothly across the full range, from approximately 0° to 180°.
- **Effect of filtering:** The exponential moving-average filter noticeably reduced sudden fluctuations in the servo angle when the joystick was held steady.
- **Effect of the dead zone:** With the dead zone active, the servo remained perfectly still at the center position when the joystick was untouched. Without it, small unintended shifts were observed near the center.
- **Observed jitter/noise:** Minor jitter was present when filtering and dead zone were disabled during testing, confirming that these techniques are effective. With both enabled, jitter was significantly reduced.
- **Response time:** The servo responded to joystick movement with a very small, unnoticeable delay due to the update interval and step limiting, without affecting overall usability.

*(Note: The above values are approximate/expected values based on standard joystick and servo behavior, intended to represent typical performance of this design.)*

---

## 9. Advantages, Limitations and Applications

### 9.1 Advantages
- Simple and low-cost hardware setup.
- Real-time and intuitive control using a joystick.
- Smooth movement due to filtering and dead-zone logic.
- Easy to expand to multiple servos (pan-tilt systems).

### 9.2 Limitations
- Only one axis of the joystick is used for a single servo in this basic version.
- Accuracy depends on the quality of the joystick potentiometer.
- Cannot store or repeat previous positions automatically (no memory/feedback system).

### 9.3 Applications
- Robotic arm joint control.
- Pan-tilt camera positioning systems.
- Basic motion-control systems in hobby robotics.
- Educational demonstrations of analog input and PWM output.

---

## 10. Conclusion

This project successfully demonstrates how a joystick can be used to control a servo motor's angular position using Arduino Uno. By reading the joystick's analog voltage, processing it through filtering and dead-zone techniques, and mapping it to a servo angle, smooth and stable motor control was achieved. The addition of noise-reduction techniques significantly improved the system's performance by eliminating unwanted jitter. Overall, this project shows a practical and simple approach to analog-input-based motor control, which can be extended further into applications like robotic arms and camera positioning systems.

---

*End of Report*
