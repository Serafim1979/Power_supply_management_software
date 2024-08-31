#ifndef SCPI_H
#define SCPI_H

#include <windows.h>
#include <string>

//Functions for controlling the power supply
bool sendCommand(const std::string& command);

std::string query(const std::string& command);

void checkError();

void SetVoltage();
void SetCurrent();
void SetRiseTime();
void SetFallTime();

void SetLedColor(HWND hLed, COLORREF color);

void StartPollingTimer(HWND hwnd, int ID_TIMER);
void StopPollingTimer(HWND hwnd, int ID_TIMER);
void UpdateVoltageDisplay(HWND hwnd, int ID_VOLTAGE_DISPLAY, const std::string& voltage);
void UpdateCurrentDisplay(HWND hwnd, int ID_CURRENT_DISPLAY, const std::string& current);

std::string removeNewLine(const std::string& str);
std::string SendSCPICommandAndGetResponse(HANDLE hComPort, const std::string& command);

void UpdateMaxVoltageDisplay(HWND hwnd, int ID_MAX_VOLTAGE_DISPLAY, const std::string& voltage);
void UpdateMaxCurrentDisplay(HWND hwnd, int ID_MAX_CURRENT_DISPLAY, const std::string& current);

void RegisterMinMaxValues(double voltage, double current);
std::string reduceTrailingZeros(double value);

#endif // SCPI_H
