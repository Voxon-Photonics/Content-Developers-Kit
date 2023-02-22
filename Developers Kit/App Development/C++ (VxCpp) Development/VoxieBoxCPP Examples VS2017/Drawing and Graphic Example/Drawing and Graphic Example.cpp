/************************************
* vxCpp Drawing Examples Application
* (c) 2021 Matthew Vecchio for Voxon.
*
* This is Voxon vxCpp program to demonstrate various drawing / graphical calls on a VX1
*
*
* This program will show all the different types of drawing graphics for a VX application. 
* The user can press the 1 - 5 keys on the keyboard to see different graphic calls.
* The code here is free to use. 
* I have not made this demo to use classes / objects for the sake of learning.
* 
*
*/

#include "pch.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>

#include "vxCPP.h"
#include "CollisionBox.h"
#include "PlotPoint.h"
#include <vector>

enum demoNames {

	MENU, POSITIONS, PRIMITIVES, MODELS, TEXURED_PLANES, TEXURED_MESH, HEIGHTMAP, TEXT,
	TOUCH_SCREEN, NORMAL_SHADER, SET_VIEW, MAX_SCENE

};


enum menuCommands {

	NEXT_SCENE, PREVIOUS_SCENE, 

};

int gInited = 0; // variable used to managed a 'run once' set of instruction (is it inited? aka initalised?)
int gCurrentScene = MENU; // the variable that holds the curVecentscene to render
#define FILL_MAX 4
#define FILL_MIN 0

// the menu update this is where the variables change when a button is pressed in the menu
static int menu_example_update(int id, char *st, double v, int how, void *userdata) {

	switch (id)
	{
	case NEXT_SCENE:
		gInited = -1;
		gCurrentScene++;
		if (gCurrentScene >= MAX_SCENE) gCurrentScene = MENU;

		break;
	case PREVIOUS_SCENE:
		gInited = -1;
		gCurrentScene--;
		if (gCurrentScene <= MENU) gCurrentScene = MAX_SCENE - 1;
		
		break;
	}


	return (0);
}




