#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>

//���������� ����������
const char g_szClassName[] = "MainWindowClass";
HWND hComboBoxPort, hComboBoxBaudRate, hComboBoxByteSize, hComboBoxParity, hComboBoxStopBits, hButtonConnect, hLED;
HWND hEditVoltage, hEditCurrent, hEditRiseTime, hEditFallTime, hEditMemoryBank;
HWND hButtonSetVoltage, hButtonSetCurrent, hButtonSetRiseTime, hButtonSetFallTime, hButtonSaveSettings;
HINSTANCE g_hInst;
HANDLE hComPort = INVALID_HANDLE_VALUE; // ���������� COM-�����

HWND hStaticVoltage, hStaticCurrent, hStaticMaxVoltage, hStaticMaxCurrent;

double maxVoltage = 0.0;
double maxCurrent = 0.0;

//��������� �������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PopulateCOMPorts();
void PopulateBaudRates();
void PopulateByteSizes();
void PopulateParities();
void PopulateStopBits();
void UpdateLED(int status);

//������� ��� ������ � COM-������
bool OpenCOMPort(const char* portName);
void CloseCOMPort();
bool ConfigureCOMPort();
bool WriteToCOMPort(const char* command);
bool ReadFromCOMPort(char* buffer, int bufferSize);

//������� ��� ���������� ���������� �������
void sendCommand(const std::string& command);
bool SetOnDelay(double onDelay);
bool SetOffDelay(double offDelay);
bool EnableCurrentProtection(bool enable);
bool EnableVoltageProtection(bool enable);

void SetVoltage();
void SetCurrent();
void SetRiseTime();
void SetFallTime();
void SaveSettings();

//������� ��� ������ � ����������� ��������
bool GetVoltage(double& voltage);
bool GetCurrent(double& current);
void RegisterMinMaxValues(double voltage, double current);

