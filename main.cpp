/*****************************************************************************************************************
 * Power Supply Control Panel
 *
 * This program provides a graphical user interface (GUI) for controlling a power supply device
 * via a serial (COM) port using the SCPI (Standard Commands for Programmable Instruments) protocol.
 *
 * Main features:
 * - Connect to a power supply device through a user-selected COM port.
 * - Set and control the output voltage and current of the power supply.
 * - Adjust the rise and fall times for voltage and current transitions.
 * - Enable or disable the output of the power supply.
 * - Monitor real-time measurements of voltage and current.
 * - Display maximum recorded values for voltage and current during operation.
 * - Simple status indicators (LED simulation) to show the connection status of the device.
 * - Timer-based polling to update the measurements periodically.
 *
 * The program is structured with the following key components:
 * - WinMain: The main entry point of the program where the window is created.
 * - WindowProc: The main window procedure handling messages, including commands from the user interface.
 * - CreatePowerSupplyControlPanel: A function that dynamically creates the controls for interacting with the power supply.
 * - Communication functions (e.g., OpenCOMPort, SendSCPICommandAndGetResponse): These handle the communication with the power supply device over the serial port.
 *
 * This code is intended to be a starting point for applications requiring serial communication with power supplies or similar devices.
 * It is also a demonstration of basic WinAPI usage for creating a simple GUI in C++.
 *
 * Author: Korolkov Ivan
 * Date: 31.08.2024
 ***************************************************************************************************************/

#include <windows.h>
#include "scpi.h"
#include "serial.h"

// Global variable for Delay
static int global_delay = 0;

static double maxVoltage = 0.0;
static double maxCurrent = 0.0;

// Global Control ids
#define BASE_ID 1000

// Button IDs for the power supply
#define ID_CONNECT_BUTTON (BASE_ID + 100)
#define ID_SET_VOLTAGE_BUTTON (BASE_ID + 101)
#define ID_SET_CURRENT_BUTTON (BASE_ID + 102)
#define ID_SET_RISE_BUTTON (BASE_ID + 103)
#define ID_SET_FALL_BUTTON (BASE_ID + 104)
#define ID_SET_DELAY_BUTTON (BASE_ID + 105)
#define ID_OUTPUT_ON_BUTTON (BASE_ID + 106)
#define ID_OUTPUT_OFF_BUTTON (BASE_ID + 107)
#define ID_SYST_REM_BUTTON (BASE_ID + 108)

// Ids of input fields for the power supply
#define ID_VOLTAGE_EDIT (BASE_ID + 200)
#define ID_CURRENT_EDIT (BASE_ID + 201)
#define ID_RISE_EDIT (BASE_ID + 202)
#define ID_FALL_EDIT (BASE_ID + 203)
#define ID_DELAY_EDIT (BASE_ID + 204)

// Static element identifiers for the power supply
#define ID_VOLTAGE_DISPLAY (BASE_ID + 300)
#define ID_CURRENT_DISPLAY (BASE_ID + 301)
#define ID_MAX_VOLTAGE_DISPLAY (BASE_ID + 302)
#define ID_MAX_CURRENT_DISPLAY (BASE_ID + 303)
#define ID_CONNECT_LED (BASE_ID + 304)
#define ID_TEXT_OUTPUT (BASE_ID + 305)

#define ID_COM_PORT_GROUP (BASE_ID + 306)

#define ID_COMBO_BOX_PORT (BASE_ID + 307)
#define ID_COMBO_BOX_BAUD_RATE (BASE_ID + 308)
#define ID_COMBO_BOX_BYTE_SIZE (BASE_ID + 309)  // ComboBoxByteSize
#define ID_COMBO_BOX_PARITY (BASE_ID + 310)  // ComboBoxParity
#define ID_COMBO_BOX_STOP_BITS (BASE_ID + 311)  // ComboBoxStopBits

#define ID_SETTINGS_GROUP (BASE_ID + 312)

#define IDT_TIMER1 1

// Structure for storing connection parameters and settings
struct PowerSupplyConfig {
    std::string name;
    std::string comPort;
    std::string baudRate;
    std::string byteSize;
    std::string parity;
    std::string stopBits;
    std::string voltage;
    std::string current;
    std::string rise;
    std::string fall;
    std::string delay;
};

// Global variables for storing configurations
PowerSupplyConfig powerSupplies;