int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	// define the variables for the demo
	int	  i = 0; // a temporary int variable used for many for loops 
	int   j = 0; // a second temporary int variable used for many for loops 

	int cbNo = 0;
	int plotNo = 0;
	int	col = 0;
	float f = 0, g = 0; // temporary float variables used for some quick math
	float r = 0;
	float scale = 1;
	float  x = 0, y = 0, z = 0; // temporary float variables used for some quick math
	float fontSize = 0.08;

	
	int fillMode = 2;
	const int FILL_MODE_MAX = 3;
	const int FILL_MODE_MIN = 0;
	char* message = (char*)" ";
	char* titleText = (char*)" ";
	int debug = 0;
	int inputMode = 0;

	poltex_t vertice1[10]; int mesh1[64];
	int choice = 0;
	int verticeNo = 0;
	int meshNo = 0;
	int meshDrawMode = 0;
	char textureFName[MAX_PATH] = "";

	// These examples are sharing some variables. 
	point3d pos = { 0 };		// 3D point for the position  
	point3d pos2 = { 0 };		// 3D point for the second position  
	point3d rVec = { 0 };		// 3D point for the right vector (x)
	point3d dVec = { 0 };		// 3D point for the down vector (y)
	point3d fVec = { 0 };		// 3D point for the forward vector (z)
	point3d posOffSet{ 0 };
	point3d cursor = { 0 };
	point3d textWidth = { 0 };
	point3d textHeight = { 0 };
	point3d modelPos = { 0 };
	point3d modelRightVec = { 0 };
	point3d modelForwardVec = { 0 };
	point3d modelDownVec = { 0 };


	// load in the VxCpp.dll so we can load in the voxiebox.dll into memory and create a voxieBox class
	HINSTANCE hVoxieDLL = LoadLibrary("vxCpp.dll");

	if (hVoxieDLL == NULL) {
		MessageBox(0, (char*)"Error: can't locate vxCpp.dll \nis vxCpp.dll in the local folder or setup as a path variable?", (char*)"", MB_OK);
		return (-1);
	}

	// create a new voxieBox object which will handle all the voxieBox related calls
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction(); // you can call 'voxie' what you like 'vx' might be better if you want to do less typing. 

	point3d actionBox = { voxie->getAspectX(),0.85, float(-voxie->getAspectZ() + 0.05) };


	std::vector <IDrawable*> IDrawables;

	voxie->menuReset(menu_example_update, 0, 0);
	voxie->menuAddTab((char*)"VxDemo", 300, 10, 600, 300);
	voxie->menuAddItem((char*)"Change Scene", 180, 30, 200, 75, 1, MENU_TEXT, 0, 0x908070, 0.0, 0.0, 0.0, 0.0, 0.0);

	voxie->menuAddItem((char*)"Next Scene ->", 320, 50, 200, 75, NEXT_SCENE, MENU_BUTTON_SINGLE, 0, 0x908070, 0.0, 0.0, 0.0, 0.0, 0.0);
	voxie->menuAddItem((char*)"<- Prev Scene", 80, 50, 200, 75, PREVIOUS_SCENE, MENU_BUTTON_SINGLE, 0, 0x908070, 0.0, 0.0, 0.0, 0.0, 0.0);

	std::vector <CollisionBox> colBoxes(20);
	for (int i = 0; i < colBoxes.size(); ++i) {
		colBoxes[i].setIVoxieBox(voxie);
		colBoxes[i].setColour(voxie->randomCol());
		IDrawables.push_back(&colBoxes[i]);

	}

	std::vector <PlotPoint> plots(50);
	for (int i = 0; i < plots.size(); ++i) {
		plots[i].setIVoxieBox(voxie);
		plots[i].setColour(voxie->randomCol());
		plots[i].setShowPos(true);
		plots[i].setActive(false);
		IDrawables.push_back(&plots[i]);
	}

	// use voxie-> to access the VX++ pointer and use various functions
	voxie->setEnableMouseClipping(true);
	voxie->setEnableNavClipping(true);


	// Breath is the update loop, after a Voxon program is initalised it is the main loop. 
	while (voxie->breath())
	{
		/** Keyboard Input */

		if (voxie->getKeyOnDown(KB_0)) { gCurrentScene = MENU; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_1)) { gCurrentScene = POSITIONS; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_2)) { gCurrentScene = PRIMITIVES; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_3)) { gCurrentScene = MODELS; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_4)) { gCurrentScene = TEXURED_PLANES; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_5)) { gCurrentScene = TEXURED_MESH; gInited = 0; } // press '0' to go to main menu
		if (voxie->getKeyOnDown(KB_6)) { gCurrentScene = HEIGHTMAP; gInited = 0; } // press '0' to go to main menu
		
		if (voxie->getKeyOnDown(KB_Q)) { fillMode++; if (fillMode > FILL_MAX) fillMode = FILL_MAX; gInited = 0;} // press 'Q' to increase fillmode
		if (voxie->getKeyOnDown(KB_A)) { fillMode--; if (fillMode < FILL_MIN) fillMode = FILL_MIN; gInited = 0;} // press 'A' to decrease fillmode
	
		if (voxie->getKeyOnDown(KB_W)) { choice++; if (choice > FILL_MAX) choice = FILL_MAX; gInited = 0; } // press 'W' to increase fillmode
		if (voxie->getKeyOnDown(KB_S)) { choice--; if (choice < FILL_MIN) choice = FILL_MIN; gInited = 0; } // press 'S' to decrease fillmode

	if (voxie->getKeyOnDown(KB_I)) { voxie->setInvertZAxis(true); } // press '0' to go to main menu
	if (voxie->getKeyOnDown(KB_O)) { voxie->setInvertZAxis(false); } // press '0' to go to main menu


	if (gInited <= 0) {
		// clear out the checkboxes 
		cbNo = 0;
		for (int i = 0; i < colBoxes.size(); ++i) {
			colBoxes[i].setActive(false);
		}

		plotNo = 0;
		for (int i = 0; i < plots.size(); ++i) {
			plots[i].setActive(false);
		}

		gInited = 0;

	}

	message = (char*)" ";

	// The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
	voxie->startFrame();

	// Drawing calls 
	switch (gCurrentScene) {
	default:
	
	case MENU:
		titleText = (char*)"Voxon Graphical Demos";

		message = (char*)"Interactive examples of Voxon Graphical calls\nPress 1 - 9 on the Keyboard to select\na scene, or use the 'VxDemo' tab to explore";

		break;

	case POSITIONS:

		if (gInited == 0) {
			// clear out the checkboxes 
			plotNo = 0;
			for (int i = 0; i < plots.size(); ++i) {
				plots[i].setActive(false);
			}

		}

		titleText = (char*)"Plotting 3D Space";

		pos = { -voxie->getAspectX(),0,0 };
		pos2 = { voxie->getAspectX(),0,0 };

		voxie->drawLine(pos, pos2, 0x0000ff);

		pos = { 0,float(-voxie->getAspectY() + 0.06),0 };
		pos2 = { 0,float(voxie->getAspectY() - 0.06),0, };

		voxie->drawLine(pos, pos2, 0x00ff00);

		pos = { 0,0, float(-voxie->getAspectZ()) };
		pos2 = { 0,0, float(voxie->getAspectZ()) };

		voxie->drawLine(pos, pos2, 0xff0000);

		rVec = { float((fontSize * .55)), 0, 0 }; // font width
		dVec = { 0, float((fontSize * 1.5) * .55), 0 }; // font height


		// put in text offset
		f = (fontSize * .55) * 6; // x offset
		g = (fontSize * .55) * 3; // y offset


		// place text at various points extreme postions
		pos = { -voxie->getAspectX(), 0, 0 };
		voxie->drawText(&pos, &rVec, &dVec, 0x0000ff, "X%0.2f", -voxie->getAspectX());

		pos = { float(voxie->getAspectX() - f), 0, 0 };
		voxie->drawText(&pos, &rVec, &dVec, 0x0000ff, "X%+0.2f", voxie->getAspectX());


		pos = { float(0 - g),  -voxie->getAspectY(), 0 };
		voxie->drawText(&pos, &rVec, &dVec, 0x00ff00, "Y%0.2f", -voxie->getAspectY());

		pos = { float(0 - g), float(voxie->getAspectY() - g), 0 };
		voxie->drawText(&pos, &rVec, &dVec, 0x00ff00, "Y+%0.2f", voxie->getAspectY());


		pos = { float(0 - g), 0, -voxie->getAspectZ() };
		voxie->drawText(&pos, &rVec, &dVec, 0xff0000, "Z%0.2f", -voxie->getAspectZ());

		pos = { float(0 - g), 0, voxie->getAspectZ() };
		voxie->drawText(&pos, &rVec, &dVec, 0xff0000, "Z+%0.2f", voxie->getAspectZ());

		// set up collision box
		pos = { -voxie->getAspectX() , float(-voxie->getAspectY() + 0.2),-.3 };
		pos2 = { float(-voxie->getAspectX() + 0.3) , float(-voxie->getAspectY() + 0.4), -.2 };
		if (gInited == 0) { colBoxes[cbNo].update(pos, pos2, (char*)"Graphics are drawn along 3 dimensions (X,Y,Z)\nX: horizontal Y: depth & Z: vertical\nUse the mouse button to plot a point", 0); cbNo++; }
		pos = { float(-voxie->getAspectX() + 0.05) , float(-voxie->getAspectY() + 0.25),-.25 };

		voxie->drawText(&pos, &rVec, &dVec, 0x00ff00, "Help");

		// get mouse or nav input

		if (voxie->getMouseButtonOnDown(MOUSE_LEFT_BUTTON) || voxie->getNavButtonOnDown(0, NAV_LEFT_BUTTON)) {

			plots[plotNo].update(cursor, 0.025, 0, voxie->randomCol());
			plots[plotNo].setActive(true);
			plotNo++;
			if (plotNo > 6) plotNo = 0;

		}

		// hold down right button to draw a line between points
		if (voxie->getMouseButtonIsDown(MOUSE_RIGHT_BUTTON) || voxie->getNavButtonIsDown(0, NAV_RIGHT_BUTTON)) {

			for (i = 0; i < plots.size(); i++) {

				if (plots[i].isActive() == false) break;

				if (plots[i + 1].isActive() == true) {
					voxie->drawLine(plots[i].getPos(), plots[i + 1].getPos(), 0xff0000);
				}
				if (i == 6) voxie->drawLine(plots[i].getPos(), plots[0].getPos(), 0xff0000);
			}

		}


		// cursor hud
		pos = { float(-voxie->getAspectX() + .2), 0.6, -voxie->getAspectZ() };
		voxie->drawText(&pos, &rVec, &dVec, 0xffff00, "Cursor Pos X: %1.2f, Y: %1.2f, Z: %1.2f", cursor.x, cursor.y, cursor.z);

		break;

	case PRIMITIVES:
		titleText = (char*)"Drawing Primitives";


		/*	Primitives are the easiest graphics to draw. There are voxels, lines, spheres, cubes / boxes and cones.

			Z axis
			|      Y axis
			|    /
			|  /
			|/_______ X axis


			(-1,-1)(+0,-1)(+1,-1)
					  |
					  |
					  |
					  |
			 ---------+---------
			(-1,+0)(+0,+0)(+1,+0)
					  |
					  |
					  |
					  |
			(-1,+1)(+0,+1)(+1,+1)

			x, y, z all at 0 are at the centre of the screen
			x -1 is to the left
			x +1 is to the right
			y -1 is to the top
			y +1 is to the bottom
			z -.4 is to the most forward
			z +.4 is to the most back

			the default aspect ratio of the VX1 display is X (-1, 1) Y (-1, 1), Z (-.4, .4)
			call voxie->getAspect() (or voxie->getAspectX(), voxie->getAspectY(), voxie->getAspectZ()  to get the current aspect ratio of the screen.

		*/

		// drawing single voxels the 'drawVox' function. drawVox draws a single voxel (a 3D pixel) - a single is often hard to see but run in a batch can be useful for drawing anything you want... 

		// you can call the method using a point3D..
		pos = { -.9, -.7, float(-voxie->getAspectZ() + 0.1) };
		voxie->drawVox(pos, 0xffffff);
		// or by putting the in x,y,z values by themselves..
		voxie->drawVox(-.7, -.7, float(-voxie->getAspectZ() + 0.1), 0x00ff00);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.95, -.75,-.3 }, point3d{ -.85, -.65,.3 }, (char*)"Draw a single voxel using drawVox()\nusing 3 floats (x,y,z) for position\nA single voxel is tiny", 0); cbNo++; }
		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.75, -.75,-.3 }, point3d{ -.65, -.65,.3 }, (char*)"Single drawVox() function can also\nuse a point 3D for position\nuse Render->DotSize to adjust size", 0); cbNo++; }



		// draw a coloring changing line of voxels
		pos = { -.55, -.7, float(-voxie->getAspectZ() + 0.1) };
		f = 0.0025; 				// f is resolution its the distance between the next voxel. lower is more voxels
		for (i = 0; i < 450; i++) {
			pos.x += f;										// with everystep adjust the next voxel's position
			if (pos.x > voxie->getAspectX()) break;			// if drawing is going outside of the volume stop the loop
			voxie->drawVox(pos, voxie->scrollCol(0));		// scrollCol is a special function that returns is a scrolling colour.. the parameter is an offset the start the colour cycle which can by any whole number
		}

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.60, -.75,-.3 }, point3d{ .6, -.66,.3 }, (char*)"using a for loop and drawVox()\nto draw a line. (450 voxels)\nUsing scrollCol() for colour cycle", 0); cbNo++; }

		// lets draw some random voxels -- these will be random every frame and in a random colour... 
		for (i = 0; i < 3000; i++) {
			pos.x = (float)(((rand() & 32767) - 16384) / 16384.f * voxie->getAspectX() * 0.5) - 0.475; // randomise on the X value all over the X axis
			pos.y = ((float)((rand() & 32767) - 16384) / 16384.f * .15) - 0.5;
			pos.z = ((float)((rand() & 32767) - 16384) / 16384.f * voxie->getAspectZ() * .5);
			voxie->drawVox(pos, voxie->randomCol()); // randomCol is a special function that returns a random colour
		}
		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -voxie->getAspectX(), -.65,-.3 }, point3d{ 0.05, -.35,.3 }, (char*)"3000 drawVox() calls in a for loop\nrandom position & colour per volume\nuses randomCol() for random colours", 0); cbNo++; }



		/* drawing a line can be done much easier with the voxie->drawLine() function..

		*/

		// lets draw a cyan line across the centre of the screen. Lets draw using two point3d
		pos = { 0.1, -.5, 0 }; // the starting position for the line
		pos2 = { voxie->getAspectX(), -.5, 0 }; // the end position for the line
		voxie->drawLine(pos, pos2, 0x00ffff);

		pos2 = { voxie->getAspectX(), -.5, -.3 }; // the end position for the line
		voxie->drawLine(pos, pos2, 0xff00ff);

		pos2 = { voxie->getAspectX(), -.5, .3 }; // the end position for the line
		voxie->drawLine(pos, pos2, 0xffff00);



		if (gInited == 0) { colBoxes[cbNo].update(point3d{ 0.05, -.65,-.3 }, point3d{ voxie->getAspectX(), -.35,.3 }, (char*)"drawLine() better way to draw a line\ndefine 2 point3Ds to draw from one\npoint to another", 0); cbNo++; }

		// you can also input all the position values as floats here is a yellow line going straight up
		voxie->drawLine(.8, -0.7, voxie->getAspectZ() - .1, .8, -0.7, -voxie->getAspectZ() + .1, 0xff0000);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ .65,-.8,-.3 }, point3d{ .90, -.6,.3 }, (char*)"drawLine() can also be used with 6\nfloats for position. colour is always\nhex colour value (this = 0xff0000)", 0); cbNo++; }



		// draw a circle of voxels
		pos = { -.6, 0.0, 0 };								// the position of the circles centre
		f = 0;												// f is the angle 
		g = 0.2;											// is the radius the overall size of the circle 
		z = 0.025 * (fillMode + 1);							// z is the resolution -- how far the voxels are apart this is the distance between the voxels
		// formula to draw a circle...
		while (f < 2 * PI) {
			// calculate x, y from a vector with known length and angle
			x = g * cos(f);
			y = g * sin(f);
			voxie->drawVox(x + pos.x, y + pos.y, pos.z, 0x00ff00);
			f += z;
		}

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.8, -0.2, -.3 }, point3d{ -.40, .2,.3 }, (char*)"Using many drawVox() calls to\ndraw a circle", 0); cbNo++; }


		// draw a cylinder shape using drawline 
		pos = { -0.15, 0, float(-voxie->getAspectZ() + 0.2) };								// the position of the cylinder's top
		pos2 = { -0.15,0, float(voxie->getAspectZ() - 0.1) };								// the position of the cylinder's base
		f = 0;												// f is the angle 
		g = 0.2;											// is the radius the overall size of the circle 
		z = 0.03 * (fillMode + 1);							// z is the resolution -- how far the voxels are apart this is the distance between the voxels
		col = 0xff00ff; // the colour of the cylinder
		// formula to draw a circle...
		while (f < 2 * PI) {
			// calculate x, y from a vector with known length and angle
			x = g * cos(f);
			y = g * sin(f);

			// draw the base
			voxie->drawLine(x + pos.x, y + pos.y, pos2.z, pos.x, pos.y, pos2.z, col); // like drawing a circle but having each outer point draw a line to the centre
			// draw the top			
			voxie->drawLine(x + pos.x, y + pos.y, pos.z, pos.x, pos.y, pos.z + (sin(voxie->getTime()) / 8), col); // here we are getting the sine of voxie->getTime() (an inbuilt timer that is always going up to modulate the hieght a simple way to create movement
			// connect the base and top with a line on the outside
			voxie->drawLine(x + pos.x, y + pos.y, pos.z, x + pos.x, y + pos.y, pos2.z, col); // the the lines around the outside of the centre connecting the top the bottom

			f += z;
		}

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.35, -0.2, -.3 }, point3d{ .05, .2,.3 }, (char*)"Using many drawLine() calls create\na cylinder. Uses a \nsin(time) trick for movement", 0); cbNo++; }

		/*
		  voxel and lines are the most basic. The next basic shape is a sphere. voxie->drawSphere()

		*/

		// to draw a sphere function needs a position, a radius (the size) the fillmode and the colour... the fill mode can be 0 for surface and 1 for a solid

		// draw a red non solid sphere using a point3D for position and 0.15 for the radius 
		pos = { .25, 0, -.15 };
		voxie->drawSphere(pos, .15, 0.25, 0xff0000);
		// draw a red solid sphere using a 3 floats for position and 0.15 for the radius 
		voxie->drawSphere(0.25, 0, 0.20, .15, 1, 0xff0000);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ .10, -0.15, -.3 }, point3d{ .4, .15,0 }, (char*)"single drawSphere() using a point3D\nfillmode is set to 0", 0); cbNo++; }
		if (gInited == 0) { colBoxes[cbNo].update(point3d{ .10, -0.15, 0.1 }, point3d{ .4, .15,0.3 }, (char*)"single drawSphere() using 3 floats\nfillmode is set to 1", 0); cbNo++; }

		// draw a series of smaller spheres inside a large sphere and slowly tween the colour from one colour to the next using the voxie->tweenCol() function
		pos = { 0.7, 0, 0 };
		f = .25; // radius
		col = 0xff0000;//voxie->scrollCol(45);
		for (i = 0; i < 20; i++) {
			col = voxie->tweenCol(col, 20, 0x00ffff); /*voxie->scrollCol(0) */ // voxie->tweenCol returns a a colour that is one step close to the desintation colour... the middle parameter is the 'speed' how much change every step
			voxie->drawSphere(pos, f, 0, col);
			f -= 0.012;
		}

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ .40, -0.25, -.3 }, point3d{ .95, .25,0.3 }, (char*)"Calling many drawSphere() functions\nwith a slightly decreased radius\nusing tweenCol() for colour change", 0); cbNo++; }

		// draw a box / quadrilateral put in the left top forward coordinates trhen the right, bottom back coordinates, the fillMode and finally the colour
		voxie->drawBox(-0.9, .35, -0.15, -0.6, 0.65, .15, 0, 0xff0000);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.95, 0.30, -.3 }, point3d{ -.55, .70,0.3 }, (char*)"drawBox() fillmode 0 + 6 positions:\n(left,top,forward,right,down,bottom)\nfillmode 0 is just vertices", 0); cbNo++; }


		// alternatively you can pass the positional data as floats	
		pos = { -.5, 0.35, -0.15 };
		pos2 = { -.2, 0.65, 0.15 };
		voxie->drawBox(pos, pos2, 1, 0x00ff00);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -.55, 0.30, -.3 }, point3d{ -.15, .70,0.3 }, (char*)"drawBox() fillmode 1 + 6 positions:\n(using 2 point3Ds for position)\nfillmode 1 is wireframe", 0); cbNo++; }
		// if you want to be able to rotate shapes you can use the drawCube function which allows for full vectors
		pos.x = -0.1; 	pos.y = 0.35; 		pos.z = -0.15;
		rVec.x = 0.3; 	rVec.y = 0; 		rVec.z = 0;
		dVec.x = 0; 	dVec.y = 0.3;		dVec.z = 0;
		fVec.x = 0; 	fVec.y = 0; 		fVec.z = 0.3;

		//		voxie->rotVex(0.7853982, &rVec, &dVec); // rotate vectors by .785 radians

		voxie->drawCube(&pos, &rVec, &dVec, &fVec, 2, 0x0000ff);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ -0.15, 0.30, -.3 }, point3d{ .25, .70,0.3 }, (char*)"drawCube() fillmode 2 this function\nuses vectors & offers rotations\nfillmode 2 is surfaces", 0); cbNo++; }

		pos.x = 0.45; 	pos.y = 0.30; 		pos.z = -0.15;
		rVec.x = 0.3; 	rVec.y = 0; 		rVec.z = 0;
		dVec.x = 0; 	dVec.y = 0.3;		dVec.z = 0;
		fVec.x = 0; 	fVec.y = 0; 		fVec.z = 0.3;

		//voxie->rotVex(cos(voxie->getTime() ) * 3, &rVec, &dVec);
		//voxie->rotVex(cos(voxie->getTime()) * 3, &fVec, &dVec);
		//voxie->rotVex(cos(voxie->getTime()) * 3, &rVec, &fVec);
		voxie->rotVexD(45, &rVec, &dVec); // rotate vectors by 45 degrees
		voxie->drawCube(&pos, &rVec, &dVec, &fVec, 3, 0xff00ff);

		if (gInited == 0) { colBoxes[cbNo].update(point3d{ 0.25, 0.30, -.3 }, point3d{ .65, .725,0.3 }, (char*)"drawCube() fillmode 3 this function\nuses rotVexD() to rotate 45' degrees\nfillmode 3 is solid", 0); cbNo++; }


		/*
		pos2.x = pos.x + ((rVec.x + rVec.y + rVec.z) / 2);
		pos2.y = pos.y + ((dVec.x + dVec.y + dVec.z) / 2);
		pos2.z = pos.z + ((fVec.x + fVec.y + fVec.z) / 2);


		voxie->drawSphere(pos2, .05, 1, 0xff0000);
		*/

		voxie->drawCone(0.85, 0.50, -0.25, 0.12, 0.85, 0.50, -0.05, 0.02, 0, 0xffff00);

		pos = { 0.85, 0.5, .15 };
		pos2 = { 0.85, 0.5, .35 };
		voxie->drawCone(pos, 0.12, pos2, 0.02, 1, 0xffff00);


		if (gInited == 0) { colBoxes[cbNo].update(point3d{ 0.7, 0.35, -.40 }, point3d{ .97, .65,-0.05 }, (char*)"drawCone() creates a cone or capsule \nuses 6 positions & 2 radii.\nFillmode 0 & 6 floats for pos", 0); cbNo++; }
		if (gInited == 0) { colBoxes[cbNo].update(point3d{ 0.7, 0.35, 0.05 }, point3d{ .97, .65,0.35 }, (char*)"drawCone() creates a cone or capsule \nuses 6 positions & 2 radii.\nFillmode 1 & 2 point3Ds for pos", 0); cbNo++; }


		break;


	case TEXURED_PLANES:
			titleText = (char*)"Textured Plane";
			message = (char*)"drawQuad() can be render a texture as a 3D plane.\nsupports .PNG,.JPEG,.GIF\nMove cursor + buttons to adjust angles.";
			if (gInited == 0) {

				pos2 = { 0,0,0 };
				f = 0; // acts as X axis rotation
				g = 0; // acts as Y axis rotation
				gInited = 1;
			}


			if (voxie->getNavButtonOnDown(0, NAV_RIGHT_BUTTON) || voxie->getMouseButtonOnDown(MOUSE_RIGHT_BUTTON)) {
				pos2 = { 0,0,0 };
				f = 0; // acts as X axis rotation
				g = 0; // acts as Y axis rotation
			}

			if (voxie->getNavButtonIsDown(0, NAV_LEFT_BUTTON) || voxie->getMouseButtonIsDown(MOUSE_LEFT_BUTTON)) {
				
				g += voxie->getNavAngleDeltaAxis(0, 0) + ( voxie->getMouseXDelta() * 0.25 );
				f += voxie->getNavAngleDeltaAxis(0, 1) + (voxie->getMouseYDelta() * 0.25  );

			}
		
			voxie->drawQuad((char*)"assets/plane.png", &pos2, .5, .6, 0, f, g, 0x404040, 1, 1);

			// mouse click hold and drag to draw a plane
			// hover over and hold 1st button to drag 
			// hover over and hold 2nd button adjust hang + vang (scroll wheel to adjust scale)
			// drag to bin to delete... 



			break;
	case MODELS:
		titleText = (char*)"Drawing 3D Models";
		message = (char*)"drawModel() can be render 3D models.\nsupports .OBJ,.STL,.KV6,.PLY filetypes\nMove cursor + buttons to adjust model.";

		textWidth = { float(fontSize / 2 * .66), 0, 0 };
		textHeight = { 0, float(fontSize / 2) , 0 };

		if (gInited == 0) { //define all the points / variables for rendering a 3D model

			scale = 1;
			modelPos = { 0,0,0 };
			modelRightVec = { 1,0,0 };
			modelDownVec = { 0,1,0 };
			modelForwardVec = { 0,0,1 };
			col = 0x404040; // for a model to show its 'natural' colour use 0x404040 higher values will tint the colour


			voxie->rotVexD(90, &modelDownVec, &modelForwardVec);
			voxie->rotVexD(-90, &modelRightVec, &modelDownVec);
		}


		// to apply scale to a model get the r,d and f vectors and times them by the scale

		// quick way to copy points from one to another..
		memcpy(&rVec, &modelRightVec, sizeof(point3d));
		memcpy(&dVec, &modelDownVec, sizeof(point3d));
		memcpy(&fVec, &modelForwardVec, sizeof(point3d));


		// use the point3d.scale function (times all the points by the scale factor) 
		rVec.scale(scale); // .scale function is same as rVec.x *= scale; rVec.y *= scale; rVec.z *= scale; 
		dVec.scale(scale);
		fVec.scale(scale);


		// draw mode == using the 'xVec' variables that have the scale info
		voxie->drawModel((char*)"assets/AntModel.stl", &modelPos, &rVec, &dVec, &fVec, col);
		voxie->drawSphere(modelPos, 0.05, 0, 0xff0000);

		// rotation controls	

		// using the space mouse to rotate an object
		if (voxie->getNavButtonIsDown(0, NAV_LEFT_BUTTON)) {
			f = .02; // move speed
		//	modelPos.x += voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f;
		//	modelPos.y += voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f;
		//	modelPos.z += voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f;

			// use the point3d amend function to add all these values together..
			modelPos.amend(voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f, voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f, voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f);
			voxie->clipInsideVolume(&modelPos, 0.2);  // when moving an object around if you want to keep it inside the volume you can use clipInsideVolume the 2nd parameter is the magnitude offset

			pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
			voxie->drawText(&pos, &textWidth, &textHeight, 0x00ff00, "Move Model Position");

		}
		else if (voxie->getNavButtonIsDown(0, NAV_RIGHT_BUTTON)) {

			f = 1; // scale  speed / sensitivity 

			if (voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) > 0) scale -= (f * voxie->getDeltaTime());
			if (voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) < 0) scale += (f * voxie->getDeltaTime());

			scale = max(min(scale, 3), .2); // use of max and min to define the max and min values in a single line
			pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
			voxie->drawText(&pos, &textWidth, &textHeight, 0xff0000, "Adjust Model Scale");
		}

		else {

			f = 2; // rotate speed / sensitivity 

			if (voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS)) * voxie->getDeltaTime(), &modelRightVec, &modelDownVec); } // yaw left and right
			if (voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS)) * voxie->getDeltaTime(), &modelDownVec, &modelForwardVec); } // pitch forward and back
			if (voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS)) * voxie->getDeltaTime(), &modelRightVec, &modelForwardVec); } // roll the object

		}
		voxie->reportNav(50, 150, false); // use reportNav to how a nav report on the touch screen



		// mouse controls
		if (voxie->getMouseButtonIsDown(MOUSE_LEFT_BUTTON)) {
			f = .001; // move speed
		//	modelPos.x += voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f;
		//	modelPos.y += voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f;
		//	modelPos.z += voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f;

			// use the point3d amend function to add all these values together..
			modelPos.amend(voxie->getMouseXDelta() * f, voxie->getMouseYDelta() * f, voxie->getMouseZDelta() * f);
			voxie->clipInsideVolume(&modelPos, 0.2);  // when moving an object around if you want to keep it inside the volume you can use clipInsideVolume the 2nd parameter is the magnitude offset
			pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
			voxie->drawText(&pos, &textWidth, &textHeight, 0x00ff00, "Move Model Position");

		}
		else if (voxie->getMouseButtonIsDown(MOUSE_RIGHT_BUTTON)) {

			f = 0.7; // scale  speed / sensitivity 

			if (voxie->getMouseZDelta() > 0) scale -= (f * voxie->getDeltaTime());
			if (voxie->getMouseZDelta() < 0) scale += (f * voxie->getDeltaTime());

			scale = max(min(scale, 3), .2); // use of max and min to define the max and min values in a single line
			pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
			voxie->drawText(&pos, &textWidth, &textHeight, 0xff0000, "Adjust Model Scale");
		}

		else {

			f = .05; // rotate speed / sensitivity 

			if (voxie->getMouseXDelta() > 0 || voxie->getMouseXDelta() < 0) { voxie->rotVex((f * voxie->getMouseXDelta()) * voxie->getDeltaTime(), &modelRightVec, &modelDownVec); } // yaw left and right
			if (voxie->getMouseYDelta() > 0 || voxie->getMouseYDelta() < 0) { voxie->rotVex((f * voxie->getMouseYDelta()) * voxie->getDeltaTime(), &modelDownVec, &modelForwardVec); } // pitch forward and back
			if (voxie->getMouseZDelta() > 0 || voxie->getMouseZDelta() < 0) { voxie->rotVex((f * voxie->getMouseZDelta()) * voxie->getDeltaTime(), &modelRightVec, &modelForwardVec); } // roll the object

		}


		pos = { float(-voxie->getAspectX() + .1), 0.3, -voxie->getAspectZ() };
		voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "Model Scale : %1.2f Model Colour %0x", scale, col);
		pos.y += 0.07;
		voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "Model Pos X: %1.2f, Y: %1.2f, Z: %1.2f", modelPos.x, modelPos.y, modelPos.z);
		pos.y += 0.07;
		voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "R Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelRightVec.x, modelRightVec.y, modelRightVec.z);
		pos.y += 0.07;
		voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "D Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelDownVec.x, modelDownVec.y, modelDownVec.z);
		pos.y += 0.07;
		voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "F Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelForwardVec.x, modelForwardVec.y, modelForwardVec.z);

		break;

	case TEXURED_MESH:



		if (gInited == 0) {


			titleText = (char*)"Draw Mesh";

			// the poltex_t file time contains x,y,z positional data, a color value and a u and v for texture

			// set up the default vertice and mesh

			//  vertice a 3d point in space

			// draw our rainbow colored polygon. This uses

			switch (choice) {

			case 0:
			default:
				i = 0;
				j = 0;
				vertice1[j].x = -0.4; vertice1[j].y = -0.8; vertice1[j].z = -0.2; vertice1[j].col = 0xff0000;
				j++;
				vertice1[j].x = -0.4; vertice1[j].y = -0.4; vertice1[j].z = +0.2; vertice1[j].col = 0x00ff00;
				j++;
				vertice1[j].x = +0.4; vertice1[j].y = -0.8; vertice1[j].z = +0.2; vertice1[j].col = 0x0000ff;
				j++;
				vertice1[j].x = +0.4; vertice1[j].y = -0.4; vertice1[j].z = -0.2; vertice1[j].col = 0xff00ff;

				// setup the triangles... a -1 in the array means move onto next triangle as Triangles and quads are supports

				mesh1[i++] = 0; mesh1[i++] = 1; mesh1[i++] = 2; mesh1[i++] = -1; /*-1 = end of polygonal facet*/
				mesh1[i++] = 1; mesh1[i++] = 0; mesh1[i++] = 3; mesh1[i++] = -1;
				mesh1[i++] = 2; mesh1[i++] = 1; mesh1[i++] = 3; mesh1[i++] = -1;
				mesh1[i++] = 0; mesh1[i++] = 2; mesh1[i++] = 3; mesh1[i++] = -1;

				verticeNo = j + 1;
				meshNo = i;
				meshDrawMode = 16 + fillMode;
				sprintf_s(textureFName, "");
					
				break;


			case 1:
				i = 0;
				j = 0;
				vertice1[j].x = -0.75; vertice1[j].y = +0.50; vertice1[j].z = 0.25;  vertice1[j].u = 0; vertice1[j].v = 0;
				j++;
				vertice1[j].x = 0.00; vertice1[j].y = -0.75; vertice1[j].z = 0.25;	  vertice1[j].u = 1; vertice1[j].v = 0;
				j++;
				vertice1[j].x = +0.75; vertice1[j].y = +0.50; vertice1[j].z = 0.25;	  vertice1[j].u = 1; vertice1[j].v = 1;
				j++;
				vertice1[j].x = -0.25; vertice1[j].y = +0.25; vertice1[j].z = 0.25;	 vertice1[j].u = 0; vertice1[j].v = 1;
				j++;
				vertice1[j].x = 0.00; vertice1[j].y = -0.25; vertice1[j].z = 0.25;	 vertice1[j].u = 1; vertice1[j].v = 0;
				j++;
				vertice1[j].x = +0.25; vertice1[j].y = +0.25; vertice1[j].z = 0.25;	 vertice1[j].u = 0; vertice1[j].v = 1;

				mesh1[i++] = 0; mesh1[i++] = 1; mesh1[i++] = 2; mesh1[i++] = -2; /*-2 = end loop*/
				mesh1[i++] = 3; mesh1[i++] = 4; mesh1[i++] = 5; mesh1[i++] = -1; /*-1 = end polygonal facet*/

				verticeNo = j + 1;
				meshNo = i;
				meshDrawMode = fillMode;
				sprintf_s(textureFName, "assets/plane.png");
				break;
			case 2:
				// draw a plane and texture with a quad
				/*
				 *  0 ---- 1
				 *  |      |
				 *  |      |
				 *  3 ---- 2
				 */

				i = 0;
				j = 0;
				vertice1[j].x =-0.7; vertice1[j].y =-0.7; vertice1[j].z = 0; vertice1[j].col = 0xff0000; // LTU
				j++;
				vertice1[j].x =-0.3; vertice1[j].y =-0.7; vertice1[j].z = 0; vertice1[j].col = 0xff0000; //  RTU
				j++;
				vertice1[j].x =-0.7; vertice1[j].y =-0.3; vertice1[j].z = 0; vertice1[j].col = 0xff0000; // LBU
				j++;
				vertice1[j].x =-0.3; vertice1[j].y =-0.3; vertice1[j].z = 0; vertice1[j].col = 0xff0000; // RBU

				j++;
				vertice1[j].x = 0.7; vertice1[j].y = 0.7; vertice1[j].z = 0; vertice1[j].col = 0x00ff00; // LTU
				j++;
				vertice1[j].x = 0.3; vertice1[j].y = 0.7; vertice1[j].z = 0; vertice1[j].col = 0x00ff00; //  RTU
				j++;
				vertice1[j].x = 0.7; vertice1[j].y = 0.3; vertice1[j].z = 0; vertice1[j].col = 0x00ff00; // LBU
				j++;
				vertice1[j].x = 0.3; vertice1[j].y = 0.3; vertice1[j].z = 0; vertice1[j].col = 0x00ff00; // RBU
			
		
			   // using a QUAD instead of two triangles - Since we support it  

			   mesh1[i++] = 0; mesh1[i++] = 1; mesh1[i++] = 3; mesh1[i++] = 2;  mesh1[i++] = -1; /*-1 = end of polygonal facet*/

				// making the mesh using two triangles

				mesh1[i++] = 4; mesh1[i++] = 5; mesh1[i++] = 7; mesh1[i++] = -1; /*-1 = end of polygonal facet*/
			    mesh1[i++] = 4; mesh1[i++] = 7; mesh1[i++] = 6; mesh1[i++] = -1; /*-1 = end of polygonal facet*/

				verticeNo = j + 1;
				meshNo = i;
				meshDrawMode = 16 + fillMode;
				sprintf_s(textureFName, "");

				break;
			case 3:
				// draw a diamond 
				i = 0;
				j = 0;
				vertice1[j].x = -0.3; vertice1[j].y = -0.3; vertice1[j].z = 0.0; vertice1[j].col = 0xff0000; // LTU
				j++;
				vertice1[j].x = 0.3; vertice1[j].y = -0.3; vertice1[j].z = 0.0; vertice1[j].col = 0xff0000; //  RTU
				j++;
				vertice1[j].x = +0.3; vertice1[j].y = 0.3; vertice1[j].z = 0.0; vertice1[j].col = 0xff0000; // RBU
				j++;
				vertice1[j].x = -0.3; vertice1[j].y = 0.3; vertice1[j].z = 0.0; vertice1[j].col = 0xff0000; // LBU
				
				j++;
				// top middle point
				vertice1[j].x = 0; vertice1[j].y = 0; vertice1[j].z = -0.3; vertice1[j].col = 0xff00ff; // middle

				j++;
				// bottom middle point
				vertice1[j].x = 0; vertice1[j].y = 0; vertice1[j].z = 0.3; vertice1[j].col = 0x00ff00; // middle


				//draw top spike

				mesh1[i++] = 0; mesh1[i++] = 4; mesh1[i++] = 1; mesh1[i++] = -1;
				mesh1[i++] = 1; mesh1[i++] = 4; mesh1[i++] = 2; mesh1[i++] = -1;
				mesh1[i++] = 2; mesh1[i++] = 4; mesh1[i++] = 3; mesh1[i++] = -1;
				mesh1[i++] = 3; mesh1[i++] = 4; mesh1[i++] = 0; mesh1[i++] = -1;

				// draw the base

				mesh1[i++] = 0; mesh1[i++] = 1; mesh1[i++] = 2; mesh1[i++] = -1;
				mesh1[i++] = 2; mesh1[i++] = 3; mesh1[i++] = 0; mesh1[i++] = -1; 

				// draw the bottom spike

				mesh1[i++] = 0; mesh1[i++] = 5; mesh1[i++] = 1; mesh1[i++] = -1; 
				mesh1[i++] = 1; mesh1[i++] = 5; mesh1[i++] = 2; mesh1[i++] = -1;
				mesh1[i++] = 2; mesh1[i++] = 5; mesh1[i++] = 3; mesh1[i++] = -1;
				mesh1[i++] = 3; mesh1[i++] = 5; mesh1[i++] = 0; mesh1[i++] = -1;
			
		
				verticeNo = j + 1;
				meshNo = i;
				meshDrawMode = 16 + fillMode;
				sprintf_s(textureFName, "");

				break;


			}
			
		}

	
	


		message = (char*)"drawMesh() can be render a 3D Mesh.\nA mesh made of vertices and faces\nClick and drag a point.";



		f = -1;
		// code to label the vertices
		for (j = 0; j < verticeNo; j++) {

			pos.x = vertice1[j].x;
			pos.y = vertice1[j].y;
			pos.z = vertice1[j].z;


			// code to collision check the cursor and 
			if (voxie->sphereCollideChk(&cursor, 0.1, &pos, 0.1)) {

				if (voxie->getMouseButtonIsDown(0) || voxie->getNavButtonIsDown(0, 0)) {
				f = j;	
				message = (char*)"Click and drag to adjust a vertice";
				}
			
				voxie->drawSphere(pos, 0.05, 1, 0x00ffff);
			}
			else {
			
				voxie->drawSphere(pos, 0.05, 1, 0x0000ff);
			}
			
			pos.x -= 0.1;
			voxie->drawTextSimp(&pos, 0.07, 0.07, 0, -30, 0, 0xffff00, (char*)"%d", j);


		}

		if (f != -1) {

			vertice1[(int)f].x = cursor.x;
			vertice1[(int)f].y = cursor.y;
			vertice1[(int)f].z = cursor.z;

		}





		// to draw a polygon

		/*

		 //  texnam: texture filename; pointer to tiletype structure if (flags&8); null for no texture
		 //          Be sure to fill .u and .v fields of poltex_t when using a texture.
		 //      vt: list of vertices
		 //     vtn: number of vertices
		 //    mesh: list of facets as vertex indices or -1 to end current primitive and start next one
		 //          For complex polygons, insert -2 at end of contour (if more loops on same plane).
		 //             Use -1 to end plane
		 //   meshn: number of entries in mesh array
		 //   flags: +0:dots, +1:lines, +2:surfaces, +3:solid, +8:texnam is tiletype * instead of filename
		 //          +16:scale col by vt[i].col for surfaces
		 //     col: 24-bit color
   //
   //NOTE: All rendering functions eventually end up calling this general rendering function, at
   //   least internally. The only exceptions to this rule are: voxie_drawsph() and voxie_drawcone()
   //   when using solid filled.
void voxie_drawmeshtex (voxie_frame_t *vf, char *texnam, const poltex_t *vt, int vtn,
						const int *mesh, int meshn, int flags, int col);


		*/

		voxie->drawMesh(textureFName, vertice1, verticeNo, mesh1, meshNo, meshDrawMode, 0x404040); /* (for colour of vertices to work need to have fill mode bit postitive at 16+  (0010000b) ) */
