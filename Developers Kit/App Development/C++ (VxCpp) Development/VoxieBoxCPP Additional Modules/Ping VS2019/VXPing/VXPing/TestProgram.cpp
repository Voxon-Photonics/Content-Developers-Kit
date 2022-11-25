#include "vxCPP.h"
#include "vxPing.h"

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();


	voxie->setEnableMouseClipping(true);
	voxie->setBorder(true);
	vxPing ping(voxie);
	point3d mousePos = { 0 };
	float currentSpeed = 20;

	while (voxie->breath())
	{
		voxie->startFrame();

		mousePos = voxie->getMousePosition();

		

		if (voxie->getMouseButtonOnDown(MOUSE_LEFT_BUTTON)) {

			ping.addPing(mousePos, voxie->randomCol(), currentSpeed, PING_FORWARDS);

		}

		if (voxie->getMouseButtonOnDown(MOUSE_RIGHT_BUTTON)) {

			ping.addPing(mousePos, PING_RAINBOW_COLOUR, currentSpeed * 2, PING_REVERSE);

		}

		if (voxie->getKeyOnDown(KB_Space_Bar)) {

			ping.clearAll();
		}
	

		if (voxie->getKeyOnDown(KB_Q)) {

			ping.addPing(mousePos, PING_RAINBOW_COLOUR, currentSpeed, PING_PULSE);
		}

		if (voxie->getKeyOnDown(KB_W)) {

			currentSpeed += 1;

		}

		if (voxie->getKeyOnDown(KB_S)) {

			currentSpeed -= 1;

		}


		voxie->drawCursor(&mousePos, INPUT_MOUSE, 0, 0xffffff);
		voxie->debugText(100, 70, 0xffffff, -1, "VXPING TEST PROGRAM\nMouse Click Left = Make forward pulse\nMouse Click Right = Make reverse pulse\nKeyboard 'Q' = Make Pulse Ping\nKeyboard 'W' / 'S' = increase/ decrease current speed\nCurrentSpeed %1.2f", currentSpeed);
		voxie->showVPS();
		ping.draw();
		ping.cleanUp();
		ping.report(100, 150);
		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
