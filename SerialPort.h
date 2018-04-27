
#ifndef SERIALPORT_H
#define SERIALPORT_H

#define _SCL_SECURE_NO_WARNINGS  

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 255

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

class SerialPort
{
private:
	HANDLE handler;
	bool connected;
	COMSTAT status;
	DWORD errors;
public:
	SerialPort(char *portName);
	~SerialPort();

	int readSerialPort(char *buffer, unsigned int buf_size);
	bool writeSerialPort(char *buffer, unsigned int buf_size);
	void sendStringToSerial(string input_string);
	char* readStringFromSerial();
	bool isConnected();
};

#endif // SERIALPORT_H