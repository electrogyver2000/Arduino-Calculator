# Arduino Serial Calculator
This project demonstrates a robust, bi-directional communication bridge between a Windows-based PC and an Arduino microcontroller. Designed as a modular calculator system, it facilitates the exchange of algebraic expressions via an RS232/UART serial interface, featuring real-time visual feedback and automated transaction logging.

## How it works
The PC Side: A C++ console application that treats the serial COM port as a file stream. It handles expression input, logs transactions to a file, and manages communication settings.

• The Hardware Side: An Arduino Uno that listens for a "CALC " prefix. It parses the string, performs the math, and manages visual feedback via status LEDs and an I2C LCD display.

• The Protocol: Simple ASCII-based communication. The PC sends "CALC [expr]\n" and the Arduino replies with the result.

## Project Structure
/PC_App: Contains the C++ source code for the Windows console application.

/Arduino_Firmware: Contains the .ino file to be uploaded to the Arduino.

## Getting Started
### 1. Hardware Requirements
• Arduino Uno (or compatible board)
• I2C LCD Display (16x2)
• 2 LEDs (1 Green for Success, 1 Red for Errors)
• 2 Resistors (1k Ohm for LEDs)
• USB-to-Serial Module (if not using the built-in USB port)

### 2. Wiring
• LCD: Connect SDA to A4 and SCL to A5.
• Green LED: Connect to Pin 7 (via 1k resistor).
• Red LED: Connect to Pin 8 (via 1k resistor).
• Ground: Ensure all components share a common ground.

### 3. Compilation & Setup
• Arduino: Open the firmware in the Arduino IDE, install the LiquidCrystal_I2C library, and upload the code. Ensure the Baud Rate is set to 4800.
• C++ Application: Open the project in Microsoft Visual Studio. Build the solution in Release or Debug mode.
• Execution: Run the generated .exe. When prompted, select the correct COM Port and ensure the baud rate matches your Arduino (4800).

### Troubleshooting
• "Garbage" Characters: This usually means your baud rate is mismatched. Check your Serial.begin() in the Arduino code against the value entered in the C++ console.
• Access Denied: If the PC app can't open the port, make sure the Arduino Serial Monitor (or any other application) is closed.
• Truncated Results: If your math results look cut off, ensure your serial buffer size (256 bytes) is sufficient for your expressions.

### Known Limitations
• This calculator currently handles two arguments and one operator at a time (e.g., CALC 10 + 5).
• It does not support order-of-operations (PEMDAS) for complex strings like CALC 2 + 3 * 4.

Project maintained by Michael Lombard. Contributions and feedback are welcome!
