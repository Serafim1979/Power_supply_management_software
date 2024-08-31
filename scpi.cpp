#include "scpi.h"
#include "serial.h"

bool sendCommand(const std::string& command) {
    DWORD bytes_written;
    std::string cmd = command + "\n";

    if (!WriteFile(hComPort, cmd.c_str(), cmd.length(), &bytes_written, NULL))
    {
        throw std::runtime_error("Error writing to serial port");
        return false;
    }
    printf("%s", cmd.c_str());
    return true;
}

std::string query(const std::string& command) {
    sendCommand(command);
    char buffer[256];
    DWORD bytes_read;
    if (!ReadFile(hComPort, buffer, sizeof(buffer), &bytes_read, NULL)) {
            throw std::runtime_error("Error reading from serial port");
    }
    return std::string(buffer, bytes_read);
}

void checkError() {
    std::string error = query("SYST:ERR?");
    if (error.find("No error") == std::string::npos) {
        throw std::runtime_error("SCPI Error: " + error);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void StartPollingTimer(HWND hwnd, int ID_TIMER) {
    SetTimer(hwnd, ID_TIMER, 500, NULL);  // Устанавливаем таймер с интервалом 1 секунда
}

void StopPollingTimer(HWND hwnd, int ID_TIMER) {
    KillTimer(hwnd, ID_TIMER);
}


void UpdateVoltageDisplay(HWND hwnd, int ID_VOLTAGE_DISPLAY, const std::string& voltage) {
    std::string displayText = "Actual Voltage: " + voltage;// + "V";
    SetWindowText(GetDlgItem(hwnd, ID_VOLTAGE_DISPLAY), displayText.c_str());
}

void UpdateCurrentDisplay(HWND hwnd, int ID_CURRENT_DISPLAY, const std::string& current) {
    std::string displayText = "Actual Current: " + current; // + "A";
    SetWindowText(GetDlgItem(hwnd, ID_CURRENT_DISPLAY), displayText.c_str());
}

void UpdateMaxVoltageDisplay(HWND hwnd, int ID_MAX_VOLTAGE_DISPLAY, const std::string& voltage) {
    std::string displayText = "Max Voltage: " + voltage;// + "V";
    SetWindowText(GetDlgItem(hwnd, ID_MAX_VOLTAGE_DISPLAY), displayText.c_str());
}

void UpdateMaxCurrentDisplay(HWND hwnd, int ID_MAX_CURRENT_DISPLAY, const std::string& current) {
    std::string displayText = "Max Current: " + current; // + "A";
    SetWindowText(GetDlgItem(hwnd, ID_MAX_CURRENT_DISPLAY), displayText.c_str());
}

std::string removeNewLine(const std::string& str)
{
    std::string result = str;
    if(!result.empty() && result[result.length() - 1] == 'n')
    {
        result.erase(result.length() - 1);
    }
    return result;
}

std::string SendSCPICommandAndGetResponse(HANDLE hComPort, const std::string& command) {
    DWORD bytesWritten;
    WriteFile(hComPort, command.c_str(), command.length(), &bytesWritten, NULL);

    // Отправляем команду завершения строки (например, \n)
    const char terminator = '\n';
    WriteFile(hComPort, &terminator, 1, &bytesWritten, NULL);

    // Чтение ответа
    char buffer[256];
    DWORD bytesRead;
    ReadFile(hComPort, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    buffer[bytesRead] = '\0';  // Завершаем строку

    return std::string(buffer);
}

std::string reduceTrailingZeros(double value)
{
    std::string stringValue = std::to_string(value);
    size_t pos = stringValue.find_last_not_of('0');
    if(pos != std::string::npos && stringValue[pos] == '.')
    {
        pos--;
    }
    return stringValue.substr(0, pos + 1);
}

void SetLedColor(HWND hLed, COLORREF color) {
    HBRUSH hBrush = CreateSolidBrush(color);
    HDC hdc = GetDC(hLed);

    RECT rect;
    GetClientRect(hLed, &rect);

    FillRect(hdc, &rect, hBrush);

    ReleaseDC(hLed, hdc);
    DeleteObject(hBrush);
}





