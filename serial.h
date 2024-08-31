#ifndef SERIAL_H
#define SERIAL_H

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

extern HANDLE hComPort;

void PopulateCOMPorts(HWND hComboBoxPort);
void PopulateByteSizes(HWND hComboBoxByteSize);
void PopulateParities(HWND hComboBoxParity);
void PopulateStopBits(HWND hComboBoxStopBits);
void PopulateBaudRates(HWND hComboBoxBaudRate);
bool ConfigureCOMPort(HWND hComboBoxPort, HWND hComboBoxBaudRate, HWND hComboBoxByteSize, HWND hComboBoxParity, HWND hComboBoxStopBits);
bool OpenCOMPort(const char* portName);

#endif // SERIAL_H


