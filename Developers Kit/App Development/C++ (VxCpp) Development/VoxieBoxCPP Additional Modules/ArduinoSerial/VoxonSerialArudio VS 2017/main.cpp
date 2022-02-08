
#include <chrono>
#include <thread>
#include <vector>
#include <math.h>
#include "vxCPP.h"
#include "VXSimpleSerial.h"
#include "main.h"
// uses the 'Simple Serial' library by 2018 David Michalik





//connection varaibles ..
char com_port[] = "\\\\.\\COM18";
char* to_send = nullptr;
bool is_sent = false;
bool closed_ = false;
int reply_wait_time = 4;
string syntax_type = "greater_less_than";
string incoming = "not ready";
string read_in;

DWORD COM_BAUD_RATE = CBR_9600;
char printBuff[50] = " ";
char printDock[19] = " ";
bool ticketMachineRun = true;



void newConnection(VXSimpleSerial * Serial) {

	const int NO_OF_ATTEMPTS = 21;
	bool AUTO_CONNECTION = true;
	char* handshake = (char*)"Handshake";
	int tens = 0;
	int attemptNo = 0;
	Serial->setHandShake(false);


	//  try to make a connection
	while (attemptNo < NO_OF_ATTEMPTS && ticketMachineRun) {

		printf("trying %s \n", com_port);
		//Serial->setHandShake(false);
		to_send = &handshake[0];
		is_sent = Serial->WriteSerialPort(to_send);
		if (is_sent) {
			printf("Handshake Sent... \n");

			incoming = Serial->ReadSerialPort(3, syntax_type);

			if (incoming == "Handshake") {
				printf("Handshake successful on %s \n", com_port);
				Serial->setHandShake(true);
				AUTO_CONNECTION = false;
				break;
			}
			else {

				printf("No response :(.. \n");
			}

		}
		

		read_in = "Quit";
		to_send = &read_in[0];
		is_sent = Serial->WriteSerialPort(to_send);
		closed_ = Serial->CloseSerialPort(); // closes serial port

		attemptNo++;

		if (AUTO_CONNECTION == true) {

			if (tens) {
				com_port[8] = 48 + (attemptNo - 10);
			}
			else {
				com_port[7] = 48 + attemptNo;
			}
			if (attemptNo > 9 && tens == 0) {
				tens = 1;
				com_port[7] = 48 + 1;
				com_port[8] = 48;
			}
		
		}

		Serial->TryNewPort(com_port, COM_BAUD_RATE);

		if (attemptNo == 20) {
			com_port[7] = 48 + 1;
			com_port[8] = ' ';
			attemptNo = 0;
			tens = 0;
		}
	

	}


}





int sendTix(VXSimpleSerial* serial) {

	using namespace std::literals::chrono_literals;

	string syntax_type = "greater_less_than";
	string incoming = "not ready";
	int NO_OF_ATTEMPTS = 5;
	int attemptNo = 0;
	int reply_wait_time = 5;

	newConnection(serial);

	while (ticketMachineRun) {

		if (serial->connected_ == false) newConnection(serial);
		serial->CheckConnection();
		


		if (printBuff[0] == 'A' || printBuff[0] == 'B' || printBuff[0] == 'C' || printBuff[0] == 'D' || printBuff[0] == 'T') {
			attemptNo = 0;

		retryConnection:
			if (serial->WriteSerialPort(printBuff) == true) {


				incoming = serial->ReadSerialPort(reply_wait_time, syntax_type);

				if (incoming != "Got Message") {
				printf("message failed to be Received - trying again... (%d) \n", attemptNo);
					attemptNo++;
					if (attemptNo < NO_OF_ATTEMPTS) goto retryConnection;

				}
				else {

					printf("Message Received '%s' \n\n", incoming.c_str());
					printBuff[0] = '\0';
				}

			}
		}
		else {
			//attemptNo++;
			//if (attemptNo < NO_OF_ATTEMPTS) goto retryConnection;
		}

		std::this_thread::sleep_for(1s);
	}
}


void tix2Char(int p1Tix, int p2Tix, int p3Tix, int p4Tix) {

	int x = 0;
	int i = 0;
	char intBuff[5] = " ";
	if (p1Tix > 0) x++;
	if (p2Tix > 0) x++;
	if (p3Tix > 0) x++;
	if (p4Tix > 0) x++;

	if (x > 1) {
		printBuff[i] = 'T';
		i++;
	}

	if (p1Tix > 0) {
		printBuff[i] = 'A';
		i++;
		sprintf_s(intBuff, "%1d", p1Tix);

		x = 0;

		for (i; true; i++) {
			if (intBuff[x] == '\0') break;
			printBuff[i] = intBuff[x];
			x++;
		}

	}

	if (p2Tix > 0) {
		printBuff[i] = 'B';
		i++;
		sprintf_s(intBuff, "%1d", p2Tix);

		x = 0;

		for (i; true; i++) {
			if (intBuff[x] == '\0') break;
			printBuff[i] = intBuff[x];
			x++;
		}



	}

	if (p3Tix > 0) {
		printBuff[i] = 'C';
		i++;
		sprintf_s(intBuff, "%1d", p3Tix);

		x = 0;

		for (i; true; i++) {
			if (intBuff[x] == '\0') break;
			printBuff[i] = intBuff[x];
			x++;
		}



	}

	if (p4Tix > 0) {
		printBuff[i] = 'D';
		i++;
		sprintf_s(intBuff, "%1d", p4Tix);

		x = 0;

		for (i; true; i++) {
			if (intBuff[x] == '\0') break;
			printBuff[i] = intBuff[x];
			x++;
		}



	}


	i++;
	printBuff[i] = '\0';
}