//������� ��� ������������ ����������
void CreateMainWindow();
void UpdateDisplay();
void VisualizeCurrent();
void UpdateLED(int ledID, int status);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // ������������� ��������� WNDCLASSEX
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    // ����������� �������� ������
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // �������� �������� ����
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Power Supply Control Panel, 4160",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ���� ���������
    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            {
            // �������� ComboBox ��� ������ COM-�����
            hComboBoxPort = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 50, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateCOMPorts();

            // �������� ComboBox ��� ������ �������� �������� ������
            hComboBoxBaudRate = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 100, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateBaudRates();

            // �������� ComboBox ��� ������ ������� �����
            hComboBoxByteSize = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 150, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateByteSizes();

            // �������� ComboBox ��� ������ �������� ��������
            hComboBoxParity = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 200, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateParities();

            // �������� ComboBox ��� ������ ����-�����
            hComboBoxStopBits = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 250, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateStopBits();

            // �������� ������ Connect
            hButtonConnect = CreateWindow("BUTTON", "Connect",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 300, 100, 30, hwnd, (HMENU)1, g_hInst, NULL);

            // �������� ���������� ��� ��������� ���������
            hLED = CreateWindow("STATIC", NULL,
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                160, 300, 20, 20, hwnd, NULL, g_hInst, NULL);
            UpdateLED(0); // ����������� ��������� (�����)


            hStaticVoltage = CreateWindow("STATIC", "Voltage: 0.0 V",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                250, 50, 150, 20, hwnd, NULL, g_hInst, NULL);

            hStaticCurrent = CreateWindow("STATIC", "Current: 0.0 A",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                250, 100, 150, 20, hwnd, NULL, g_hInst, NULL);

            hStaticMaxVoltage = CreateWindow("STATIC", "Max Voltage: 0.0 V",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                250, 150, 150, 20, hwnd, NULL, g_hInst, NULL);

            hStaticMaxCurrent = CreateWindow("STATIC", "Max Current: 0.0 A",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                250, 200, 150, 20, hwnd, NULL, g_hInst, NULL);

            hEditVoltage = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                50, 350, 100, 20, hwnd, NULL, g_hInst, NULL);
            hButtonSetVoltage = CreateWindow("BUTTON", "Set Voltage", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 350, 100, 20, hwnd, (HMENU)2, g_hInst, NULL);

            hEditCurrent = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                50, 380, 100, 20, hwnd, NULL, g_hInst, NULL);
            hButtonSetCurrent = CreateWindow("BUTTON", "Set Current", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 380, 100, 20, hwnd, (HMENU)3, g_hInst, NULL);

            hEditRiseTime = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                50, 410, 100, 20, hwnd, NULL, g_hInst, NULL);
            hButtonSetRiseTime = CreateWindow("BUTTON", "Set Rise Time", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 410, 100, 20, hwnd, (HMENU)4, g_hInst, NULL);

            hEditFallTime = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                50, 440, 100, 20, hwnd, NULL, g_hInst, NULL);
            hButtonSetFallTime = CreateWindow("BUTTON", "Set Fall Time", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 440, 100, 20, hwnd, (HMENU)5, g_hInst, NULL);

            hEditMemoryBank = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
                50, 470, 100, 100, hwnd, NULL, g_hInst, NULL);
            for (int i = 1; i <= 9; ++i) {
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "%d", i);
                SendMessage(hEditMemoryBank, CB_ADDSTRING, 0, (LPARAM)buffer);
            }
            hButtonSaveSettings = CreateWindow("BUTTON", "Save Settings", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 470, 100, 20, hwnd, (HMENU)6, g_hInst, NULL);
            }
            break;

        case WM_COMMAND:
        {
            if(LOWORD(wParam) == 1) { // ������ ������ Connect
                char portName[10];
                GetWindowText(hComboBoxPort, portName, 10);

            // ��������, ��� �� ������ COM-����
            if (strlen(portName) == 0) {
                MessageBox(hwnd, "Please select a COM port.", "Error", MB_OK | MB_ICONERROR);
                UpdateLED(2); // ������� (�������)
                return 0;
            }

            // ����������� ������� � ��������� COM-����
            bool portOpen = OpenCOMPort(portName);
            bool portConfigured = ConfigureCOMPort(); //(baudRate, byteSize, parity, stopBits);

            if (portOpen && portConfigured) {
                UpdateLED(1); // ����� (�������)

                // ��������� ������� ��������
                double voltage = 0.0, current = 0.0;
                    if (GetVoltage(voltage) && GetCurrent(current)) {

                        sendCommand("SYST:REM");
                        sendCommand("OUTP ON");

                        RegisterMinMaxValues(voltage, current);

                        // ���������� GUI � �������� ����������
                        char buffer[50];

                        snprintf(buffer, sizeof(buffer), "Voltage: %.2f V", voltage);
                        SetWindowText(hStaticVoltage, buffer);
                        snprintf(buffer, sizeof(buffer), "Current: %.2f A", current);
                        SetWindowText(hStaticCurrent, buffer);

                        snprintf(buffer, sizeof(buffer), "Max Voltage: %.2f V", maxVoltage);
                        SetWindowText(hStaticMaxVoltage, buffer);
                        snprintf(buffer, sizeof(buffer), "Max Current: %.2f A", maxCurrent);
                        SetWindowText(hStaticMaxCurrent, buffer);
                    } else {
                        UpdateLED(2); // ������� (�������)
                    }
                } else {
                    UpdateLED(2); // ������� (�������)
                }
            }
            else if (LOWORD(wParam) == 2) { // ������ ������ Set Voltage
                SetVoltage();
            } else if (LOWORD(wParam) == 3) { // ������ ������ Set Current
                SetCurrent();
            } else if (LOWORD(wParam) == 4) { // ������ ������ Set Rise Time
                SetRiseTime();
            } else if (LOWORD(wParam) == 5) { // ������ ������ Set Fall Time
                SetFallTime();
            } else if (LOWORD(wParam) == 6) { // ������ ������ Save Settings
                SaveSettings();
            }
        }
        break;

        case WM_CLOSE:
            // ������� COM-���� ��� ���������� ������
            if (hComPort != INVALID_HANDLE_VALUE) {
                CloseHandle(hComPort);
            }
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}



