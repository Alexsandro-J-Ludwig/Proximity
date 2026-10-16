# Proximity

My first attempt at building an autonomous rover whose only goal is to avoid obstacles.
(Spoiler: it didn't go well :D)

<p align="center">
  <img width="700" alt="Proximity rover, first prototype" src="https://github.com/user-attachments/assets/13ad84d6-1f29-4139-a3c2-0dd4d8bdbcf0" />
  <br>
  <em>The first prototype</em>
</p>

<p align="center">
  <img width="500" alt="Proximity rover, final version" src="https://github.com/user-attachments/assets/79c470b3-1504-4e2f-b926-ac5d1dd789a4" />
  <br>
  <em>The final version (sorry, I didn't take a better picture of it)</em>
</p>

I used my [line follower robot](https://github.com/Alexsandro-J-Ludwig/IntEng001.v1.0) as a reference for this project. It was also a great way to learn PID control.

## Hardware
- **MCU:** ESP32
- **Distance sensor:** VL53L0X (Time-of-Flight), mounted on a servo
- **Servo:** SG90 (sweeps the sensor to scan the surroundings)
- **Motors:** 4x TT motors with an L298N driver
- **Power:** 18650 batteries + XL4015 step-down converter

## What I learned
- How ToF sensors work, and their limitations. A single-point sensor only sees one direction at a time.
- Because of that, the servo had to sweep the sensor left and right, which was slow. The rover had to stop to scan before deciding where to go.

## What I'd do differently
Use a multizone VL53L5CX sensor that returns several distance readings at once. That would:
- remove the need for the servo,
- make scanning much faster,
- let the rover keep moving while it decides which direction to take.

## Stack
C++ · ESP32

## Demo
Here is a video of the rover working, before I disassembled it:

[▶ Watch the demo on YouTube](https://youtube.com/shorts/9kqtbndG6p0)
