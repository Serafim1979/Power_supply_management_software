#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

// Глобальные переменные
const char g_szClassName[] = "MainWindowClass";
HWND hComboBoxPort, hComboBoxBaudRate, hComboBoxByteSize, hComboBoxParity, hComboBoxStopBits, hButtonConnect, hLED;
HINSTANCE g_hInst;
HANDLE hComPort = INVALID_HANDLE_VALUE; // Дескриптор COM-порта

// Прототипы функций
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PopulateCOMPorts();
void PopulateBaudRates();
void PopulateByteSizes();
void PopulateParities();
void PopulateStopBits();
void UpdateLED(int status);

// Функции для работы с COM-портом
bool OpenCOMPort(const char* portName);
void CloseCOMPort();
bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits);
bool WriteToCOMPort(const char* command);
bool ReadFromCOMPort(char* buffer, int bufferSize);

// Функции для управления источником питания
bool SetVoltage(double voltage);
bool SetCurrent(double current);
bool SetRiseTime(double riseTime);
bool SetFallTime(double fallTime);
bool SetOnDelay(double onDelay);
bool SetOffDelay(double offDelay);
bool EnableCurrentProtection(bool enable);
bool EnableVoltageProtection(bool enable);
bool SaveSettings(int memoryBank);

// Функции для чтения и регистрации значений
bool GetVoltage(double& voltage);
bool GetCurrent(double& current);
void RegisterMinMaxValues(double voltage, double current);

// Функции для графического интерфейса
void CreateMainWindow();
void UpdateDisplay();
void VisualizeCurrent();
void UpdateLED(int ledID, int status);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // Инициализация структуры WNDCLASSEX
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

    // Регистрация оконного класса
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Создание главного окна
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

    // Цикл сообщений
    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            // Создание ComboBox для выбора COM-порта
            hComboBoxPort = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 50, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateCOMPorts();

            // Создание ComboBox для выбора скорости передачи данных
            hComboBoxBaudRate = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 100, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateBaudRates();

            // Создание ComboBox для выбора размера байта
            hComboBoxByteSize = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 150, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateByteSizes();

            // Создание ComboBox для выбора проверки четности
            hComboBoxParity = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 200, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateParities();

            // Создание ComboBox для выбора стоп-битов
            hComboBoxStopBits = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                50, 250, 150, 200, hwnd, NULL, g_hInst, NULL);
            PopulateStopBits();

            // Создание кнопки Connect
            hButtonConnect = CreateWindow("BUTTON", "Connect",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 300, 100, 30, hwnd, (HMENU)1, g_hInst, NULL);

            // Создание светодиода для индикации состояния
            hLED = CreateWindow("STATIC", NULL,
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                160, 300, 20, 20, hwnd, NULL, g_hInst, NULL);
            UpdateLED(0); // Нейтральное состояние (серый)
            break;

        case WM_COMMAND:
            if(LOWORD(wParam) == 1) { // Нажата кнопка Connect
                char portName[10];
                GetWindowText(hComboBoxPort, portName, 10);

                // Проверка, был ли выбран COM-порт
                if (strlen(portName) == 0) {
                    MessageBox(hwnd, "Please select a COM port.", "Error", MB_OK | MB_ICONERROR);
                    UpdateLED(2); // Неудача (красный)
                    return 0;
                }

                char baudRateStr[10];
                GetWindowText(hComboBoxBaudRate, baudRateStr, 10);
                int baudRate = atoi(baudRateStr);

                char byteSizeStr[10];
                GetWindowText(hComboBoxByteSize, byteSizeStr, 10);
                int byteSize = atoi(byteSizeStr);

                char parityStr[10];
                GetWindowText(hComboBoxParity, parityStr, 10);
                int parity = atoi(parityStr);

                char stopBitsStr[10];
                GetWindowText(hComboBoxStopBits, stopBitsStr, 10);
                int stopBits = atoi(stopBitsStr);

                // Попробовать открыть и настроить COM-порт
                bool portOpen = OpenCOMPort(portName);
                bool portConfigured = ConfigureCOMPort(baudRate, byteSize, parity, stopBits);

                // Если порт успешно открыт и настроен, диод будет зеленым, иначе красным
                if (portOpen && portConfigured) {
                    UpdateLED(1); // Успех (зеленый)
                } else {
                    UpdateLED(2); // Неудача (красный)
                }
            }
            break;

        case WM_CLOSE:
            // Закрыть COM-порт при завершении работы
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