/*
		
		i = 0;
	
		//       ^ 0
		//      / \
		//     / . \
		//    / /_\ \
		//   /       \
		//  /---------\

		// more complex mesh and texture example

		/*
		vt[0].x = -0.75; vt[0].y = +0.50; vt[0].z = 0.25;  vt[0].u = 0; vt[0].v = 0;
		vt[1].x = 0.00; vt[1].y =-0.75; vt[1].z = 0.25;	  vt[1].u = 1; vt[1].v = 0;
	    vt[2].x =+0.75; vt[2].y =+0.50; vt[2].z = 0.25;	  vt[2].u = 1; vt[2].v = 1;
		vt[3].x = -0.25; vt[3].y = +0.25; vt[3].z = 0.25;	 vt[3].u = 0; vt[3].v = 1;
	    vt[4].x = 0.00; vt[4].y =-0.25; vt[4].z = 0.25;	 vt[4].u = 1; vt[4].v = 0;
	    vt[5].x =+0.25; vt[5].y =+0.25; vt[5].z = 0.25;	 vt[5].u = 0; vt[5].v = 1;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -2; /*-2 = end loop*/
		//mesh[i++] = 3; mesh[i++] = 4; mesh[i++] = 5; mesh[i++] = -1; /*-1 = end polygonal facet*/

		/*
		voxie->drawMesh((char*)"assets/plane.png", vt, 6, mesh, 8, fillMode, 0x404040); // use flag 8 to use texture
		*/

		break;
		case HEIGHTMAP:
			titleText = (char*)"Drawing HeightMap";
			message = (char*)"drawModel() can be render 3D models.\nsupports .OBJ,.STL,.KV6,.PLY filetypes\nMove cursor + buttons to adjust model.";


			textWidth = { float(fontSize / 2 * .66), 0, 0 };
			textHeight = { 0, float(fontSize / 2) , 0 };

			if (gInited == 0) {

				scale = 1;
				modelPos = { (float)-voxie->getAspectX() + (float)0.1, (float)-voxie->getAspectY() + (float)0.1,-0.1 };
				modelRightVec = { 1,0,0 };
				modelDownVec = { 0,1,0 };
				modelForwardVec = { 0,0,voxie->getAspectZ()  };
				col = 0x404040; // for a model to show its 'natural' colour use 0x404040 higher values will tint the colour
			
			}


			// to apply scale to a model get the r,d and f vectors and times them by the scale

			// quick way to copy points from one to another..
			memcpy(&rVec, &modelRightVec, sizeof(point3d));
			memcpy(&dVec, &modelDownVec, sizeof(point3d));
			memcpy(&fVec, &modelForwardVec, sizeof(point3d));


			// use the point3d.scale function (times all the points by the scale factor) 
			rVec.scale(scale); // .scale function is same as rVec.x *= scale; rVec.y *= scale; rVec.z *= scale; 
			dVec.scale(scale);
			fVec.scale(scale);


			// draw mode == using the 'xVec' variables that have the scale info

			voxie->drawSphere(modelPos, 0.05, 0, 0xff0000);
			voxie->drawSphere(rVec, 0.05, 0, 0x00ff00);
			voxie->drawSphere(dVec, 0.05, 0, 0xff00ff0);
			voxie->drawSphere(fVec, 0.05, 0, 0x00ffff);

