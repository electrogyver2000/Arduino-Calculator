#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Convert narrow string to wide string
std::wstring StringToWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

// Convert parity  ('N', 'E', 'O', etc.) to Windows format
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

// convert stop bit to Windows format
BYTE GetStopBits(int s) {
    switch (s) {
    case 1: return ONESTOPBIT;
    case 2: return TWOSTOPBITS;
    case 3: return ONE5STOPBITS;
    default: return ONESTOPBIT;
    }
}

int main() {
    // select COM port 
    std::vector<std::string> availablePorts = { "COM1", "COM2", "COM3", "COM4", "COM5" };
    std::cout << "\033[1;34m*******************************\n";
    std::cout << "* Hello, welcome to your    *\n";
    std::cout << "*   Arduino Calculator!     *\n";
    std::cout << "*******************************\n";  
    std::cout << "Available COM ports : \n";
    for (size_t i = 0; i < availablePorts.size(); i++) {
        std::cout << i + 1 << ". " << availablePorts[i] << "\n";
    }

    int choice = 0;
    std::cout << "Please select the COM port the arduino is connected to: ";
    std::cin >> choice;
    std::cin.ignore(); // clear newline
    if (choice < 1 || choice > availablePorts.size()) {
        std::cerr << "Invalid selection!\n";
        return 1;
    }
    std::string comPort = availablePorts[choice - 1];

    // Baud rate 
    int baudRate = 9600;
    std::cout << "Please enter the baud rate (default 9600): ";
    std::string brInput;
    std::getline(std::cin, brInput);
    if (!brInput.empty()) baudRate = std::stoi(brInput);

    // Parity selection
    char parityChar = 'N';
    std::cout << "Please enter the parity bit [N=none, E=even, O=odd] (default N): ";
    std::string pInput;
    std::getline(std::cin, pInput);
    if (!pInput.empty()) parityChar = toupper(pInput[0]);

    // Stop bits
    int stopBits = 1;
    std::cout << "Lastly, please enter the stop bits [1, 2] (default 1): ";
    std::string sInput;
    std::getline(std::cin, sInput);
    if (!sInput.empty()) stopBits = std::stoi(sInput);

    // Open COM port
    std::wstring wComPort = StringToWString("\\\\.\\") + StringToWString(comPort);
    HANDLE hSerial = CreateFileW(wComPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, 0, OPEN_EXISTING, 0, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Oh no! We have an error opening the COM port " << comPort << "\n";
        return 1;
    }

    // Configure serilal
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

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    std::cout << "Connected to " << comPort << " @ " << baudRate << " baud, now the magic happens!\n Please enter CALC followed by your values you want to be calculated\n For example: CALC 15 x 25, etc. \n";

    std::ofstream logFile("log.txt", std::ios::app);
    char buffer[256];
    DWORD bytesRead, bytesWritten;
    std::string input;

    // Main loop
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        // Send calculation
        std::string sendStr = input + "\n";
        if (!WriteFile(hSerial, sendStr.c_str(), sendStr.length(), &bytesWritten, NULL)) {
            std::cerr << "Failed to communicate with Arduino, check connection to board\n";
            continue;
        }

        logFile << "Sent: " << sendStr;

        // ResponseS
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << buffer;
            logFile << "Received: " << buffer;
        }
    }

    logFile.close();
    CloseHandle(hSerial);
    std::cout << "Exiting...\n";
    return 0;
}
