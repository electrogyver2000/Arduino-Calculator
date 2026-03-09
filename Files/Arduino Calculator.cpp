/*
 * Arduino Calculator using RS232
 * * How it works:
 * 1. The user picks a COM port and sets the baud rate/parity/stop bits.
 * 2. We open the serial handle using the Windows API—it's a bit of a headache
 * with Unicode strings, but it works reliably once you get it right.
 * 3. The program loops, taking input, sending it over as an ASCII string,
 * and waiting for the Arduino to send back the result.
 * * I've included a log.txt feature because serial communication can be tricky,
 * and seeing the exact history of what was sent vs. received has been a
 * total lifesaver for debugging.
 * * To stop the program, just type 'exit'.
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Converts a standard C++ narrow string to a wide string.
// This is needed because Windows API functions expects Unicode wide character strings instead of standard ASCII strings.
std::wstring StringToWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

// Converts a entered parity character('N', 'E', 'O', etc.) into the
// right Windows API parity constant used in the serial port
// configuration (DCB structure).
BYTE GetParity(char p) {
    switch (toupper(p)) {
    case 'N': return NOPARITY;
    case 'E': return EVENPARITY;
    case 'O': return ODDPARITY;
    case 'M': return MARKPARITY;
    case 'S': return SPACEPARITY;
    default:  return NOPARITY;
    }
}

// Converts a entered number of stop bits into the right
// Windows API constant used when configuring the serial port (DCB structure)
BYTE GetStopBits(int s) {
    switch (s) {
    case 1: return ONESTOPBIT;
    case 2: return TWOSTOPBITS;
    case 3: return ONE5STOPBITS;
    default: return ONESTOPBIT;
    }
}

int main() {
    // Display welcome message. Select the COM port that the Arduino is connected to
    std::vector<std::string> availablePorts = { "COM1", "COM2", "COM3", "COM4", "COM5" };
    std::cout << "\033[1;34m*******************************\n";
    std::cout << "* Hello, welcome to your    *\n";
    std::cout << "*   Arduino Calculator!     *\n";
    std::cout << "*******************************\n";  
    std::cout << "Available COM ports : \n";
    for (size_t i = 0; i < availablePorts.size(); i++) {
        std::cout << i + 1 << ". " << availablePorts[i] << "\n";
    }

    // Check the selected COM port and verify.
    // If the selection is outside the available range, the program
    // exits to prevent attempting to open an invalid serial port.
    int choice = 0;
    std::cout << "Please select the COM port the arduino is connected to: ";
    std::cin >> choice;
    std::cin.ignore(); // clear newline
    if (choice < 1 || choice > availablePorts.size()) {
        std::cerr << "Invalid selection!\n";
        return 1;
    }
    std::string comPort = availablePorts[choice - 1];

    // Configure the serial communication baud rate.
    // The user may enter a custom value, otherwise the default
    // baud rate of 9600 is used for communication with the Arduino.
    int baudRate = 9600;
    std::cout << "Please enter the baud rate (default 9600): ";
    std::string brInput;
    std::getline(std::cin, brInput);
    if (!brInput.empty()) baudRate = std::stoi(brInput);

    // Configure parity setting for the serial connection.
    // The user can select the parity mode (None, Even, or Odd).
    // If no input is provided, the default parity 'N' (None) is used.
    char parityChar = 'N';
    std::cout << "Please enter the parity bit [N=none, E=even, O=odd] (default N): ";
    std::string pInput;
    std::getline(std::cin, pInput);
    if (!pInput.empty()) parityChar = toupper(pInput[0]);

    // Configure the number of stop bits for the serial communication.
    // The user may enter 1 or 2 stop bits. If no value is provided,
    // the default value of 1 stop bit is used.
    int stopBits = 1;
    std::cout << "Lastly, please enter the stop bits [1, 2] (default 1): ";
    std::string sInput;                                                         //
    std::getline(std::cin, sInput);                                             // 
    if (!sInput.empty()) stopBits = std::stoi(sInput);                          //

    // Open the selected COM port using the Windows API.
    // The port name must be converted to wide-character format and written as
    // "\\\\.\\COMx" to allow Windows to access serial ports correctly.
    // CreateFileW returns a handle to the serial device, which will be used
    // for reading from and writing to the Arduino.
    std::wstring wComPort = StringToWString("\\\\.\\") + StringToWString(comPort);
    HANDLE hSerial = CreateFileW(wComPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, 0, OPEN_EXISTING, 0, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Oh no! We have an error opening the COM port " << comPort << "\n";
        return 1;
    }

    // Configure the serial communication parameters using the
    // Windows Device Control Block (DCB) structure. This defines
    // settings such as baud rate, data size, parity, and stop bits
    // required for correct communication with the Arduino.
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial, &dcb)) {
        std::cerr << "Error getting COM state\n";
        CloseHandle(hSerial);
        return 1;
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = GetStopBits(stopBits);
    dcb.Parity = GetParity(parityChar);

    if (!SetCommState(hSerial, &dcb)) {
        std::cerr << "Error setting COM state\n";
        CloseHandle(hSerial);
        return 1;
    }

    // Configure serial port timeouts to prevent the program from
    // hanging if no data is received or if writing is delayed.
    // - ReadIntervalTimeout: max time between characters during a read
    // - ReadTotalTimeoutConstant/Multiplier: total read timeout settings
    // - WriteTotalTimeoutConstant/Multiplier: total write timeout settings
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    // Inform the user that the connection is established and provide instructions
    std::cout << "Connected to " << comPort << " @ " << baudRate << " baud, now the magic happens!\n Please enter CALC followed by your values you want to be calculated\n For example: CALC 15 x 25, etc. \n";
    
    // Open the log file to record all communication
    std::ofstream logFile("log.txt", std::ios::app);
    // Prepare buffers and variables for serial communication
    char buffer[256];                   // buffer to store incoming data from Arduino
    DWORD bytesRead, bytesWritten;      // track number of bytes read/written
    std::string input;                  // store user input from console

    // Main program loop: repeatedly prompts the user for input,
    // sends calculation requests to the Arduino, and receives responses.
    // The loop continues until the user enters "exit".
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        // Send the user’s calculation command to the Arduino via the serial port.
        // A newline character is appended to signal the end of the command.
        // If WriteFile fails, an error is displayed and the loop continues.
        // The sent command is also logged to log.txt for debugging and traceability.
        std::string sendStr = input + "\n";
        if (!WriteFile(hSerial, sendStr.c_str(), sendStr.length(), &bytesWritten, NULL)) {
            std::cerr << "Failed to communicate with Arduino, check connection to board\n";
            continue;
        }

        // Record the sent command in the transaction log
        logFile << "Sent: " << sendStr;

        // Read the response from the Arduino via the serial port.
        // ReadFile stores the incoming data in 'buffer' and returns the number of bytes read.
        // If data is received, a null terminator is added to make it a proper C-string.
        // The response is displayed to the user and also logged in log.txt for traceability
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';               
            std::cout << buffer;                    
            logFile << "Received: " << buffer;      // log response
        }
    }

    // Close resources and exit the program safely.
    // - Close the log file to ensure all transactions are saved.
    // - Close the serial port handle to release the COM port.
    logFile.close();
    CloseHandle(hSerial);
    std::cout << "Exiting...\n";
    return 0;
}