// Function for creating a power supply control panel
void CreatePowerSupplyControlPanel(HWND hwnd, const PowerSupplyConfig& config, int offsetX);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "PowerSupplyWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Power Supply Control Panel",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 435, 700,
        NULL, NULL, hInstance, NULL
    );

    if(hwnd == NULL)
    {
        return 0;
    }

    CreatePowerSupplyControlPanel(hwnd, powerSupplies, 0);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_TIMER:
        {
            if(wParam == IDT_TIMER1)
            {
                if(hComPort != INVALID_HANDLE_VALUE)
                {
                    std::string voltage = SendSCPICommandAndGetResponse(hComPort, "MEAS:VOLT?");
                    UpdateVoltageDisplay(hWnd, ID_VOLTAGE_DISPLAY, voltage);

                    std::string current = SendSCPICommandAndGetResponse(hComPort, "MEAS:CURR?");
                    UpdateCurrentDisplay(hWnd, ID_CURRENT_DISPLAY, current);

                    if(std::stod(voltage) > maxVoltage)
                    {
                        maxVoltage = std::stod(voltage);
                    }
                    if(std::stod(current) > maxCurrent)
                    {
                        maxCurrent = std::stod(current);
                    }

                    UpdateMaxVoltageDisplay(hWnd, ID_MAX_VOLTAGE_DISPLAY, reduceTrailingZeros(maxVoltage));
                    UpdateMaxCurrentDisplay(hWnd, ID_MAX_CURRENT_DISPLAY, reduceTrailingZeros(maxCurrent));
                }
            }
        }
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            if(wmId == ID_CONNECT_BUTTON)
            {
                StartPollingTimer(hWnd, IDT_TIMER1);
                HWND hComboBoxPortLocal = GetDlgItem(hWnd, ID_COMBO_BOX_PORT);
                char selectedPort[256];
                GetWindowText(hComboBoxPortLocal, selectedPort, sizeof(selectedPort));
                if(strlen(selectedPort) == 0)
                {
                    MessageBox(hWnd, "Please select a COM port.", "Error", MB_OK | MB_ICONERROR);
                    return 0;
                }

                HWND hComboBoxBaudRateLocal = GetDlgItem(hWnd, ID_COMBO_BOX_BAUD_RATE);
                HWND hComboBoxByteSizeLocal = GetDlgItem(hWnd, ID_COMBO_BOX_BYTE_SIZE);
                HWND hComboBoxParityLocal = GetDlgItem(hWnd, ID_COMBO_BOX_PARITY);
                HWND hComboBoxStopBitsLocal = GetDlgItem(hWnd, ID_COMBO_BOX_STOP_BITS);

                HWND hConnectLedLocal = GetDlgItem(hWnd, ID_CONNECT_LED);

                if (OpenCOMPort(selectedPort) && ConfigureCOMPort(hComboBoxPortLocal, hComboBoxBaudRateLocal, hComboBoxByteSizeLocal, hComboBoxParityLocal, hComboBoxStopBitsLocal))
                {
                    // Successful opening of the COM port
                    // getting information about the source
                    std::string str = query("*IDN?");
                    std::string id_supply_power;
                    if(!str.empty())
                    {
                        id_supply_power = str.substr(12, 26);
                    }

                    HWND hTextOutputLocal = GetDlgItem(hWnd, ID_TEXT_OUTPUT);

                    SetWindowText(hTextOutputLocal, id_supply_power.c_str());

                    // Successful connection, set the green color of the diode
                    SetLedColor(hConnectLedLocal, RGB(0, 255, 0));  // Green
                }
                else
                {
                    // Failed to connect, set the red color of the diode
                    SetLedColor(hConnectLedLocal, RGB(255, 0, 0));  // Red
                    MessageBox(hWnd, "Failed to open COM port.", "Error", MB_OK | MB_ICONERROR);
                }
            }

            if(wmId == ID_SYST_REM_BUTTON)
            {
                sendCommand("SYST:REM");
            }
            else if(wmId == ID_OUTPUT_ON_BUTTON)
            {
                if(global_delay > 0)
                {
                    Sleep(global_delay);
                }
                sendCommand("OUTP ON");
            }
            else if(wmId == ID_OUTPUT_OFF_BUTTON)
            {
                sendCommand("OUTP OFF");
            }
            else if(wmId == ID_SET_VOLTAGE_BUTTON)
            {
                char buffer[256];
                GetWindowText(GetDlgItem(hWnd, ID_VOLTAGE_EDIT), buffer, sizeof(buffer));
                powerSupplies.voltage = buffer;
                std::string cmd = "VOLT " + powerSupplies.voltage;
                sendCommand(cmd);
            }
            else if(wmId == ID_SET_CURRENT_BUTTON)
            {
                char buffer[256];
                GetWindowText(GetDlgItem(hWnd, ID_CURRENT_EDIT), buffer, sizeof(buffer));
                powerSupplies.current = buffer;
                std::string cmd = "CURR " + powerSupplies.current;
                sendCommand(cmd);
            }
            else if(wmId == ID_SET_RISE_BUTTON)
            {
                char buffer[256];
                GetWindowText(GetDlgItem(hWnd, ID_RISE_EDIT), buffer, sizeof(buffer));
                powerSupplies.rise = buffer;
                std::string cmd = "RISE " + powerSupplies.rise;
                sendCommand(cmd);
            }
            else if(wmId == ID_SET_FALL_BUTTON)
            {
                char buffer[256];
                GetWindowText(GetDlgItem(hWnd, ID_FALL_EDIT), buffer, sizeof(buffer));
                powerSupplies.fall = buffer;
                std::string cmd = "FALL " + powerSupplies.fall;
                sendCommand(cmd);
            }
            else if(wmId == ID_SET_DELAY_BUTTON)
            {
                char buffer[256];
                GetWindowText(GetDlgItem(hWnd, ID_DELAY_EDIT), buffer, sizeof(buffer));
                powerSupplies.delay = buffer;
                global_delay = std::stoi(powerSupplies.delay);
            }
        }
        break;


    case WM_DESTROY:
        {
            StopPollingTimer(hWnd, IDT_TIMER1);
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Function for creating a power supply control panel
void CreatePowerSupplyControlPanel(HWND hwnd, const PowerSupplyConfig& config, int offsetX) {
    // Panel Options
    const int margin = 10;
    const int groupBoxHeight = 600;
    const int groupBoxWidth = 400;

    // Creating a text field for query output
    CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_CENTER,
                 margin + offsetX, margin, 250, 20, hwnd, (HMENU)ID_TEXT_OUTPUT, NULL, NULL);

    // Creating a GroupBox for the COM port
    HWND hComPortGroup = CreateWindow("BUTTON", "COM Port Settings", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                      margin + offsetX, 40, groupBoxWidth, groupBoxHeight / 2, hwnd, (HMENU)ID_COM_PORT_GROUP, NULL, NULL);

    // Creating a combo box for selecting a COM port
    HWND hComboBoxPort = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
                                  margin + offsetX + 10, 75, 100, 100, hwnd, (HMENU)ID_COMBO_BOX_PORT, NULL, NULL);
    PopulateCOMPorts(hComboBoxPort);

    // Creating a ComboBox to select the data transfer rate
    HWND hComboBoxBaudRate = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                margin + offsetX + 10, 120, 100, 200, hwnd, (HMENU)ID_COMBO_BOX_BAUD_RATE, NULL, NULL);
    PopulateBaudRates(hComboBoxBaudRate);

    // Creating a ComboBox to select the byte size
    HWND hComboBoxByteSize = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                margin + offsetX + 10, 165, 100, 200, hwnd, (HMENU)ID_COMBO_BOX_BYTE_SIZE, NULL, NULL);
    PopulateByteSizes(hComboBoxByteSize);

    // Creating a ComboBox to select a parity check
    HWND hComboBoxParity = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                margin + offsetX + 10, 210, 100, 200, hwnd, (HMENU)ID_COMBO_BOX_PARITY, NULL, NULL);
    PopulateParities(hComboBoxParity);

    // Creating a ComboBox for selecting stop bits
    HWND hComboBoxStopBits = CreateWindow("COMBOBOX", NULL,
                CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                margin + offsetX + 10, 255, 100, 200, hwnd, (HMENU)ID_COMBO_BOX_STOP_BITS, NULL, NULL);
    PopulateStopBits(hComboBoxStopBits);

    // Creating a Connect button and a diode simulator
    CreateWindow("BUTTON", "Connect", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 10, 300, 100, 30, hwnd, (HMENU)ID_CONNECT_BUTTON, NULL, NULL);

    HWND hConnectLed = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_CENTER,
                                     margin + offsetX + 120, 300, 30, 30, hwnd, (HMENU)ID_CONNECT_LED, NULL, NULL);

    // Set the initial color of the diode (gray)
    HBRUSH hGrayBrush = CreateSolidBrush(RGB(128, 128, 128));
    RECT ledRect = { 0, 0, 30, 30 };  // Creating a RECT object
    HDC hdc = GetDC(hConnectLed);
    FillRect(hdc, &ledRect, hGrayBrush); // Transmitting the address of the RECT object
    ReleaseDC(hConnectLed, hdc);
    DeleteObject(hGrayBrush);


    // Creating a GroupBox for power supply settings
    HWND hSettingsGroup = CreateWindow("BUTTON", "Power Supply Settings", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                                       margin + offsetX, 360, groupBoxWidth, groupBoxHeight / 2, hwnd, (HMENU)ID_SETTINGS_GROUP, NULL, NULL);

    // Creating text fields and buttons for setting values
    CreateWindow("STATIC", "Voltage:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                 margin + offsetX + 10, 400, 60, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", config.voltage.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER,
                 margin + offsetX + 70, 400, 50, 20, hwnd, (HMENU)ID_VOLTAGE_EDIT, NULL, NULL);
    CreateWindow("BUTTON", "Set Voltage", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 120, 400, 80, 20, hwnd, (HMENU)ID_SET_VOLTAGE_BUTTON, NULL, NULL);

    CreateWindow("STATIC", "Current:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                 margin + offsetX + 10, 430, 60, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", config.current.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER,
                 margin + offsetX + 70, 430, 50, 20, hwnd, (HMENU)ID_CURRENT_EDIT, NULL, NULL);
    CreateWindow("BUTTON", "Set Current", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 120, 430, 80, 20, hwnd, (HMENU)ID_SET_CURRENT_BUTTON, NULL, NULL);

    // Field for entering the Rise value
    CreateWindow("STATIC", "Rise:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                 margin + offsetX + 10, 460, 60, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", config.rise.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER,
                 margin + offsetX + 70, 460, 50, 20, hwnd, (HMENU)ID_RISE_EDIT, NULL, NULL);
    CreateWindow("BUTTON", "Set Rise", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 120, 460, 80, 20, hwnd, (HMENU)ID_SET_RISE_BUTTON, NULL, NULL);

    // Field for entering the Fall value
    CreateWindow("STATIC", "Fall:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                 margin + offsetX + 10, 490, 60, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", config.fall.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER,
                 margin + offsetX + 70, 490, 50, 20, hwnd, (HMENU)ID_FALL_EDIT, NULL, NULL);
    CreateWindow("BUTTON", "Set Fall", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 120, 490, 80, 20, hwnd, (HMENU)ID_SET_FALL_BUTTON, NULL, NULL);

    // Field for entering the Delay value
    CreateWindow("STATIC", "Delay:", WS_VISIBLE | WS_CHILD | SS_LEFT,
                 margin + offsetX + 10, 520, 60, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", config.delay.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER,
                 margin + offsetX + 70, 520, 50, 20, hwnd, (HMENU)ID_DELAY_EDIT, NULL, NULL);
    CreateWindow("BUTTON", "Set Delay", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 120, 520, 80, 20, hwnd, (HMENU)ID_SET_DELAY_BUTTON, NULL, NULL);


    // Creating fields to display current values
    CreateWindow("STATIC", "Actual Voltage: 0.0 V", WS_VISIBLE | WS_CHILD | SS_CENTER,
                 margin + offsetX + 230, 400, 150, 20, hwnd, (HMENU)ID_VOLTAGE_DISPLAY, NULL, NULL);
    CreateWindow("STATIC", "Actual Current: 0.0 A", WS_VISIBLE | WS_CHILD | SS_CENTER,
                 margin + offsetX + 230, 430, 150, 20, hwnd, (HMENU)ID_CURRENT_DISPLAY, NULL, NULL);

    CreateWindow("STATIC", "Max Voltage: 0.0 V", WS_CHILD | WS_VISIBLE | SS_CENTER,
                margin + offsetX + 230, 470, 150, 20, hwnd, (HMENU)ID_MAX_VOLTAGE_DISPLAY, NULL, NULL);

    CreateWindow("STATIC", "Max Current: 0.0 A", WS_CHILD | WS_VISIBLE | SS_CENTER,
                margin + offsetX + 230, 510, 150, 20, hwnd, (HMENU)ID_MAX_CURRENT_DISPLAY, NULL, NULL);


    CreateWindow("BUTTON", "OUTP ON", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 10, 560, 100, 30, hwnd, (HMENU)ID_OUTPUT_ON_BUTTON, NULL, NULL);
    CreateWindow("BUTTON", "OUTP OFF", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 145, 560, 100, 30, hwnd, (HMENU)ID_OUTPUT_OFF_BUTTON, NULL, NULL);
    CreateWindow("BUTTON", "SYST:REM", WS_VISIBLE | WS_CHILD,
                 margin + offsetX + 280, 560, 100, 30, hwnd, (HMENU)ID_SYST_REM_BUTTON, NULL, NULL);
}

