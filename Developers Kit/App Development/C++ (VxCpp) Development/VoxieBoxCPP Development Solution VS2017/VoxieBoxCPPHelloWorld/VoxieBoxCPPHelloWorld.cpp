/************************************
* VxCpp Hello World Application
* (c) 2021 Matthew Vecchio for Voxon.
*
* You may use this as a template to start creating your own volumetric applications.
*
*/


#include "pch.h"
// VX++ APP HELLO WORLD EXAMPLE 
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

	// variables to setup text positions to display 'Hello World' -- feel free to delete this  
	point3d textPos{ -0.5, 0, 0 };     // text positions x,y,z values
	point3d textWidth{ 0.1,  0, 0 };     // text rVector  x,y,z ... the x value determines the width of the text the other values deal with rotating text
	point3d textHeight{ 0, 0.15, 0 }; // text dVector  x,y,z ... the y value determines the height of the text the other values deal with rotating text

	voxie->setBorder(true); // if true draws a border around the perimeter of the display

	// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume. 
	while (voxie->breath())
	{
		voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
		voxie->drawText(&textPos, &textWidth, &textHeight, 0xffffff, "Hello World"); // draw text onto the volumetric display.
		textPos.z = cos(voxie->getTime()) / 5; // move the text's Z position over time using cos (creates a moving up and down effect)
		voxie->debugText(35, 100, 0xffffff, -1, "Hello World On the Touch Screen!"); // draw text onto the secondary (touch) screen.
		voxie->showVPS(); //show VPS data and VxCpp.DLL version unto the touch screen.  
		voxie->endFrame(); // the end of drawing to the volumetric display.
	}

	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;
	return 0;   // After the program quits the de-constructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}