//		colorkey: ARGB 32 - bit color to be transparent(supported in nearest mode only)
	//	flags: (1 << 0) : reserved(height dither now controlled by vw.dither)
			//          (1<<1): 0=nearest filter   , 1=bilinear filter  (recommended)
			//          (1<<2): 0=color dither off , 1=color dither on  (recommended)
			//          (1<<3): 0=filnam is filename string, 1=filnam is tiletype * or pointer to 2d array.
			//          (1<<4): 0=texture clamp    , 1=texture wrap
			//          (1<<5): 0=8-bit height     , 1=mapzen.com terrarium style height

			voxie->drawHeightMap((char*)"assets/heightmap.png", &modelPos, &rVec, &dVec, &fVec, 0x1234567, 0, (1 << 1 ) + (1 << 2));
		//	voxie->drawHeightMap((char*)"assets/canyon.png", &modelPos, &rVec, &dVec, &fVec, 0x1234567, 0, (1 << 1) + (1<<2));
			// rotation controls	

		// using the space mouse to rotate an object
			if (voxie->getNavButtonIsDown(0, NAV_LEFT_BUTTON)) {
				f = .02; // move speed
			//	modelPos.x += voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f;
			//	modelPos.y += voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f;
			//	modelPos.z += voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f;

				// use the point3d amend function to add all these values together..
				modelPos.amend(voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f, voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f, voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f);
				voxie->clipInsideVolume(&modelPos, 0.2);  // when moving an object around if you want to keep it inside the volume you can use clipInsideVolume the 2nd parameter is the magnitude offset

				pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
				voxie->drawText(&pos, &textWidth, &textHeight, 0x00ff00, "Move Model Position");

			}
			else if (voxie->getNavButtonIsDown(0, NAV_RIGHT_BUTTON)) {

				f = 100; // scale  speed / sensitivity 

				if (voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) > 0) modelForwardVec.z -= (f * voxie->getDeltaTime());
				if (voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) < 0) modelForwardVec.z  += (f * voxie->getDeltaTime());

				//scale = max(min(scale, 3), .2); // use of max and min to define the max and min values in a single line
				//pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
				voxie->drawText(&pos, &textWidth, &textHeight, 0xff0000, "Adjust Model Scale");
			}

			else {

				f = 2; // rotate speed / sensitivity 

				if (voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_X_AXIS)) * voxie->getDeltaTime(), &modelRightVec, &modelDownVec); } // yaw left and right
				if (voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_Y_AXIS)) * voxie->getDeltaTime(), &modelDownVec, &modelForwardVec); } // pitch forward and back
				if (voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS) > 0 || voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS) < 0) { voxie->rotVex((f * voxie->getNavAngleDeltaAxis(0, NAV_Z_AXIS)) * voxie->getDeltaTime(), &modelRightVec, &modelForwardVec); } // roll the object

			}
			voxie->reportNav(50, 150, false); // use reportNav to how a nav report on the touch screen



			// mouse controls
			if (voxie->getMouseButtonIsDown(MOUSE_LEFT_BUTTON)) {
				f = .001; // move speed
			//	modelPos.x += voxie->getNavDirectionDeltaAxis(0, NAV_X_AXIS) * f;
			//	modelPos.y += voxie->getNavDirectionDeltaAxis(0, NAV_Y_AXIS) * f;
			//	modelPos.z += voxie->getNavDirectionDeltaAxis(0, NAV_Z_AXIS) * f;

				// use the point3d amend function to add all these values together..
				modelPos.amend(voxie->getMouseXDelta() * f, voxie->getMouseYDelta() * f, voxie->getMouseZDelta() * f);
				voxie->clipInsideVolume(&modelPos, 0.2);  // when moving an object around if you want to keep it inside the volume you can use clipInsideVolume the 2nd parameter is the magnitude offset
				pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
				voxie->drawText(&pos, &textWidth, &textHeight, 0x00ff00, "Move Model Position");

			}
			else if (voxie->getMouseButtonIsDown(MOUSE_RIGHT_BUTTON)) {

				f = 0.7; // scale  speed / sensitivity 

				if (voxie->getMouseZDelta() > 0) { modelForwardVec.z -= (f * voxie->getDeltaTime()); modelPos.z += (f * voxie->getDeltaTime());				}
				if (voxie->getMouseZDelta() < 0) { modelForwardVec.z += (f * voxie->getDeltaTime()); modelPos.z -= (f * voxie->getDeltaTime());  			}


			


				//modelForwardVec.z = max(min(scale, 3), .2); // use of max and min to define the max and min values in a single line
				pos = { float(-voxie->getAspectX() + .1), -0.8, -voxie->getAspectZ() };
				voxie->drawText(&pos, &textWidth, &textHeight, 0xff0000, "Adjust Model Scale");
			}

			else {

				f = .05; // rotate speed / sensitivity 

				if (voxie->getMouseXDelta() > 0 || voxie->getMouseXDelta() < 0) { voxie->rotVex((f * voxie->getMouseXDelta()) * voxie->getDeltaTime(), &modelRightVec, &modelDownVec); } // yaw left and right
				if (voxie->getMouseYDelta() > 0 || voxie->getMouseYDelta() < 0) { voxie->rotVex((f * voxie->getMouseYDelta()) * voxie->getDeltaTime(), &modelDownVec, &modelForwardVec); } // pitch forward and back
				if (voxie->getMouseZDelta() > 0 || voxie->getMouseZDelta() < 0) { voxie->rotVex((f * voxie->getMouseZDelta()) * voxie->getDeltaTime(), &modelRightVec, &modelForwardVec); } // roll the object

			}


			pos = { float(-voxie->getAspectX() + .1), 0.3, -voxie->getAspectZ() };
			voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "Model Scale : %1.2f Model Colour %0x", scale, col);
			pos.y += 0.07;
			voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "Model Pos X: %1.2f, Y: %1.2f, Z: %1.2f", modelPos.x, modelPos.y, modelPos.z);
			pos.y += 0.07;
			voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "R Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelRightVec.x, modelRightVec.y, modelRightVec.z);
			pos.y += 0.07;
			voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "D Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelDownVec.x, modelDownVec.y, modelDownVec.z);
			pos.y += 0.07;
			voxie->drawText(&pos, &textWidth, &textHeight, 0xffff00, "F Vector X: %1.2f, Y: %1.2f, Z: %1.2f", modelForwardVec.x, modelForwardVec.y, modelForwardVec.z);


			/*
			
				if (!img.f) { img.x = 512; img.y = 512; img.p = (INT_PTR)img.x*4; img.f = (INT_PTR)malloc(img.p*img.y); }

		for(y=0;y<img.y;y++)
			for(x=0;x<img.x;x++)
			{
				int rgb24 = (128<<16)+(y<<8)+x, hgt8 = sqrt(256.f*256.f - (x-256)*(x-256) - (y-256)*(y-256));
				*(int *)(img.p*y + (x<<2) + img.f) = rgb24 + (hgt8<<24);
			}


		rr.x = vw.aspy*2.f; dd.x =         0.f; ff.x =          0.f; pp.x = rr.x*-.5f;
		rr.y =         0.f; dd.y = vw.aspy*2.f; ff.y =          0.f; pp.y = dd.y*-.5f;
		rr.z =         0.f; dd.z =         0.f; ff.z = vw.aspz*-2.f; pp.z = ff.z*-.5f;
		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,+vw.aspx,+vw.aspy,+vw.aspz);
	/	voxie_drawheimap(&vf,(char *)&img, &pp,&rr,&dd,&ff, 0x12345678,-1,(1<<2)/*color dither*/// +(1 << 3)/*filnam is tiletype*/);
	//		*/

			break;
	}




	switch (inputMode) {
	case 0: // mouse
		cursor = voxie->getMousePosition();
		break;
	case 2: // space Nav
		cursor = voxie->getNavPosition(0);
		break;
	}

	if (voxie->getNavButtonState(0) != 0) inputMode = 2;
	if (voxie->getMouseButtonState() != 0) inputMode = 0;


	voxie->drawCursor(&cursor, 3, 0, 0xffffff);


	// draw title
	pos = { -.75, -.95, actionBox.z };
	rVec = { fontSize, 0, 0 }; // font width
	dVec = { 0, float(fontSize * 1.5), 0 }; // font height
	voxie->drawText(&pos, &rVec, &dVec, voxie->scrollCol(0), "%s", titleText);





	// draw all idrawables

	for (int i = 0; i < cbNo; ++i) {
		if (colBoxes[i].checkCollision(cursor) && colBoxes[i].isActive()) {
			message = colBoxes[i].getMessage();
			colBoxes[i].setDrawable(true);
		}
		else {
			colBoxes[i].setDrawable(false);
		}


	}


	for (auto item : IDrawables) {
		if (item->isDrawable()) item->draw();


	}

	
	pos = { float(-voxie->getAspectX() + 0.05), .7, -voxie->getAspectZ() };
	rVec = { float((fontSize * .66)), 0, 0 }; // font width
	dVec = { 0, float((fontSize * 1.5) * .66), 0 }; // font height
	voxie->drawText(&pos, &rVec, &dVec, 0xffffff, "%s", message);

	// draw actionbox
	pos = { -actionBox.x, -actionBox.y,-actionBox.z };
	//voxie->drawBox(actionBox, pos, 1, 0xffffff);


	//voxie->reportVoxieWind(50, 100);
	voxie->debugText(20, 100, 0xffffff, -1, "Press 0 - 9 to select scene   (current scene: %d)", gCurrentScene);
	voxie->debugText(20, 110, 0xff0000, -1, "Press Q and A to change fillmode	(fillmode: %d)", fillMode);
	voxie->debugText(20, 120, 0xff0000, -1, "Press W and S to change choice		(choice: %d)", choice);
	voxie->showVPS();
	voxie->endFrame(); // the end of drawing to the volumetric display
	if (gInited == 0) gInited = 1;


}

	// quit() stops the hardware from physically moving andVec ends the breath() loop  
	voxie->quitLoop();
	delete voxie;
	return 0;
	// After the program quits the deconstructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}