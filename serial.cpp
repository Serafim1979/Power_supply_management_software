#include "serial.h"


HANDLE hComPort = {INVALID_HANDLE_VALUE};
// Function for opening the COM port
bool OpenCOMPort(const char* portName) {
    // ������� ������ ����������, ���� ��� ����
    if (hComPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hComPort);
        hComPort = INVALID_HANDLE_VALUE;
    }

    // ������� ����� COM-����
    hComPort = CreateFile(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,              // No sharing
        NULL,           // No security attributes
        OPEN_EXISTING,  // Open existing port
        0,              // No overlapped I/O
        NULL            // No template file
    );

    if (hComPort == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open COM port: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

// ������� ��� ������������ COM-����� � ��������� ������� ��������� �������
bool ConfigureCOMPort(HWND hComboBoxPort, HWND hComboBoxBaudRate, HWND hComboBoxByteSize, HWND hComboBoxParity, HWND hComboBoxStopBits) {
    DCB dcbSerialParams = {0};

    char portName[256];
    char baudRate[256];
    char dataBits[256];
    char parity[256];
    char stopBits[256];

    // �������� ������� ��������� �����
    if (!GetCommState(hComPort, &dcbSerialParams)) {
        std::cerr << "Failed to get COM port state: " << GetLastError() << std::endl;
        return false;
    }

    GetWindowText(hComboBoxPort, portName, sizeof(portName));
    GetWindowText(hComboBoxBaudRate, baudRate, sizeof(baudRate));
    GetWindowText(hComboBoxByteSize, dataBits, sizeof(dataBits));
    GetWindowText(hComboBoxParity, parity, sizeof(parity));
    GetWindowText(hComboBoxStopBits, stopBits, sizeof(stopBits));

    // ��������� ��������� �����
    dcbSerialParams.BaudRate = std::stoi(baudRate);
    dcbSerialParams.ByteSize = std::stoi(dataBits);
    dcbSerialParams.Parity = (parity == std::string("None")) ? NOPARITY :
                              ((parity == std::string("Odd")) ? ODDPARITY :
                              ((parity == std::string("Even")) ? EVENPARITY :
                              ((parity == std::string("Mark")) ? MARKPARITY : SPACEPARITY)));
    dcbSerialParams.StopBits = (stopBits == std::string("1")) ? ONESTOPBIT :
                                ((stopBits == std::string("1.5")) ? ONE5STOPBITS : TWOSTOPBITS);

    // ���������� ��������� �����
    if (!SetCommState(hComPort, &dcbSerialParams)) {
        std::cerr << "Failed to set COM port state: " << GetLastError() << std::endl;
        return false;
    }

    // ��������� �������� (�� ���������)
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hComPort, &timeouts)) {
        std::cerr << "Failed to set COM port timeouts: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

//������� ��� ���������� ComboBox
void PopulateCOMPorts(HWND hComboBoxPort) {
    for (int i = 1; i <= 256; i++) {
        char portName[10];
        snprintf(portName, sizeof(portName), "COM%d", i);

        HANDLE hCOM = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCOM != INVALID_HANDLE_VALUE) {
            CloseHandle(hCOM);
            SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)portName);
            printf("%s\n", portName);
        }
    }
}

void PopulateByteSizes(HWND hComboBoxByteSize) {
    std::vector<std::string> byteSizes = {"5", "6", "7", "8"};
    for (const std::string& size : byteSizes) {
        SendMessage(hComboBoxByteSize, CB_ADDSTRING, 0, (LPARAM)size.c_str());
    }
    SendMessage(hComboBoxByteSize, CB_SETCURSEL, 3, 0); // ������������� 8 ��� ��� �������� �� ���������
}

void PopulateParities(HWND hComboBoxParity) {
    std::vector<std::string> parities = {"None", "Odd", "Even", "Mark", "Space"};
    for (const std::string& parity : parities) {
        SendMessage(hComboBoxParity, CB_ADDSTRING, 0, (LPARAM)parity.c_str());
    }
    SendMessage(hComboBoxParity, CB_SETCURSEL, 0, 0); // ������������� None ��� �������� �� ���������
}
void PopulateStopBits(HWND hComboBoxStopBits) {
    std::vector<std::string> stopBits = {"1", "1.5", "2"};
    for (const std::string& bits : stopBits) {
        SendMessage(hComboBoxStopBits, CB_ADDSTRING, 0, (LPARAM)bits.c_str());
    }
    SendMessage(hComboBoxStopBits, CB_SETCURSEL, 0, 0); // ������������� 1 ��� �������� �� ���������
}

void PopulateBaudRates(HWND hComboBoxBaudRate)
{
    //���������� ������ ��������� �������� ������
    std::vector<std::string> baudRates = {"4800", "9600", "19200", "38400", "57600", "115200"};
    for (const std::string& rate : baudRates) {
        SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)rate.c_str());
    }

    // ��������� �������� �� ���������
    SendMessage(hComboBoxBaudRate, CB_SETCURSEL, 0, 0);
}
