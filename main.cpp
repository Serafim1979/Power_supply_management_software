#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

// ���������� ����������
const char g_szClassName[] = "MainWindowClass";
HWND hComboBoxPort, hComboBoxBaudRate, hButtonConnect, hLED;
HINSTANCE g_hInst;

// ��������� �������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PopulateCOMPorts();
void PopulateBaudRates();
void UpdateLED(int status);

// ������� ��� ������ � COM-������
bool OpenCOMPort(const char* portName);
void CloseCOMPort();
bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits);
bool WriteToCOMPort(const char* command);
bool ReadFromCOMPort(char* buffer, int bufferSize);

// ������� ��� ���������� ���������� �������
bool SetVoltage(double voltage);
bool SetCurrent(double current);
bool SetRiseTime(double riseTime);
bool SetFallTime(double fallTime);
bool SetOnDelay(double onDelay);
bool SetOffDelay(double offDelay);
bool EnableCurrentProtection(bool enable);
bool EnableVoltageProtection(bool enable);
bool SaveSettings(int memoryBank);

// ������� ��� ������ � ����������� ��������
bool GetVoltage(double& voltage);
bool GetCurrent(double& current);
void RegisterMinMaxValues(double voltage, double current);

// ������� ��� ������������ ����������
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
        "Control Panel",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
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

            // �������� ������ Connect
            hButtonConnect = CreateWindow("BUTTON", "Connect",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 150, 100, 30, hwnd, (HMENU)1, g_hInst, NULL);

            // �������� ���������� ��� ��������� ���������
            hLED = CreateWindow("STATIC", NULL,
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                160, 150, 20, 20, hwnd, NULL, g_hInst, NULL);
            UpdateLED(0); // ����������� ��������� (�����)
            break;

        case WM_COMMAND:
            if(LOWORD(wParam) == 1) { // ������ ������ Connect
                char portName[10];
                GetWindowText(hComboBoxPort, portName, 10);

                char baudRateStr[10];
                GetWindowText(hComboBoxBaudRate, baudRateStr, 10);
                int baudRate = atoi(baudRateStr);

                if(OpenCOMPort(portName) && ConfigureCOMPort(baudRate, 8, NOPARITY, ONESTOPBIT)) {
                    UpdateLED(1); // ����� (�������)
                } else {
                    UpdateLED(2); // ������� (�������)
                }
            }
            break;

        case WM_CLOSE:
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


// ������� ��� ������ � COM-������
bool OpenCOMPort(const char* portName) {
    std::cout << "OpenCOMPort" << std::endl;
    return true;
}

void CloseCOMPort() {
    std::cout << "CloseCOMPort" << std::endl;
}

bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits) {
    std::cout << "ConfigureCOMPort" << std::endl;
    return true;
}

bool WriteToCOMPort(const char* command) {
    std::cout << "WriteToCOMPort" << std::endl;
    return true;
}

bool ReadFromCOMPort(char* buffer, int bufferSize) {
    std::cout << "ReadFromCOMPort" << std::endl;
    return true;
}

// ������� ��� ���������� ���������� �������
bool SetVoltage(double voltage) {
    std::cout << "SetVoltage" << std::endl;
    return true;
}

bool SetCurrent(double current) {
    std::cout << "SetCurrent" << std::endl;
    return true;
}

bool SetRiseTime(double riseTime) {
    std::cout << "SetRiseTime" << std::endl;
    return true;
}

bool SetFallTime(double fallTime) {
    std::cout << "SetFallTime" << std::endl;
    return true;
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

bool SaveSettings(int memoryBank) {
    std::cout << "SaveSettings" << std::endl;
    return true;
}

// ������� ��� ������ � ����������� ��������
bool GetVoltage(double& voltage) {
    std::cout << "GetVoltage" << std::endl;
    return true;
}

bool GetCurrent(double& current) {
    std::cout << "GetCurrent" << std::endl;
    return true;
}

void RegisterMinMaxValues(double voltage, double current) {
    std::cout << "RegisterMinMaxValues" << std::endl;
}

// ������� ��� ������������ ����������
void CreateMainWindow() {
    std::cout << "CreateMainWindow" << std::endl;
}

void UpdateDisplay() {
    std::cout << "UpdateDisplay" << std::endl;
}

void VisualizeCurrent() {
    std::cout << "VisualizeCurrent" << std::endl;
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

void PopulateCOMPorts() {
    // �������� ��� ���������� ������ COM-������
    SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)"COM1");
    SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)"COM2");
    SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)"COM3");
    SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)"COM4");
}

void PopulateBaudRates() {
    // ���������� ������ ��������� �������� ������
    SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)"9600");
    SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)"19200");
    SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)"38400");
    SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)"57600");
    SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)"115200");

    // ��������� �������� �� ���������
    SendMessage(hComboBoxBaudRate, CB_SETCURSEL, 0, 0);
}

