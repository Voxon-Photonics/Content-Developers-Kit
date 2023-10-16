/************************************
* vxCpp Drawing Examples Application
* (c) 2021 Matthew Vecchio for Voxon.
*
* This is Voxon vxCpp program to demonstrate input calls for a Voxon App
*
*/

#include "pch.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>

#include "vxCPP.h"
int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	// load in the VxCpp.dll so we can load in the voxiebox.dll into memory and create a voxieBox class
	HINSTANCE hVoxieDLL = LoadLibrary("vxCpp.dll");

	if (hVoxieDLL == NULL) {
		MessageBox(0, (char *)"Error: can't locate vxCpp.dll \nis vxCpp.dll in the local folder or setup as a path variable?", (char *)"", MB_OK);
		return (-1);
	}

	// crate a new voxieBox object which will handle all the voxieBox related calls
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	Ivoxiebox* voxie = pEntryFunction();

	// use voxie-> to use various functions
	voxie->setBorder(true);

	// Breath is the update loop, after a Voxon program is initalised the main loop. 
	while (voxie->breath())
	{
		// The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
		voxie->startFrame();




		voxie->showVPS();
		voxie->endFrame(); // the end of drawing to the volumetric display
	}

	// quit() stops the hardware from physically moving andd ends the breath() loop  
	voxie->quitLoop();
	delete voxie;
	return 0;
	// After the program quits the deconstructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}