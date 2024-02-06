# eer components

[eer](http://github.com/fefa4ka/eer) is a C99 library for creating applications using a declarative approach. It allows developers to create simple components for each subsystem and compile a system using these functional components.

In general, a declarative programming model can be useful for applications because it allows you to **describe the desired output of the system based on its state**, rather than specifying the exact steps to be taken to achieve that output. This can help to simplify the code and make it easier to understand and maintain.

Using functional components can also be a useful approach for applications because it allows you to break down the system into smaller, self-contained units of code that can be developed and tested independently. This can help to improve the modularity and maintainability of the code.

-   **Hardware Abstraction Layer (HAL)**: provides a common interface for interacting with the hardware.
-   **Components**: a set of components that can be used to build applications.
-   **Data Processing**: components for data processing, such as data compression, encryption, and integrity.
-   **Communication**: components for communication, such as Bluetooth, Zigbee, LoRa, OTA, NFC, USB, I2C, SPI, UART, and CAN.
-   **Human Interface**: components for user interfaces, such as displays, menus, and sliders.
-   **System Monitoring**: components for system monitoring, such as error handling, fault tolerance, and self-test.
-   **Control Loop**: components for control loops, such as motor control and data acquisition.
-   **Safety**: components for safety, such as temperature management and resource management.

## H/W

H/W components are used to interact with the hardware, such as clocks, ADCs, and EEPROMs. These components can be used to control the hardware and to store data.

List of H/W related Components:

-   [IO](./IO)
-   [Clock](./Clock)
-   [ADC](./ADC)
-   EEPROM
-   RTC
-   USB
-   DMA
-   Watchdog
-   Interrupt
-   File System
-   Data Logging
-   Power Management
-   Security

#### I/O

I/O components are used to interact with the hardware, such as buttons, encoders, and serial ports. These components can be used to control the hardware and to receive data from sensors.

List of I/O related Components:

-   [Button](./Button)
-   [Serial](./Serial)
-   [Encoder](./Encoder)
-   [OneWire](./OneWire)
-   [PWM](./PWM)
-   [Scheduler](./Scheduler)
-   [Bitbang](./Bitbang)
-   [SPIComputer](./SPIComputer)
-   [SPIPeriphery](./SPIPeriphery)
-   [Servo](./Servo)
-   [SIPO](./SIPO)

#### Communication

Communication components are used to send and receive data between devices. These components can be used to enable communication between different devices, such as Bluetooth, Zigbee, LoRa, OTA, NFC, USB, I2C, SPI, UART, and CAN.

List of Communication related Components:

-   Bluetooth
-   Zigbee
-   LoRa
-   OTA
-   NFC
-   USB
-   I2C
-   SPI
-   UART
-   CAN

#### Human Interface

Human interface components are used to create user interfaces, such as displays, menus, and sliders. These components can be used to create user-friendly interfaces that allow users to interact with the system.

List of Human Interface related Components:

-   Display
-   [Menu](./Menu)
-   Slider
-   [Shell](./Shell)

## Data Processing

Data processing components are used to process data, such as compressing, encrypting, and verifying the integrity of data. These components can be used to improve the performance of applications by reducing the amount of data that needs to be transferred or stored.

List of Data Processing related Components:

-   Data Compression
-   Data Encryption
-   Data Integrity
-   Error Correction

#### System Monitoring

System monitoring components are used to monitor the system, such as error handling, fault tolerance, and self-test. These components can be used to ensure that the system is running correctly and to detect and respond to any errors or faults.

List of System Monitoring related Components:

-   Error Handling
-   Fault Tolerance
-   Self-Test

#### Control Loop

Control loop components are used to control the system, such as motor control and data acquisition. These components can be used to control the system in real-time and to acquire data from sensors.

List of Control Loop related Components:

-   Motor Control
-   Data Acquisition
-   Data Analysis
-   Control Loop

#### Safety

Safety components are used to ensure the safety of the system, such as temperature management and resource management. These components can be used to ensure that the system is operating within safe limits and to prevent any potential hazards.

List of Safety related Components:

-   Temperature Management
-   Resource Management

#### Functional Components

Functional components are components that use eer abstractions to perform specific tasks. These components can be used to create applications that are tailored to specific needs.

List of Functional Components:

-   Network Stack
-   Web Server
-   Web Client
-   Cloud Connectivity
-   Device Management
-   Remote Access
-   Alarm
-   Event Logging
-   Backup and Recovery
-   Firmware Update
-   Task Scheduling
-   Sensor
-   Actuator
-   GPS
-   Audio
-   Image Processing
-   Voice Recognition
-   Voice Synthesis
-   Object Detection
-   Machine Learning