// Функции для работы с COM-портом (заглушки)
bool OpenCOMPort(const char* portName) {
    // Закрыть старое соединение, если оно есть
    if (hComPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hComPort);
    }

    // Открыть новый COM-порт
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
    SendMessage(hComboBoxByteSize, CB_SETCURSEL, 3, 0); // Устанавливаем 8 бит как значение по умолчанию
}

void PopulateParities() {
    std::vector<std::string> parities = {"None", "Odd", "Even", "Mark", "Space"};
    for (const std::string& parity : parities) {
        SendMessage(hComboBoxParity, CB_ADDSTRING, 0, (LPARAM)parity.c_str());
    }
    SendMessage(hComboBoxParity, CB_SETCURSEL, 0, 0); // Устанавливаем None как значение по умолчанию
}
void PopulateStopBits() {
    std::vector<std::string> stopBits = {"1", "1.5", "2"};
    for (const std::string& bits : stopBits) {
        SendMessage(hComboBoxStopBits, CB_ADDSTRING, 0, (LPARAM)bits.c_str());
    }
    SendMessage(hComboBoxStopBits, CB_SETCURSEL, 0, 0); // Устанавливаем 1 как значение по умолчанию
}

bool ConfigureCOMPort(int baudRate, int byteSize, int parity, int stopBits) {
    DCB dcbSerialParams = {0};

    // Получить текущие параметры порта
    if (!GetCommState(hComPort, &dcbSerialParams)) {
        std::cerr << "Failed to get COM port state: " << GetLastError() << std::endl;
        return false;
    }

    // Настроить параметры порта
    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = byteSize;
    dcbSerialParams.Parity = parity;
    dcbSerialParams.StopBits = stopBits;

    // Установить параметры порта
    if (!SetCommState(hComPort, &dcbSerialParams)) {
        std::cerr << "Failed to set COM port state: " << GetLastError() << std::endl;
        return false;
    }

    // Настроить таймауты (по умолчанию)
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
    std::cout << "WriteToCOMPort" << std::endl;
    return true;
}

bool ReadFromCOMPort(char* buffer, int bufferSize) {
    std::cout << "ReadFromCOMPort" << std::endl;
    return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции для управления источником питания
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

// Функции для чтения и регистрации значений
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

// Функции для графического интерфейса
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
        case 0: // Нейтральное состояние (серый)
            hBrush = CreateSolidBrush(RGB(128, 128, 128));
            break;
        case 1: // Успех (зеленый)
            hBrush = CreateSolidBrush(RGB(0, 255, 0));
            break;
        case 2: // Неудача (красный)
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

// Функции для заполнения ComboBox

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
//    HKEY hKey;
//    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
//        char valueName[256];
//        BYTE valueData[256];
//        DWORD valueNameSize, valueDataSize, type, index = 0;
//
//        while (true) {
//            valueNameSize = sizeof(valueName);
//            valueDataSize = sizeof(valueData);
//            if (RegEnumValue(hKey, index, valueName, &valueNameSize, NULL, &type, valueData, &valueDataSize) != ERROR_SUCCESS) {
//                break;
//            }
//            if (type == REG_SZ) {
//                SendMessage(hComboBoxPort, CB_ADDSTRING, 0, (LPARAM)valueData);
//            }
//            index++;
//        }
//        RegCloseKey(hKey);
//    }
}

void PopulateBaudRates() {
     // Заполнение списка скоростей передачи данных
    std::vector<std::string> baudRates = {"9600", "19200", "38400", "57600", "115200"};
    for (const std::string& rate : baudRates) {
        SendMessage(hComboBoxBaudRate, CB_ADDSTRING, 0, (LPARAM)rate.c_str());
    }

    // Установка значения по умолчанию
    SendMessage(hComboBoxBaudRate, CB_SETCURSEL, 0, 0);
}

