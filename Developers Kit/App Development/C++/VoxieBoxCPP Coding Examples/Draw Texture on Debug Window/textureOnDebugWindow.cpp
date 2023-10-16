/************************************
* Draw a tile type texture on debug window
* (c) 2021 Matthew Vecchio for Voxon.
*
*
*/


#include "pch.h"
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


	tiletype sampleImg;                 // define new tiletype type
	sampleImg.x = 272;                  // get the image's x dimension (look at the details of the file - Windows explorer -> properties)
	sampleImg.y = 170;                  // get the image's y dimension (look at the details of the file - Windows explorer -> properties)
	sampleImg.p = (sampleImg.x << 2);   // define the pitch number of bytes per horizontal line (usually x*4 but may be higher or negative)
	sampleImg.f = (INT_PTR)malloc(sampleImg.p*sampleImg.y); // create pointer to 1st pixel

    // internal function used to process images - Ken's picture library
	voxie->_kpzload((char*)"sample.png", &sampleImg.f, &sampleImg.p, &sampleImg.x, &sampleImg.y); // load the image into file memory pass in the pointers
	// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume. 

	while (voxie->breath())
	{
	
		voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
	
    	voxie->debugDrawTile(&sampleImg, 100, 100);
	
    	voxie->endFrame(); // the end of drawing to the volumetric display.
	}

	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;
	return 0;   // After the program quits the deconstructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}