// ������� ��� ������ � COM-������ (��������)
bool OpenCOMPort(const char* portName) {
    // ������� ������ ����������, ���� ��� ����
    if (hComPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hComPort);
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


void CloseCOMPort() {
    std::cout << "CloseCOMPort" << std::endl;
}

void PopulateByteSizes() {
    std::vector<std::string> byteSizes = {"5", "6", "7", "8"};
    for (const std::string& size : byteSizes) {
        SendMessage(hComboBoxByteSize, CB_ADDSTRING, 0, (LPARAM)size.c_str());
    }
    SendMessage(hComboBoxByteSize, CB_SETCURSEL, 3, 0); // ������������� 8 ��� ��� �������� �� ���������
}

void PopulateParities() {
    std::vector<std::string> parities = {"None", "Odd", "Even", "Mark", "Space"};
    for (const std::string& parity : parities) {
        SendMessage(hComboBoxParity, CB_ADDSTRING, 0, (LPARAM)parity.c_str());
    }
    SendMessage(hComboBoxParity, CB_SETCURSEL, 0, 0); // ������������� None ��� �������� �� ���������
}
void PopulateStopBits() {
    std::vector<std::string> stopBits = {"1", "1.5", "2"};
    for (const std::string& bits : stopBits) {
        SendMessage(hComboBoxStopBits, CB_ADDSTRING, 0, (LPARAM)bits.c_str());
    }
    SendMessage(hComboBoxStopBits, CB_SETCURSEL, 0, 0); // ������������� 1 ��� �������� �� ���������
}

bool ConfigureCOMPort() {
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
    dcbSerialParams.Parity = (parity == std::string("None")) ? NOPARITY : ((parity == std::string("Odd")) ? ODDPARITY : ((parity == std::string("Even")) ? EVENPARITY : ((parity == std::string("Mark")) ? MARKPARITY : SPACEPARITY)));
    dcbSerialParams.StopBits = (stopBits == std::string("1")) ? ONESTOPBIT : ((stopBits == std::string("1.5")) ? ONE5STOPBITS : TWOSTOPBITS);

    // ���������� ��������� �����   ����� ����������� ������
    if (!SetCommState(hComPort, &dcbSerialParams)) {
        std::cerr << "Failed to set COM port state:? " << GetLastError() << std::endl;
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


bool WriteToCOMPort(const char* command) {
    DWORD bytesWritten;
    if (!WriteFile(hComPort, command, strlen(command), &bytesWritten, NULL)) {
        std::cerr << "Failed to write to COM port" << std::endl;
        return false;
    }
    return true;
}

bool ReadFromCOMPort(char* buffer, int bufferSize) {
    DWORD bytesRead;
    if (!ReadFile(hComPort, buffer, bufferSize - 1, &bytesRead, NULL)) {
        std::cerr << "Failed to read from COM port" << std::endl;
        return false;
    }
    buffer[bytesRead] = '\0';

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//������� ��� ���������� ���������� �������
void SetVoltage() {
    char buffer[20];
    GetWindowText(hEditVoltage, buffer, sizeof(buffer));
    double voltage = atof(buffer);
    char command[50];
    snprintf(command, sizeof(command), "VOLT %.2f\n", voltage);
    if (WriteToCOMPort(command)) {
        std::cout << "Voltage set to " << voltage << " V" << std::endl;
    }
}

void SetCurrent() {
    char buffer[20];
    GetWindowText(hEditCurrent, buffer, sizeof(buffer));
    double current = atof(buffer);
    char command[50];
    snprintf(command, sizeof(command), "CURR %.2f\n", current);
    if (WriteToCOMPort(command)) {
        std::cout << "Current set to " << current << " A" << std::endl;
    }
}

void SetRiseTime() {
    char buffer[20];
    GetWindowText(hEditRiseTime, buffer, sizeof(buffer));
    int riseTime = atoi(buffer);
    char command[50];
    snprintf(command, sizeof(command), "CURR:RISE %d\n", riseTime);
    if (WriteToCOMPort(command)) {
        std::cout << "Rise time set to " << riseTime << " ms" << std::endl;
    }
}

void SetFallTime() {
    char buffer[20];
    GetWindowText(hEditFallTime, buffer, sizeof(buffer));
    int fallTime = atoi(buffer);
    char command[50];
    snprintf(command, sizeof(command), "CURR:FALL %d\n", fallTime);
    if (WriteToCOMPort(command)) {
        std::cout << "Fall time set to " << fallTime << " ms" << std::endl;
    }
}

void SaveSettings() {
    char buffer[10];
    GetWindowText(hEditMemoryBank, buffer, sizeof(buffer));
    int memoryBank = atoi(buffer);
    char command[50];
    snprintf(command, sizeof(command), "*SAV %d\n", memoryBank);
    if (WriteToCOMPort(command)) {
        std::cout << "Settings saved to memory bank " << memoryBank << std::endl;
    }
}

bool SetOnDelay(double onDelay) {
    std::cout << "SetOnDelay" << std::endl;
    return true;
}

bool SetOffDelay(double offDelay) {
    std::cout << "SetOffDelay" << std::endl;
    return true;
}

bool EnableCurrentProtection(bool enable) {
    std::cout << "EnableCurrentProtection" << std::endl;
    return true;
}

bool EnableVoltageProtection(bool enable) {
    std::cout << "EnableVoltageProtection" << std::endl;
    return true;
}


//������� ��� ������ � ����������� ��������
bool GetVoltage(double& voltage) {
    if (hComPort == INVALID_HANDLE_VALUE) {
        std::cerr << "COM port not open." << std::endl;
        return false;
    }

    //�������� ������� ��� ��������� �������� ����������
    const char* command = "MEAS:VOLT?\n";
    if (!WriteToCOMPort(command)) {
        return false;
    }

    //������ ������
    char buffer[256];
    if (!ReadFromCOMPort(buffer, sizeof(buffer))) {
        return false;
    }

    //������� ������
    voltage = atof(buffer);

    return true;
}

bool GetCurrent(double& current) {
    if (hComPort == INVALID_HANDLE_VALUE) {
        std::cerr << "COM port not open." << std::endl;
        return false;
    }

    //�������� ������� ��� ��������� �������� ����
    const char* command = "MEAS:CURR?\n";
    if (!WriteToCOMPort(command)) {
        return false;
    }

    //������ ������
    char buffer[256];
    if (!ReadFromCOMPort(buffer, sizeof(buffer))) {
        return false;
    }

    //������� ������
    current = atof(buffer);

    return true;
}

void RegisterMinMaxValues(double voltage, double current) {
    static double maxVoltage = 0;
    static double maxCurrent = 0;

    if (voltage > maxVoltage) {
        maxVoltage = voltage;
    }

    if (current > maxCurrent) {
        maxCurrent = current;
    }

    //�������� GUI � ������������� ����������
    UpdateDisplay();
}

// ������� ��� ������������ ����������
void CreateMainWindow() {
    std::cout << "CreateMainWindow" << std::endl;
}

void UpdateDisplay() {
    std::cout << "UpdateDisplay()" << std::endl;
}

void VisualizeCurrent() {
    std::cout << "VisualizeCurrent" << std::endl;
}

void sendCommand(const std::string& command) {
    DWORD bytes_written;
    std::string cmd = command + "\n";
    if (!WriteFile(hComPort, cmd.c_str(), cmd.length(), &bytes_written, NULL)) {
            throw std::runtime_error("Error writing to serial port");
    }
}

void UpdateLED(int status) {
    HWND hwndLED = hLED;
    HDC hdc = GetDC(hwndLED);
    HBRUSH hBrush;

    switch(status) {
        case 0: // ����������� ��������� (�����)
            hBrush = CreateSolidBrush(RGB(128, 128, 128));
            break;
        case 1: // ����� (�������)
            hBrush = CreateSolidBrush(RGB(0, 255, 0));
            break;
        case 2: // ������� (�������)
            hBrush = CreateSolidBrush(RGB(255, 0, 0));
            break;
        default:
            hBrush = CreateSolidBrush(RGB(128, 128, 128));
            break;
    }

    RECT rect;
    GetClientRect(hwndLED, &rect);
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    ReleaseDC(hwndLED, hdc);
}

//������� ��� ���������� ComboBox
void PopulateCOMPorts() {
    for (int i = 1; i <= 256; i++) {
        char portName[10];
        snprintf(portName, sizeof(portName), "COM%d", i);

        HANDLE hCOM = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCOM != INVALID_HANDLE_VALUE) {
            CloseHandle(hCOM);
            SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)portName);
        }
    }
}

void PopulateBaudRates() {
    //���������� ������ ��������� �������� ������
    std::vector<std::string> baudRates = {"9600", "19200", "38400", "57600", "115200"};
    for (const std::string& rate : baudRates) {
        SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)rate.c_str());
    }

    // ��������� �������� �� ���������
    SendMessage(hComboBoxBaudRate, CB_SETCURSEL, 0, 0);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include <windows.h>
