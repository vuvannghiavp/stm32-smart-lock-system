# STM32 Smart Lock System

An embedded smart door lock system developed using **STM32**, integrating password authentication, RFID access control, LCD user interface, keypad input, and servo motor control.

The project focuses on embedded firmware development, peripheral integration, state-machine-based application logic, and hardware control using the STM32 HAL library.

## Features

 **Password Authentication**

  * Enter password using a matrix keypad.
  * `#` is used to confirm the password.
  * `*` clears the current password input.
  * Displays authentication status on the LCD.
  * Correct password unlocks the door.
  * Incorrect password triggers buzzer and LED feedback.

  **RFID Access Control**

  * RFID card detection using the **RC522** module.
  * Supports a **Master Card** and multiple **Member Cards**.
  * Add a Master Card.
  * Add Member Cards.
  * Delete Member Cards.
  * Verify cards before granting access.

   **Servo Door Control**

  * Servo motor is used as the door-lock actuator.
  * The door automatically locks again after the configured unlock period.

  **User Feedback**

  * LCD displays system status and authentication results.
  * Buzzer provides success/error feedback.
  * LED indicates access and error states.

  **Sleep Mode**

  * The system enters a low-activity sleep state after a period of inactivity.
  * Keypad input or a valid RFID card can wake the system.

## System Architecture

```text
                    +----------------------+
                    |       STM32 MCU      |
                    |                      |
                    |   Application Logic  |
                    |   State Machine      |
                    +----------+-----------+
                               |
          +--------------------+--------------------+
          |                    |                    |
          v                    v                    v
   +-------------+      +-------------+      +-------------+
   |   Keypad    |      |    RC522    |      |     LCD     |
   |   4x4 Input |      | RFID Reader |      |   Display   |
   +-------------+      +-------------+      +-------------+
          |                    |                    |
          +--------------------+--------------------+
                               |
                    +----------+----------+
                    |                     |
                    v                     v
             +-------------+       +-------------+
             |    Servo    |       |   Buzzer /  |
             | Door Lock   |       |     LED     |
             +-------------+       +-------------+
```

## Authentication Flow

### Password Authentication

```text
STATE_LOCK_INPUT
       |
       v
Enter Password
       |
       +------ "*" ------> Clear Input
       |
       +------ "#" ------> Verify Password
                              |
                    +---------+---------+
                    |                   |
                  Valid              Invalid
                    |                   |
                    v                   v
              Unlock Door        Error Feedback
                    |             Buzzer + LED
                    v
              Wait 10 Seconds
                    |
                    v
               Lock Door
```

### RFID Authentication

The RFID subsystem uses a hierarchical access model:

```text
                 RFID Card
                     |
                     v
               Card Detected
                     |
                     v
               Card Validation
                 /       \
              Valid      Invalid
                |           |
                v           v
           Unlock Door   Error Message
```

The system also provides an administration menu for managing cards:

```text
RFID Menu
   |
   +-- B: Add Master Card
   |
   +-- C: Add Member Card
   |
   +-- D: Delete Member Card
   |
   +-- *: Return
```

A Master Card must exist before Member Cards can be added or deleted.

## Firmware State Machine

The application is organized around a state-machine architecture to separate authentication, card management, and sleep behavior.

Main states include:

```text
STATE_LOCK_INPUT
STATE_RFID_ADD_MENU
STATE_ADD_MASTER
STATE_ADD_MEMBER
STATE_DELETE_MEMBER
STATE_DELETE_WAIT
STATE_SLEEP
```

Example state transition:

```text
                    +-------------------+
                    |  STATE_LOCK_INPUT |
                    +---------+---------+
                              |
                +-------------+-------------+
                |             |             |
             Password       "A"           Timeout
                |             |             |
                v             v             v
           Verification   RFID Menu    STATE_SLEEP
                              |
                +-------------+-------------+
                |             |             |
                B             C             D
                |             |             |
                v             v             v
          Add Master     Add Member    Delete Member
```

