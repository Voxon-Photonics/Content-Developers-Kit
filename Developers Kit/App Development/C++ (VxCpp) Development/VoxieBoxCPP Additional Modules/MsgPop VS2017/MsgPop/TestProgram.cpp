#include "vxCPP.h"
#include "MsgPop.h"

/* Test Class for MsgPop
 *
 * MessagePopper designed to be a static class in your program to send messages 
 * to be displayed on the volumetric or touch screens for a VX application
 *
 *
 *
 */

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();
	int i = 0;
	int inited = 0;
	float f = 3.4324;
	
	//typedef struct { char * message; int color, destination, assignedOrientation;  double displayTime; } PopMessage_t;
	PopMessage_t tstMsg = { (char*)"test", 0xff00ff, MSGPOP_2_TOUCH_SCREEN, -1, 3};

	MsgPop msgPop(voxie);
	point2d p2 = { 0 };
	Orientation_t ori = { 0 };

	// options for msgPop
	msgPop.enableCommonExpiry(true); // enable this setting for all fade times for the messages to be common
	

	while (voxie->breath())
	{
		voxie->startFrame();

		if (voxie->getKeyOnDown(KB_1) || inited == 0) {

			inited = 1;
			for (i = 0; i < 3; i++) {
				tstMsg.destination = MSGPOP_2_TOUCH_SCREEN;
				tstMsg.color = voxie->randomCol();
				tstMsg.displayTime = rand() % 5;
				tstMsg.position.x = rand() % 1000;
				tstMsg.position.y = 50 + rand() % 700;
				tstMsg.assignedOrientation = 0;
				
				// use this utility to add variables to the message 
				msgPop.msgAmend(&tstMsg, (char*)"test %d %1.3f", i, voxie->getTime());

				msgPop.add(tstMsg);
			}

			tstMsg.destination = MSGPOP_2_VOLUMETRIC;
			tstMsg.color = voxie->randomCol();
			tstMsg.displayTime = rand() % 5;
			tstMsg.position.x = -0.5;
			tstMsg.position.y = 0;
			tstMsg.assignedOrientation = 0;

			// use this utility to concatenate various variables into the message 
			msgPop.msgAmend(&tstMsg, (char*)"test %d %1.3f", i, voxie->getTime());

			msgPop.add(tstMsg);


			tstMsg.message =  (char*)"Ori 1";
			tstMsg.assignedOrientation = 1;
			msgPop.add(tstMsg);

			tstMsg.message = (char*)"Ori 2";
			tstMsg.assignedOrientation = 2;
			msgPop.add(tstMsg);

			tstMsg.message = (char*)"Ori 3";
			tstMsg.assignedOrientation = 3;
			msgPop.add(tstMsg);

			tstMsg.message = (char*)"Ori 4";
			tstMsg.assignedOrientation = 4;
			msgPop.add(tstMsg);


		}


		// Various keyboard buttons to change settings...

		if (voxie->getKeyOnDown(KB_2)) {
			msgPop.setAutoAlign(!msgPop.getAutoAlign());
		}

		if (voxie->getKeyOnDown(KB_3)) {
			msgPop.setDrawable(!msgPop.getDrawable());
		}

		if (voxie->getKeyOnDown(KB_4)) {
			msgPop.flush();
		}

		if (voxie->getKeyOnDown(KB_5)) {
			msgPop.enableCommonExpiry(2);
		}

		if (voxie->getKeyOnDown(KB_Minus)) {
			msgPop.setFadeTime(msgPop.getFadeTime() - 1);
		}

		if (voxie->getKeyOnDown(KB_Equals)) {
			msgPop.setFadeTime(msgPop.getFadeTime() + 1);
		}

		if (voxie->getKeyIsDown(KB_Q)) {
			tstMsg.hang++;
		}

		if (voxie->getKeyIsDown(KB_W)) {
			tstMsg.hang--;
		}

		if (voxie->getKeyIsDown(KB_A)) {
			tstMsg.vang++;
		}

		if (voxie->getKeyIsDown(KB_S)) {
			tstMsg.vang--;
		}

		if (voxie->getKeyIsDown(KB_E)) {
			tstMsg.twist++;
		}
		
		if (voxie->getKeyIsDown(KB_D)) {
			tstMsg.twist--;
		}

		if (voxie->getKeyIsDown(KB_I)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.y -= 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_K)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.y += 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_J)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.x -= 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_L)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.x += 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_NUMPAD_Minus)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.z -= 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_NUMPAD_Plus)) {
			ori = msgPop.getOrientation3D(0);
			ori.position.z += 0.01;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_U)) {
			ori = msgPop.getOrientation3D(0);
			ori.hang -= 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_O)) {
			ori = msgPop.getOrientation3D(0);
			ori.hang += 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_P)) {
			ori = msgPop.getOrientation3D(0);
			ori.vang += 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_Semicolon)) {
			ori = msgPop.getOrientation3D(0);
			ori.vang -= 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_Comma)) {
			ori = msgPop.getOrientation3D(0);
			ori.twist -= 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_Full_Stop)) {
			ori = msgPop.getOrientation3D(0);
			ori.twist += 1;
			msgPop.setOrientation3D(0, ori);
		}

		if (voxie->getKeyIsDown(KB_G)) {
			p2 = msgPop.getOrientation2D(0);
			p2.x -= 1;
			msgPop.setOrientation2D(0, p2);
		}

		if (voxie->getKeyIsDown(KB_H)) {
			p2 = msgPop.getOrientation2D(0);
			p2.x += 1;
			msgPop.setOrientation2D(0, p2);
		}

		if (voxie->getKeyIsDown(KB_B)) {
			p2 = msgPop.getOrientation2D(0);
			p2.y -= 1;
			msgPop.setOrientation2D(0, p2);
		}

		if (voxie->getKeyIsDown(KB_N)) {
			p2 = msgPop.getOrientation2D(0);
			p2.y += 1;
			msgPop.setOrientation2D(0, p2);
		}

		if (voxie->getKeyIsDown(KB_1) && voxie->getKeyIsDown(KB_Shift_Left)) {

			msgPop.setOrientation3D(1, msgPop.getOrientation3D(0));

		
			tstMsg.message = (char*)"Orientation Saved!";
			msgPop.add(tstMsg, 0);


		}

		if (voxie->getKeyIsDown(KB_2) && voxie->getKeyIsDown(KB_Shift_Left)) {

			msgPop.setOrientation3D(2, msgPop.getOrientation3D(0));

			tstMsg.message = (char*)"Orientation Saved!";
			msgPop.add(tstMsg, 0);
		}

		if (voxie->getKeyIsDown(KB_3) && voxie->getKeyIsDown(KB_Shift_Left)) {

			msgPop.setOrientation3D(3, msgPop.getOrientation3D(0));

			tstMsg.message = (char*)"Orientation Saved!";
			msgPop.add(tstMsg, 0);
		}

		if (voxie->getKeyIsDown(KB_4) && voxie->getKeyIsDown(KB_Shift_Left)) {

			msgPop.setOrientation3D(3, msgPop.getOrientation3D(0));

			tstMsg.message = (char*)"Orientation Saved!";
			msgPop.add(tstMsg, 0);
		}




		voxie->showVPS();
		msgPop.report(300, 550);
	
		msgPop.draw();

		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
