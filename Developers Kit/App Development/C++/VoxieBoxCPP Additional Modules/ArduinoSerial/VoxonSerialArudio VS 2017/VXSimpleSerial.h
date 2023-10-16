#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>
#include <time.h>
#include <fstream>
#include <vxCpp.h>

using namespace std;

class VXSimpleSerial 
{

private:
	HANDLE io_handler_;
	COMSTAT status_;
	DWORD errors_;

	string syntax_name_;
	char front_delimiter_;
	char end_delimiter_;

	void CustomSyntax(string syntax_type);	
	int posX = 50;
	int posY = 100;
	 
public:
	VXSimpleSerial(char* com_port, DWORD COM_BAUD_RATE );
	VXSimpleSerial(char* com_port, DWORD COM_BAUD_RATE, int DebugPosX, int DebugPosY);
	void TryNewPort(char* com_port, DWORD COM_BAUD_RATE);
	string ReadSerialPort(int reply_wait_time, string syntax_type);	
	bool WriteSerialPort(char *data_sent);
	bool CloseSerialPort();
	~VXSimpleSerial();
	bool CheckConnection();
	bool CheckhandShake();
	void setHandShake(bool option);
	bool connected_ = false;
	bool handshakeState = false;
	char* connect_com_port = NULL;
	DWORD connected_baud_rate;
};