int p1Tix = 0;
int p2Tix = 0;
int p3Tix = 0;
int p4Tix = 0;


void decodeTixString() {

	int i = 0;
	int x = -1;
	int y = 0;
	int z = 0;
	int j = 0;



	
	for (i = 0; true; i++) {

		if (printBuff[i] == '\0') break;


		switch (x) {
		case -1:
			break;
		case 1:

			switch (z) {
			case 1:
				p1Tix += (printBuff[i] - 48);
				break;
			case 2:
				p1Tix += (printBuff[i] - 48) * (10);
				break;
			case 3:
				p1Tix += (printBuff[i] - 48) * (100);
				break;
			case 4:
				p1Tix += (printBuff[i] - 48) * (1000);
				break;
			}

			z--;


			break;
		case 2:

			switch (z) {
			case 1:
				p2Tix += (printBuff[i] - 48);
				break;
			case 2:
				p2Tix += (printBuff[i] - 48) * (10);
				break;
			case 3:
				p2Tix += (printBuff[i] - 48) * (100);
				break;
			case 4:
				p2Tix += (printBuff[i] - 48) * (1000);
				break;


			}

			z--;


			break;
		case 3:

			switch (z) {
			case 1:
				p3Tix += (printBuff[i] - 48);
				break;
			case 2:
				p3Tix += (printBuff[i] - 48) * (10);
				break;
			case 3:
				p3Tix += (printBuff[i] - 48) * (100);
				break;
			case 4:
				p3Tix += (printBuff[i] - 48) * (1000);
				break;


			}

			z--;


			break;
		case 4:

			switch (z) {
			case 1:
				p4Tix += (printBuff[i] - 48);
				break;
			case 2:
				p4Tix += (printBuff[i] - 48) * (10);
				break;
			case 3:
				p4Tix += (printBuff[i] - 48) * (100);
				break;
			case 4:
				p4Tix += (printBuff[i] - 48) * (1000);
				break;


			}

			z--;


			break;
		}


		if (printBuff[i] == 'A') { x = 1; y = 1; z = 0; }
		if (printBuff[i] == 'B') { x = 2; y = 1; z = 0; }
		if (printBuff[i] == 'C') { x = 3; y = 1; z = 0; }
		if (printBuff[i] == 'D') { x = 4; y = 1; z = 0; }

		if (y == 1) {
			for (j = i + 1; true; j++) {
				if (printBuff[j] == '\0' || printBuff[j] == 'A' || printBuff[j] == 'B' || printBuff[j] == 'C' || printBuff[j] == 'D') break;
				z++;
			}
		}

		y = 0;



	}

	





}


int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow) {

	
	VXSimpleSerial Serial(com_port, COM_BAUD_RATE);

	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();

	thread th1(sendTix, &Serial);


	int x = 0;
	int y = 0;
	int i = 0;


	int col = 0xff0000;

	while (voxie->breath())
	{
		voxie->startFrame();

		voxie->drawSphere(.1 + cos(voxie->getTime()), 0, 0, 0.1, 0, voxie->randomCol());

	
		col = 0xff0000;
		if (Serial.connected_ && Serial.handshakeState) col = 0x00ff00;
		if (Serial.connected_ && !Serial.handshakeState) col = 0xffff00;
		
		voxie->debugText(100, 75, col, -1, "Connected: %d Shake: %d Port: %s ::printBuff : %s", Serial.connected_, Serial.handshakeState, com_port, printBuff);
		 
		if (voxie->getKeyOnDown(KB_A) == 1) {

			p1Tix = 10; // rand() % 100;
			p2Tix = 20; // rand() % 100;
			p3Tix = 0; // rand() % 100;
			p4Tix = 0; // rand() % 100;

			// COLLECT ALL THE TICKETS

			if (printBuff[0] != 'A' || printBuff[0] != 'B' || printBuff[0] != 'C' || printBuff[0] != 'D' || printBuff[0] != 'T') {

				tix2Char(p1Tix, p2Tix, p3Tix, p4Tix);

			



			}

			p1Tix = 0;
			p2Tix = 0;
			decodeTixString();
			i = 0;

		}

		voxie->showVPS();
		voxie->endFrame();
	}

	ticketMachineRun = false;
	th1.join();
	if (Serial.connected_) 	closed_ = Serial.CloseSerialPort(); // closes serial port
	voxie->quitLoop();
	delete voxie;
	return 0;

}

