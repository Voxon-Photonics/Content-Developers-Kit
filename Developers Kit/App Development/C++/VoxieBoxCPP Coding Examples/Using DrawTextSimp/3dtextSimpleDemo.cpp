/************************************
* VxCpp APP 3D TEXT SIMPLE DEMO
* (c) 2021 Matthew Vecchio for Voxon.
*
* You may use this as a template to start creating your own volumetric applications.
*
*/


#include "pch.h"
// VX++ APP 3D TEXT SIMPLE DEMO
// You may use this as a template to start creating your own volumetric applications.
#include "VxCpp.h"
int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	// load in the VxCpp.dll so we can create the VoxieBox object...
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("VxCpp.dll");
	if (hVoxieDLL == NULL) return 1;
	// Access and call "CreateVoxieBoxObject" function from VxCpp.dll. The CreateVoxieBoxObject function creates a new VoxieBox object.
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	// Set a new IVoxieBox pointer to point to the entry function (the voxiebox object) now 'voxie' get access to the VoxieBox class
	IVoxieBox* voxie = pEntryFunction();

	voxie->setBorder(true); // if true draws a border around the perimeter of the display

	point3d pos = { -.5f, 0.f, 0.f };
	float sizeX = 0.3, sizeY = 0.5, hang = 0, vang = 0, tilt = 0; 
	int i = 0, numframes = 0;


	// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume. 
	while (voxie->breath())
	{
		voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
		
		if (voxie->getKeyIsDown(0x10)) sizeX += voxie->getDeltaTime() * .5f; //Q
		if (voxie->getKeyIsDown(0x1e)) sizeX -= voxie->getDeltaTime() * .5f; //A
		if (voxie->getKeyIsDown(0x11)) sizeY += voxie->getDeltaTime() * .5f; //W
		if (voxie->getKeyIsDown(0x1f)) sizeY -= voxie->getDeltaTime() * .5f; //S

		if (voxie->getKeyIsDown(0x13)) hang += voxie->getDeltaTime() * 64.f; //R
		if (voxie->getKeyIsDown(0x14)) hang -= voxie->getDeltaTime() * 64.f; //T
		if (voxie->getKeyIsDown(0x15)) vang += voxie->getDeltaTime() * 64.f; //Y
		if (voxie->getKeyIsDown(0x23)) vang -= voxie->getDeltaTime() * 64.f; //H
		if (voxie->getKeyIsDown(0x16)) tilt += voxie->getDeltaTime() * 64.f; //U
		if (voxie->getKeyIsDown(0x17)) tilt -= voxie->getDeltaTime() * 64.f; //I


		if (voxie->getKeyIsDown(0x35)) { hang = 0.f; vang = 0.f; tilt = 0.f; } // /

			//Arrows, - and + change the position of the planes
		if (voxie->getKeyIsDown(0xcb)) pos.x -= voxie->getDeltaTime() * .5f; //LEFT
		if (voxie->getKeyIsDown(0xcd)) pos.x += voxie->getDeltaTime() * .5f; //RIGHT
		if (voxie->getKeyIsDown(0xc8)) pos.y -= voxie->getDeltaTime() * .5f; //UP
		if (voxie->getKeyIsDown(0xd0)) pos.y += voxie->getDeltaTime() * .5f; //DOWN
		if (voxie->getKeyIsDown(0x0c) || voxie->getKeyIsDown(0x4a)) pos.z -= voxie->getDeltaTime() * .5f; //- or numpad -
		if (voxie->getKeyIsDown(0x0d) || voxie->getKeyIsDown(0x4e)) pos.z += voxie->getDeltaTime() * .5f; //+ or numpad +
		
		// draw text Simple
		voxie->drawTextSimp(&pos, sizeX, sizeY, hang, vang, tilt, voxie->randomCol(), (char*)"Test Msg, %d", numframes); // draw text onto the volumetric display.
		
		voxie->debugText(30, 160, 0xff0000, -1, "pos %1.3f %1.3f %1.3f ", pos.x, pos.y, pos.z);
		voxie->debugText(30, 180, 0xff0000, -1, "hang %1.3f vang %1.3f", hang, vang);
		voxie->debugText(30, 200, 0xff0000, -1, "Size %1.3f %1.3f ", sizeX, sizeY);
		voxie->debugText(30, 280, 0x00ff00, -1, "Keys:\n\nArrows Move Pos\n-,+    Raise / Lower Pos\nR,T    Adjust Horiz ang\nY,H    Adjnst Vert ang\nU,I    Adjust Twist ang\n/      Reset angs\nQ,A    Adjust Xsize\nW,S    Adjust Ysize");

		voxie->showVPS(); //show VPS data and VxCpp.DLL version unto the touch screen.  
		voxie->endFrame(); // the end of drawing to the volumetric display.
		numframes++;
	}

	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;
	return 0;   // After the program quits the de-constructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}