# ECE198 Lockout buzzer design

## Overview

This project is a lockout buzzer system that is designed to be used in a quiz bowl competition. The system is composed of a master unit and multiple player units. The master unit is responsible for controlling the game and the player units are responsible for signaling the players when they can buzz in. The system is designed to be modular and expandable, allowing for more player units to be added to the system.

## Features

- Master unit
  - Controls the game
  - Displays the current score
  - Displays the current question
  - Controls the buzzer lockout
  - Controls the player lockout
  - Controls the reset button
  - Controls the player units
- Player unit
  - Signals the player when they can buzz in
  - Signals the player when they are locked out
  - Signals the player when they have buzzed in
  - Signals the player when they have answered correctly
  - Signals the player when they have answered incorrectly

## Hardware

- Master unit
  - STM32F401RE Nucleo-64 development board
  - LCD display
  - Push button
  - LEDs
  - Buzzer
  - Jumper wires
  - etc

- Player unit
  - STM32F401RE Nucleo-64 development board
  - LEDs
  - Buzzer
  - Jumper wires
  - etc

## Software

- Master unit
  - C
  - STM32CubeIDE
  - etc

- Player unit
  - C
  - STM32CubeIDE
  - etc

## Contributors
Hayat Ahmad
William Zeng
Chloe Lau

