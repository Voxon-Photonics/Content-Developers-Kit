/************************************
* vxCpp Template 
* (c) 2023 Matthew Vecchio for Voxon.
*
* You may use this as a template to start creating your own volumetric applications.
*
*/

// VX++ APP HELLO WORLD EXAMPLE V2
// You may use this as a template to start creating your own volumetric applications.
#include "vxCPP.h"
int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	// load in the vxCpp.dll so we can create the VoxieBox object...
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (!hVoxieDLL) return 1;

	// Access and call "CreateVoxieBoxObject" function from vxCpp.dll. The CreateVoxieBoxObject function creates a new VoxieBox object.
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
	if (pEntryFunction == 0) return 1;

	// Set a new IVoxieBox pointer to point to the entry function (the voxiebox object) now 'voxie' get access to the VoxieBox class
	IVoxieBox* voxie = pEntryFunction();


	// Some graphic examples
	// variables to setup text positions to display 'Hello World' -- feel free to delete this  
	point3d textPos{ -0.5, 0, 0 };		// text positions x,y,z values
	point3d textWidth{ 0.1,  0, 0 };    // text rVector  x,y,z ... the x value determines the width of the text the other values deal with rotating text
	point3d textHeight{ 0, 0.15, 0 };	// text dVector  x,y,z ... the y value determines the height of the text the other values deal with rotating text


	// Define Variables for our application
	int fillmode = 2; // variable to explore the different fill modes.
	point3d pos = { 0,0,0 };  // some 3d points to help to set positioning 
	point3d rVec = { 0,0,0 };
	point3d dVec = { 0,0,0 };
	point3d fVec = { 0,0,0 };

	// variables for drawing polygons
	poltex_t vt[4]; int mesh[16], i = 0;
	// setting vertice and mesh for shape
	vt[0].x = -0.4; vt[0].y = -0.8; vt[0].z = -0.2; vt[0].col = 0xff0000;
	vt[1].x = -0.4; vt[1].y = -0.4; vt[1].z = +0.2; vt[1].col = 0x00ff00;
	vt[2].x = +0.4; vt[2].y = -0.8; vt[2].z = +0.2; vt[2].col = 0x0000ff;
	vt[3].x = +0.4; vt[3].y = -0.4; vt[3].z = -0.2; vt[3].col = 0xff00ff;
	mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1; /*-1 = end of polygonal facet*/
	mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
	mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
	mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;


	voxie->setBorder(true); // if true draws a border around the perimeter of the display

	// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume. 
	while (voxie->breath())
	{
		/**********
		*  Input
		*/

		if (voxie->getKeyOnDown(KB_0) == 1) fillmode = 0; // press kb 0
		if (voxie->getKeyOnDown(KB_1) == 1) fillmode = 1; // press kb 1
		if (voxie->getKeyOnDown(KB_2) == 1) fillmode = 2; // press kb 2
		if (voxie->getKeyOnDown(KB_3) == 1) fillmode = 3; // press kb 3

		// use arrow keys to move text
		if (voxie->getKeyIsDown(KB_Arrow_Left))	textPos.x -= 1 * voxie->getDeltaTime();
		if (voxie->getKeyIsDown(KB_Arrow_Right))	textPos.x += 1 * voxie->getDeltaTime();
		if (voxie->getKeyIsDown(KB_Arrow_Up))		textPos.y -= 1 * voxie->getDeltaTime();
		if (voxie->getKeyIsDown(KB_Arrow_Down))	textPos.y += 1 * voxie->getDeltaTime();

		/**********
		*   Draw
		*/

		voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)

		// draw text onto the volumetric display 
		voxie->drawText(&textPos, &textWidth, &textHeight, 0xffffff, "Hello World"); // draw text onto the volumetric display.

		// Draw a mesh either from a file or from vertice and mesh data
		voxie->drawMesh(0, vt, 4, mesh, i, 16 + fillmode, 0x404040); /* (to colour of vertice to work need to have fill mode 16+ (0010000b) ) */

		// draw a box
		voxie->drawBox(-0.9, 0, -0.4, -0.5, 0.4, 0, fillmode, 0xff0000);

		pos.x = -0.4; 		pos.y = 0; 			pos.z = -0.4;
		rVec.x = 0.4; 		rVec.y = 0; 		rVec.z = 0;
		dVec.x = 0; 		dVec.y = 0.4;		dVec.z = 0;
		fVec.x = 0; 		fVec.y = 0; 		fVec.z = 0.4;

		// draw cube (need to set right, down and forward vectors)
		voxie->drawCube(&pos, &rVec, &dVec, &fVec, fillmode, 0x00ff00);

		// draw a sphere
		voxie->drawSphere(0.8, 0.2, -0.2, .15, fillmode, 0xffff00);

		// draw a line
		voxie->drawLine(-0.8, -0.2, -0.4, 0.8, -0.2, 0.4, 0x00ffff);

		// draw a cone
		voxie->drawCone(-0.8, 0.7, 0.2, 0.1, 0.8, 0.7, -0.2, 0.2, fillmode, 0xff00ff);

		// draw text unto the 2D touch screen
		voxie->debugText(30, 128, 0x00ff80, -1, "Polygon and primitive shape demo press 0 - 3 to change fillmode");
		voxie->debugText(30, 138, 0x00ffff, -1, "fillmode : %d", fillmode);
		voxie->debugText(30, 148, 0x00ff80, -1, "This demo also shows how to use vertice shading");

		voxie->debugText(30, 168, 0xffff80, -1, "what is being shown on the screen from top left to bottom right:\ndrawMesh\ndrawLine\ndrawText\ndrawBox\ndrawCube\ndrawSphere\ndrawCone");

		voxie->showVPS(); //show VPS data and VxCpp.DLL version unto the touch screen.  

		voxie->endFrame(); // the end of drawing to the volumetric display.
	}


	voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
	delete voxie;
	return 0;   // After the program quits the deconstructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}