This approach makes the firmware easier to extend because new authentication methods or system states can be added without completely restructuring the main application loop.

## Hardware

| Component     | Function                               |
| ------------- | -------------------------------------- |
| STM32 MCU     | Main embedded controller               |
| Matrix Keypad | Password input and system control      |
| RC522         | RFID card detection and authentication |
| LCD           | User interface and system status       |
| Servo Motor   | Door lock actuator                     |
| Buzzer        | Audio feedback                         |
| LED           | Status/error indication                |

## Software & Technologies

* **C**
* **STM32 HAL**
* **STM32CubeMX**
* **Keil MDK / ARM Compiler**
* **GPIO**
* **SPI**
* **Timer**
* **Matrix Keypad Scanning**

## Project Structure

```text
stm32-smart-lock-system/
│
├── Core/
│   ├── Inc/
│   └── Src/
│
├── Drivers/
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
│
├── Libraries/
│   ├── KEY_PAD/
│   │   ├── keypad.c
│   │   └── keypad.h
│   │
│   ├── LCD/
│   │
│   ├── RC522/
│   │
│   └── SERVO/
│
├── MDK-ARM/
│
├── .vscode/
│
├── ex.ioc
└── README.md
```

The repository separates the STM32 generated project files, HAL/CMSIS drivers, and application-level peripheral libraries into dedicated directories.

### 1. Peripheral Integration

Multiple hardware peripherals are integrated into a single MCU application:

* GPIO for keypad, LED and buzzer
* SPI for RC522
* Timer/PWM for servo control
* Display interface for LCD
* STM32 HAL for hardware abstraction

### 2. State Machine Design

The application is not implemented as a simple sequence of blocking operations. Instead, different operating modes are represented by explicit system states.

This allows the system to handle:

* Authentication
* Card management
* Door control
* Error handling
* Sleep/wake behavior

independently.

### 3. Modular Driver Design

Peripheral-specific functionality is separated into individual libraries.

For example:

```text
KEY_PAD/
LCD/
RC522/
SERVO/
```


### 4. Access Control

The RFID subsystem distinguishes between:

* **Master Card**
* **Member Card**

The Master Card is required for administrative operations such as adding or deleting Member Cards.

## How It Works

### Normal Operation

1. The system starts in the lock input state.
2. The user can enter a password using the keypad.
3. Alternatively, an RFID card can be presented to the RC522 reader.
4. The authentication result is displayed on the LCD.
5. If authentication succeeds:

   * The servo unlocks the door.
   * The buzzer/LED provide feedback.
   * The door remains unlocked for a configured period.
6. The system returns to the locked state.

### Card Management

The administrator can enter the RFID management menu using the keypad.

```text
A → RFID Management

B → Add Master Card
C → Add Member Card
D → Delete Member Card
* → Exit
```

The implementation also checks whether a Master Card has already been registered before allowing Member Card management operations.

## Getting Started

### Requirements

* STM32 development board
* ST-Link programmer/debugger
* Matrix keypad
* RC522 RFID module
* LCD module
* Servo motor
* Buzzer
* LED
* STM32CubeMX
* Keil MDK or compatible ARM toolchain

## Future Improvements

Possible extensions for the system include:

*  Password storage in internal Flash
*  More robust user/card management
*  Access-event logging
*  Lockout after multiple failed authentication attempts
*  Low-power optimization
*  Bluetooth/BLE remote access
*  Wi-Fi connectivity
*  Cloud-based access logging
*  Firmware OTA update
*  Encrypted authentication and secure credential storage

## Learning Objectives

This project was developed as a practical embedded-system project to improve skills in:

* STM32 firmware development
* Embedded C programming
* Peripheral driver development
* SPI communication
* GPIO control
* PWM and timer configuration
* Matrix keypad scanning
* RFID communication
* State machine design
* Modular firmware architecture
* Hardware/software integration

GitHub:
https://github.com/vuvannghiavp

---
Youtube: https://youtu.be/yFuyCFFplpU?si=D8Lx6cwB--oxFIfd

