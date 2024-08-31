# Power Supply Control Panel

This project provides a graphical user interface (GUI) for controlling a power supply device via a serial (COM) port using the SCPI (Standard Commands for Programmable Instruments) protocol.

## Features

- **Connect to Power Supply**: Establish a connection to a power supply device through a user-selected COM port.
- **Set Output Parameters**: Configure output voltage and current, and adjust the rise and fall times for voltage and current transitions.
- **Control Output**: Enable or disable the output of the power supply.
- **Real-Time Monitoring**: Display real-time measurements of voltage and current, along with the maximum recorded values during operation.
- **Connection Status**: Visual indicator (LED simulation) showing the connection status of the device.
- **Polling Mechanism**: Timer-based polling for updating measurements periodically.

## Getting Started

### Prerequisites

- **Windows Operating System**: The application is designed for Windows using the WinAPI.
- **C++ Compiler**: A C++ compiler (such as MSVC) that supports the Windows API is required.
- **Serial Communication Library**: The project assumes you have a library or code for handling serial communication (e.g., `serial.h` and `scpi.h`).

### Building the Project

1. Clone the repository to your local machine:
   git clone https://github.com/yourusername/power-supply-control.git
   
2. Open the project in your preferred C++ IDE or compile it directly from the command line:
  cl /EHsc /DUNICODE /D_UNICODE power_supply_control.cpp /link user32.lib gdi32.lib

3. Run the executable to launch the control panel.

Usage:
1. Select COM Port: Use the dropdown to select the COM port connected to your power supply device.
2. Configure Connection: Set the baud rate, data bits, parity, and stop bits according to your device's specifications.
3. Connect: Click the "Connect" button to establish communication with the device.
4. Set Parameters: Input the desired voltage, current, rise, and fall times, and click the corresponding "Set" buttons.
5. Enable/Disable Output: Use the "OUTP ON" and "OUTP OFF" buttons to control the power supply output.
6.Monitor: Observe the real-time voltage and current readings, as well as the maximum values recorded during the session.

File Structure
  power_supply_control.cpp: The main source file containing the GUI implementation and communication logic.
  serial.h and scpi.h: Headers for serial communication and SCPI protocol handling.
  
Contributions
Contributions are welcome! Please fork the repository and submit a pull request with your changes. Ensure your code follows the project's coding style and includes relevant comments.

License
This project is licensed under the MIT License - see the LICENSE file for details.

Contact
For questions or feedback, please contact korolkov.ivan@mail.ru
