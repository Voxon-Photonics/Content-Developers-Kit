#include "vxCPP.h"
#include "vxCursor.h"
#include "VxInput.h"

/**
* Example of using VxInput and VxCursor
* 10/06/2022 Matthew Vecchio for Voxon
* 
*/

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();

	voxie->setBorder(true);

	VxCursor curs(voxie);
	VxInput input(voxie);
	


	curs.setEnabledClip(true);
	curs.setMovementSpeed(0.2, 0.05);

	int i = 0;
	
	point3d spheres[10] = { 0 };

	for (i = 0; i < 10; i++) spheres[i] = voxie->randomPos();
	

	while (voxie->breath())
	{

		//***********************
		// Input update

		input.update(); // run this to update the various inputs

		for (i = 0; i < input.getInput().size(); i++) {

			switch (input.getInput()[i].inputAction) {
			case AX_CURSOR_MOVEMENT:
				curs.movePos(input.getInput()[i].point);
				break;
			case AX_CURSOR_FLASH:
				curs.setFlashing(true,input.getInput()[i].i);
				break;
			case AX_RESET_SPHERES:
				for (i = 0; i < 10; i++) spheres[i] = voxie->randomPos();
				break;

			}

		}

		//***********************
		// Update

		voxie->startFrame();
		voxie->showVPS();


		//***********************
		// Draw
	
		curs.setDrawState(0);
		for (i = 0; i < 10; i++) {

			voxie->drawSphere(spheres[i], 0.1, 0, 0xffff00);
		
			if (curs.colCheck(spheres[i], 0.1)) {
				curs.setDrawState(3);
			}
		}

		curs.draw();

		curs.report(20, 100);
		input.report(20, 200);

		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