//#include <commctrl.h>
//#include <iostream>
//#include <string>
//#include <cstdio>
//
//// ���������� ���������� � �����������
//HINSTANCE g_hInst;
//HWND hMainWnd;
//HWND hComboBoxPort, hComboBoxBaudRate, hComboBoxByteSize, hComboBoxParity, hComboBoxStopBits;
//HWND hButtonConnect, hLED;
//HWND hStaticVoltage, hStaticCurrent, hStaticMaxVoltage, hStaticMaxCurrent;
//HWND hEditVoltage, hEditCurrent, hEditRiseTime, hEditFallTime, hEditMemoryBank;
//HWND hButtonSetVoltage, hButtonSetCurrent, hButtonSetRiseTime, hButtonSetFallTime, hButtonSaveSettings;
//HANDLE hComPort = INVALID_HANDLE_VALUE;
//double maxVoltage = 0.0, maxCurrent = 0.0;
//
//bool OpenCOMPort(const char* portName);
//bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits);
//bool WriteToCOMPort(const char* command);
//bool ReadFromCOMPort(char* buffer, int bufferSize);
//bool GetVoltage(double& voltage);
//bool GetCurrent(double& current);
//void RegisterMinMaxValues(double voltage, double current);
//void UpdateDisplay();
//void PopulateCOMPorts();
//void PopulateBaudRates();
//void PopulateByteSizes();
//void PopulateParities();
//void PopulateStopBits();
//void UpdateLED(int status);
//
//void SetVoltage();
//void SetCurrent();
//void SetRiseTime();
//void SetFallTime();
//void SaveSettings();
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//
//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//    g_hInst = hInstance;
//    WNDCLASS wc = {};
//    wc.lpfnWndProc = WndProc;
//    wc.hInstance = hInstance;
//    wc.lpszClassName = "MainWindowClass";
//
//    if(!RegisterClass(&wc)) {
//        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
//        return 1;
//    }
//
//    hMainWnd = CreateWindow("MainWindowClass", "Power Supply Control", WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
//
//    if(!hMainWnd) {
//        MessageBox(NULL, "Failed to create main window", "Error", MB_OK);
//        return 1;
//    }
//
//    ShowWindow(hMainWnd, nCmdShow);
//    UpdateWindow(hMainWnd);
//
//    MSG msg;
//    while(GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return msg.wParam;
//}
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    switch(msg) {
//        case WM_CREATE:
//            hComboBoxPort = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 50, 150, 100, hwnd, NULL, g_hInst, NULL);
//            PopulateCOMPorts();
//
//            hComboBoxBaudRate = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 100, 150, 100, hwnd, NULL, g_hInst, NULL);
//            PopulateBaudRates();
//
//            hComboBoxByteSize = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 150, 150, 100, hwnd, NULL, g_hInst, NULL);
//            PopulateByteSizes();
//
//            hComboBoxParity = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 200, 150, 100, hwnd, NULL, g_hInst, NULL);
//            PopulateParities();
//
//            hComboBoxStopBits = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 250, 150, 100, hwnd, NULL, g_hInst, NULL);
//            PopulateStopBits();
//
//            hButtonConnect = CreateWindow("BUTTON", "Connect", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                50, 300, 150, 30, hwnd, (HMENU)1, g_hInst, NULL);
//
//            hLED = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_CENTER,
//                220, 300, 30, 30, hwnd, NULL, g_hInst, NULL);
//
//            hStaticVoltage = CreateWindow("STATIC", "Voltage: 0.0 V",
//                WS_CHILD | WS_VISIBLE | SS_CENTER,
//                250, 50, 150, 20, hwnd, NULL, g_hInst, NULL);
//
//            hStaticCurrent = CreateWindow("STATIC", "Current: 0.0 A",
//                WS_CHILD | WS_VISIBLE | SS_CENTER,
//                250, 100, 150, 20, hwnd, NULL, g_hInst, NULL);
//
//            hStaticMaxVoltage = CreateWindow("STATIC", "Max Voltage: 0.0 V",
//                WS_CHILD | WS_VISIBLE | SS_CENTER,
//                250, 150, 150, 20, hwnd, NULL, g_hInst, NULL);
//
//            hStaticMaxCurrent = CreateWindow("STATIC", "Max Current: 0.0 A",
//                WS_CHILD | WS_VISIBLE | SS_CENTER,
//                250, 200, 150, 20, hwnd, NULL, g_hInst, NULL);
//
//            hEditVoltage = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
//                50, 350, 100, 20, hwnd, NULL, g_hInst, NULL);
//            hButtonSetVoltage = CreateWindow("BUTTON", "Set Voltage", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                160, 350, 100, 20, hwnd, (HMENU)2, g_hInst, NULL);
//
//            hEditCurrent = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
//                50, 380, 100, 20, hwnd, NULL, g_hInst, NULL);
//            hButtonSetCurrent = CreateWindow("BUTTON", "Set Current", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                160, 380, 100, 20, hwnd, (HMENU)3, g_hInst, NULL);
//
//            hEditRiseTime = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
//                50, 410, 100, 20, hwnd, NULL, g_hInst, NULL);
//            hButtonSetRiseTime = CreateWindow("BUTTON", "Set Rise Time", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                160, 410, 100, 20, hwnd, (HMENU)4, g_hInst, NULL);
//
//            hEditFallTime = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
//                50, 440, 100, 20, hwnd, NULL, g_hInst, NULL);
//            hButtonSetFallTime = CreateWindow("BUTTON", "Set Fall Time", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                160, 440, 100, 20, hwnd, (HMENU)5, g_hInst, NULL);
//
//            hEditMemoryBank = CreateWindow("COMBOBOX", "", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE,
//                50, 470, 100, 100, hwnd, NULL, g_hInst, NULL);
//            for (int i = 1; i <= 9; ++i) {
//                char buffer[10];
//                snprintf(buffer, sizeof(buffer), "%d", i);
//                SendMessage(hEditMemoryBank, CB_ADDSTRING, 0, (LPARAM)buffer);
//            }
//            hButtonSaveSettings = CreateWindow("BUTTON", "Save Settings", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//                160, 470, 100, 20, hwnd, (HMENU)6, g_hInst, NULL);
//
//            UpdateLED(0); // ����������� ��������� (�����)
//            break;
//
//        case WM_COMMAND:
//            if (LOWORD(wParam) == 1) { // ������ ������ Connect
//                char portName[10];
//                GetWindowText(hComboBoxPort, portName, 10);
//
//                // ��������, ��� �� ������ COM-����
//                if (strlen(portName) == 0) {
//                    MessageBox(hwnd, "Please select a COM port.", "Error", MB_OK | MB_ICONERROR);
//                    UpdateLED(2); // ������� (�������)
//                    return 0;
//                }
//
//                char baudRateStr[10];
//                GetWindowText(hComboBoxBaudRate, baudRateStr, 10);
//                int baudRate = atoi(baudRateStr);
//
//                char byteSizeStr[10];
//                GetWindowText(hComboBoxByteSize, byteSizeStr, 10);
//                int byteSize = atoi(byteSizeStr);
//
//                char parityStr[10];
//                GetWindowText(hComboBoxParity, parityStr, 10);
//                int parity;
//                if (strcmp(parityStr, "NONE") == 0) {
//                    parity = NOPARITY;
//                } else if (strcmp(parityStr, "ODD") == 0) {
//                    parity = ODDPARITY;
//                } else if (strcmp(parityStr, "EVEN") == 0) {
//                    parity = EVENPARITY;
//                } else if (strcmp(parityStr, "MARK") == 0) {
//                    parity = MARKPARITY;
//                } else if (strcmp(parityStr, "SPACE") == 0) {
//                    parity = SPACEPARITY;
//                } else {
//                    MessageBox(hwnd, "Invalid parity selected.", "Error", MB_OK | MB_ICONERROR);
//                    UpdateLED(2); // ������� (�������)
//                    return 0;
//                }
//
//                char stopBitsStr[10];
//                GetWindowText(hComboBoxStopBits, stopBitsStr, 10);
//                int stopBits = atoi(stopBitsStr);
//
//                if (OpenCOMPort(portName) && ConfigureCOMPort(baudRate, byteSize, parity, stopBits)) {
//                    UpdateLED(1); // ����� (�������)
//                } else {
//                    UpdateLED(2); // ������� (�������)
//                }
//            } else if (LOWORD(wParam) == 2) { // ������ ������ Set Voltage
//                SetVoltage();
//            } else if (LOWORD(wParam) == 3) { // ������ ������ Set Current
//                SetCurrent();
//            } else if (LOWORD(wParam) == 4) { // ������ ������ Set Rise Time
//                SetRiseTime();
//            } else if (LOWORD(wParam) == 5) { // ������ ������ Set Fall Time
//                SetFallTime();
//            } else if (LOWORD(wParam) == 6) { // ������ ������ Save Settings
//                SaveSettings();
//            }
//            break;
//
//        case WM_DESTROY:
//            PostQuitMessage(0);
//            break;
//
//        default:
//            return DefWindowProc(hwnd, msg, wParam, lParam);
//    }
//
//    return 0;
//}
//
//bool OpenCOMPort(const char* portName) {
//    hComPort = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
//    if (hComPort == INVALID_HANDLE_VALUE) {
//        std::cerr << "Failed to open COM port" << std::endl;
//        return false;
//    }
//    return true;
//}
//
//bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits) {
//    DCB dcb = {};
//    dcb.DCBlength = sizeof(DCB);
//
//    if (!GetCommState(hComPort, &dcb)) {
//        std::cerr << "Failed to get COM port state" << std::endl;
//        return false;
//    }
//
//    dcb.BaudRate = baudRate;
//    dcb.ByteSize = byteSize;
//    dcb.Parity = parity;
//    dcb.StopBits = stopBits;
//
//    if (!SetCommState(hComPort, &dcb)) {
//        std::cerr << "Failed to set COM port state" << std::endl;
//        return false;
//    }
//
//    return true;
//}
//
//bool WriteToCOMPort(const char* command) {
//    DWORD bytesWritten;
//    if (!WriteFile(hComPort, command, strlen(command), &bytesWritten, NULL)) {
//        std::cerr << "Failed to write to COM port" << std::endl;
//        return false;
//    }
//    return true;
//}
//
//bool ReadFromCOMPort(char* buffer, int bufferSize) {
//    DWORD bytesRead;
//    if (!ReadFile(hComPort, buffer, bufferSize - 1, &bytesRead, NULL)) {
//        std::cerr << "Failed to read from COM port" << std::endl;
//        return false;
//    }
//    buffer[bytesRead] = '\0';
//    return true;
//}
//
//bool GetVoltage(double& voltage) {
//    if (!WriteToCOMPort("MEAS:VOLT?\n")) {
//        return false;
//    }
//
//    char buffer[100];
//    if (!ReadFromCOMPort(buffer, sizeof(buffer))) {
//        return false;
//    }
//
//    voltage = atof(buffer);
//    RegisterMinMaxValues(voltage, 0.0);
//    UpdateDisplay();
//    return true;
//}
//
//bool GetCurrent(double& current) {
//    if (!WriteToCOMPort("MEAS:CURR?\n")) {
//        return false;
//    }
//
//    char buffer[100];
//    if (!ReadFromCOMPort(buffer, sizeof(buffer))) {
//        return false;
//    }
//
//    current = atof(buffer);
//    RegisterMinMaxValues(0.0, current);
//    UpdateDisplay();
//    return true;
//}
//
//void RegisterMinMaxValues(double voltage, double current) {
//    if (voltage > maxVoltage) {
//        maxVoltage = voltage;
//    }
//
//    if (current > maxCurrent) {
//        maxCurrent = current;
//    }
//
//    // �������� GUI � ������������� ����������
//    UpdateDisplay();
//}
//
//void UpdateDisplay() {
//    char buffer[50];
//    snprintf(buffer, sizeof(buffer), "Max Voltage: %.2f V", maxVoltage);
//    SetWindowText(hStaticMaxVoltage, buffer);
//    snprintf(buffer, sizeof(buffer), "Max Current: %.2f A", maxCurrent);
//    SetWindowText(hStaticMaxCurrent, buffer);
//}
//
//void PopulateCOMPorts() {
//    // ����� ��������� COM ������ � ���������� ComboBox
//    // ��������, �� ������ ������������ CreateFile ��� �������� ����������� ������
//    for (int i = 1; i <= 256; ++i) {
//        char portName[10];
//        snprintf(portName, sizeof(portName), "COM%d", i);
//
//        HANDLE hPort = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
//        if (hPort != INVALID_HANDLE_VALUE) {
//            CloseHandle(hPort);
//            SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)portName);
//        }
//    }
//}
//
//void PopulateBaudRates() {
//    // ���������� ������ ����������� �������� BaudRate
//    const int baudRates[] = {9600, 19200, 38400, 57600, 115200};
//    for (int rate : baudRates) {
//        char buffer[10];
//        snprintf(buffer, sizeof(buffer), "%d", rate);
//        SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)buffer);
//    }
//}
//
//void PopulateByteSizes() {
//    // ���������� ������ ����������� �������� ByteSize
//    const int byteSizes[] = {5, 6, 7, 8};
//    for (int size : byteSizes) {
//        char buffer[10];
//        snprintf(buffer, sizeof(buffer), "%d", size);
//        SendMessage(hComboBoxByteSize, CB_ADDSTRING, 0, (LPARAM)buffer);
//    }
//}
//
//void PopulateParities() {
//    // ���������� ������ ����������� �������� Parity
//    const char* parities[] = {"NONE", "ODD", "EVEN", "MARK", "SPACE"};
//    for (const char* parity : parities) {
//        SendMessage(hComboBoxParity, CB_ADDSTRING, 0, (LPARAM)parity);
//    }
//}
//
//void PopulateStopBits() {
//    // ���������� ������ ����������� �������� StopBits
//    const int stopBits[] = {1, 2};
//    for (int bits : stopBits) {
//        char buffer[10];
//        snprintf(buffer, sizeof(buffer), "%d", bits);
//        SendMessage(hComboBoxStopBits, CB_ADDSTRING, 0, (LPARAM)buffer);
//    }
//}
//
//void UpdateLED(int status) {
//    // ��������� ����� LED
//    HBRUSH hBrush;
//    switch (status) {
//        case 0: // ����������� ��������� (�����)
//            hBrush = CreateSolidBrush(RGB(192, 192, 192));
//            break;
//        case 1: // ����� (�������)
//            hBrush = CreateSolidBrush(RGB(0, 255, 0));
//            break;
//        case 2: // ������� (�������)
//            hBrush = CreateSolidBrush(RGB(255, 0, 0));
//            break;
//        default:
//            return;
//    }
//    HDC hdc = GetDC(hLED);
//    FillRect(hdc, &RECT{0, 0, 30, 30}, hBrush);
//    ReleaseDC(hLED, hdc);
//    DeleteObject(hBrush);
//}
//
//void SetVoltage() {
//    char buffer[20];
//    GetWindowText(hEditVoltage, buffer, sizeof(buffer));
//    double voltage = atof(buffer);
//    char command[50];
//    snprintf(command, sizeof(command), "VOLT %.2f\n", voltage);
//    if (WriteToCOMPort(command)) {
//        std::cout << "Voltage set to " << voltage << " V" << std::endl;
//    }
//}
//
//void SetCurrent() {
//    char buffer[20];
//    GetWindowText(hEditCurrent, buffer, sizeof(buffer));
//    double current = atof(buffer);
//    char command[50];
//    snprintf(command, sizeof(command), "CURR %.2f\n", current);
//    if (WriteToCOMPort(command)) {
//        std::cout << "Current set to " << current << " A" << std::endl;
//    }
//}
//
//void SetRiseTime() {
//    char buffer[20];
//    GetWindowText(hEditRiseTime, buffer, sizeof(buffer));
//    int riseTime = atoi(buffer);
//    char command[50];
//    snprintf(command, sizeof(command), "RISET %d\n", riseTime);
//    if (WriteToCOMPort(command)) {
//        std::cout << "Rise time set to " << riseTime << " ms" << std::endl;
//    }
//}
//
//void SetFallTime() {
//    char buffer[20];
//    GetWindowText(hEditFallTime, buffer, sizeof(buffer));
//    int fallTime = atoi(buffer);
//    char command[50];
//    snprintf(command, sizeof(command), "FALLT %d\n", fallTime);
//    if (WriteToCOMPort(command)) {
//        std::cout << "Fall time set to " << fallTime << " ms" << std::endl;
//    }
//}
//
//void SaveSettings() {
//    char buffer[10];
//    GetWindowText(hEditMemoryBank, buffer, sizeof(buffer));
//    int memoryBank = atoi(buffer);
//    char command[50];
//    snprintf(command, sizeof(command), "SAV %d\n", memoryBank);
//    if (WriteToCOMPort(command)) {
//        std::cout << "Settings saved to memory bank " << memoryBank << std::endl;
//    }
//}


