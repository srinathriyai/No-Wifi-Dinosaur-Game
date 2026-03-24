# No-Wifi-Dinosaur-Game (Embedded System)
A hardware-based replica of the Chrome offline dinosaur game implemented on an embedded system using an ATmega328 microcontroller.

🎥 **Demo Video:** [https://www.youtube.com/watch?v=1_N93-YyqSs]

## Overview
This project recreates the classic Dino game where the player controls a dinosaur to avoid obstacles (cactus and pterodactyl) using a joystick. The game runs in real-time on an LCD display, with score tracking and audio feedback.

## Features
- Real-time gameplay with jump and duck controls (joystick input)
- Obstacle spawning and movement (cactus + pterodactyl)
- Collision detection and game-over logic
- Score display using 4-digit 7-segment display
- Background audio using passive buzzer

## Hardware
- ATmega328 Microcontroller  
- Analog Joystick  
- ST7735S LCD Display  
- 74HC595 Shift Register  
- Passive Buzzer  
- 4-digit 7-segment display  

## Implementation Highlights
- Designed using multiple state machines for game logic (input, movement, collision, display)
- Task-based architecture with periodic scheduling (1ms–100ms tasks)
- Randomized obstacle spawning using `rand()`

## Notes
- Minor collision detection inaccuracies and sprite overlap issues remain
