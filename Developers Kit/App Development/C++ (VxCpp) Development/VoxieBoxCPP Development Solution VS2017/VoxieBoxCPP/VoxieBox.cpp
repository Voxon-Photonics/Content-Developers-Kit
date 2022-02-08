#include "stdafx.h"
#include "VxCpp.h"
#include <string>

HINSTANCE VoxieBox::hvoxie = 0;

/** \class VoxieBox
 *  VoxieBox class. All voxiebox.dll functions presented as class with additional helper functions included. 
 */

 /** VoxieBox constructor checks to see if another instance of VoxieBox has been created.
  *	If not; then loads the internal voxie_wind_t struct (vw) into memory.
  *	Then calls VoxieBox:init() & initialise the data, loads settings from voxiebox.ini
  * and voxie_menu_0.ini  
  */
VoxieBox::VoxieBox()
{
	if (success) return;

	if (voxie_load(&this->vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini.
	{
		MessageBox(0, "Error: Can't load or locate voxiebox.dll. Put voxiebox.dll in the current folder or check that voxiebox.dll is included in systems path variables", "", MB_OK); success = false;
	}
	if (voxie_init(&this->vw) < 0) //Start video and audio.
	{
		MessageBox(0, "Error: voxie_init() failed", "", MB_OK); success = false;
	}

	//check if hardware is a spinner or up/down system and adjust clip shape
	// Update : this is called too early and fails to detect a spinner... had to move it to the breath loop 
	
	//if (vw.nblades > 0)	vw.clipshape = 1;
	//else  				vw.clipshape = 0;

	success = true;
}
//! Deconstructor for VoxieBox calls VoxieBox:shutdown to stop hardware and release voxiebox.dll
VoxieBox::~VoxieBox()
{
	this->shutdown();
}

//!  Initializes and updates voxiebox.dll's voxie window (voxie_wind_t) 
/**
 * 
 *  The voxie window is the struct which holds all the settings and values to do with the volumetric display.  @see vxDataTypes.h::voxie_wind_t
 * 
 *  This function is called first time by the VoxieBox constructor. So there is no need for a developer to initialise it
 *  But can be used to update the voxie_window if the VoxieBox class's vw (voxie_wind_t) has been updated.
 *  On the first call, this function sets up the window and starts the hardware motor.
 *  On later calls, it can be used to override some parameters of the vw (voxie_window_t) structure.
 *
 *     For example:
 *       vw.useJoy = 0; voxie_init(&vw);
 *    would change the joystick input method to direct input (joyInfoEx) emulation regardless of the setting in voxiebox.ini.
 *	     if voxie_init returns -1 the function returns an error could not initialise or update
 *   @return 0 if Initialization or updating is successful. -1 if an error  
 *
 *  Note : This function always passing in the VoxieBox class's voxie_wind_t which is known as 'vw'
 *
 */
int VoxieBox::init()
{
	return voxie_init(&vw);
}


//! returns a pointer to the internal voxie_wind_t struct
voxie_wind_t * VoxieBox::getVoxieWindow()
{
	return &vw;
}

//! returns a pointer to the internal voxie_frame_t struct
voxie_frame_t * VoxieBox::getVoxieFrame()
{
	return &vf;
}

//! Overrides internal mouse position with a new point3d position
/**
 * Useful to jump / move the mouse cursor to a specific part of the display.
 *
 * @param newPos (x, y, z) positional information of where to set new mouse position
 */
void VoxieBox::setMousePosition(point3d newPos)
{
	mousePos.x = newPos.x;
	mousePos.y = newPos.y;
	mousePos.z = newPos.z;
}

//! Update the current mouse positions by applying all input deltas - called once per frame within VoxieBox::breath()
/**
 * This function is intended to update the mouse input. updates the internal mouse position and checks the input states of the buttons.
 * It is called automatically within the VoxieBox::breath() function. There wouldn't be a reason why a developer 
 * would need to call this function unless they are doing something specific with the mouse inputs
 *
 * see VoxieBox::breath()
 */
void VoxieBox::updateMousePosition()
{

	point2d a = { in.dmousx , in.dmousy  };
	a = oriCorrection(getMouseOrientation(), a.x, a.y);
	in.dmousx = a.x;
	in.dmousy = a.y;

	mousePos.x += float(in.dmousx * mouseXYSensitivity);
	mousePos.y += float(in.dmousy * mouseXYSensitivity);
	if (invertZAxis)	mousePos.z += float(-in.dmousz * mouseZSensitivity); // inverted mouse movement
	else				mousePos.z += float(in.dmousz * mouseZSensitivity);  // normal mouse movement

	if (enableMouseClip) clipInsideVolume(&mousePos);

	if (getMouseButtonOnDown(0)) startLastPressedMouse[0] = time;
	if (getMouseButtonOnDown(1)) startLastPressedMouse[1] = time;
	if (getMouseButtonOnDown(2)) startLastPressedMouse[2] = time;

	if (getMouseButtonIsDown(0)) durationLastPressedMouse[0]	= time - startLastPressedMouse[0];
	if (getMouseButtonIsDown(1)) durationLastPressedMouse[1]	= time - startLastPressedMouse[1];
	if (getMouseButtonIsDown(2)) durationLastPressedMouse[2]	= time - startLastPressedMouse[2];

	if (getMouseButtonOnUp(0))	durationLastPressedMouse[0] = 0;
	if (getMouseButtonOnUp(1))	durationLastPressedMouse[1] = 0;
	if (getMouseButtonOnUp(2))	durationLastPressedMouse[2] = 0;

	if ((getMouseButtonState() &1) == 0) durationLastPressedMouse[0] = 0;
	if ((getMouseButtonState() &2) == 0) durationLastPressedMouse[1] = 0;
	if ((getMouseButtonState() &4) == 0) durationLastPressedMouse[2] = 0;
	   
}

//! Rotate two point3d vectors a & b around their common plane, by angle expressed in radians.
void VoxieBox::rotVex(float angInRadians, point3d *a, point3d *b)
{
	float f, c, s;

	c = cos(angInRadians);
	s = sin(angInRadians);
	f = a->x;
	a->x = f * c + b->x * s;
	b->x = b->x * c - f * s;
	f = a->y;
	a->y = f * c + b->y * s;
	b->y = b->y * c - f * s;
	f = a->z;
	a->z = f * c + b->z * s;
	b->z = b->z * c - f * s;
}

//! Rotate two point3d vectors a & b around their common plane, by angle expressed in degrees
void VoxieBox::rotVexD(float angInDegrees, point3d *a, point3d *b)
{
	float f, c, s;
	float ang = (angInDegrees * PI) / 180;
	c = cos(ang);
	s = sin(ang);
	f = a->x;
	a->x = f * c + b->x * s;
	b->x = b->x * c - f * s;
	f = a->y;
	a->y = f * c + b->y * s;
	b->y = b->y * c - f * s;
	f = a->z;
	a->z = f * c + b->z * s;
	b->z = b->z * c - f * s;
}

//! Rotate two point2d vectors a & b around their common plane, by angle expressed in radians
void VoxieBox::rotVex(float angInRadians, point2d *a, point2d *b)
{
	float f, c, s;

	c = cos(angInRadians);
	s = sin(angInRadians);
	f = a->x;
	a->x = f * c + b->x * s;
	b->x = b->x * c - f * s;
	f = a->y;
	a->y = f * c + b->y * s;
	b->y = b->y * c - f * s;
	
}

//! Rotate two point2d vectors a & b around their common plane, by angle expressed in degrees 
void VoxieBox::rotVexD(float angInDegrees, point2d *a, point2d *b)
{
	float f, c, s;
	float ang = (angInDegrees * PI) / 180;
	c = cos(ang);
	s = sin(ang);
	f = a->x;
	a->x = f * c + b->x * s;
	b->x = b->x * c - f * s;
	f = a->y;
	a->y = f * c + b->y * s;
	b->y = b->y * c - f * s;
	
	
}


//! Renders a 2D textured (.png, .jpg... most image formats) quad (plane) onto the volumetric display. Useful to rendering 2D textures. Must be called between startFrame() & endFrame() functions.
/**
 * @param filename	the filename / path for the texture to load (.png, .jpg... most image formats supported) 
 * @param pos		the center position of the quad to render
 * @param width		x dimension of the quad (how wide).
 * @param height	y dimension of the quad (how high). 
 * @param hang		the horizontal angle (yaw)  . 0 is front facing. 180 is facing the back of the display. presented in degrees. 
 * @param hang		the vertical angle (pitch). 0 is horizontal facing. 90 is facing vertical. presented in degrees.
 * @param twist		how much 'twist' is in the quad. (roll) in the quad 0 is flat. presented in degrees.
 * @param col		the color value of the texture 0x404040 is the natural color from the texture anything less or more will add a tint to the texture.
 * @param u			the u value of the texture. Adjusting this stretches / the horizontal texture size. Default is 1 
 * @param u			the v value of the texture. Adjusting this stretches / the vertical texture size. Default is 1
 *
 * Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawQuad(char *filename, point3d *pos, float width, float height, float hAng, float vAng, float twist, int col = 0x404040, float uValue = 1, float vValue = 1)
{
	poltex_t vt[4];
	point3d vx, vy;
	float f, g;
	int i, mesh[4 + 1] = { 0,1,2,3,-1 }, cols[4] = { 0xff00ff,0x00ffff,0x0000ff,0xff00ff };

	hAng *= PI / 180.0;
	vAng *= PI / 180.0;
	twist *= PI / 180.0;

	vx.x = cos(hAng);
	vx.y = sin(hAng);
	vx.z = 0.f;

	f = cos(vAng);
	vy.x = -vx.y*f;
	vy.y = vx.x*f;
	vy.z = -sin(vAng);

	rotVex(twist, &vx, &vy);
		
	//voxie_drawsph(&vf, pos->x, pos->y, pos->z, .10, 0, 0x00ff00); // for debugging
	for (i = 0; i < 4; i++)
	{
		if ((i + 1) & 2) { f = -width; vt[i].u = 0.f; }
		else { f = width; vt[i].u = uValue; }
		if (!(i & 2)) { g = -height; vt[i].v = 0.f; }
		else { g = height; vt[i].v = vValue; }

		vt[i].x = vx.x*f + vy.x*g + pos->x;
		vt[i].y = vx.y*f + vy.y*g + pos->y;
		vt[i].z = vx.z*f + vy.z*g + pos->z;
		vt[i].col = 0;
	//	voxie_drawsph(&vf, vt[i].x, vt[i].y, vt[i].z, .10, 0, cols[i]); // for debugging
	}
	
	drawMesh(filename, vt, 4, mesh, 5, 2, col);
}


//! Box inside collision check. Check if a position is inside a box shape.  boxTLU = Top, Left, Up, boxBRD = bottom, right, down.
/** 
 * @param LUTpos			pointer to the Left, Up, Top position of the box. 
 * @param RDBpos			pointer to the Right, Down, Bottom position of the box.
 * @param collisionPos		the collision position to check.
 * @param showCollisionBox  For debugging set to false by default. Set to true to render the collision box onto the volumetric display. (Note : The collision check must be called within the start and end frame)
 *
 * @returns 1 if collision position is within (inside) the box otherwise returns 0
 * 
 */
int VoxieBox::boxInsideCollideChk(point3d * LUTpos, point3d * RDBpos, point3d collisionPos, bool showCollisionBox) {

	int result = 0;
	int collisionCol = 0xff0000;
	   
	if (collisionPos.x < RDBpos->x &&
		collisionPos.x > LUTpos->x
		&&
		collisionPos.y < RDBpos->y &&
		collisionPos.y > LUTpos->y
		&&
		collisionPos.z < RDBpos->z &&
		collisionPos.z > LUTpos->z
		) {
		result = 1;
	}

	if (showCollisionBox) {
		if (result) collisionCol = 0x00ff00;
		drawSphere(collisionPos, 0.025, 0, collisionCol);
		drawBox(*LUTpos, *RDBpos, 2, collisionCol);

	}

	return result;
}

//! Box inside collision check. Check if a position is inside a box shape.  boxTLU = Top, Left, Up, boxBRD = bottom, right, down.
/**
 * @param TLpos				pointer to the Top, Left position of the box.
 * @param BRpos				pointer to the Bottom, Right position of the box.
 * @param collisionPos		the collision position to check.
 * @param showCollisionBox  For debugging set to false by default. Set to true to render the collision box onto the volumetric display. (Note : The collision check must be called within the start and end frame)
 *
 * @returns 1 if collision position is within (inside) the box otherwise returns 0
 *
 */
int VoxieBox::boxInsideCollideChk2D(point2d * TLpos, point2d * BRpos, point2d * collisionPos, bool showCollisionBox)
{
	int result = 0;
	int collisionCol = 0xff0000;

	if (collisionPos->x < BRpos->x &&
		collisionPos->x > TLpos->x
		&&
		collisionPos->y < BRpos->y &&
		collisionPos->y > TLpos->y

		) {
		result = 1;
	}

	if (showCollisionBox) {
		if (result) collisionCol = 0x00ff00;
		debugDrawPix((int)collisionPos->x, (int)collisionPos->y, collisionCol);
		debugDrawBoxFill((int)TLpos->x, (int)TLpos->y, (int)BRpos->x, (int)BRpos->y, 0x003300);

	}

	return result;
}


//! Circle inside collision check. Check if a position is inside a 2D circle. Can be used for the 2D or volumetric display. 
/**
 * @param circlePos			pointer to circle's position
 * @param radius			the size of the circle's collision box
 * @param collisionPos		the collision position to check.
 * @param showCollisionBox  show Collision circle For debugging set to 0 by default. 1 is for the volumetric display. 2 is for touch screen. 
 *
 * @returns 1 if collision position is within (inside) the circle otherwise returns 0
 *
 */
int VoxieBox::ciricle2DChk(point2d * circlePos, float radius, point2d * collisionPos, int showCollisionBox)
{
	int result = 0;
	float x, y;
	float angle = 0;
	int collisionCol = 0xff0000;


	// circle 2D check
	float sidea = fabs(double(circlePos->x) - double(collisionPos->x));
	float sideb = fabs(double(circlePos->y) - double(collisionPos->y));
	sidea = sidea * sidea;
	sideb = sideb * sideb;
	float distance = (float)sqrt(double(sidea) + double(sideb));
	if (distance < radius) {

		result = 1;
	}


	switch (showCollisionBox) {
		case 1:
			if (result) collisionCol = 0x00ff00;

			while (angle < 2 * PI) {
				// calculate x, y from a vector with known length and angle
				x = radius * cos(angle);
				y = radius * sin(angle);
				voxie_drawvox(&vf, x + circlePos->x, y + circlePos->y, 0, 0x00ff00);
				angle += 0.05;
			}

			voxie_drawvox(&vf, collisionPos->x, collisionPos->y, 0, collisionCol);
		break;
		case 2:
			if (result) collisionCol = 0x00ff00;
			debugDrawPix((int)collisionPos->x, (int)collisionPos->y, collisionCol);
			debugDrawCircle((int)circlePos->x, (int)circlePos->y, radius, 0x003300);
			break;

	}

	return result;
}
//! Box to Box collision check. Check if two box shapes colliding.  boxTLU = Top, Left, Up, boxBRD = bottom, right, down.
/**
 * @param LUTpos			pointer to the Left, Up, Top position of the first box.
 * @param RDBpos			pointer to the Right, Down, Bottom position of the first box.
 * @param LUTpos			pointer to the Left, Up, Top position of the second box.
 * @param RDBpos			pointer to the Right, Down, Bottom position of the second  box.
 * @param showCollisionBox  For debugging set to false by default. Set to true to render the collision box onto the volumetric display. (Note : The collision check must be called within the start and end frame)
 *
 * @returns 1 if collision position is within (inside) the box otherwise returns 0
 * To check to just see if a position is inside of a box use boxInsideCollidChk()
 */

int VoxieBox::boxCollideChk(point3d * LUTpos1, point3d * RDBpos1, point3d * LUTpos2, point3d * RDBpos2, bool showCollisionBox)
{
	int result = 0;
	int collisionCol = 0xff0000;


	if (RDBpos1->x > LUTpos2->x &&
		LUTpos1->x < RDBpos2->x
		&&
		RDBpos1->y > LUTpos2->y &&
		LUTpos1->y < RDBpos2->y
		&&
		RDBpos1->z > LUTpos2->z &&
		LUTpos1->z < RDBpos2->z 

		
		)

		result = 1;
	

	if (showCollisionBox) {
		if (result) collisionCol = 0x00ff00;
		drawBox(*LUTpos1, *RDBpos1, 2, collisionCol);
		drawBox(*LUTpos2, *RDBpos2, 2, collisionCol);

	}

	return result;
}



//! Sphere collision check. Check if two spheres are touching. Returns 1 if collision is found otherwise returns 0 
/**
 * @param sphereAPos		pointer of first sphere's position
 * @param sphereARadius		first sphere's radius
 * @param sphereBPos		pointer of second sphere's position
 * @param sphereBRadius		second sphere's radius
 * @param showCollisionBox  For debugging set to false by default. Set to true to render the collision box onto the volumetric display. (Note : The collision check must be called within the start and end frame)
 *
 * @returns 1 if there is a collision and 0 if not
 */
int VoxieBox::sphereCollideChk(point3d * sphereAPos, double sphereARadius, point3d * sphereBPos, double sphereBRadius, bool showCollisionBox) {
	float sidea = fabs(sphereAPos->x - sphereBPos->x);
	float sideb = fabs(sphereAPos->y - sphereBPos->y);
	sidea = sidea * sidea;
	sideb = sideb * sideb;
	float distance = (double)sqrt(sidea + sideb);

	int result = 0;
	int collisionCol = 0xff0000;

	
	if (distance < sphereARadius + sphereBRadius && sphereAPos->z - sphereARadius < sphereBPos->z + sphereBRadius && sphereAPos->z + sphereARadius > sphereBPos->z - sphereBRadius) {
		result =  1;
	}

	if (showCollisionBox) {
		if (result) collisionCol = 0x00ff00;
		drawSphere(*sphereAPos, sphereARadius, 0, collisionCol);
		drawSphere(*sphereBPos, sphereBRadius, 0, collisionCol);
		
	}

	return result;
}

//! Clips a point to ensures it is within the volumes display's bounds. 
/** 
 * Adjust a point if it is outside of the volume.
 * @param  pos		  pointer of the point3d to check
 * @param  radius	  the size of radius to check
 *
 * @return in bits of what dimension is outside the range for Up/Down 1st bit x, 2nd bit y, 3rd bit z. For Spinner 1st bit x or y, 2nd bit z 
 */
int VoxieBox::clipInsideVolume(point3d * pos, float radius)
{
	int response = 0;

	if (vw.clipshape == 0) { // if up/down display
		
		if (pos->x > vw.aspx - radius)	{ response += 1; pos->x = vw.aspx - radius;  }
		if (pos->x < -vw.aspx + radius)	{ response += 1; pos->x = -vw.aspx + radius; }
		if (pos->y > vw.aspy - radius)	{ response += 2; pos->y = vw.aspy - radius;	}
		if (pos->y < -vw.aspy + radius) { response += 2; pos->y = -vw.aspy + radius;	}
		if (pos->z > vw.aspz - radius)  { response += 4; pos->z = vw.aspz - radius;  }
		if (pos->z < -vw.aspz + radius) { response += 4; pos->z = -vw.aspz + radius; }
	}
	else { // if spinner
		point3d centrePoint = { 0,0,0 };
		while (sphereCollideChk(&centrePoint, vw.aspr - radius, pos, 0.01) == 0) {
			response = 1;
			centrePoint.z = pos->z;
			moveToPos(pos, point3d{ 0,0,pos->z }, 1, float(0.0001));
		}
		

		if (pos->z > vw.aspz - radius)	{ response += 2; pos->z = vw.aspz - radius; }
		if (pos->z < -vw.aspz + radius)	{ response += 2; pos->z = -vw.aspz + radius; }
	}

	return response;

}

//! @return returns the internal mouse position as a point3d (x, y, z) 
point3d VoxieBox::getMousePosition() 
{
	return point3d{ mousePos.x, mousePos.y, mousePos.z };
}

/** The 'update loop' for a VX application also updates voxie_input_t struct
 *
 *  Call this once per frame to update variables in voxie_inputs_t structure.
 *  returns typically returns a 1. returns a 0 when the program wants to quit.
 *	run as a while loop " while (voxiebox::breath() )"
 *  using breath() with no parameters uses internal voxie_input_t structure (in) 
 *	to input updates.
 *
 *  Note the VxCpp version of breath() also updates: the internal timers,
 *  checks if the hardware is a rotating or up / down display,
 *  calls other input checks (for nav, joy and keyboard),
 *  listens for key input to rotate the emulator screen,
 *  and the 'Esc' key to quit the application,
 *  tracks the mouse position and updates the rainbow color variable.
 *  (most of) These are common tasks a typical VX developer would have to do themselves.
 *
 *  @return typically returns a 1, returns a 0 when the program wants to quit.
 */
int VoxieBox::breath() 
{
	return breath(&in);

} 

//! A breath is a complete volume sweep.
/**  Call this once per frame to update variables in voxie_inputs_t structure.
 *  returns typically returns a 1. returns a 0 when the program wants to quit.
 *	run as a while loop " while (voxiebox::breath() )"
 *  using breath() with no parameters uses internal voxie_input_t structure(in)
 *	to input updates.
 *
 *  Note the VxCpp version of breath() also updates: the internal timers,
 *  checks if the hardware is a rotating or up / down display,
 *  calls other input checks (for nav, joy and keyboard),
 *  listens for key input to rotate the emulator screen, 
 *  and the 'Esc' key to quit the application,
 *  tracks the mouse position and updates the rainbow color variable. 
 *  (most of) These are common tasks a typical VX developer would have to do themselves.
 *
 * @param in pointer to the voxie_inputs_t to update the input state ( voxie_inputs_t() manages the mouse input ) 
 * @return typically returns a 1, returns a 0 when the program wants to quit.
 */
int VoxieBox::breath(voxie_inputs_t * input)
{
	// update timers
	oldTime = time;
	time = voxie_klock();
	deltaTime = time - oldTime;
	averageTime += (deltaTime - averageTime) * .1;
	
	// emulator rotation controls 
	int i = 0;
	i = (this->voxie_keystat(0x1b) & 1) - (this->voxie_keystat(0x1a) & 1); // keys '[' and ']'
	if (i)
	{
		/* Rotation controls for emulator view */
		if (this->voxie_keystat(0x2a) | this->voxie_keystat(0x36))
			vw.emuvang = min(max(double(vw.emuvang) + (double)i * deltaTime * 2.0, -3.14159265358979323 * .5), 0.1268); //Shift+[,]
		else if (this->voxie_keystat(0x1d) | this->voxie_keystat(0x9d))
			vw.emudist = max(vw.emudist - (float)i * deltaTime * 2048.0, 400.0); //Ctrl+[,]
		else
			vw.emuhang += (float)i * deltaTime * 2.0; //[,]
		
		this->init();
	}

	// color scroll 
	if (rainbowTim < time) {
		rainbowTim = time + colScrollSpeed;
		rainbowCounter++;
		if (rainbowCounter > COL_SCROLL_MAX - 1) rainbowCounter = 0;
		colScrollcolor = PALETTE_COLOR[rainbowCounter];
	}

	// press 'esc' to quit program
	if (this->voxie_keystat(0x1) && this->enableEscQuit == true) this->quitLoop();

	updateMousePosition();

	if (!manualKeyManage) updateKeyboardHistory();
	if (!manualJoyManage) updateJoyInput();
	if (!manualNavManage) updateNavInput();
	if (!manualTouchManage && touchUpdate) updateTouch();


	// check if the hardware is a spinner
	// unfortunately we have to do it every breath otherwise it might get missed....
	if (vw.nblades > 0 && vw.clipshape == 0) {
		vw.clipshape = 1; init();
	}
	else  if (vw.nblades <= 0 && vw.clipshape == 1)	vw.clipshape = 0;


	return (voxie_breath(input) == 0);
}

//! start building the frame buffer.
/**
  *	 All volumetric and secondary (touch) screen draw calls need to happen between VoxieBox::startFrame() and VoxieBox::endFrame()
  *	 the startframe() function prepares the vxTypes::voxie_frame_t struct to start a new frame and empty out the voxel buffer. 
  *  any draw call used after this point is loaded into the voxel buffer.
  *  the startFrame() function uses the internal vf voxie_frame_t to manage its voxel data. It is possible for a developer to 
  *  write directly to the vf for their own low-level drawing. 
  *	
  *  The startFrame function also sets the view to the correct aspect ration (traditionally a user would call 'voxie_setview()' after the frame to set the views dimensions)
  *  the startFrame function also draws a border around the display (if VoxieBox::setBorder() has been set to true)
  *
  */
void VoxieBox::startFrame()
{
	voxie_frame_start(&vf);
	//vf.flags |= VOXIEFRAME_FLAGS_IMMEDIATE; //<-Mandatory for internal multi-threading!
	if (invertZAxis)	voxie_setview(&vf, -vw.aspx, -vw.aspy, vw.aspz, vw.aspx, vw.aspy, -vw.aspz); // inverted
	else				voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); // normal

	int i = 0, n = 0, j = 0;
	float r = vw.aspr *.99;

	

	if (drawBorder == true) { //draw wire frame box
		// Check if hardware is up/down or spinner  
		if (vw.clipshape == 0) 	voxie_drawbox(&vf, -vw.aspx + 1e-3f, -vw.aspy + 1e-3f, -vw.aspz, +vw.aspx - 1e-3f, +vw.aspy - 1e-3f, +vw.aspz, 1, 0xffffff);
		 else { // if spinner draw around the outside of the shape
			n = 64;
			for (j = -64; j <= 64; j++)
			{
				if (j == -62) j = 62;
				 for (i = 0; i < n; i++)
				{
					voxie_drawlin(&vf, cos((float)(i + 0)*PI*2.0 / (float)n)*(r), sin((float)(i + 0)*PI*2.0 / (float)n)*(r), (float)j*vw.aspz / 64.f,
						cos((float)(i + 1)*PI*2.0 / (float)n)*(r), sin((float)(i + 1)*PI*2.0 / (float)n)*(r), (float)j*vw.aspz / 64.f, 0xffffff);
				}
			}

			n = 32;
			for (i = 0; i < n; i++)
			{
				voxie_drawlin(&vf, cos((float)(i + 0)*PI*2.0 / (float)n)*(r), sin((float)(i + 0)*PI*2.0 / (float)n)*(r), -vw.aspz,
					cos((float)(i + 0)*PI*2.0 / (float)n)*(r), sin((float)(i + 0)*PI*2.0 / (float)n)*(r), +vw.aspz, 0xffffff);
			}
		}
	
	}
	
}

//! function to signify the end of a volumetric frame. 
/*
 * Call this once after all draw calls and within the VoxieBox::Breath() loop. Sends the voxel buffer to the 
 * volumetric display and secondary screen. 
 * Note : Any draw calls before VoxieBox::startFrame() and after VoxieBox::endFrame will not render onto 
 * the volume. 
 */
void VoxieBox::endFrame()
{
	if (!manualTouchManage && touchIsDrawing) touchDraw(); // this is placed just before the end of the frame to ensure its on top
	voxie_frame_end();
	voxie_getvw(&vw);
}

//! Returns the running time (in seconds) from program execution till present.
/**
 * @return in seconds the running time (in seconds) from program execution till present.
 */
double VoxieBox::getTime()
{
	return time;
}

//! Returns the system's current VPS (volumes per second).
/**
 * @return the current VPS value. At least 15 VPS on a Up/Down system and At least 30 VPS for a Spinner is recommended.   
 */
double VoxieBox::getVPS()
{
	return 1.0 / averageTime;
}

//! Displays volumes per second (volumetric equivalent of 'FPS') and version info on the secondary (touch) screen. 
/**
 *  @param posX	X position for output
 *  @param posY	Y position for output
 */
void VoxieBox::showVPS(int posX, int posY)
{	
	int pcol = 0x006666;
	int wcol = 0x00ffff;

	if (getVPS() <= 60) { pcol = 0x006600; wcol = 0x00FF60; }
	if (getVPS() <= 40) { pcol = 0x666600; wcol = 0x00FF00;	}
	if (getVPS() <= 15) { pcol = 0x663300; wcol = 0xFFFF00; }
	if (getVPS() <= 10) { pcol = 0x660000; wcol = 0xFF3030;	}
	if (getVPS() <= 5)  { pcol = 0xDD0000; wcol = 0xFF0000;	}
	
	debugDrawBoxFill(posX, posY, posX + 235, posY + 66, pcol);
//	debugText(posX, posY, 0xffffff, -1, "VPS %2.2f\n\nRun Time %2.1f (delta %2.3f) \nVxBxCPP.DLL ver	 : %lld\nVoxieBox.DLL ver	: %lld\nCompiled On : %s, %s ", getVPS(), getTime(), getDeltaTime(), getVxCppVersion(), voxie_getversion(), __DATE__, __TIME__);
	
	if (getVPS() > 60) { debugText(posX + 155, posY, 0x00FF00, -1, "Great VPS"); }
	if (getVPS() <= 15 && int(getTime()) % 2) { debugText(posX + 150, posY, 0xFF6666, -1, "Low VPS"); }

	debugBar(posX - 5, posY + 5, getVPS(), 100, 0, (char*)" ", 2);

	debugText(posX + 20, posY, wcol, -1, "VPS %2.2f", getVPS());
	posY += 11;
	debugText(posX + 20, posY, 0xffffff, -1, "Run Time %2.1f", getTime());
	debugText(posX + 20, posY, 0x00ff00, -1, "                     (delta %2.3f)", getDeltaTime());
	posY += 24;
	debugText(posX + 20, posY, 0xffffff, -1, "VxCpp.DLL ver	   : %lld", getVxCppVersion());
	posY += 11;
	debugText(posX + 20, posY, 0xffffff, -1, "VoxieBox.DLL ver   : %lld", voxie_getversion());
	posY += 11;
	debugText(posX + 20, posY, 0xffffff, -1, "Compiled On : %s, %s ", __DATE__, __TIME__);
	
}

/**
 *  Exposes some of the variables from the internal voxie_frame_t onto the secondary (touch) screen.
 *  Intended for debug purposes
 *  @param posX the X position of the report
 *  @param posY the Y position of the report
 */
void VoxieBox::reportVoxieFrame(int posX, int posY) {

	reportVoxieFrame(posX, posY, &this->vf);

}

/**
* Exposes some of the variables a voxie_frame_t struct onto the secondary (touch) screen.
* Intended for debug purposes
* @param posX the X position of the report
* @param posY the Y position of the report
* @param VF a pointer for the voxie_frame_t struct to report on
*/
void VoxieBox::reportVoxieFrame(int posX, int posY, voxie_frame_t * VF)
{
	int col = 0xffff00;
	debugDrawBoxFill(posX, posY, posX + 160, posY + 65, 0x333300);
	debugText(posX, posY, 0xffffff, -1, "     Voxie Frame Report  ");
	debugText(posX, posY + 12, col, -1, "viewport : x %d, y %d\nexents x0 : %d, x1 : %d\n       y0 : %d, y1 : %d\nf_ptr  : %p\nfp_ptr : %p\np_ptr  : %p",
		VF->x, VF->y, VF->x0, VF->x1, VF->y0, VF->y1, VF->f, VF->fp, VF->p
	);
}

/**
* Exposes all the variables from the internal (vw) voxie_wind_t onto the secondary (touch) screen.
* Intended for debug purposes
* @param posX the X position of the report
* @param posY the Y position of the report
*/
void VoxieBox::reportVoxieWind(int posX, int posY) {

	reportVoxieWind(posX, posY, &this->vw);

}

/**
* Exposes all the variables of a voxie_wind_t struct onto the secondary (touch) screen.
* Intended for debug purposes
* @param posX the X position of the report
* @param posY the Y position of the report
* @param VW a pointer fo the voxie_wind_t struct to report on
*/
void VoxieBox::reportVoxieWind(int posX, int posY, voxie_wind_t * VW )
{

	int col[7] = { 0xAA0000, 0xFF9900, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x4444FF, 0x9900FF };
	int i = 0;
	int displayDetect = VW->dispnum;
	if (displayDetect > 3) displayDetect = 3;
	debugDrawBoxFill(posX, posY, posX + 850, posY + 170 + (displayDetect * 36) , 0x3333333);
	debugText(posX + 350, posY, 0xFFFFFF, -1, "Voxie Window Report");

	posY += 24;

	debugText(posX, posY, col[0], -1, "Emulator -=> useemu = %d, emuhang = %1.2f, emuvang = %1.2f, emudist = %1.2f, emugamma = %1.2f\n Projector: xdim = %d, ydim = %d, projrate = %d, framepervol = %d, usecol = %d, bitspervol = %d, dispnum = %d",
		VW->useemu, VW->emuhang, VW->emuvang, VW->emudist, VW->emugam,
		VW->xdim, VW->ydim, VW->projrate, VW->framepervol, VW->usecol, VW->bitspervol, VW->dispnum
	);

	debugText(posX, posY + 24, col[1], -1, "Display Info:");

	for (i = 0; i < displayDetect; i++) {
		debugText(posX, posY + 36 + (i * 33), col[2], -1, "Display %d -=>, keyst0 = %1.3g, keyst1 = %1.3g, keyst2 = %1.3g, keyst3 = %1.3g\n               keyst4 = %1.3g, keyst5 = %1.3g, keyst6 = %1.3g, keyst7 = %1.3g\n               colo_r = %d, colo_g = %d, colo_b = %d mono_r = %d, mono_g = %d, mono_b = %d, mirrorx = %d, mirrory = %d",
			i, VW->disp[i].keyst[0], VW->disp[i].keyst[1], VW->disp[i].keyst[2], VW->disp[i].keyst[3], VW->disp[i].keyst[4], VW->disp[i].keyst[5], VW->disp[i].keyst[6], VW->disp[i].keyst[7],
			VW->disp[i].colo_r, VW->disp[i].colo_g, VW->disp[i].colo_b,
			VW->disp[i].mono_r, VW->disp[i].mono_g, VW->disp[i].mono_b,
			VW->disp[i].mirrorx, VW->disp[i].mirrory);
	}
	

	posY += 36 + (displayDetect * 36);
	// Sync & Hardware
	debugText(posX, posY, col[3], -1, "Sync -=> hwsync_frame0 = %d, hwsync_phase = %d\nhwsync_amp[0] = %d, hwsync_amp[1] = %d, hwsync_amp[2] = %d, hwsync_amp[3] = %d, hwsync_levthres = %d, voxie_vol = %d\n HWType : upndow = %d, nblades = %d, ianghak = %d, cpmaxrpm = %d, goalrpm = %d, motortyp = %d, sawtoothrat = %1.2f",
		VW->hwsync_frame0, VW->hwsync_phase, VW->hwsync_amp[0], VW->hwsync_amp[1], VW->hwsync_amp[2], VW->hwsync_amp[3], VW->hwsync_levthresh, VW->voxie_vol,
		VW->upndow, VW->nblades, VW->ianghak, VW->cpmaxrpm, VW->goalrpm, VW->motortyp, VW->sawtoothrat
	);
	posY += 36;
	// Display
	debugText(posX, posY, col[4], -1, "Display -=> ilacemode = %d, drawstroke = %d, dither = %d, smear = %d, usekeystone = %d, flip = %d, menu_on_voxie = %d clipshape = %d\n aspx = %1.2f, aspy = %1.2f, aspz = %1.2f, aspr = %1.2f, asprmin = %1.2f gamma = %1.2f, density = %1.2f dotsize = %d, dimcaps = %d\n outcol[0] = %#08x, outcol[1] = %#08x, outcol[2] = %#08x, sensemask[0] = %#08x, sensemask[1] = %#08x, sensemask[2] = %#08x\n normhax %d",
		VW->ilacemode, VW->drawstroke, VW->dither, VW->smear, VW->usekeystone, VW->flip, VW->menu_on_voxie, VW->clipshape,
		VW->aspx, VW->aspy, VW->aspz, VW->aspr, VW->asprmin, VW->gamma, VW->density, VW->ldotnum, VW->dimcaps,
		VW->outcol[0], VW->outcol[1], VW->outcol[2], VW->sensemask[0], VW->sensemask[1], VW->sensemask[2], VW->normhax
	);
	posY += 36;
	// Sound
	debugText(posX, posY, col[5], -1, "Sound -=> sndfx_vol = %d, voxie_aud = %d, excl_audio = %d, sndfx_aud[0] = %d, sndfx_aud[1] = %d,\n playsamprate = %d, playnchans = %d, recsamprate = %d, recnhans = %d, isrecording = %d",
		VW->sndfx_vol, VW->voxie_aud, VW->excl_audio, VW->sndfx_aud[0], VW->sndfx_aud[1], VW->playsamprate, VW->playnchans, VW->recsamprate, VW->recnchans, VW->isrecording
	);
	posY += 24;
	// Hacks
	debugText(posX, posY, col[6], -1, "Misc -=> hacks = %d, dispcur = %d, thread_override_hack = %d, usejoy = %d",
		VW->hacks, VW->dispcur, VW->thread_override_hack, VW->usejoy
	);


}

//! Returns delta time (the time between volume updates) delta time is CPU speed dependent and can be used to make ensure timing is consistent between various computers / systems.
/**
 * @return in seconds the delta time (time between volume updates)
 */
double VoxieBox::getDeltaTime()
{
	return deltaTime;
}

/**
  *  Set a custom view. Usually you'll pass in the voxie windows's (voxie_wind_t) aspect values
  *  but it can be used like 'camera' and be moved around by passing in different values.
  *
  *  You may call this function any number of times during rendering.
  *  This function is called automatically during the VoxieBox::startFrame() function.
  *  it is passed with internal vw voxie_window_t aspect ratio values.
  *
  *  Checks to see if invertZAxis is enabled and applies swap if needed
  *
  *  @param xMin minimum x resolution (left)
  *  @param yMin minimum y resolution (up)
  *  @param zMin minimum z resolution (top)
  *  @param xMax maximum x resolution (right)
  *  @param yMax maximum y resolution (down)
  *  @param zMax maximum z resolution (bottom)
  */
void VoxieBox::setView(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax)
{
	if (invertZAxis)		voxie_setview(&vf, xMin, yMin, -zMin, xMax, yMax, -zMax);	//inverted Z
	else					voxie_setview(&vf, xMin, yMin, zMin, xMax, yMax, zMax);		// normal
}

/**
 *  Same setView() but using point3D for coordinates instead of 6 floats.
 *  
 *  @param LUT the Left, Upper, Top position 
 *  @param RDB the Right, Down, Bottom position
 */
void VoxieBox::setView(point3d LUT, point3d RDB)
{
	setView(LUT.x, LUT.y, LUT.z, RDB.x, RDB.y, RDB.z);
}

/**
 *  Call after each VoxieBox::setView() to mask off a plane of a specified thickness. Used for viewing
 *  a slice of a scene.
 *  @param x0 x point on the center of the mask plane
 *  @param y0 y point on the center of the mask plane
 *  @param z0 z point on the center of the mask plane
 *  @param nx The normal vector; the magnitude of this vector determines the thickness of the plane.
 *  @param ny The normal vector; the magnitude of this vector determines the thickness of the plane. 
 *  @param nz The normal vector; the magnitude of this vector determines the thickness of the plane.
 */
void VoxieBox::setMaskPlane(float x0, float y0, float z0, float nx, float ny, float nz)
{
	voxie_setmaskplane(&vf, x0, y0, z0, nx, ny, nz);
}


/** changes the global normal vector (vw.normhax) for shading. Pass in three 0's to disable. 
 * Alters the global normal vector for shading. Shading only works with models which have vertices
 * Pass 3 0's to disable shading.
 * @param horizontalAngle the horizontal angle -180 to 180
 * @param verticalAngle  the vertical angle -90 to 90
 * @param amplitude the amplitude (intensity) 0 to 100

 */
void VoxieBox::setGlobalShader(float horizontalAngle, float verticalAngle, float amplitude)
{
	voxie_setnorm(horizontalAngle, verticalAngle, amplitude);
}

/**
 * Flush all graphics commands on internal buffer, causing all graphics commands
 * to actually execute and complete. This might be used if one wanted to do some low level access to the
 * voxel buffer. For advanced users only.
 */
void VoxieBox::flushGfx(void)
{
	voxie_flush();
}


/** Frees a filename from VoxieBox.dll internal cache (any filename passed to voxie_drawmeshtex() / drawMesh, voxie_drawheimap() / drawHeightMap, voxie_drawspr() / drawModel)
 *  @param fileName the file path and file name of the file to remove from the internal cache 
 *  NOTE: Pass a '*' file name to remove entire internal cache
 *
 */
void VoxieBox::freeGfx(char * fileName)
{
	voxie_free(fileName);
}

/**
 *  Advanced utility function for keystone calibration (used by graphcalc and keystone calibration in voxiedemo)
 *  see the source code for voxiedemo.c for an understanding of how this works.
 *
 *  @param  disp display number: {0..vw.dispnum-1}
 *  @param  dir direction. 1=forward, -1=inverse
 *  @param  x x input point; {-1.f .. +1.f}
 *	@param  y y input point; {-1.f .. +1.f}
 *  @param  z  input plane: {0 .. vw.framepervol*24-1}
 *  @param  xo xo keystone adjusted output point
 *	@param  yo yo keystone adjusted output point
 */
void VoxieBox::setProject(int dispNum, int dir, float x, float y, int z, float * xo, float * yo)
{
	voxie_project(dispNum, dir, x, y, z, xo, yo); 
	
}

/**
 *  Set LEDs values on projector hardware.  WARNING: Be careful not to overheat projector!
 *  Range per component: 0:darkest, 255:brightest
 *  WARNING: Be careful not to overheat projector!
 *	@param	dispNum	the display's number to adjust
 *	@param	r the LED value for the red channel {0..128}  (go over 128 at your own risk!)
 *	@param  g the LED value for the green channel {0..128}  (go over 128 at your own risk!)
 *	@param	b the LED value for the blue channel {0..128}  (go over 128 at your own risk!)
 *
 */
void VoxieBox::setLeds(int dispNum, int r, int g, int b)
{
	voxie_setleds(dispNum, r, g, b);
}

//! Turns off the reciprocating screen and effectively makes the display a '2D' screen. (Works on Voxon hardware only)
void VoxieBox::setDisplay2D()
{
	if (vw.useemu == 0) {
		if (vw.voxie_vol > 0)	display_volume = vw.voxie_vol;

		vw.voxie_vol = 0;
		voxie_init(&vw);
	}
}

//! Turns on the reciprocating screen / activates the volumetric display. (Works on Voxon hardware only)
void VoxieBox::setDisplay3D()
{
	if (vw.useemu == 0) {
		if (vw.voxie_vol == 0 && display_volume > 0)	vw.voxie_vol = display_volume;

		voxie_init(&vw);
	}
}

//! Returns the internal voxie_wind_t's aspect X ratio
/**
 * @return returns the internal voxie_wind_t's aspect X ratio (default is 1)
 */
float VoxieBox::getAspectX()
{
	return vw.aspx;
}

//! Returns the internal voxie_wind_t's aspect Y ratio
/**
 * @return returns the internal voxie_wind_t's aspect Y ratio (default is 1)
 */
float VoxieBox::getAspectY()
{
	return vw.aspy;
}

//! Returns the internal voxie_wind_t's aspect Z ratio
/**
 * @return returns the internal voxie_wind_t's aspect Z ratio (default is .40)
 */
float VoxieBox::getAspectZ()
{
	return vw.aspz;
}

//! Returns all the internal voxie_wind_t's aspect ratio values as a point3d (x,y,z)
/**
 *  @return returns all the internal voxie_wind_t's aspect ratio values as a point3d (x,y,z)
 */
point3d VoxieBox::getAspect()
{
	point3d aspect = { vw.aspx, vw.aspy, vw.aspz };
	return aspect;
}


//! Set the internal voxie_wind_t's X aspect ratio value and update the voxie_wind_t
/**
 *  @param newAspectX  the new Aspect X value to apply. th
 */
void VoxieBox::setAspectX(float newAspectX)
{
	vw.aspx = newAspectX;
	this->init();
}

//! Set the internal voxie_wind_t's Y aspect ratio value and update the voxie_wind_t
/**
 *  @param newAspectY  the new Aspect Y value to apply. th
 */
void VoxieBox::setAspectY(float newAspectY)
{
	vw.aspx = newAspectY;
	this->init();
}

//! Set the internal voxie_wind_t's Z aspect ratio value and update the voxie_wind_t
/**
 *  @param newAspectZ  the new Aspect Z value to apply. th
 */
void VoxieBox::setAspectZ(float newAspectZ)
{
	vw.aspx = newAspectZ;
	this->init();
}

//! Set all the internal voxie_wind_t's aspect ratio values and update the voxie_wind_t
/**
 *  @param newAspect the new aspect ratios for the volumetric display. 
 */
void VoxieBox::setAspect(point3d newAspect)
{
	vw.aspx = newAspect.x;
	vw.aspy = newAspect.y;
	vw.aspz = newAspect.z;

	this->init();

}

//! frees the voxiebox.DLL from memory and allows the file to be accessed by other processes
void VoxieBox::shutdown()
{
	//MessageBox(0, "Shutting Down VoxieBox!", "", MB_OK); // debug
	voxie_uninit(0);
}

//! loads the VoxieBox.dll library into memory to activate VoxieBox.dll's functions. 
int VoxieBox::voxie_load(voxie_wind_t *vw)
{
#if defined(_WIN32)

	hvoxie = LoadLibrary("voxiebox.dll");
	
	if (!hvoxie) { 
		const char * result = std::to_string(GetLastError()).c_str(); 
		MessageBox(0, result, "", MB_OK);
		return(-1); 
	};

	voxie_loadini_int = (void(__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie, "voxie_loadini_int");
	voxie_getvw = (void(__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie, "voxie_getvw");
	voxie_init = (int(__cdecl *)(voxie_wind_t*))GetProcAddress(hvoxie, "voxie_init");
	voxie_uninit_int = (void(__cdecl *)(int))GetProcAddress(hvoxie, "voxie_uninit_int");
	voxie_mountzip = (void(__cdecl *)(char*))GetProcAddress(hvoxie, "voxie_mountzip");
	voxie_free = (void(__cdecl *)(char*))GetProcAddress(hvoxie, "voxie_free");
	voxie_getversion = (__int64(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_getversion");
	voxie_gethwnd = (HWND(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_gethwnd");
	voxie_breath = (int(__cdecl *)(voxie_inputs_t*))GetProcAddress(hvoxie, "voxie_breath");
	voxie_quitloop = (void(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_quitloop");
	voxie_klock = (double(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_klock");
	voxie_keystat = (int(__cdecl *)(int))GetProcAddress(hvoxie, "voxie_keystat");
	voxie_keyread = (int(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_keyread");
	voxie_touch_custom = (void(__cdecl *)(const touchkey_t *, int))GetProcAddress(hvoxie, "voxie_touch_custom");
	voxie_xbox_read = (int(__cdecl *)(int, voxie_xbox_t *))GetProcAddress(hvoxie, "voxie_xbox_read");
	voxie_xbox_write = (void(__cdecl *)(int, float, float))GetProcAddress(hvoxie, "voxie_xbox_write");
	voxie_nav_read = (int(__cdecl *)(int, voxie_nav_t *))GetProcAddress(hvoxie, "voxie_nav_read");
	voxie_touch_read = (int(__cdecl *)(int *index, int *xVal, int *yVal, int *packetIndex))GetProcAddress(hvoxie, "voxie_touch_read");
	voxie_menu_reset = (void(__cdecl *)(int(*)(int, char*, double, int, void*), void*, char*))GetProcAddress(hvoxie, "voxie_menu_reset");
	voxie_menu_addtab = (void(__cdecl *)(char*, int, int, int, int))GetProcAddress(hvoxie, "voxie_menu_addtab");
	voxie_menu_additem = (void(__cdecl *)(char*, int, int, int, int, int, int, int, int, double, double, double, double, double))GetProcAddress(hvoxie, "voxie_menu_additem");
	voxie_menu_updateitem = (void(__cdecl *)(int, char*, int, double))GetProcAddress(hvoxie, "voxie_menu_updateitem");
	voxie_volcap = (void(__cdecl *)(const char*, int, int))GetProcAddress(hvoxie, "voxie_volcap");
	voxie_setview = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, float, float))GetProcAddress(hvoxie, "voxie_setview");
	voxie_setmaskplane = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, float, float))GetProcAddress(hvoxie, "voxie_setmaskplane");
	voxie_setnorm = (void(__cdecl *)(float nx, float ny, float nz))GetProcAddress(hvoxie, "voxie_setnorm");
	voxie_frame_start = (int(__cdecl *)(voxie_frame_t*))GetProcAddress(hvoxie, "voxie_frame_start");
	voxie_flush = (void(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_flush");
	voxie_frame_end = (void(__cdecl *)(void))GetProcAddress(hvoxie, "voxie_frame_end");
	voxie_setleds = (void(__cdecl *)(int, int, int, int))GetProcAddress(hvoxie, "voxie_setleds");
	voxie_drawvox = (void(__cdecl *)(voxie_frame_t*, float, float, float, int))GetProcAddress(hvoxie, "voxie_drawvox");
	voxie_drawbox = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, float, float, int, int))GetProcAddress(hvoxie, "voxie_drawbox");
	voxie_drawlin = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, float, float, int))GetProcAddress(hvoxie, "voxie_drawlin");
	voxie_drawpol = (void(__cdecl *)(voxie_frame_t*, pol_t*, int, int))GetProcAddress(hvoxie, "voxie_drawpol");
	voxie_drawmeshtex = (void(__cdecl *)(voxie_frame_t*, char*, poltex_t*, int, int*, int, int, int))GetProcAddress(hvoxie, "voxie_drawmeshtex");
	voxie_drawsph = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, int, int))GetProcAddress(hvoxie, "voxie_drawsph");
	voxie_drawcone = (void(__cdecl *)(voxie_frame_t*, float, float, float, float, float, float, float, float, int, int))GetProcAddress(hvoxie, "voxie_drawcone");
	voxie_drawspr = (int(__cdecl *)(voxie_frame_t*, const char*, point3d*, point3d*, point3d*, point3d*, int))GetProcAddress(hvoxie, "voxie_drawspr");
	voxie_drawspr_ext = (int(__cdecl *)(voxie_frame_t*, const char*, point3d*, point3d*, point3d*, point3d*, int, float, float, int))GetProcAddress(hvoxie, "voxie_drawspr_ext");
	voxie_printalph = (void(__cdecl *)(voxie_frame_t*, point3d*, point3d*, point3d*, int, const char*))GetProcAddress(hvoxie, "voxie_printalph");
	voxie_printalph_ext = (void(__cdecl *)(voxie_frame_t*, point3d*, point3d*, point3d*, float, int, const char*))GetProcAddress(hvoxie, "voxie_printalph_ext");
	voxie_drawcube = (void(__cdecl *)(voxie_frame_t*, point3d*, point3d*, point3d*, point3d*, int, int))GetProcAddress(hvoxie, "voxie_drawcube");
	voxie_drawheimap = (float(__cdecl *)(voxie_frame_t*, char*, point3d*, point3d*, point3d*, point3d*, int, int, int))GetProcAddress(hvoxie, "voxie_drawheimap");
	voxie_drawdicom = (void(__cdecl *)(voxie_frame_t*, voxie_dicom_t*, const char *, point3d*, point3d*, point3d*, point3d*, int*, int*))GetProcAddress(hvoxie, "voxie_drawdicom");
	voxie_debug_print6x8 = (void(__cdecl *)(int x, int y, int fcol, int bcol, const char *st))      GetProcAddress(hvoxie, "voxie_debug_print6x8");
	voxie_debug_drawpix = (void(__cdecl *)(int x, int y, int col))                                 GetProcAddress(hvoxie, "voxie_debug_drawpix");
	voxie_debug_drawhlin = (void(__cdecl *)(int x0, int x1, int y, int col))                        GetProcAddress(hvoxie, "voxie_debug_drawhlin");
	voxie_debug_drawline = (void(__cdecl *)(float x0, float y0, float x1, float y1, int col))       GetProcAddress(hvoxie, "voxie_debug_drawline");
	voxie_debug_drawcirc = (void(__cdecl *)(int xc, int yc, int r, int col))                        GetProcAddress(hvoxie, "voxie_debug_drawcirc");
	voxie_debug_drawrectfill = (void(__cdecl *)(int x0, int y0, int x1, int y1, int col))               GetProcAddress(hvoxie, "voxie_debug_drawrectfill");
	voxie_debug_drawcircfill = (void(__cdecl *)(int x, int y, int r, int col))               GetProcAddress(hvoxie, "voxie_debug_drawcircfill");
	voxie_debug_drawtile =	(void(__cdecl *)(tiletype *src, int x, int y))					 GetProcAddress(hvoxie, "voxie_debug_drawtile");
	voxie_playsound = (int(__cdecl *)(const char*, int, int, int, float))GetProcAddress(hvoxie, "voxie_playsound");
	voxie_playsound_update = (void(__cdecl *)(int, int, int, int, float))GetProcAddress(hvoxie, "voxie_playsound_update");
	voxie_playsound_seek = (void(__cdecl *)(int, double, int))GetProcAddress(hvoxie, "voxie_playsound_seek");
	voxie_setaudplaycb = (void(__cdecl *)(void(*userplayfunc)(int*, int)))GetProcAddress(hvoxie, "voxie_setaudplaycb");
	voxie_setaudreccb = (void(__cdecl *)(void(*userrecfunc)(int*, int)))GetProcAddress(hvoxie, "voxie_setaudreccb");
	voxie_rec_open = (int(__cdecl *)(voxie_rec_t*, char*, int))GetProcAddress(hvoxie, "voxie_rec_open");
	voxie_rec_play = (int(__cdecl *)(voxie_rec_t*, int))      GetProcAddress(hvoxie, "voxie_rec_play");
	voxie_rec_close = (void(__cdecl *)(voxie_rec_t*))          GetProcAddress(hvoxie, "voxie_rec_close");
		
	//Ken's ZIP functions
	kpzload = (void(__cdecl *)(const char* fileName, INT_PTR* fptr, INT_PTR* bpl, INT_PTR* xsiz, INT_PTR* ysiz))        GetProcAddress(hvoxie, "kpzload");
	kpgetdim = (int(__cdecl *)(const char * buffer, int nby, int * xsiz, int * ysiz))                  GetProcAddress(hvoxie, "kpgetdim");
	kprender = (int(__cdecl *)(const char * buffer, int nby, INT_PTR fptr, int bpl, int xsiz, int ysiz, int xoff, int yoff))GetProcAddress(hvoxie, "kprender");
	kzaddstack = (int(__cdecl *)(const char * fileName))GetProcAddress(hvoxie, "kzaddstack");
	kzuninit = (void(__cdecl *)(void))       GetProcAddress(hvoxie, "kzuninit");
	kzsetfil = (void(__cdecl *)(FILE* fileName))      GetProcAddress(hvoxie, "kzsetfil");
	kzopen = (INT_PTR(__cdecl *)(const char* ))GetProcAddress(hvoxie, "kzopen");
	kzfindfilestart = (void(__cdecl *)(const char* st))GetProcAddress(hvoxie, "kzfindfilestart");
	kzfindfile = (int(__cdecl *)(kzfind_t * find, kzfileinfo_t *fileinfo))      GetProcAddress(hvoxie, "kzfindfile");
	kzread = (int(__cdecl *)(kzfile_t * kzfile, void * buffer, unsigned int leng))  GetProcAddress(hvoxie, "kzread");
	kzfilelength = (int(__cdecl *)(kzfile_t * kzfile))       GetProcAddress(hvoxie, "kzfilelength");

	kzseek = (int(__cdecl *)(kzfile_t * kzfile, int offset, int whence))    GetProcAddress(hvoxie, "kzseek");
	kztell = (int(__cdecl *)(kzfile_t * kzfile))       GetProcAddress(hvoxie, "kztell");
	kzgetc = (int(__cdecl *)(kzfile_t * kzfile))       GetProcAddress(hvoxie, "kzgetc");
	kzeof = (int(__cdecl *)(kzfile_t * kzfile))       GetProcAddress(hvoxie, "kzeof");
	kzclose = (void(__cdecl *)(kzfile_t * kzfile))       GetProcAddress(hvoxie, "kzclose");
#endif

	voxie_loadini_int(vw);
	return(0);
}
//!  Calls voxie_uninit_int() internally and closes the DLL handle. Called within VoxieBox::shutDown()
void VoxieBox::voxie_uninit(int mode)
{
	voxie_uninit_int(mode);
#if defined(_WIN32)
	if (!mode) { if (hvoxie) { FreeLibrary(hvoxie); hvoxie = 0; } }
#endif
}

//! Renders a string (printf-style) unto the volumetric display.
/** 
 *  Used for displaying text on the volumetric display. Best to 
 *  keep the text to be near the top or bottom of the display and flat for maximum readability 
 *
 *	@param pos	the left, up, top position for the text to start. 
 *	@param r right vector use r.x value to set the width of the text. Set r.y and r.z to 0 for straight text 
 *	@param d down vector use d.y value to set the height of the text. Set d.x and d.z to 0 for straight text
 *	@param col int hex color value. (0xRGB)
 *	@param fmt*	Extension for C/C++ allowing use of this function printf-style. Example '(char*) Hello World Show a int %d", myInt'
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawText(point3d *pos, point3d *rVector, point3d *dVector, int col, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist, fmt);
#if defined(_WIN32)
	if (_vsnprintf((char*)&st, sizeof(st) - 1, fmt, arglist)) st[sizeof(st) - 1] = 0;
#else
	if (vsprintf(((char*))&st, fmt, arglist)) st[sizeof(st) - 1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_printalph(&vf, pos, rVector, dVector, col, st);
}

//! Renders a string (printf-style) unto the volumetric display w/ addition parameter to set size / radial width. 
/**
 *  Used for displaying text on the volumetric display. Best to
 *  keep the text to be near the top or bottom of the display and flat for maximum readability
 *
 *	@param pos	the left, up, top position for the text to start.
 *	@param r right vector use r.x value to set the width of the text. Set r.y and r.z to 0 for straight text
 *	@param d down vector use d.y value to set the height of the text. Set d.x and d.z to 0 for straight text
 *  @oaram size the radius of the font.
 *	@param col int hex color value. (0xRGB)
 *	@param fmt*	Extension for C/C++ allowing use of this function printf-style. Example '(char*) Hello World Show a int %d", myInt'
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawText(point3d *pos, point3d *rVector, point3d *dVector, float size, int col, const char *fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist, fmt);
#if defined(_WIN32)
	if (_vsnprintf((char*)&st, sizeof(st) - 1, fmt, arglist)) st[sizeof(st) - 1] = 0;
#else
	if (vsprintf(((char*))&st, fmt, arglist)) st[sizeof(st) - 1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_printalph_ext(&vf, pos, rVector, dVector, size, col, st);
}


//! Renders a string (printf-style) unto the volumetric display. Simple syntax
/**
 *  Used for displaying text on the volumetric display. Best to
 *  keep the text to be near the top or bottom of the display and flat for maximum readability
 *
 *	@param pos			the	left, up, top position for the text to start.
 *	@param textWidth	width of the text
 *	@param textHeight	height of the text
 *	@param hang			horizontal angle expressed in degrees
 *	@param vang			vertical angle expressed in degrees
 *  @param tilt			the tilt of the text expressed in degrees
 *  @param col			col int hex color value. (0xRGB)
 *	@param fmt*	Extension for C/C++ allowing use of this function printf-style. Example '(char*) Hello World Show a int %d", myInt'
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
 void VoxieBox::drawTextSimp(point3d * pos, float textWidth, float textHeight, float hang, float vang, float tilt, int col, char * fmt, ...)
 {
	 va_list arglist;
	 point3d vx, vy;
	 float f, ch, sh, cv, sv;
	 char buf[1024];

	 if (!fmt) return;
	 va_start(arglist, fmt);
#if defined(_WIN32)
	 if (_vsnprintf((char *)&buf, sizeof(buf) - 1, fmt, arglist)) buf[sizeof(buf) - 1] = 0;
#else
	 if (vsprintf((char *)&buf, fmt, arglist)) buf[sizeof(buf) - 1] = 0; //NOTE:unsafe!
#endif
	 va_end(arglist);

	 f = hang * (PI / 180.f); ch = cos(f); sh = sin(f);
	 f = vang * (PI / 180.f); cv = cos(f); sv = sin(f);
	 f = tilt * (PI / 180.f);

	 vx.x = ch;
	 vx.y = sh;
	 vx.z = 0.f;

	 vy.x = -sh * cv;
	 vy.y = ch * cv;
	 vy.z = -sv;

	 this->rotVex(f, &vx, &vy);

	 f = textWidth * .5f; vx.x *= f; vx.y *= f; vx.z *= f;
	 f = textHeight * .5f; vy.x *= f; vy.y *= f; vy.z *= f;
	 voxie_printalph(&vf, pos, &vx, &vy, col, buf);


 }

//! display text and/or variables unto to secondary (touch) screen. Supports in printf() format specifiers. -1 for transparent color
/**
 *				@example debugText(100,100, 0xffffff, -1, "Integer value %d, float value %1.2f", myInt, myFloat);
 *              would display a white text message 100 pixels from the left and 100 pixels from the top of touch the screen. 
 *				The message will say "Integer value X, float value Y" with X and Y being the values of those variables. 
 * 
 *   @param x  x position to render text on the secondary (touch) screen
 *   @param y  y position to render text on the secondary (touch) screen
 *   @param fcol foreground color expressed as hexadecimal value (RGB) (-1 to indicate transparent)
 *   @param bcol background color expressed as hexadecimal value (RGB) (-1 to indicate transparent)
 *   @param fmt  ASCII null terminated text string (must be pre-formatted by caller).
 *
 *   Note: must be called within the start and end of the frame functions to appear.  
 */
void VoxieBox::debugText(int x, int y, int fcol, int bcol, const char *fmt, ...)
{
	
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist, fmt);
#if defined(_WIN32)
	if (_vsnprintf((char *)&st, sizeof(st) - 1, fmt, arglist)) st[sizeof(st) - 1] = 0;
#else
	if (vsprintf((char *)&st, fmt, arglist)) st[sizeof(st) - 1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	voxie_debug_print6x8(x, y, fcol, bcol, st);
	
}

//! Draws a vertical bar for help tracking variables onto the secondary (touch) screen.
/**
 *	@param posx			the x position for the debug bar
 *	@param posy			the y position for the debug bar
 *	@param currentVal	the current value (the variable you want to track) to pass to the debug bar
 *	@param maxVal		the max value to render to the debug bar
 *  @param minVal		the min value to render to the debug bar
 *  @param text			any text to place with the debug bar (the title)
 *  @param type			the type of debug bar. 0 = default closer to max value is 'good'. 1 = closer to max value is 'bad', 2 = mini bar (smaller version with no text), 3 = mini bar centered at 0 for results that can be postive and negative
 *
 *	@return a float which is a percentage of how close the currentVal is to the maxVal
 */
double VoxieBox::debugBar(int posx, int posy, double currentVal, double maxVal, double minVal, char* text, int type) {

	float x = posx;
	float y = posy;
	const int POINTS = 4;
	int BAR_WIDTH = 25;
	int BAR_HEIGHT = 200;
	
	if (type == 2 || type == 3) {
		BAR_WIDTH = 10;
		BAR_HEIGHT = 50;
	}
	
	int BAR_BACKGROUND_COL = 0x404040;
	int MARKER_COL = 0x808080;
	int BAR_COL[6] = { 0xFFFFFF,0x00FF90, 0x00FF00, 0x90FF00, 0xFFFF00, 0xFF0000 };
	int col = 0x00ffff; // default color of the bar

	if (type == 1) { // color values for type 1 (low = good, high = bad)  
		BAR_COL[5] = 0x00FF00;		BAR_COL[4] = 0x90FF00;		BAR_COL[3] = 0xFFFF00;
		BAR_COL[2] = 0xFF0000;		BAR_COL[1] = 0xFF0000;		BAR_COL[0] = 0xFF0000;
	}

	double v = currentVal / (maxVal - minVal); // put value into percentage 
	float stepVal = (maxVal - minVal) / POINTS; //( 5 - 1 )
	double pointval[5] = { maxVal, minVal + (stepVal * 3), minVal + (stepVal * 2), minVal + stepVal, minVal }; // work out steps
	int barDiv = BAR_HEIGHT / POINTS; // work out divider heights

	// draw background 
	voxie_debug_drawrectfill(x + 10, y, x + 10 + BAR_WIDTH, y + (BAR_HEIGHT), BAR_BACKGROUND_COL);

	// draw markers 
	for (int i = 0; i < POINTS + 1; i++) {
		if (type < 2) voxie_debug_drawhlin(x + 10, x + 50, y + (barDiv * i), MARKER_COL);
		if (type < 2) debugText(x + 50, y + (barDiv * i) - 5, 0xffff80, -1, "%1.1f", pointval[i]);
	}

	// recolor bar based on its value
	col = BAR_COL[5];
	if (fabs(v) > .25) col = BAR_COL[4];
	if (fabs(v) > .5) col = BAR_COL[3];
	if (fabs(v) > .75) col = BAR_COL[2];
	if (fabs(v) >= .95) col = BAR_COL[1];
	if (fabs(v) > 1.1) col = BAR_COL[0];

	// draw text
	if (type < 2) debugText(x, y + (BAR_HEIGHT)+5, col, -1, "%s", text);
	if (type < 2) debugText(x, y + (BAR_HEIGHT)+15, col, -1, "%2.1f %3.0f%c", currentVal, v * 100, 37);

	double vdisplay = v;
	if (vdisplay > 1.10) vdisplay = 1.10; // limit value to show no more than 110% 
	if (type < 3) voxie_debug_drawrectfill(x + 10, y + (BAR_HEIGHT - (BAR_HEIGHT * vdisplay)), x + 10 + BAR_WIDTH, y + (BAR_HEIGHT), col);
	else {
		vdisplay /= 2;
	
		voxie_debug_drawrectfill(x + 10, y + (BAR_HEIGHT /2 - (BAR_HEIGHT * vdisplay) - 2), x + 10 + BAR_WIDTH, y + (BAR_HEIGHT / 2 - (BAR_HEIGHT * vdisplay) + 2), col);
	}
	return v * 100;
}

//! draws a cursor unto the volumetric display at the position specified.  
/** 
 *  Draws a cursor on the display based on various input types that can be used.
 *  Supports custom positions or Mouse, Gamepad or SpaceNav
 *  @param pos			the position of the cursor to track
 *  @param inputType	the input type (0 is mouse, 1 is spaceNav, 2 is Joystick, 3 is all three, 4 is none, 5 is filled state (so you can make your own) )
 *  @param inputID		the identification number of the input type (which SpaceNav or Joystick?)
 *  @param col			the color of the cursor
 *
 *  Note : as with all drawing calls, Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawCursor(point3d * pos, int inputType, int inputID, int col) {

	int curscolor = col;
	int cursFillState = 0;
	const float CURSOR_RADIUS = 0.02; // + (cos(tim * 3) * .01);
	const float LINE_WIDTH = 0.05;
	float z = 0;
	float y = 0;
	float x = 0;
	int   i = 0;
	float len = 0;

	switch (inputType) {
	default:
		case 0: // mouse
		if (getMouseButtonState() != 0) cursFillState = 1;
		break;
			case 1: // joystick
		if (getJoyButtonIsDown(inputID, JOY_A)) cursFillState = 1;
		if (getJoyButtonIsDown(inputID, JOY_B)) cursFillState = 1;
			break;
		case 2: // spaceNav
		if (getNavButtonState(inputID) != 0) cursFillState = 1;
			break;
		case 3:
			if (getMouseButtonState() != 0) cursFillState = 1;
			for (i = 0; i < 4; i++) { if(getJoyButtonIsDown(i, JOY_A)) cursFillState = 1; }
			for (i = 0; i < 4; i++) { if(getJoyButtonIsDown(i, JOY_B)) cursFillState = 1; }
			for (i = 0; i < 4; i++) { if (getNavButtonState(i)) cursFillState = 1; }
		break;
			case 4:

		break;
		case 5:
			cursFillState = 1;
			break;
	}

	drawSphere(pos->x, pos->y, pos->z, CURSOR_RADIUS, cursFillState, curscolor);
	drawVox(pos->x, pos->y, pos->z, curscolor);

	if (inputType <= 1 ) { // if mouse or joy

		drawLine(pos->x + LINE_WIDTH, pos->y, pos->z, pos->x - LINE_WIDTH, pos->y, pos->z, curscolor);
		drawLine(pos->x, pos->y + LINE_WIDTH, pos->z, pos->x, pos->y - LINE_WIDTH, pos->z, curscolor);
		drawLine(pos->x, pos->y, pos->z + LINE_WIDTH, pos->x, pos->y, pos->z - LINE_WIDTH, curscolor);

	
	}
	else { // for spacenav - incorporates angle
		z = getNavAngleDeltaAxis(inputID, 2) * .025;
		x = getNavAngleDeltaAxis(inputID, 0) * .025;
		y = getNavAngleDeltaAxis(inputID, 1) * .025;

		drawLine(pos->x + (LINE_WIDTH), pos->y + z, pos->z + x, pos->x - (LINE_WIDTH), pos->y - z, pos->z - x, curscolor);	
		drawLine(pos->x - z, pos->y + (LINE_WIDTH ), pos->z + y, pos->x + z, pos->y - LINE_WIDTH, pos->z - y, curscolor);
		drawLine(pos->x - x, pos->y - y, pos->z + LINE_WIDTH, pos->x + x, pos->y + y, pos->z - LINE_WIDTH, curscolor);

	}

}

//! returns the internal (in) voxie_inputs_t struct.
/**
 * could be useful for users who want access to what the internal mouse inputs variables are at 
 * the internal voxie_inputs_t (which really just hold the mouse input variables) when updated when breath() is called.  
 * you can use setMouseState to override these values. 
 *
 * @return the internal mouse state as a voxie_inputs_t
 */
voxie_inputs_t VoxieBox::getMouseState()
{
	return voxie_inputs_t(in);
}
//! Override the internal voxie_input_t mouse state 
/**
 * Probably not that useful but its here for some advanced use case
 * see voxie_inputs_t to understand the mouse structure*
 *  
 * @param newMouse the new mouse settings to pass in (pass in a full voxie_inputs_t struct)
 */
void VoxieBox::setMouseState(voxie_inputs_t newMouse)
{
	in.bstat = newMouse.bstat;
	in.dmousx = newMouse.dmousx;
	in.dmousy = newMouse.dmousy;
	in.dmousz = newMouse.dmousz;
	in.obstat = newMouse.obstat;
}


//! Set the Space Nav's sensitivity. Used to manage the Space Nav being used as a cursor. 0.0025 is default. Higher values increase sensitivity  
/**
  * @param spaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *  @param newAmount	the new sensitivity amount. 0.0025 is default. Higher values increase sensitivity
 */
void VoxieBox::setNavSensitivity(int spaceNavID, double newAmount)
{

	if (newAmount >= 1) newAmount *= 0.001; // if new amount is very high automatically scale it down
	this->navSensitivity[spaceNavID] = newAmount;

}


//! Set the Mouse's X and Y sensitivity. 0.001 is default. (0.0001 = low sensitive, 0.9 = extremely sensitive) Mouse movements work in integers the amount is scaled down
void VoxieBox::setMouseXYSensitivity(float newAmount)
{

	if (newAmount >= 1) newAmount *= 0.001; // if new amount is very high automatically scale it down
	this->mouseXYSensitivity = newAmount;

}

//! Set the Mouse's Z sensitivity. 0.0005 is default. Since Mouse movements work in integers the amount is scaled down
void VoxieBox::setMouseZSensitivity(float newAmount)
{

	if (newAmount >= 1) newAmount *= 0.001; // if new amount is very high automatically scale it down
	this->mouseZSensitivity = newAmount;

}
//! Set the time between mouse clicks to register a 'double click' (which triggers a true setting for getMouseDoubleClick())
/**
 * @param timeThreshold		the time in seconds for 2 clicks to register as a 'double click' 
 * Note use with VoxieBox::getMouseDoubleClick() to register double clicks.
 *
 * @see VoxieBox::getMouseDoubleClick()
 * @see VoxieBox::getMouseDoubleClickThreshold()
 */
void VoxieBox::setMouseDoubleClickThreshold(double timeThreshold)
{
	mouseClickThreshold = timeThreshold;
}
//! Returns the mouse double click threshold. (how quickly 2 mouse clicks signify a 'double click'). Presented in seconds.
/**
 * @return Returns the mouse double click threshold. (how quickly 2 mouse clicks signify a 'double click'). Presented in seconds.
 */
double VoxieBox::getMouseDoubleClickThreshold()
{
	return mouseClickThreshold;
}

//! Returns the current mouse Z sensitivity. Default is 0.0005
/**
 * @return Returns the current mouse Z sensitivity. Default is 0.0005
 */
float VoxieBox::getMouseZSensitivity() 
{
	return this->mouseZSensitivity;
}

//! Returns the current mouse XY sensitivity. Default is 0.001
/**
 * @return Returns the current mouse XY sensitivity. Default is 0.001
 */
float VoxieBox::getMouseXYSensitivity()
{
	return this->mouseXYSensitivity;
}



//! @return Returns the current Space Nav internal sensitivity setting
/**
 * @param spaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *
 * @return returns the current Space Nav internal sensitivity setting. Number between 1 and 0.0001.
 **/
double VoxieBox::getNavSensitivity(int spaceNavID)
{
	return this->navSensitivity[spaceNavID];
}


//! Reports mouse input state information onto secondary (touch) screen
/**
 *	This report uses the internal voxie_input_t struct as reference 
 *	(this could be overwritten with VoxieBox::setMouseState)
 *
 *	@param posX			the pixel x (horizontal) position to display the report
 *	@param posY			the pixel y (vertical) position to display the report
 *  @param showCursor	if true shows the mouse cursor on the volumetric display. Set to true by default
 */
void VoxieBox::reportMouse(int posX, int posY, bool showCursor)
{

	point3d mPos = getMousePosition();
	point3d mDelta = getMouseDelta();
	debugDrawBoxFill(posX, posY, posX + 215, posY + 110, 0x003333);
	debugText(posX, posY, 0xFFFFFF, -1, "            Mouse Report");
	debugText(posX, posY, 0x33FFFF, -1, "\n\nPos	X:      , Y:      , Z:      \nDelta  X:		, Y:		, Z:      \nSensitivity XY:  %0.5f, Z: %0.5f", getMouseXYSensitivity(), getMouseZSensitivity());
	debugText(posX, posY, 0xFF00FF, -1, "\n\n        %+1.3f    %+1.3f     %+1.3f\n           %-5.0f      %-5.0f       %-5.0f\n",
		mPos.x, mPos.y, mPos.z,
		mDelta.x, mDelta.y, mDelta.z);

	posY += 40;

	debugText(posX, posY, 0x33FFFF, -1, "But State:  Prev But State:   Ori:");
	debugText(posX, posY, 0xFFFFFF, -1, "          %d                %d      %d",
		getMouseButtonState(), getMousePrevButtonState(), getMouseOrientation()
		);

	posY += 13;

	debugText(posX, posY, 0x33FFFF, -1, "Double Click Threshold %1.3f", mouseClickThreshold);

	posY += 13;

	debugText(posX, posY, 0x33FFFF, -1, "LEFT   BUTTON (1)");
	if (getMouseButtonOnUp(MOUSE_LEFT_BUTTON))			debugText(posX, posY, 0xFF00FF, -1, "                  U");
	if (getMouseButtonOnDown(MOUSE_LEFT_BUTTON))			debugText(posX, posY, 0xFFFFFF, -1, "                   O");
	if (getMouseButtonIsDown(MOUSE_LEFT_BUTTON))			debugText(posX, posY, 0x00FF00, -1,	"                    D");
	if (getMouseButtonDownTime(MOUSE_LEFT_BUTTON) > 1)	debugText(posX, posY, 0xFF0000, -1, "                     H");
	if (getMouseDoubleClick(MOUSE_LEFT_BUTTON))			debugText(posX, posY, 0x00FFFF, -1, "                      C");

	debugText(posX, posY, 0xFFFF00, -1,                                          "                        %1.2f", getMouseButtonDownTime(MOUSE_LEFT_BUTTON));
	
	posY += 13;

	debugText(posX, posY, 0x33FFFF, -1, "RIGHT  BUTTON (2)");
	if (getMouseButtonOnUp(MOUSE_RIGHT_BUTTON))			debugText(posX, posY, 0xFF00FF, -1, "                  U");
	if (getMouseButtonOnDown(MOUSE_RIGHT_BUTTON))			debugText(posX, posY, 0xFFFFFF, -1, "                   O");
	if (getMouseButtonIsDown(MOUSE_RIGHT_BUTTON))			debugText(posX, posY, 0x00FF00, -1, "                    D");
	if (getMouseButtonDownTime(MOUSE_RIGHT_BUTTON) > 1)	debugText(posX, posY, 0xFF0000, -1, "                     H");
	if (getMouseDoubleClick(MOUSE_RIGHT_BUTTON))			debugText(posX, posY, 0x00FFFF, -1, "                      C");

	debugText(posX, posY, 0xFFFF00, -1, "                        %1.2f", getMouseButtonDownTime(MOUSE_RIGHT_BUTTON));

	posY += 13;
	debugText(posX, posY, 0x33FFFF, -1, "MIDDLE BUTTON (4)");
	if (getMouseButtonOnUp(MOUSE_MIDDLE_BUTTON))			debugText(posX, posY, 0xFF00FF, -1, "                  U");
	if (getMouseButtonOnDown(MOUSE_MIDDLE_BUTTON))		debugText(posX, posY, 0xFFFFFF, -1, "                   O");
	if (getMouseButtonIsDown(MOUSE_MIDDLE_BUTTON))		debugText(posX, posY, 0x00FF00, -1, "                    D");
	if (getMouseButtonDownTime(MOUSE_MIDDLE_BUTTON) > 1)	debugText(posX, posY, 0xFF0000, -1, "                     H");
	if (getMouseDoubleClick(MOUSE_MIDDLE_BUTTON) )		debugText(posX, posY, 0x00FFFF, -1, "                      C");

	debugText(posX, posY, 0xFFFF00, -1, "                        %1.2f", getMouseButtonDownTime(MOUSE_MIDDLE_BUTTON));

	if (showCursor) drawCursor(&mousePos, 0, 0, 0xffffff );

}


//! Add custom touch keys. (enable touch keyboard under 'Misc' menu tab) 
/** 
 *	pass through an array of const touchkey_t setup your own custom touch screen layout
 *  Make sure 'touch controls' are enabled via on the misc. tab on the VoxieMenu
 *  @params touchkey_t * touchkey		pointer to an array of const touchkey_t structs with definitions. 
 *  @params sizeOfArray				size of how many touch buttons in the array to add
 *
 *  Note : call touchAddCustomLayout(NULL,NULL) to return to default layout
 *  each touchkey_t struct contains one key the params are

				char* 	title		- the title of the key
				int 	xpos  		- the x position of where to draw the touch button
				int 	ypos  		- the y position of where to draw the touch button
				int 	xsize 		- the size or the horizontal length of the button written as pixels
				int 	ysize 		- the size or the horizontal length of the button written as pixels
				int		frontColor	- the hexadecimal color value for the button
				int		backColor	- the hexadecimal color value for the button's background. -1 for transparent
				int 	scanCode	- the scancode / keyboard scan code * 256 + ASCII code (0 if N/A).

				Note special keycodes for mouse: Mouse:0x0000, LBut:0x0001, RBut:0x0002, MBut:0x0003

				// To make a custom function callback link the touchkey_t struct to a scancode that isn't being used for example (0x8080, 0x8181)
				//               Scan codes used:                                   ASCII codes used:
				//00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    00 01 02 03  .  .  .  . 08 09  .  .  . 0d  .  .
				//10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f     .  .  .  .  .  .  .  .  .  .  . 1b  .  .  .  .
				//20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f    20  .  .  .  .  .  . 27  .  . 2a 2b 2c 2d 2e 2f
				//30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f    30 31 32 33 34 35 36 37 38 39  . 3b  . 3d  .  .
				//40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
				//50 51 52 53  .  .  . 57 58  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  . 5b 5c 5d  .  .  <--0x54..0x56 (3), 0x59..0x5a (2), 0x5e..0x5f (2)
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .    70 71 72 73 74 75 76 77 78 79 7a  .  .  .  .  .  <--0x7b..0x9b (33)
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
				// .  .  .  .  .  .  .  .  .  .  .  . 9c 9d  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  <--0x9e..0xb4 (23)
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
				// .  .  .  .  . b5  . b7 b8  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  <--0xb9..0xc4 (12)
				// .  .  .  .  . c5  . c7 c8 c9  . cb  . cd  . cf     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
				//d0 d1 d2 d3  .  .  .  .  .  .  .  .  . dd  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  <--0xd4..0xdc (9), 0xde..0xff (34)
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
				// .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .     .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .

				example: 
				touchkey_t = { "Touch\nonly\nFunc0", 80,100,200,200,0x405060,-1,0x8080 };
				voxie->getKeyIsDown(0x80) { ... your custom function here ... }



 * to actually see the custom touch controls you need to have TOUCH CONTROLLS ENABLED
 * by default they are turned off - they can be turned on by pressing the button in the 'misc' menu
 * or adding 'touchcontrols=1' to voxiebox.ini
 */

void VoxieBox::AddTouchKeyboardLayout(const touchkey_t * touchkey , int  sizeOfArray )
{
	voxie_touch_custom(touchkey, sizeOfArray);
}


/**! reads out the current Touch input states - raw return codes from the voxiebox.dll 
 * use this function if you want to manage advanced touch input manually. 
 * @param	touchIndex		index of finger in this packet
 * @param	xVal			the x value coordinate of touch instance
 * @param	yVal			the y value coordinate of touch instance
 * @param	packetState		if (packetState&1): this is 1st  packet in path
 *							if (packetState&2): this is last packet in path ('touchIndex' will not exist in later calls)
 *
 * 
 * @returns 0:no more data left (touchIndex,xVal,yVal,packetState not written)
 *          1:got more data (touchIndex,xVal,yVal,packetState written)
 *
 *					@example	This function should be called in a while loop until 0 is returned, ex:
 *								while (voxie_touch_read(&i,&x,&y,&j)) { //process i,x,y,j }
 */
int VoxieBox::touchManualRead(int * touchIndex, int * xVal, int * yVal, int * packetState)
{
	return this->voxie_touch_read(touchIndex, xVal, yVal, packetState);
}

/**! set to run to allow the secondary (touch) to be used as an input device.  
 * if you don't allow this any touch on the touchscreen will open up the voxieMenu
 * @param option set to true or false to enable 
 *
 */
void VoxieBox::setEnableTouchInput(bool option)
{
	switch (option) {
		case 1: //set this to disable menu touch
			if (!(vw.hacks >> 6 & 1)) vw.hacks += 64; 
			touchUpdate = true;
			voxie_init(&vw);
		break;
		case 0: //set this to enable menu touch
			if (vw.hacks >> 6 & 1) vw.hacks -= 64; 
			touchUpdate = false;
			voxie_init(&vw);
		break;
	}
}

/** boolean to enable / disable drawing the touch inputs unto the (secondary) touch screen
 *  @param	option set to true to enable drawing and to false to not draw.
 *
 */
void VoxieBox::setDrawTouchInput(bool option)
{
	touchIsDrawing = option;
}

/** returns the X delta movement of a touch point. To determine what touch point an index is used.
 *  @param index the index of the touch point to check
 *  @return a pixel horizontal X location of the touch point
 *  Note :  If index = -1 the global delta is used (the sum of all touch inputs deltas)
 */
int VoxieBox::getTouchDeltaX(int index)
{
	if (index > TOUCH_MAX || index < -1) index = -1;

	if (index == -1) return touch.gDeltaX;

	return touch.tPoint[index].deltax;

}
/** returns the Y delta movement of a touch point. To determine what touch point an index is used.
 *  @param index the index of the touch point to check
 *  @return a pixel horizontal Y location of the touch point
 *  Note :  If index = -1 the global delta is used (the sum of all touch inputs deltas)
 */
int VoxieBox::getTouchDeltaY(int index)
{
	if (index > TOUCH_MAX - 1|| index < -1) index = -1;

	if (index == -1) return touch.gDeltaY;

	return touch.tPoint[index].deltay;

}
/** returns the a touch point's X position. To determine what touch point an index is used
 *	@param index the index of the touch point to check
 *	@return a pixel vertical X location of the touch point
 *  Note : A for loop (i = 0; i < TOUCH_MAX; i++) can be used to iterate though all the touch points
 */
int VoxieBox::getTouchPosX(int index)
{
	if (index < 0 || index > TOUCH_MAX - 1) return -1;

	return this->touch.tPoint[index].posx;

}
/** returns the a touch point's Y position. To determine what touch point an index is used   
 *	@param index the index of the touch point to check 
 *	@return a pixel vertical Y location of the touch point 
 *  Note : A for loop (i = 0; i < TOUCH_MAX; i++) can be used to iterate though all the touch points
 */
int VoxieBox::getTouchPosY(int index)
{
	if (index < 0 || index > TOUCH_MAX - 1) return -1;

	return this->touch.tPoint[index].posy;
}
/** returns the touch point's state. Requires to know which touch point index to check 
 *	@param index the index of the touch point to check 
 *	@return 0 = not pressed, 1 = is down, 2 = is held, 3 = just pressed, 4 = on up
 */
int VoxieBox::getTouchState(int index)
{
	int result = TOUCH_STATE_NOT_PRESSED;
	if (index < 0 || index > TOUCH_MAX - 1) return -1;

	if (touch.tPoint[index].onUp)				result = TOUCH_STATE_ON_UP;
	else if (touch.tPoint[index].justPressed)	result = TOUCH_STATE_JUST_PRESSED;
	else if (touch.tPoint[index].isHeld)		result = TOUCH_STATE_IS_HELD;
	else if (touch.tPoint[index].isDown)		result = TOUCH_STATE_IS_DOWN;

	return result;
}
//! returns a pinch's touch rotation delta (the amount the two points of the pinch have changed in distance) @return float in radians of rotation
float VoxieBox::getTouchRotationDelta()
{
	return  touch.pinchRotationDelta;
}
//! returns a pinch's touch distance delta (the amount the two points of the pinch have changed in distance) @return float in pixels of distance 
float VoxieBox::getTouchDistanceDelta()
{
	return touch.pinchDistanceDelta;
}


/** checks if a touch input has been registered within a 2D box returns a positive number if touch has been detected
 *  @param TLpos the top left point of the rectangle to check within
 *	@param BRpos the bottom right point of the rectangle to check within
 *	@param drawCollision	set to true to draw the collision box on the secondary (touch) screen
 *	@returns 0 = no touch, 1 = touch is down, 2 = touch is held, 3 = touch just pressed 4 = touch on up.
 */
int VoxieBox::getTouchPressState(point2d TLpos, point2d BRpos, bool drawCollision )
{
	int i = 0;
	int result = TOUCH_STATE_NOT_PRESSED;
	point2d tp = { 0 };

	for (i = 0; i < TOUCH_MAX; i++) {
		if (touch.tPoint[i].active == false) continue;

		tp.x = touch.tPoint[i].posx;
		tp.y = touch.tPoint[i].posy;
	
		if (boxInsideCollideChk2D(&TLpos, &BRpos, &tp, drawCollision)) {

			if (touch.tPoint[i].onUp)				result = TOUCH_STATE_ON_UP;
			else  if (touch.tPoint[i].justPressed)	result = TOUCH_STATE_JUST_PRESSED;
			else if (touch.tPoint[i].isHeld)		result = TOUCH_STATE_IS_HELD;
			else if	(touch.tPoint[i].isDown)		result = TOUCH_STATE_IS_DOWN;

			break;
		}

	}

	return result; 
}

/** checks if a touch input has been registered within a 2D circle returns a positive number if touch has been detected
 *  @param CirPos			the X Y circle position
 *	@param radius			the radius of the circle. 
 *  @param drawCollision	set to true to draw the collision box on the secondary (touch) screen
 *	@returns 0 = no touch, 1 = touch is down, 2 = touch is held, 3 = touch just pressed 4 = touch on up.
 */
int VoxieBox::getTouchPressStateCir(point2d CirPos, float radius, bool drawCollision)
{
	int i = 0;
	int result = TOUCH_STATE_NOT_PRESSED;
	point2d tp = { 0 };
	if (drawCollision != 0) drawCollision = 2;

	for (i = 0; i < TOUCH_MAX; i++) {
		if (touch.tPoint[i].active == false) continue;

		tp.x = touch.tPoint[i].posx;
		tp.y = touch.tPoint[i].posy;

		if (ciricle2DChk(&CirPos, radius, &tp, drawCollision)) {

			if (touch.tPoint[i].onUp)				result = TOUCH_STATE_ON_UP;
			else  if (touch.tPoint[i].justPressed)	result = TOUCH_STATE_JUST_PRESSED;
			else if (touch.tPoint[i].isHeld)		result = TOUCH_STATE_IS_HELD;
			else if (touch.tPoint[i].isDown)		result = TOUCH_STATE_IS_DOWN;

			break;
		}

	}

	return result;
}


/** checks if a touch input has been registered within a 2D box returns a positive number if touch has been detected
 *  @param TLpos			the top left point of the rectangle to check within
 *	@param BRpos			the bottom right point of the rectangle to check within
 *  @param drawCollision	set to true to draw the collision box on the secondary (touch) screen
 *	@returns the touch input index number
 */
int VoxieBox::getTouchPressIndex(point2d TLpos, point2d BRpos, bool drawCollision)
{
	int i = 0;
	int result = TOUCH_STATE_NOT_PRESSED;
	point2d tp = { 0 };

	for (i = 0; i < TOUCH_MAX; i++) {
		if (touch.tPoint[i].active == false) continue;

		tp.x = touch.tPoint[i].posx;
		tp.y = touch.tPoint[i].posy;

		if (boxInsideCollideChk2D(&TLpos, &BRpos, &tp, drawCollision)) {

			return i;

			break;
		}

	}

	return -1;
}

/** checks if a touch input has been registered within a 2D circle returns the touch inputs index number
 *  @param CirPos the X Y circle position
 *	@param radius the radius of the circle.
 *  @param drawCollision	set to true to draw the collision box on the secondary (touch) screen
 *	@returns the touch input index number
 */
int VoxieBox::getTouchPressIndexCir(point2d CirPos, float radius, bool drawCollision)
{
	int i = 0;
	point2d tp = { 0 };
	if (drawCollision != 0) drawCollision = 2;

	for (i = 0; i < TOUCH_MAX; i++) {
		if (touch.tPoint[i].active == false) continue;

		tp.x = touch.tPoint[i].posx;
		tp.y = touch.tPoint[i].posy;

		if (ciricle2DChk(&CirPos, radius, &tp, drawCollision)) {

			return i;

			break;
		}

	}

	return -1;
}

//! gets a pointer to the internal vxInputTypes.h::touchInput_t struct. @returns a touchInput_t * pointer
touchInput_t * VoxieBox::getTouchInputStruct()
{
	return &this->touch;
}

/** Replaces the internal touchInput_t struct with a new one could be to make your own touch I/O. 
 *	@param newTouchInput * pointer to the new touchInput_t struct to replace
 */
void VoxieBox::setTouchInputStruct(touchInput_t * newTouchInput)
{
	memcpy(&this->touch, newTouchInput,  sizeof(touchInput_t));
}

/**  sets the touch sensitivity (1 is default. the higher the value the more sensitivity) @returns float value of sensitivity.   
 * @param newSensitivityValue the new sensitivity value.
 */
void VoxieBox::setTouchSensitivity(float newSensitivityValue)
{
	if (newSensitivityValue < 0) newSensitivityValue = 0.1;
	if (newSensitivityValue > 5) newSensitivityValue = 5;

	touch.sensitivity = newSensitivityValue;
}

//! returns the touch sensitivity (1 is default. the higher the value the more sensitivity) @returns float value of sensitivity.  
float VoxieBox::getTouchSensitivity()
{
	return touch.sensitivity;
}
/** Toggles focus pinch (off by default) if set to true if a pinch has been registered all other touch inputs are nulled
 *
 *	@param choice set to true or false to enable or disable focus pinch
 */
void VoxieBox::enableTouchFocusPinch(bool choice)
{
	touch.focusPinch = choice;
}
/** Shows a debug report for advanced touch input
 *
 *	@param posX			the horizontal X position of the secondary (touch) screen to display the report
 *	@param posX			the vertical Y position of the secondary (touch) screen to display the report
 *
 */
void VoxieBox::reportTouch(int posX, int posY)
{

	debugDrawBoxFill(posX, posY, posX + 500, posY + 180, 0x502020);
	debugText(posX +  175, posY, 0xffffff, -1, "Touch Report");
	posY += 13;
	int i = 0xffffff;
	debugText(posX, posY, 0xffc080, -1, "Touch Total %d  Ext Enabled %d, Sense %1.2f Pin Priority %d Focus Pinch %d", touch.currentTouchNo, vw.hacks >> 6 & 1, touch.sensitivity, touch.pinchPriority, touch.focusPinch);
	posY += 13;
	debugText(posX, posY, 0xffc080, -1, "Pinch A:%d 0X %1.0f 0Y %1.0f 1X %1.0f 1Y %1.0f\nDist %1.2f ODist %1.2f Delta %1.2f\nAng %1.2f OAng %1.2f AngD %1.2f", touch.pinchActive, touch.opinch0.x, touch.opinch0.y, touch.opinch1.x, touch.opinch1.y, touch.pinchDistance, touch.opinchDistance, touch.pinchDistanceDelta, touch.pinchRotation, touch.opinchRotation, touch.pinchRotationDelta);
	
	if (touch.pinchRotationDelta > TOUCH_ANGLE_DEAD_ZONE) i = 0x00ff00;
	else if (touch.pinchRotationDelta < -TOUCH_ANGLE_DEAD_ZONE) i = 0xff0000;
	posY += 26;
	debugText(posX, posY, i, -1, "Angle");

	i = 0xffffff;
	if (touch.pinchDistanceDelta > TOUCH_DISTANCE_DEAD_ZONE) i = 0x00ff00;
	else if (touch.pinchDistanceDelta < -TOUCH_DISTANCE_DEAD_ZONE) i = 0xff0000;

	debugText(posX, posY, i, -1, "             Distance");
	
	posY += 13;
	for (i = 0; i < TOUCH_MAX; i++) {

		debugText(posX, posY + (i * 10), 0xffc080, -1, "touch X %d Y %d Delta X %d Y %d state %d ostate %d utim %1.2f stim %1.2f  ", touch.tPoint[i].posx, touch.tPoint[i].posy, touch.tPoint[i].deltax, touch.tPoint[i].deltay, touch.tPoint[i].state, touch.tPoint[i].ostate, touch.tPoint[i].lastUpdate, touch.tPoint[i].startTime);

		if (touch.tPoint[i].active)			debugText(posX, posY + (i * 10), 0xff0000, -1, "								   	                                           A  ");
		if (touch.tPoint[i].isDown)			debugText(posX, posY + (i * 10), 0x00ffff, -1, "								   	                                            D %1.2f  ", time - touch.tPoint[i].startTime);
		if (touch.tPoint[i].justPressed) 	debugText(posX, posY + (i * 10), 0x00ff00, -1, "									                                                    P");
		if (touch.tPoint[i].onUp) 			debugText(posX, posY + (i * 10), 0xffff00, -1, "									                                                     U");
		if (touch.tPoint[i].inPinch) 		debugText(posX, posY + (i * 10), 0xff00ff, -1, "									                                                       I");

	}
}

//!  toggles a white border around the perimeter of the volumetric display.
void VoxieBox::setBorder(bool option)
{
	drawBorder = option;
}


//! when enabled and VoxieBox::quitLoop is called the Voxiebox library is freed from memory
void VoxieBox::setCleanExitOnQuitLoop(bool option)
{
	enableCleanExit = option;
}

//! toggles when enabled and VoxieBox::quitLoop() is called the Voxiebox library is freed from memory. Set to true by default.
void VoxieBox::setEnableLegacyKeyInput(bool option)
{
	manualKeyManage = option;
}

//! Set to true to handle Joy input manually. Disables many of the joy input functions.	Set to false by default
void VoxieBox::setEnableLegacyJoyInput(bool option)
{
	manualJoyManage = option;
}
//! Set to true to handle Nav input manually. Disables many of the nav input functions.	Set to false by default
void VoxieBox::setEnableLegacyNavInput(bool option) 
{
	manualNavManage = option;
}
void VoxieBox::setEnableLegacyTouchInput(bool option)
{
	manualTouchManage = option;
}
//! Inverts the Z axis. (by default - values are at the top of the screen and + values are below). Set to false by default.
void VoxieBox::setInvertZAxis(bool option)
{
	invertZAxis = option;
}
//! Toggles a joystick / gamepad dead zone on analog sticks. Set to true by default
void VoxieBox::setEnableJoyDeadZone(bool option)
{
	enableJoyDeadZone = option;
}
//! Toggles a dead zone for Space Navigator. Set to true by default.
void VoxieBox::setEnableNavDeadZone(bool option)
{
	enableNavDeadZone = option;
}

//! Toggles the use of the 'esc' key to escape a Voxon program (by calling VoxieBox::quitLoop())
void VoxieBox::setEnableExitOnEsc(bool option)
{
	enableEscQuit = option;
}

//! Toggles nav clipping (cursor / position is always inside the volume) when using the VoxieBox::getNavPosition or the VoxieBox::drawCursor() functions.  Set to false by default.
void VoxieBox::setEnableNavClipping(bool option)
{
	enableNavClip = option;
}

//! Set to true if you want the mouse to clip (cursor is always inside the volume) when using the VoxieBox::getMousePosition or the VoxieBox::drawCursor() functions.
void VoxieBox::setEnableMouseClipping(bool option)
{
	enableMouseClip = option;
}

//! Sets VoxieBox class to use the XInput API to read joystick inputs (for modern game controllers).
void VoxieBox::setJoyInputToXInput()
{
	vw.usejoy = 1;
	init();
}

//! Set VoxieBox class to use the DirectInput API to read joystick inputs (for older game controllers). Uses joyGetPosEx function.
void VoxieBox::setJoyInputToDirectInput()
{
	vw.usejoy = 0;
	init();
}

//! Returns 1 if controller's button is pressed down. See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes  
/**
 * @param controllerID	Which game controller to check (0 - 3)
 * @param joyButtonCode Which button press to check 
 *
 *  	    JOY_DPAD_UP				= 0,	//!< bit 0 value 1			Digital Dpad Up				
 *	        JOY_DPAD_DOWN			= 1,	//!< bit 1 value 2			Digital Dpad Down 
 *	        JOY_DPAD_LEFT			= 2,	//!< bit 2 value 4			Digital Dpad Left
 * 	        JOY_DPAD_RIGHT			= 3,	//!< bit 3 value 8			Digital Dpad Right
 *	        JOY_START				= 4,	//!< bit 4 value 16			Start Button
 *       	JOY_BACK				= 5,	//!< bit 5 value 32			Back Button
 *       	JOY_LEFT_THUMB			= 6,	//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 *       	JOY_RIGHT_THUMB			= 7,	//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 *       	JOY_LEFT_SHOULDER		= 8,	//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
 *       	JOY_RIGHT_SHOULDER		= 9,	//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
 *       	JOY_A					= 12,	//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 *       	JOY_B					= 13,	//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 *      	JOY_X					= 14,	//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 *       	JOY_Y					= 15	//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller
 *
 * 
 * @returns 1 if controller's button is held down and 0 if not 
 *
 */
int VoxieBox::getJoyButtonIsDown(int controllerID, int joyButtonCode)
{
	return ((vx[controllerID].but >> joyButtonCode) & 1);
}

//! Returns 1 if controller's button is just pressed. See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes.
/**
 * @param controllerID	Which game controller to check (0 - 3)
 * @param joyButtonCode Which button press to check
 * 
 *  	    JOY_DPAD_UP				= 0,	//!< bit 0 value 1			Digital Dpad Up				
 *	        JOY_DPAD_DOWN			= 1,	//!< bit 1 value 2			Digital Dpad Down 
 *	        JOY_DPAD_LEFT			= 2,	//!< bit 2 value 4			Digital Dpad Left
 * 	        JOY_DPAD_RIGHT			= 3,	//!< bit 3 value 8			Digital Dpad Right
 *	        JOY_START				= 4,	//!< bit 4 value 16			Start Button
 *       	JOY_BACK				= 5,	//!< bit 5 value 32			Back Button
 *       	JOY_LEFT_THUMB			= 6,	//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 *       	JOY_RIGHT_THUMB			= 7,	//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 *       	JOY_LEFT_SHOULDER		= 8,	//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
 *       	JOY_RIGHT_SHOULDER		= 9,	//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
 *       	JOY_A					= 12,	//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 *       	JOY_B					= 13,	//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 *      	JOY_X					= 14,	//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 *       	JOY_Y					= 15	//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller
 *
 * 
 * @returns 1 if controller's button is just pressed  0 if not. 
 * 
 * Note : this function works differently to 'IsDown' is down just registers if the button is being 
 * pressed while on down is the first instance of the button being pressed before it is released again.
 */
int VoxieBox::getJoyButtonOnDown(int controllerID, int joyButtonCode)
{
	return ((vx[controllerID].but >> joyButtonCode) & 1) && !((ovxbut[controllerID] >> joyButtonCode) & 1);
}
//! Returns 1 if controller's button is just released.  See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes.
/**
 * @param controllerID	Which game controller to check (0 - 3)
 * @param joyButtonCode Which button press to check
 *
 *  	    JOY_DPAD_UP				= 0,	//!< bit 0 value 1			Digital Dpad Up
 *	        JOY_DPAD_DOWN			= 1,	//!< bit 1 value 2			Digital Dpad Down
 *	        JOY_DPAD_LEFT			= 2,	//!< bit 2 value 4			Digital Dpad Left
 * 	        JOY_DPAD_RIGHT			= 3,	//!< bit 3 value 8			Digital Dpad Right
 *	        JOY_START				= 4,	//!< bit 4 value 16			Start Button
 *       	JOY_BACK				= 5,	//!< bit 5 value 32			Back Button
 *       	JOY_LEFT_THUMB			= 6,	//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 *       	JOY_RIGHT_THUMB			= 7,	//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 *       	JOY_LEFT_SHOULDER		= 8,	//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog
 *       	JOY_RIGHT_SHOULDER		= 9,	//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog
 *       	JOY_A					= 12,	//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 *       	JOY_B					= 13,	//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 *      	JOY_X					= 14,	//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 *       	JOY_Y					= 15	//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller
 *
 *
 * @returns 1 if controller's button is just released  0 if not.
 *
 */
int VoxieBox::getJoyButtonOnUp(int controllerID, int joyButtonCode)
{
	return !((vx[controllerID].but >> joyButtonCode) & 1) && ((ovxbut[controllerID] >> joyButtonCode) & 1);
}


//! Updates the game controller's input state. Used to manage game controller inputs when setEnableLegacyJoyInput() is set to true
/**
 *  Use this function to manually update gamepad inputs. 
 *  The return state is designed to be used within a for loop. 
 *  This function is only needed when setEnableLegacyJoyInput() is set to true
 *
 *	@param id which controller to query (0-3)
 *	@param voxie_xbox_t structure to be filled in by function (if successful)
 *	@return 1 = success (safe to check next controller (id+1)) , 0=fail
 */
int VoxieBox::updateJoyState(int controllerID, voxie_xbox_t * vx)
{
	return voxie_xbox_read( controllerID,  vx);
}

//! Activates vibration motors in XBox controller. XInput controllers only. To stop Be sure to call again with lmot=0.f, rmot=0.f!
/**
 * @param id				which controller to set (0-3)
 * @param leftMotorSpeed	left motor speed (0.f-1.f, 0.f=off)
 * @param rightMotorSpeed	right motor speed (0.f-1.f, 0.f=off)
 *
 */
void VoxieBox::setJoyVibrate(int controllerID, float leftMotorSpeed, float rightMotorSpeed)
{
	voxie_xbox_write(controllerID, leftMotorSpeed, rightMotorSpeed);
}

//! Returns the number of USB game controllers detected by the system.
/**
 *  @return Returns the number of USB game controllers detected by the system.
 */
int VoxieBox::getJoyNum()
{
	return vxnplays;
}

//! @return returns the number of SpaceNavs detected by the system.
/** Note for a SpaceNav to be detected it must be moved (is detected when any of the SpaceNav's axis movements) 
 *
 *
 */
int VoxieBox::getNavNum() 
{
	return navplays;
}

//! returns a value in seconds of how long a controller's button has been held down for. Requires setEnableLegacyJoyInput() to be set to false to work.  
/**
 * @param controllerID		Which game controller to check(0 - 3)
 * @param joyButtonCode		Which button press to check
 *
 *  	    JOY_DPAD_UP = 0,			//!< bit 0 value 1			Digital Dpad Up
 *          JOY_DPAD_DOWN = 1,			//!< bit 1 value 2			Digital Dpad Down
 *          JOY_DPAD_LEFT = 2,			//!< bit 2 value 4			Digital Dpad Left
 *          JOY_DPAD_RIGHT = 3,			//!< bit 3 value 8			Digital Dpad Right
 *          JOY_START = 4,				//!< bit 4 value 16			Start Button
 *          JOY_BACK = 5,				//!< bit 5 value 32			Back Button
 *          JOY_LEFT_THUMB = 6,			//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 *          JOY_RIGHT_THUMB = 7,		//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 *          JOY_LEFT_SHOULDER = 8,		//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog
 *          JOY_RIGHT_SHOULDER = 9,		//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog
 *          JOY_A = 12,					//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 *          JOY_B = 13,					//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 *          JOY_X = 14,					//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 *          JOY_Y = 15					//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller
 *
 * @returns a value in seconds of how long controller's button has been held. Returns 0 if not button is not being pressed.
 *
 */
double VoxieBox::getJoyButtonDownTime(int controllerID, int joyButtonCode)
{
	return joyHistory[controllerID].history[joyButtonCode].duration;
}


/**	Copies 3DConnexion Space Navigator controller state to voxie_nav_t structure
 * @param id  which controller to query (ignored for now)
 * @param nav  structure to be filled in by function (if successful)
 * @return 1=success (safe to check next controller (id+1)), 0=fail
 * 
 * Works similar to VoxieBox::updateJoyState() use if setEnableLegacyNavInput() is set to true. Otherwise Nav updates are maintained by VxCpp.dll
 */
int VoxieBox::updateNavState(int spaceNavID, voxie_nav_t * nav)
{
	return voxie_nav_read(spaceNavID, nav);
}


/** Reports all SpaceNav input state onto the secondary (touch screen)
 * @param posX the horizontal position to display the report
 * @param posY the vertical position to display the report
 * @param showCursor (true by default) draw the cursor of the position of the SpaceNav
 *
 *
 * Note: Space Navs are only detected by the system after an inital movement. 
 */
void VoxieBox::reportNav(int posX, int posY, bool showCursor)
{

	if (manualNavManage == true) {
		debugText(posX, posY, 0xFF0000, -1, "Nav Report - disabled (manual Nav manage mode is enabled) ");
		return;
	}
	int nposY = posY;
	int i = getNavNum();
	point3d navDDelta = { 0 };
	point3d navADelta = { 0 };
	int col[4] = { 0xffff00, 0x00ffff, 0xff0000, 0x00ff00 };
	debugDrawBoxFill(posX, posY, posX + 290, posY + 60 + (130 * i), 0x003300);
	debugText(posX, posY, 0xFFFFFF, -1, "                 SpaceNav Report");
	int navZsize = 0;

	int navCol = 0xffffff;
	point2d plotPos = { 0 };
	nposY += 14;
	debugText(posX, nposY, 0xFFFFFF, -1, "# Controllers:    ");
	debugText(posX, nposY, 0x00FF00, -1, "               %d ", i);
	nposY += 14;
	debugText(posX, nposY, 0xFFFFFF, -1, "Dead Zone On:    Threshold: ");
	debugText(posX, nposY, 0xFF00FF, -1, "              %d              %1.3f", enableNavDeadZone, getNavDeadZone());

	for (i = 0;  i < navplays; i++) {
		navDDelta = getNavDirectionDelta(i);
		navADelta = getNavAngleDelta(i);
		nposY += 14;
		nposY += 14;
		debugText(posX, nposY, 0xFFFFFF, -1,						"Sensitivity:  %0.5f", getNavSensitivity(i));
		debugText(posX, nposY, col[getNavOrientation(i)], -1, "                            Ori: %d", getNavOrientation(i));

		nposY += 14;
		debugText(posX, nposY, 0xffFFff, -1, "Pos	X:      , Y:      , Z:   ");
		debugText(posX, nposY, 0x00FF00, -1, "        %+1.2f     %+1.2f     %+1.2f",
			navPos[i].x , navPos[i].y, navPos[i].z);
		nposY += 14;
		debugText(posX, nposY, 0xFF0000, -1, "              %+3.3f", navDDelta.x);
		debugText(posX, nposY, 0x00FF00, -1, "                          %+3.3f", navDDelta.y);
		debugText(posX, nposY, 0x00aaFF, -1, "                                       %+3.3f", navDDelta.z);
		debugText(posX, nposY, 0xffffff, -1, "Delta Move  X:		    Y:		     Z:      ");
		nposY += 14;
		debugText(posX, nposY, 0xFF0000, -1, "              %+3.3f", navADelta.x);
		debugText(posX, nposY, 0x00FF00, -1, "                          %+3.3f", navADelta.y);
		debugText(posX, nposY, 0x00aaFF, -1, "                                       %+3.3f", navADelta.z);
		debugText(posX, nposY, 0xffffff, -1, "Delta Angle X:		    Y:		     Z:      ");
		nposY += 20;
		debugText(posX, nposY, 0xffffff, -1, "But State :   Prev State : ");
		debugText(posX, nposY, 0xffff00, -1, "           %d              %d", getNavButtonState(i), getNavPrevButtonState(i));
		
		nposY += 14;
		debugText(posX, nposY, 0x33FFFF, -1, "LEFT    BUTTON (1)");
		if (getNavButtonOnUp(i, NAV_LEFT_BUTTON))			debugText(posX, nposY, 0xFF00FF, -1, "                  U");
		if (getNavButtonOnDown(i, NAV_LEFT_BUTTON))			debugText(posX, nposY, 0xFFFFFF, -1, "                   O");
		if (getNavButtonIsDown(i, NAV_LEFT_BUTTON))			debugText(posX, nposY, 0x00FF00, -1, "                    D");
		if (getNavButtonDownTime(i, NAV_LEFT_BUTTON) > 1)	debugText(posX, nposY, 0xFF0000, -1, "                     H");
		if (getNavDoubleClick(i, NAV_LEFT_BUTTON))			debugText(posX, nposY, 0x00FFFF, -1,  "                      C");
		debugText(posX, nposY, 0xFFFF00, -1, "                        %1.2f", getNavButtonDownTime(i, NAV_LEFT_BUTTON));
		nposY += 14;
		debugText(posX, nposY, 0xFFFF33, -1, "RIGHT   BUTTON (2)");
		if (getNavButtonOnUp(i, NAV_RIGHT_BUTTON))			debugText(posX, nposY, 0xFF00FF, -1, "                  U");
		if (getNavButtonOnDown(i, NAV_RIGHT_BUTTON))			debugText(posX, nposY, 0xFFFFFF, -1, "                   O");
		if (getNavButtonIsDown(i, NAV_RIGHT_BUTTON))			debugText(posX, nposY, 0x00FF00, -1, "                    D");
		if (getNavButtonDownTime(i, NAV_RIGHT_BUTTON) > 1)	debugText(posX, nposY, 0xFF0000, -1, "                     H");
		if (getNavDoubleClick(i, NAV_RIGHT_BUTTON))			debugText(posX, nposY, 0x00FFFF, -1, "                      C");
		debugText(posX, nposY, 0xFFFF00, -1, "                        %1.2f", getNavButtonDownTime(i, NAV_RIGHT_BUTTON));

		plotPos.x = getNavDirectionDeltaAxis(i, NAV_X_AXIS);
		plotPos.y = getNavDirectionDeltaAxis(i, NAV_Y_AXIS);
		navCol = 0x00ff00;
		if (fabs(plotPos.x) < getNavDeadZone() && fabs(plotPos.y) < getJoyDeadZone()) navCol = 0xff0000;

		debugDrawCircFill(posX + 215, nposY, 25, 0x9d9d9d);
		debugDrawCircFill(posX + 215, nposY, 0 + ( getNavDeadZone() * 25), 0x904040);
		navZsize = (4 * (1 + (-navPos[i].z * 2)));
		if (navZsize > 13) navZsize = 13;
		if (navZsize < 0) navZsize = 1;

		debugDrawCircFill(posX + 215 + ((plotPos.x) * 25), nposY + ((plotPos.y) * 25), 2 + navZsize, navCol);
		nposY += 14;

		debugBar(posX + 245, nposY - 40, -(getNavDirectionDeltaAxis(i, NAV_Z_AXIS)) * 1000, 1000, 0, (char*)" ", 3);
		debugText(posX + 200, nposY + 15, 0xffffff, -1, " XY       Z");

		if (showCursor) drawCursor(&getNavPosition(i), 2, i, col[i]);

	}


}


//! Function used to reset the voxie menu and set a new custom menu update function (the menu which is on the secondary touch screen)
/**	
 *
 * This function is essential if you want to make your own menu tabs as you'll need to create a function which holds all your menu's logic
 * The menu_update function is where a user makes their own function which manages the menu's input.
 *
 *
 *
 * @param menu_update		callback function to handle interaction updates. See example user function below.
 * @param userdata			a pointer to a user-defined structure (optional / for convenience only).
 * @param bgImageFileName	an image file to be used as background (should be 1024x600 for the LCD display)
 *
 *
 * The menu_update( int id, char *st, double val. int how, void * userdata) is called every time a menu button is pressed.
 * each menu item has an id and passes through certain values depending on its item type.
 *
 *                  Parameters for the menu_update() custom function:
 *                      id = the id for the menu item which is calling the function.
 *                      st = the custom pointer char array being passed in from the menu item. (for user edit box (MENU_EDIT))
 *                      val = the custom val passed in from the menu item this is how slider values are passed in
 *						how = tells how button or slider was changed: (0:enter, 1:left/right arrow, 2:click button, 3:drag slider, 4:slider arrow)
 *						userdata = pointer to user-defined structure (helpful to avoid global variables)
 *
 *
 *
 *            @example Example of custom menu call back function 
 *			  enum {MENU_SPEED_BUTTON_1,MENU_SPEED_BUTTON_2,MENU_SPEED_BUTTON_3,MENU_SPEED_SLIDER,MENU_FILE};
 *			  static int gspeed = 1;
 *            static int user_menu_update(int id, char *st, double v, int how, void *userdata)
 *             {
 *             	switch (id)
 *             	{
 *             	case MENU_SPEED_BUTTON_1: gspeed = 1; break; (MENU_SPEED_BUTTON_1 (1) is menuAddItem's id - defined when the menu item was created)
 *             	case MENU_SPEED_BUTTON_2: gspeed = 2; break;
 *             	case MENU_SPEED_BUTTON_3: gspeed = 3; break;
 *             	case MENU_SPEED_SLIDER: gspeed = (int)v; break;
 *             	case MENU_FILE: printf("File:%s\n", st); break; //NOTE:don't use printf in graphical app :P
 *             	}
 *             	return(1);
 *             }
 *
 *			   At some point in your VX program (usually before your breath() update loop). call menuReset and define your call back function. menuReset(user_menu_update, 0, (char*)"test.png");
 *
 * Note: call with voxie_menu_reset(0,0); to remove the user menu

 */
void VoxieBox::menuReset(int(*menu_update)(int id, char *st, double val, int how, void *userdata), void * userdata, char * bgImageFileName)
{
	voxie_menu_reset(menu_update, userdata, bgImageFileName);
}

//! Add custom menu tab on secondary (touch) screen  menu (NOTE: there's only space for 2 more tabs on the 7" 1024x600 LCD screen)
/**  @param	st	name of tab
 *   @param	x	horizontal position for the tab item area 
 *	 @param y	vertical position for tab item area
 *	 @param	xs	width in pixels of the tab item area
 *	 @param ys	height in pixels of the tab item area
 *  
 *  menu functions only needs to be called once. Not every volume/frame.  
 */
void VoxieBox::menuAddTab(char *st, int x, int y, int xs, int ys)
{
	voxie_menu_addtab(st, x, y, xs, ys);
}

//! Add item to a menu tab
/**
* Use this function to add a new menu item to the menu's tab. See the enum from vxInputTypes.h::menuTypes for a list of menu item to add.
* @param label name of text/button/slider
* @param x starting x position for menu item
* @param y starting y position for menu item
* @param xSize horizontal size of item
* @param ySize vertical size of item
* @param id user-defined low integer (use enum to differentiate easily). Assign a unique number (ID) so the menu item can be recalled by other functions
* @param type the type of menu item to create see vxInputTypes.h::menuTypes
*           MENU_TEXT:     text (decoration only)
*           MENU_LINE:     line (decoration only)
*           MENU_BUTTON+3: push button (single button)
*           MENU_BUTTON+1: push button (first in group - auto-depresses others so only 1 on)
*           MENU_BUTTON  : push button (in middle of group)
*           MENU_BUTTON+2: push button (last in group - auto-depresses others so only 1 on)
*           MENU_HSLIDER:  horizontal slider
*           MENU_VSLIDER:  vertical slider
*           MENU_EDIT:     edit text box
*           MENU_EDIT_DO:  edit text box, click next item on 'Enter'
*           MENU_TOGGLE:   combo box (w/o the drop down). Useful for saving space in dialog.
*                          Specify multiple strings in 'st' using \r as separator.
*           MENU_PICKFILE: file selector. Specify type in 2nd string. Ex: "Browse\r*.kv6"
*  @param col color of item (typically 0xffffff)
*  @param startingVal the starting value of menu item
*  @param minVal min value  
*  @param maxVal max value 
*  @param minStepVal  minor step in value increase/decrease amount
*  @param majStepVal  major step in value increase/decrease amount
*/
void VoxieBox::menuAddItem(char * label, int x, int y, int xSize, int ySize, int id, int type, int state, int col, double startingVal, double minVal, double maxVal, double minStepVal, double majStepVal)
{
	voxie_menu_additem(label, x, y, xSize, ySize, id, type, state, col, startingVal, minVal, maxVal, minStepVal, majStepVal);

}

/** Once the menu is created, use this function to update a string, button status, or slider value
 *
 *  In the menu item. It will change the menu items for any id values that match. If st is null,
 *  it will be left alone. 'state' is the status of a menu button (0=up, 1=down), and 'v' is the value
 *  for sliders. If a field is not applicable, just pass a 0.
 *	@param	id		the menu item
 *	@param	st		changing the text (if applicable)
 *	@param  state	the state of the item (if applicable)
 *	@param	v		the value (if applicable)
 */
void VoxieBox::updateMenu(int id, char * st, int state, double v)
{
	voxie_menu_updateitem(id,  st, state, v);
}


/** Draws single pixel on the secondary (touch) screen. Must be called within the startFrame() & endFrame() functions.
 *  @param x		x position of pixel location
 *  @param y		y position of pixel location 
 *  @param col		24-bit RGB color
 */
void VoxieBox::debugDrawPix(int x, int y, int col = 0xffffff)
{
	voxie_debug_drawpix(x, y, col);
}

/** Draws horizontal line on the secondary (touch) screen.
 *
 * Drawing is always from left to right. Must be called between startFrame() & endFrame() functions.
 *  
 * @param xStartPos coordinate of the starting x pixel. Needs to be the left most pixel.
 * @param xEndPos	coordinate of the ending x pixel. Needs to be the right most pixel.
 * @param y			coordinate of the y pixel. Where the line is drawn on the y axis.
 * @param col		24-bit RGB color of the line.
 */
void VoxieBox::debugDrawHLine(int xStartPos, int xEndPos, int y, int col = 0xffffff)
{
	voxie_debug_drawhlin(xStartPos, xEndPos,y,col);
}
/** Draws a line on the secondary (touch) screen. Must be called between startFrame() & endFrame() functions.
 *
 * @param xStartPos coordinate of the starting x pixel. Needs to be the left most pixel.
 * @param xEndPos	coordinate of the ending x pixel. Needs to be the right most pixel.
 * @param yStartPos coordinate of the starting y pixel. Needs to be the left most pixel.
 * @param yEndPos	coordinate of the ending y pixel. Needs to be the right most pixel.
 * @param col		24-bit RGB color of the line.
 */

void VoxieBox::debugDrawLine(float xStartPos, float yStartPos, float xEndPos, float yEndPos, int col = 0xffffff)
{
	voxie_debug_drawline(xStartPos, yStartPos, xEndPos, yEndPos,col);
}

/** Draw circle on the secondary (touch) screen. Must be called between startFrame() & endFrame() functions.
 *
 * @param y			y coordinate of the circle's center.
 * @param x			x coordinate of the circle's center. 
 * @param radius	radius size in pixels.
 * @param col		24-bit RGB color of the line.
 */ 
void VoxieBox::debugDrawCircle(int xCenterPos, int yCenterPos, int radius, int col)
{
	 voxie_debug_drawcirc(xCenterPos, yCenterPos, radius, col);
}

//! Draw filled rectangle on the secondary (touch) screen.  Must be called between startFrame() & endFrame() functions. 
/** 
 *	@param	xStartPos	left most starting position of box
 *	@param	yStartPos	top most starting position of box
 *	@param	xEndPos		right most ending position of box
 *	@param	yEndPos		bottom most ending position of box
 *	@param	col			24-bit hexadecimal RGB color value
 */
void VoxieBox::debugDrawBoxFill(int xStartPos, int yStartPos, int xEndPos, int yEndPos, int col)
{
	voxie_debug_drawrectfill(xStartPos, yStartPos, xEndPos, yEndPos,col);
}

//! Draw filled circle on the secondary (touch) screen.  Must be called between startFrame() & endFrame() functions. 
/**  
 *	@param xCenterPos	x horizontal circle's center position
 *	@param yCenterPos	y vertical circle's center position
 *  @param radius	radius of circle. Size in pixels
 *	@param col	24-bit hexadecimal RGB color value
 */ 
void VoxieBox::debugDrawCircFill(int xCenterPos, int yCenterPos, int radius, int col)
{
	voxie_debug_drawcircfill(xCenterPos, yCenterPos, radius,col);
}

//!  Draws a texture onto the secondary (touch) screen. 
/**
 * @param tiletype * source		pointer to a tile type    
 * @param int	xpos			the x position to render onto the secondary (touch) screen
 * @param int	ypos			the y position to render onto the secondary (touch) screen
 *
 * Tile must be fully within bounds of screen.
 *
 *							Note :  Alpha bytes (bits 24-31) of source pixels control what's drawn:
 *	  				        0: fully transparent (pixel is ignored)
 *	  						1-254: blending in between (NOTE: will render slower than 0 or 255)
 *	  						255: fully opaque (pixel is copied to screen)
 *
 *							@Example on how to create a tile type data (the pure pixel data from an image
 *
 *							// how to create a tile type to show onto the 2D screen.
 * 							tiletype example; 								// define new tiletype type
 *							example.x = 800;								// get the image's x dimension (look at the details of the file - Windows explorer -> properties)
 *							example.y = 600;								// get the image's y dimension (look at the details of the file - Windows explorer -> properties)
 *							example.p = (example.x<<2);						// define the pitch number of bytes per horizontal line (usually x*4 but may be higher or negative)
 *							example.f = (INT_PTR)malloc(balls.p*balls.y);	// create pointer to 1st pixel
 *							voxie->_kpzload((char*)"example.jpg", &example.f, &example.p, &example.x, &example.y); 		// load the image into file memory pass in the pointers
 *
 *							to render your tiletype unto the 2D display call 
 *							voxie->debugDrawTile(&example, xposition, yposition);
 */
void VoxieBox::debugDrawTile(tiletype * source, int xpos, int ypos)
{
	voxie_debug_drawtile(source, xpos, ypos);
}
//! Returns the internal mouse's X delta movement. Reads from internal (in) voxie_input_t.
/**
 * @return Returns the internal mouse's X delta movement. Reads from internal (in) voxie_input_t.
 */
int VoxieBox::getMouseXDelta()
{
	return in.dmousx;
}
//! Returns the internal mouse's Y delta movement. Reads from the internal (in) voxie_input_t.
/**
 * @return Returns the internal mouse's Y delta movement. Reads from the internal (in) voxie_input_t.
 */
int VoxieBox::getMouseYDelta()
{
	return in.dmousy;
}
//! Returns the internal mouse's Z delta movement. Reads from the internal (in) voxie_input_t.
/**
 * @return Returns the internal mouse's Z delta movement. Reads from the internal (in) voxie_input_t.
 */
int VoxieBox::getMouseZDelta()
{
	return in.dmousz;
}
//! Returns the internal mouse's X,Y and Z delta movements as a point3d. Reads from the internal (in) voxie_input_t.
/**
 * @return Returns the internal mouse's X,Y and Z delta movements as a point3d. Reads from the internal (in) voxie_input_t.
 */
point3d VoxieBox::getMouseDelta()
{
	return point3d{ float(in.dmousx), float(in.dmousy), float(in.dmousz) };
}
//! Returns the internal mouse button state. Reads from the internal (in) voxie_input_t.
/**
 * @return Returns the internal mouse button state. Reads from the internal (in) voxie_input_t.
 */
int VoxieBox::getMouseButtonState()
{
	return in.bstat;
}
//! Returns the internal mouse previous button state. Reads from the internal (in) voxie_input_t.  
/**
 * Could be used to write your own input functions. 
 *
 * @return Returns the internal mouse previous button state. Reads from the internal (in) voxie_input_t.
 */
int VoxieBox::getMousePrevButtonState()
{
	return in.obstat;
}

//! Returns the state of mouse's buttonCode. @return 1 if currently pressed down and 0 if not being pressed. 
/**
 * @param buttonCode the buttonCode for the button to check. 0 = Left, 1 = Right, 2 = Center (mouse wheel button).
 */
int VoxieBox::getMouseButtonIsDown(int buttonCode)
{
	return ((in.bstat >> buttonCode) & 1);
}
//! Returns the state of mouse's buttonCode. @return 1 if button has been pressed during this update frame otherwise 0 if not just pressed. 
/**
 * @param buttonCode the buttonCode for the button to check. 0 = Left, 1 = Right, 2 = Center (mouse wheel button).
 * Note : This input check is for the instance of it being just pressed. It will return 0 if you held down a button. 
 */
int VoxieBox::getMouseButtonOnDown(int buttonCode)
{
	return ((in.bstat >> buttonCode) & 1) && !((in.obstat >> buttonCode) & 1);
}
//! Checks to see if desired MoustButton has been just released  ('on up')
/**
 * @param buttonCode the mouse button code you wish to check for (0 = left, 1 = right, 2 = both)
 * @return true if just released otherwise false
 */
int VoxieBox::getMouseButtonOnUp(int buttonCode )
{
	int a = !((in.bstat >> buttonCode) & 1);
	int b = ((in.obstat >> buttonCode) & 1);
	return a && b;
}

//! Returns the mount of time (in seconds) a mouse's button has been pressed. 
/**
 *  @param buttonCode the mouse button code to check (0 = left, 1 = right, 2 = middle)
 *
 *  @return in seconds the mount of time that button has been pressed.
 */
double VoxieBox::getMouseButtonDownTime(int buttonCode)
{
	return durationLastPressedMouse[buttonCode];
}

//! Returns 1 if particular Mouse's buttonCode has been clicked twice within the double click threshold.
/**
 *  @param buttonCode the mouse's button code to check (0 = left, 1 = right, 2 = middle)
 *
 *  @return 1 if double click has been registered otherwise return 0.
 *
 *  To adjust double click threshold @see setMouseoubleClickThreshold() 
 */
int VoxieBox::getMouseDoubleClick(int buttonCode)
{
	if (time - startLastPressedMouse[buttonCode] < mouseClickThreshold && getMouseButtonOnDown(buttonCode)) return 1;
	return 0;
}


//! Renders a single voxel onto the volumetric using a single point3d for coordinates.
/**
 *	Draw single voxel at specified location.
 *	@param  pos	x,y,z position of the voxel's location
 *	@param  col 24-bit hexadecimal color value
 *
 *	NOTE: The intensity of each color component in col is used for dithering, meaning a voxel might
 *	not plot if the intensity of the color component is less than 255. This is by design to allow
 *	shades. If you want the voxel to always plot, use 255's in each color component.
 */
void VoxieBox::drawVox(point3d pos, int col  = 0xffffff)
{
	voxie_drawvox(&vf, pos.x, pos.y, pos.z, col);
}



//! Renders a single voxel onto the volumetric using 3 floats for coordinates
/**
 *	Draw single voxel at specified location.
 *	@param  x	x position of the voxel's location
 *	@param  y	y position of the voxel's location
 *	@param  z	z position of the voxel's location
 *	@param  col 24-bit hexadecimal color value 
 *	
 *	NOTE: The intensity of each color component in col is used for dithering, meaning a voxel might
 *	not plot if the intensity of the color component is less than 255. This is by design to allow
 *	shades. If you want the voxel to always plot, use 255's in each color component.
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawVox(float x, float y, float z, int col = 0xffffff)
{
	voxie_drawvox(&vf, x, y, z, col);
}



//! Renders a rectangle / box on the volumetric display using 2 point3ds for coordinates. 
/**
 *  @param  posLeftUpTop			left up top corner of the box
 *  @param  posRightDownBottom		bottom, right, down corner of the box
 *  @param  fillmode				The fillmode type 0:dots, 1:lines, 2:surfaces, 3:solid
 *  @param  col						RGB 24-bit hexadecimal color value 
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawBox(point3d posLeftUpTop, point3d posRightDownBottom, int fillmode = 2, int col = 0xffffff)
{
	voxie_drawbox(&vf, posLeftUpTop.x, posLeftUpTop.y, posLeftUpTop.z, posRightDownBottom.x, posRightDownBottom.y, posRightDownBottom.z, fillmode, col);
}

//! Renders a rectangle/box on the volumetric display using 6 floats for coordinates
/*

 *  @param  x0			left for the LeftUpTop coordinate.
 *  @param  y0			up for the LeftUpTop coordinate.
 *  @param  z0			top for the LeftUpTop coordinate. 
 *  @param  x1			right for the RightDownBottom coordinate.
 *  @param  y1			down for the RightDownBottom coordinate.
 *  @param  z1			bottom for the RightDownBottom coordinate.
 *  @param  fillmode	the fillmode type.  0:dots, 1:lines, 2:surfaces, 3:solid
 *  @param  col			24-bit color RGB color value.
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawBox(float x0, float y0, float z0, float x1, float y1, float z1, int fillmode = 2, int col = 0xffffff)
{
	voxie_drawbox(&vf, x0, y0, z0, x1, y1, z1, fillmode, col);
}

//! Renders a line on the volumetric display using point3d for coordinates. 
/**
 *	@param startPos point3d of first (starting) position.
 *	@param endPos point3d of second (end) position.
 *   
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawLine(point3d startPos, point3d endPos, int col = 0xffffff)
{
	voxie_drawlin(&vf, startPos.x, startPos.y, startPos.z, endPos.x, endPos.y, endPos.z, col);
}


//! Renders a line on the volumetric display using 6 floats for coordinates.
/**
 *  @param xStartPos	x position for 1st (starting) point
 *  @param yStartPos	y position for 1st (starting) point
 *  @param zStartPos	z position for 1st (starting) point 
 *  @param xEndPos		x position for 2nd (ending) point
 *  @param yEndPos		y position for 2nd (ending) point
 *  @param zEndPos		z position for 2nd (ending) point
 *  @param col			24-bit hexadecimal color value (RGB)
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawLine(float xStartPos, float yStartPos, float zStartPos, float xEndPos, float yEndPos, float zEndPos, int col = 0xffffff)
{
	voxie_drawlin(&vf, xStartPos, yStartPos, zStartPos, xEndPos, yEndPos, zEndPos, col);
}


//!Renders a filled polygon. Assumes points are in loop order and coplanar. 
/**
 *	@param pt			pointer to the pol_t array (consists list of vertices and their 'next point index')
 *	@param pt_count		the number of vertices in the pol_t array
 *	@param p2			is an index to the next point on the loop. Holes/multiple loops are supported. 
 *  @param col			24-bit hexadecimal color value (RGB)
 *
 *           @example For example, this would draw a flat draw a rectangle:
 *           pol_t pt[4];
 *           pt[0].x = 0.0; pt[0].y = 0.0; pt[0].z = 0.0; pt[0].p2 = 1;
 *           pt[1].x = 0.5; pt[1].y = 0.0; pt[1].z = 0.0; pt[1].p2 = 2;
 *           pt[2].x = 0.5; pt[2].y = 0.8; pt[2].z = 0.0; pt[2].p2 = 3;
 *           pt[3].x = 0.0; pt[3].y = 0.8; pt[3].z = 0.0; pt[3].p2 = 0;
 *			drawPoly(pol_t &pt, 4, 0xffffff);
 *  @see vxDataTypes.h::pol_t
 *
 *  Must be called between startFrame() & endFrame() functions.
*/

void VoxieBox::drawPoly(pol_t * pt, int ptCount, int col = 0xffffff)
{
	voxie_drawpol(&vf, pt, ptCount, col);
}


//! Draws a mesh from a list of vertices can render in as dots, lines, polygons, or filled mesh.
/**
*  @param  fileNam 	    texture filename or pointer to tiletype structure if (flags&8) or leave null for no texture
*						Be sure to fill .u and .v fields of poltex_t when using a texture.
*  @param  verticeList	pointer to the list of vertices array (as a poltex_t data type)
*  @param  verticeNum	max number of vertices in the vertices array 
*  @param  meshList		pointer to the list of facets as vertex indices or -1 to end current primitive and start next one, -2 to end loop
*  @param  meshNum		number of entries in mesh array
*  @param  flags		mostly fill mode and texture settings +0:dots, +1:lines, +2:surfaces, +3:solid, +8:texnam is tiletype * instead of filename
*  @param  col			24-bit hexadecimal color value (RGB)
*
*			@example
*          
*           //Example for fillmode 0 (dots) mesh and meshn are ignored - pass 0's in their place.
*           poltex_t vt[4]; int i = 0;
*           vt[0].x =-0.8; vt[0].y =-0.8; vt[0].z = 0.0; vt[0].col = 0xffffff;
*           vt[1].x =+0.8; vt[1].y =+0.8; vt[1].z = 0.0; vt[1].col = 0xffffff;
*           vt[2].x =-0.8; vt[2].y =+0.8; vt[2].z = 0.0; vt[2].col = 0xffffff;
*           vt[3].x =+0.8; vt[3].y =-0.8; vt[3].z = 0.0; vt[3].col = 0xffffff;
*           voxie_drawmeshtex(&vf,0,vt,4,0,0,0,0xffffff);
*
*           //Example for fillmode 1 (line list): wireframe 'X'
*           poltex_t vt[4]; int mesh[6]; i = 0;
*           vt[0].x =-0.8; vt[0].y =-0.8; vt[0].z = 0.0; vt[0].col = 0xffffff;
*           vt[1].x =+0.8; vt[1].y =+0.8; vt[1].z = 0.0; vt[1].col = 0xffffff;
*           vt[2].x =-0.8; vt[2].y =+0.8; vt[2].z = 0.0; vt[2].col = 0xffffff;
*           vt[3].x =+0.8; vt[3].y =-0.8; vt[3].z = 0.0; vt[3].col = 0xffffff;
*           mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = -1; //-1 = end of line sequence
*           mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;
*           voxie_drawmeshtex(&vf,0,vt,4,mesh,i,1,0xffffff);
*
*           //Example for fillmode 2 (polygon list) or fillmode 3 (solid filled): tetrahedron
*           poltex_t vt[4]; int mesh[16], i = 0;
*           vt[0].x =-0.4; vt[0].y =-0.4; vt[0].z =-0.4; vt[0].col = 0xffffff;
*           vt[1].x =-0.4; vt[1].y =+0.4; vt[1].z =+0.4; vt[1].col = 0xffffff;
*           vt[2].x =+0.4; vt[2].y =-0.4; vt[2].z =+0.4; vt[2].col = 0xffffff;
*           vt[3].x =+0.4; vt[3].y =+0.4; vt[3].z =-0.4; vt[3].col = 0xffffff;
*           mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1; //-1 = end of polygonal facet
*           mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
*           mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
*           mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1; 
*           voxie_drawmeshtex(&vf,0,vt,4,mesh,i,2 (or 3) ,0xffffff);
*
*           //Example for fillmode 2 (complex poly w/hole: 2-triangle donut)
*           poltex_t vt[6]; int mesh[8], i = 0;
*           vt[0].x =-0.75; vt[0].y =+0.50; vt[0].z = 0.0; vt[0].col = 0xffffff;  //     / \
*           vt[1].x = 0.00; vt[1].y =-0.75; vt[1].z = 0.0; vt[1].col = 0xffffff;  //    / . \
*           vt[2].x =+0.75; vt[2].y =+0.50; vt[2].z = 0.0; vt[2].col = 0xffffff;  //   / /_\ \
*           vt[3].x =-0.25; vt[3].y =+0.25; vt[3].z = 0.0; vt[3].col = 0xffffff;  //  /       \
*           vt[4].x = 0.00; vt[4].y =-0.25; vt[4].z = 0.0; vt[4].col = 0xffffff;  // /---------\
*           vt[5].x =+0.25; vt[5].y =+0.25; vt[5].z = 0.0; vt[5].col = 0xffffff;
*           mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -2; //-2 = end loop
*           mesh[i++] = 3; mesh[i++] = 4; mesh[i++] = 5; mesh[i++] = -1; //-1 = end polygonal facet
*           voxie_drawmeshtex(&vf,0,vt,4,mesh,i,2 (or 3),0xffffff);
*
*	Must be called between startFrame() & endFrame() functions.
*/
void VoxieBox::drawMesh(char * fileName, poltex_t * verticeList, int verticeNum, int * meshList, int meshNum, int flags, int col = 0xffffff)
{
	voxie_drawmeshtex(&vf, fileName, verticeList, verticeNum, meshList, meshNum, flags, col);
}


//! Renders a cone shape on the volumetric display with rounded ends (also capable of rendering a cylinder/sphere) using 2 point3ds for coordinates.
/**
 *  @param  startPos		the x,y,z position of starting point (1st sphere)
 *  @param  startRadius		the radius size of the starting point (1st sphere)
 *  @param  endPos			the x,y,z position of ending point (2nd sphere)
 *  @param  endRadius		the radius size of the ending point (2nd sphere)
 *  @param  fillmode		the type of fill 0=shell, 1=solid filled
 *  @param  col				RGB 24-bit color
 *
 *  Shape is basically two spheres connected together like a tube.
 *
 *  Note: be careful with using the solid filled (fillmode 1) type as it can quite a performance hit. (draws many voxels)
 *  Must be called between startFrame() & endFrame() functions.
 */

void VoxieBox::drawCone(point3d startPos, float startRadius, point3d endPos, float endRadius, int fillmode, int col = 0xffffff )
{
	voxie_drawcone(&vf, startPos.x, startPos.y, startPos.z, startRadius, endPos.x, endPos.y, endPos.z, endRadius, fillmode, col);
}

//! Renders a cone shape on the volumetric display with rounded ends (also capable of rendering a cylinder/sphere) using 6 floats for coordinates
/**
 *  @param  xStartPos		the x position of starting point (1st sphere)
 *  @param  yStartPos		the y position of starting point (1st sphere)
 *  @param  zStartPos		the z position of starting point (1st sphere)
 *  @param  startRadius		the radius size of the starting point (1st sphere)
 *  @param  xEndPos			the x position of ending point (2nd sphere)
 *  @param  yEndPos			the y position of ending point (2nd sphere)
 *  @param  zEndPos			the z position of ending point (2nd sphere)
 *  @param  endRadius		the radius size of the ending point (2nd sphere)
 *  @param  fillmode		the type of fill 0=shell, 1=solid filled
 *  @param  col				RGB 24-bit color
 *
 *  Shape is basically two spheres connected together like a tube.
 *
 *  Note: be careful with using the solid filled (fillmode 1) type as it can quite a performance hit. (draws many voxels)
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawCone(float xStartPos, float yStartPos, float zStartPos, float startRadius, float xEndPos, float yEndPos, float zEndPos, float endRadius, int fillmode, int col = 0xffffff)
{
	voxie_drawcone(&vf, xStartPos, yStartPos, zStartPos, startRadius, xEndPos, yEndPos, zEndPos, endRadius, fillmode, col);
}


//! Renders 3D model (.obj, .ply, .stl, .kv6). Displays a filename mesh onto the volumetric display.
/**
 *  known as voxie_drawspr ("draw sprite") in the voxiebox.h / voxiebox.dll. Renamed to "draw model" 
 *  as a more apt description.
 *
 *  @param		fileName	filename. Cached internally. Currently supports .KV6,.STL,.OBJ,.PLY
 *  @param      pos			position of center of model (pivot)
 *  @param      rVector		right vector set to {1,0,0} for a unwarped normal view 
 *  @param		dVector		down vector set to {0,1,0} for an unwarped normal view
 *	@param		fVector		forward vector set to {0,0,1} for an unwarped normal view
 *  @param		col			color multiplier. 24-bit color, each 8 bits scales intensity of respective
 *			                component. 64=1.0 or no scale. Use 0x404040 for no change; 0x808080 to draw as
 *				            double brightness, etc.. (currently ignored)
 *
 *  
 *  @return	1=found file & valid, 0=bad file
 *
 *  Note : the col parameter works differently to most col values. As it is scaled 0x404040 is equal
 *  to 0xFFFFFF color values are scaled up by x 4. You can use VoxieBox::colorHexDivide(col, 4) to 
 *  divide the intended color by a 4th to render it in its true color. This is useful when you want 
 *  your model to be shown at the correct color.
 *
 *  For more options when using this function @see VoxieBox::drawModelExt
 *  Must be called between startFrame() & endFrame() functions.
 */
 int VoxieBox::drawModel(const char * fileName, point3d *pos, point3d *rVector, point3d *dVector, point3d *fVector, int col = 0x404040)
{
	return voxie_drawspr(&vf, fileName, pos, rVector, dVector, fVector, col );
}



//! Renders 3D model (.obj, .ply, .stl, .kv6) Displays a filename mesh onto the volumetric display. Extended from VoxieBox:drawModel
/**
 *  Known as voxie_drawspr_ext ("draw sprite extended") in the voxiebox.h / voxiebox.dll. Renamed to "draw model extended"
 *  as a more apt description.
 *
 *  @param		fileName		filename. Cached internally. Currently supports .KV6,.STL,.OBJ,.PLY
 *  @param      pos				position of center of model (pivot)
 *  @param      rVector			right vector set to {1,0,0} for a unwarped normal view
 *  @param		dVector			down vector set to {0,1,0} for an unwarped normal view
 *	@param		fVector			forward vector set to {0,0,1} for an unwarped normal view
 *  @param		col				color multiplier. 24-bit color, each 8 bits scales intensity of respective
 *								component. 64=1.0 or no scale. Use 0x404040 for no change; 0x808080 to draw as
 *								double brightness, etc.. (currently ignored)
 *  @param		forceScale		for STL files, forces scale factor (for alignment inside ZIP animations) Leave this
 *								0.f typically.
 *  @param		fdrawratio		fraction of model to draw (hack for animation)
 *  @param		flags			Bit 0:0=normal, 1:wireframe
 *
 *  @return	1=found file&valid, 0=bad file
 *
 *  Note : the col parameter works differently to most col values. As it is scaled 0x404040 is equal
 *  to 0xFFFFFF color values are scaled up by x 4. You can use VoxieBox::colorHexDivide(col, 4) to
 *  divide the intended color by a 4th to render it in its true color. This is useful when you want
 *  your model to be shown at the correct color.
 *
 *  For a more simpler function @see VoxieBox::drawModel
 *  Must be called between startFrame() & endFrame() functions.
 */
int VoxieBox::drawModelExt(const char * fileName, point3d *pos, point3d *rVector, point3d *dVector, point3d *fVector, int color, float forceScale, float fdrawratio, int flags)
{
	return voxie_drawspr_ext(&vf, fileName, pos, rVector, dVector, fVector, color, forceScale, fdrawratio, flags);
}






//! Renders a cube using specified vectors to volumetric display. Similar to VoxieBox:drawModel() but only draws a cube
/**
 *	@param  pos			position for left-up-top corner
 *	@param  rVector		right vector
 *	@param  dVector		down vector
 *	@param  fVector		forward vector
 *	@param	fillmode	0:dots, 1:edges/wire frame, 2:surfaces, 3:solid filled
 *	@param  col			24-bit color as RGB hexadecimal
 *
 *  Must be called between startFrame() & endFrame() functions.
 */
void VoxieBox::drawCube(point3d * pos, point3d * rVector, point3d * dVector, point3d * fVector, int fillmode = 2, int col = 0xffffff)
{
	voxie_drawcube(&vf, pos, rVector, dVector, fVector, fillmode, col);
}

//! Renders a heightmap (.jpg, .png or tiletype data) onto the volumetric display
/**
 *  A heightmap is a 2D image which also contains a height channel along side it. 
 *  Open up some of the heightmap examples for a demo. Demview uses them to render all its information. 
 *  
 *  @param  fileName	filename or pointer to 2d array containing image & heightmap stored alpha channel.
 *  @param  pos			the position of the top-left corner of the heightmap.
 *  @param  rVector		the right vector. rVector.x indicates how wide the height map is also holds rotational data
 *  @param  dVector		the down vector.  rVector.y indicates how long the height map is also holds rotational data
 *  @param  fVector		the forward vector.  fVector.z indicates the height the height map it also holds rotational data
 *  @
 *  @param colorkey		 ARGB 32-bit color to be transparent (supported in nearest mode only)
 *  @param flags		 various flags to adjust render settings. see flags examples.
 *
 *                   @example of different flag settings to use
 *		  	         (1<<0): reserved (height dither now controlled by vw.dither)
 *                   (1<<1): 0=nearest filter , 1=bilinear filter  (recommended)
 *                   (1<<2): 0=color dither off , 1=color dither on  (recommended)
 *                   (1<<3): 0=filnam is filename string, 1=filnam is tiletype * or pointer to 2d array.
 *                   (1<<4): 0=texture clamp    , 1=texture wrap
 *                   (1<<5): 0=8-bit height     , 1=mapzen.com terrarium style height
 *   
 *
 *  Note : about scaling and sizing the rVector.z, dVector.z, fVector.x, fVector.y are all not used to render the heightMap 
 *  but their values are important for storing rotational information.  the height / scale of the heightmap is the difference 
 *	between pos.z and fVector.z values. pos.z is height=0, pos.z+fVector.z is height=255
 *
 *  @return average height in middle region, range:{0..255}
 *  Must be called between startFrame() & endFrame() functions.
 */
float VoxieBox::drawHeightMap(char * fileName, point3d * pos, point3d * rVector, point3d * dVector, point3d * fVector, int colorKey, int reserved, int flags)
{
	return voxie_drawheimap(&vf, fileName, pos, rVector, dVector, fVector, colorKey, reserved, flags);
}


//! renders a sphere on the volumetric display using a point3d for position
/**
*   @param pos		x,y,z  position of the sphere's center position
*	@param radius	the radius (size) of the sphere
*	@param fill		the fill mode 0 = shell, 1 = filled
*	@param col		the color as a RGB hex value
*
*   Must be called between startFrame() & endFrame() functions.
*   fillmode 1 (filled) will only color absolute colors ( 0xFF0000, 0x00FF00, 0xFFFF00, etc...)
*/
void VoxieBox::drawSphere(point3d pos, float radius, int fillmode, int col = 0xffffff)
{
	voxie_drawsph(&vf, pos.x, pos.y, pos.z, radius, fillmode, col);
}

//! renders a sphere on the volumetric display using 3 floats
/**
*   @param x		x position of the sphere's center position
*   @param y		y position of the sphere's center position
*   @param z		z position of the sphere's center position
*	@param radius	the radius (size) of the sphere
*	@param fill		the fill mode 0 = shell, 1 = filled
*	@param col		the color as a RGB hex value
*
*   Must be called between startFrame() & endFrame() functions.
*   fillmode 1 (filled) will only color absolute colors ( 0xFF0000, 0x00FF00, 0xFFFF00, etc...)
*/
void VoxieBox::drawSphere(float x, float y, float z, float radius, int fillmode, int col = 0xffffff)
{
	voxie_drawsph(&vf, x, y, z, radius, fillmode, col);
}


//! returns a key's input state (0 = no press, 1 = just pressed, 3 = held down) see vxInputTypes.h::Keys for scancodes
/**
 *  @param	scancode	each key has a unique scancode key. For a list scancodes see vxInputTypes.h::Keys
 *
 *  vxInputTypes.h contains an enum for all the scancodes which all start with the prefix 'KB_'
 *  @return 0 = no press, no change, 1 = just pressed, 3 = held down.
 */ 
int VoxieBox::getKeyState(int scancode)
{

	if (manualKeyManage == true) {
		return voxie_keystat(scancode);
	}
	else {
		for (int i = 0; i < KEY_HISTORY_LENGTH; i++) if (keyHistory.history[i].inputCode == scancode) return keyHistory.history[i].state;
	}
	return -1;
}

//! returns 1 if the scancode's key is held down (pressed). For scancodes see vxInputTypes.h::Keys
/**
 * @param	scancode	each key has a unique scancode key. For a list scancodes see vxInputTypes.h::Keys
 *
 * @return 0 = no press, 1 = is down.
 * Note : Does not return 1 'on down' for the specific on down function @see VoxieBox::getKeyOnDown().
 */
int VoxieBox::getKeyIsDown(int scancode)
{
	if (voxie_keystat(scancode) == 3) return 1;
	else return 0;
}

//! returns a 1 if the scancode's key is just pressed. Requires VoxieBox::setEnableLegacyKeyInput() set to false to work. For scancodes see vxInputTypes.h::Keys 
/**
 * @param	scancode	each key has a unique scancode key. For a list scancodes see vxInputTypes.h::Keys
 *
 * @return 1 if key has just been pressed otherwise returns 0
 *
 * @see voxieInputTypes.h::keys
 */
int VoxieBox::getKeyOnDown(int scancode)
{

	if (manualKeyManage == true) {
		if (voxie_keystat(scancode) == 1 ) return 1;
	}
	else {

		for (int i = 0; i < KEY_HISTORY_LENGTH; i++) if (keyHistory.history[i].inputCode == scancode && keyHistory.history[i].state == 1) return 1;

	}
	return 0;
}


//! return the time (in seconds) The scancode's key has been held down for requires VoxieBox::setEnableLegacyKeyInput() to be set to false to work
/**
 * @param	scancode	each key has a unique scancode key. For a list scancodes see vxInputTypes.h::Keys
 *
 * @returns in seconds the duration of the key's being held down. Returns -1 if legacyKeyInput is enabled
 *
 * Note: VoxieBox::setEnableLegacyKeyInput() to be set to false to work otherwise will always return -1; 
 */
double VoxieBox::getKeyDownTime(int scancode)
{
	if (manualKeyManage == true) return -1;

	for (int i = 0; i < KEY_HISTORY_LENGTH; i++) {

		if (keyHistory.history[i].inputCode == scancode && keyHistory.history[i].isHeld == true) {
			return keyHistory.history[i].duration;
			break;
		}

	}
	return 0;
}
//! returns a 1 if the scancode's key is just released. Requires VoxieBox::setEnableLegacyKeyInput() set to false to work. For scancodes see vxInputTypes.h::Keys 
/**
 * @param	scancode	each key has a unique scancode key. For a list scancodes see vxInputTypes.h::Keys
 *
 * @return 1 if key has just been released otherwise returns 0. Returns -1 if legacyKeyInput is enabled
 *
 * Note: VoxieBox::setEnableLegacyKeyInput() to be set to false to work otherwise will always return -1
 *
 * @see voxieInputTypes.h::keys
 */
int VoxieBox::getKeyOnUp(int scancode)
{

	if (manualKeyManage == true) return -1;

	for (int i = 0; i < KEY_HISTORY_LENGTH; i++) {

		if (keyHistory.history[i].inputCode == scancode && keyHistory.history[i].onUp == true) {
			return 1;
			break;
		}

	}
	return 0;
}

/**
* Returns buffered ASCII keyboard input. Useful for typing stuff like your name, for example.
* You may continue to call this function in a while loop until it returns 0. This function only works when 
* setEnableLegacyKeyInput() is set to true. Otherwise use getKeyHistory() instead.
*     @return 
*                     0: buffer is empty otherwise 
*             bits  7-0: ASCII code ('A'=65,'a'=97,'0'=48,etc..)
*             bits 15-8: Keyboard Scan Code
*             bit    16: Left  Shift was down at time of this keypress
*             bit    17: Right Shift was down at time of this keypress
*             bit    18: Left  Ctrl  was down at time of this keypressdrawSphere
*             bit    19: Right Ctrl  was down at time of this keypress
*             bit    20: Left  Alt   was down at time of this keypress
*             bit    21: Right Alt   was down at time of this keypress
*/
int VoxieBox::getKeyStream()
{
	return voxie_keyread();
}

//! Similar to getKeyStream but returns only the scancode of the keystate of a streaming state. To be run with a while loop. Returns 0 if no more keys are in the stream
int VoxieBox::getKeyStreamScanCode()
{
	return voxie_keystat(voxie_keyread() >> 8);
}


//! returns the game controllers button state. See vxInputTypes::JoyButtonCodes for details. 
/**
 * @param controllerID the ID number of the controller to check (0 = 1st, 1 = 2nd, 2 = 3rd, 4th)
 *
 *
 * @return the controls button state. Each bit represents a button. @see vxInputTypes::JoyButtonCodes for details.
 * 
 *	          JOY_DPAD_UP			= 0,	//!< bit 0 value 1			Digital Dpad Up				
 *	          JOY_DPAD_DOWN			= 1,	//!< bit 1 value 2			Digital Dpad Down 
 *	          JOY_DPAD_LEFT			= 2,	//!< bit 2 value 4			Digital Dpad Left
 *	          JOY_DPAD_RIGHT		= 3,	//!< bit 3 value 8			Digital Dpad Right
 *	          JOY_START				= 4,	//!< bit 4 value 16			Start Button
 *	          JOY_BACK				= 5,	//!< bit 5 value 32			Back Button
 *	          JOY_LEFT_THUMB		= 6,	//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 *	          JOY_RIGHT_THUMB		= 7,	//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 *	          JOY_LEFT_SHOULDER		= 8,	//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
 *	          JOY_RIGHT_SHOULDER	= 9,	//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
 *	          JOY_A					= 12,	//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 *	          JOY_B					= 13,	//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 *	          JOY_X					= 14,	//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 *	          JOY_Y					= 15	//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller
 *               
 */
int VoxieBox::getJoyButtonState(int controllerID) {

	return vx[controllerID].but;

}

//! reports the state of the game controllers on to the secondary (touch) screen
/**
*	@param posX			the X position to render the report
*   @param posY			the Y position to render the report
*/
void VoxieBox::reportJoy(int posX, int posY )
{
	if (manualJoyManage == true) {

		debugText(posX, posY, 0xFF0000, -1, "Joy Report - disabled (manual joy manage mode is enabled) ");
		return;
	}
	int i = 0;
	int b = 0;
	int j = 0;
	int oriCol[4] = { 0xFFFF00, 0x00FFFF, 0xFF0000, 0x00FF00 };
	int nPosY = posY;
	debugDrawBoxFill(posX, nPosY, posX + 225, posY + 35 + (getJoyNum() * 110), 0x330033);
	debugText(posX, nPosY, 0xFFFFFF, -1, "            Joystick Report");
	nPosY += 13;
	debugText(posX, nPosY, 0xFFFFFF, -1, "XInput Enabled:     # Controllers:    ");
	debugText(posX, nPosY, 0x00FF00, -1, "                %d                  %d ", vw.usejoy, getJoyNum());
	nPosY += 13;
	debugText(posX, nPosY, 0xFFFFFF, -1, "Dead Zone On:    Threshold: ");
	debugText(posX, nPosY, 0xFF00FF, -1, "              %d             %1.3f", enableJoyDeadZone, getJoyDeadZone());


	nPosY += 13;
	double heldTime = 0;
	point2d lAnaPos = { 0 };
	point2d rAnaPos = { 0 };
	int lStickCol = 0xffffff;
	int rStickCol = 0xffffff;

	for (i = 0; i < getJoyNum(); i++) {
		b = getJoyButtonState(i);
		nPosY = posY + 39 + (i * 110); 
		debugText(posX, nPosY, 0xFFFFFF, -1,						  "Joy %d :", i + 1);
		if (getJoyButtonIsDown(i, JOY_A)) debugText(posX, nPosY, 0x00FF00, -1,			   "         A");
		if (getJoyButtonIsDown(i, JOY_B)) debugText(posX, nPosY, 0xFF0000, -1,			   "          B");
		if (getJoyButtonIsDown(i, JOY_X)) debugText(posX, nPosY, 0x0000FF, -1,			   "           X");
		if (getJoyButtonIsDown(i, JOY_Y)) debugText(posX, nPosY, 0xFFFF00, -1,			   "            Y");
		if (getJoyButtonIsDown(i, JOY_BACK)) debugText(posX, nPosY, 0x333333, -1,		   "             Bk");
		if (getJoyButtonIsDown(i, JOY_START)) debugText(posX, nPosY, 0xFFFFFF, -1,		   "               St");
		if (getJoyButtonIsDown(i, JOY_LEFT_SHOULDER)) debugText(posX, nPosY, 0x9d9d9d, -1,  "                 L");
		if (getJoyButtonIsDown(i, JOY_RIGHT_SHOULDER)) debugText(posX, nPosY, 0x9d9d9d, -1, "                  R");
		if (getJoyButtonIsDown(i, JOY_LEFT_THUMB)) debugText(posX, nPosY, 0xFF00FF, -1,     "                   Al");
		if (getJoyButtonIsDown(i, JOY_RIGHT_THUMB)) debugText(posX, nPosY, 0x00FFFF, -1,    "                     Ar");
		if (getJoyButtonIsDown(i, JOY_DPAD_UP)) debugText(posX, nPosY, 0xFFFFFF, -1,		   "                       U");
		if (getJoyButtonIsDown(i, JOY_DPAD_DOWN)) debugText(posX, nPosY, 0xFFFFFF, -1,	   "                        D");
		if (getJoyButtonIsDown(i, JOY_DPAD_LEFT)) debugText(posX, nPosY, 0xFFFFFF, -1,	   "                         L");
		if (getJoyButtonIsDown(i, JOY_DPAD_RIGHT)) debugText(posX, nPosY, 0xFFFFFF, -1,	   "                          R");
		debugText(posX, nPosY, 0xFFFFFF, -1,									   "                           Raw:%d", getJoyButtonState(i));
	//	debugText(posX, nPosY, 0xFFFFFF, -1,                                               "                            %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", b >> 0 & 1, b >> 1 & 1, b >> 2 & 1, b >> 3 & 1, b >> 4 & 1, b >> 5 & 1, b >> 6 & 1, b >> 7 & 1, b >> 8 & 1, b >> 9 & 1, b >> 10 & 1, b >> 11 & 1, b >> 12 & 1, b >> 13 & 1, b >> 14 & 1, b >> 15 & 1);

		nPosY += 13;
		debugText(posX, nPosY, 0xFFFF00, -1, "State:");

		if (getJoyButtonOnDown(i, JOY_A))					debugText(posX, nPosY, 0x00FFFF, -1, "         D");
		if (getJoyButtonOnDown(i, JOY_B))					debugText(posX, nPosY, 0x00FFFF, -1, "          D");
		if (getJoyButtonOnDown(i, JOY_X))					debugText(posX, nPosY, 0x00FFFF, -1, "           D");
		if (getJoyButtonOnDown(i, JOY_Y))					debugText(posX, nPosY, 0x00FFFF, -1, "            D");
		if (getJoyButtonOnDown(i, JOY_BACK))				debugText(posX, nPosY, 0x00FFFF, -1, "             D ");
		if (getJoyButtonOnDown(i, JOY_START))				debugText(posX, nPosY, 0x00FFFF, -1, "               D ");
		if (getJoyButtonOnDown(i, JOY_LEFT_SHOULDER))		debugText(posX, nPosY, 0x00FFFF, -1, "                 D");
		if (getJoyButtonOnDown(i, JOY_RIGHT_SHOULDER))		debugText(posX, nPosY, 0x00FFFF, -1, "                  D");
		if (getJoyButtonOnDown(i, JOY_LEFT_THUMB))			debugText(posX, nPosY, 0x00FFFF, -1, "                   D ");
		if (getJoyButtonOnDown(i, JOY_RIGHT_THUMB))			debugText(posX, nPosY, 0x00FFFF, -1, "                     D ");
		if (getJoyButtonOnDown(i, JOY_DPAD_UP))				debugText(posX, nPosY, 0x00FFFF, -1, "                       D");
		if (getJoyButtonOnDown(i, JOY_DPAD_DOWN))			debugText(posX, nPosY, 0x00FFFF, -1, "                        D");
		if (getJoyButtonOnDown(i, JOY_DPAD_LEFT))			debugText(posX, nPosY, 0x00FFFF, -1, "                         D");
		if (getJoyButtonOnDown(i, JOY_DPAD_RIGHT))			debugText(posX, nPosY, 0x00FFFF, -1, "                          D");

		if (getJoyButtonOnUp(i, JOY_A))						debugText(posX, nPosY, 0xFF00FF, -1, "         U");
		if (getJoyButtonOnUp(i, JOY_B))						debugText(posX, nPosY, 0xFF00FF, -1, "          U");
		if (getJoyButtonOnUp(i, JOY_X))						debugText(posX, nPosY, 0xFF00FF, -1, "           U");
		if (getJoyButtonOnUp(i, JOY_Y))						debugText(posX, nPosY, 0xFF00FF, -1, "            U");
		if (getJoyButtonOnUp(i, JOY_BACK))					debugText(posX, nPosY, 0xFF00FF, -1, "             U ");
		if (getJoyButtonOnUp(i, JOY_START))					debugText(posX, nPosY, 0xFF00FF, -1, "               U ");
		if (getJoyButtonOnUp(i, JOY_LEFT_SHOULDER))			debugText(posX, nPosY, 0xFF00FF, -1, "                 U");
		if (getJoyButtonOnUp(i, JOY_RIGHT_SHOULDER))		debugText(posX, nPosY, 0xFF00FF, -1, "                  U");
		if (getJoyButtonOnUp(i, JOY_LEFT_THUMB))			debugText(posX, nPosY, 0xFF00FF, -1, "                   U ");
		if (getJoyButtonOnUp(i, JOY_RIGHT_THUMB))			debugText(posX, nPosY, 0xFF00FF, -1, "                     U ");
		if (getJoyButtonOnUp(i, JOY_DPAD_UP))				debugText(posX, nPosY, 0xFF00FF, -1, "                       U");
		if (getJoyButtonOnUp(i, JOY_DPAD_DOWN))				debugText(posX, nPosY, 0xFF00FF, -1, "                        U");
		if (getJoyButtonOnUp(i, JOY_DPAD_LEFT))				debugText(posX, nPosY, 0xFF00FF, -1, "                         U");
		if (getJoyButtonOnUp(i, JOY_DPAD_RIGHT))			debugText(posX, nPosY, 0xFF00FF, -1, "                          U");
	
		if (getJoyButtonDownTime(i, JOY_A)				> 1)						debugText(posX, nPosY, 0xFF0000, -1, "         H");
		if (getJoyButtonDownTime(i, JOY_B)				> 1)						debugText(posX, nPosY, 0xFF0000, -1, "          H");
		if (getJoyButtonDownTime(i, JOY_X)				> 1)						debugText(posX, nPosY, 0xFF0000, -1, "           H");
		if (getJoyButtonDownTime(i, JOY_Y)				> 1)						debugText(posX, nPosY, 0xFF0000, -1, "            H");
		if (getJoyButtonDownTime(i, JOY_BACK)			> 1)						debugText(posX, nPosY, 0xFF0000, -1, "             H ");
		if (getJoyButtonDownTime(i, JOY_START)			> 1)						debugText(posX, nPosY, 0xFF0000, -1, "               H ");
		if (getJoyButtonDownTime(i, JOY_LEFT_SHOULDER)  > 1)						debugText(posX, nPosY, 0xFF0000, -1, "                 H");
		if (getJoyButtonDownTime(i, JOY_RIGHT_SHOULDER) > 1)						debugText(posX, nPosY, 0xFF0000, -1, "                  H");
		if (getJoyButtonDownTime(i, JOY_LEFT_THUMB)		> 1)						debugText(posX, nPosY, 0xFF0000, -1, "                   H ");
		if (getJoyButtonDownTime(i, JOY_RIGHT_THUMB)	> 1)						debugText(posX, nPosY, 0xFF0000, -1, "                     H ");
		if (getJoyButtonDownTime(i, JOY_DPAD_UP)		> 1)						debugText(posX, nPosY, 0xFF0000, -1, "                       H");
		if (getJoyButtonDownTime(i, JOY_DPAD_DOWN)		> 1)						debugText(posX, nPosY, 0xFF0000, -1, "                        H");
		if (getJoyButtonDownTime(i, JOY_DPAD_LEFT)		> 1) 						debugText(posX, nPosY, 0xFF0000, -1, "                         H");
		if (getJoyButtonDownTime(i, JOY_DPAD_RIGHT)		> 1)						debugText(posX, nPosY, 0xFF0000, -1, "                          H");

		heldTime = 0;
		for (j = 0; j < 16; j++) {
			if (getJoyButtonDownTime(i, j) > 0) heldTime = getJoyButtonDownTime(i, j);
		}

		debugText(posX, nPosY, oriCol[getJoyOrientation(i)], -1, "                           %1.2f Ori:%d", heldTime, getJoyOrientation(i));


		nPosY += 40;

		lAnaPos = getJoyAnalogAxisValueP2D(i, 0);
		lStickCol = 0x00ff00;
		if (fabs(lAnaPos.x) < getJoyDeadZone() && fabs(lAnaPos.y) < getJoyDeadZone()) lStickCol = 0xff0000;

		debugDrawCircFill(posX + 30, nPosY, 25, 0x9d9d9d);
		debugDrawCircFill(posX + 30, nPosY, 0 + (getJoyDeadZone() * 25) , 0x904040);

		debugDrawCircFill(posX + 30 + (lAnaPos.x * 25), nPosY + (lAnaPos.y * 25), 5, lStickCol);
		debugText(posX + 5, nPosY + 30, lStickCol, -1, "X:%+1.3f\nY:%+1.3f", getJoyAnalogAxisValue(i,0), getJoyAnalogAxisValue(i, 1));

		rAnaPos = getJoyAnalogAxisValueP2D(i, 1);
		rStickCol = 0x00ff00;
		if (fabs(rAnaPos.x) < getJoyDeadZone() && fabs(rAnaPos.y) < getJoyDeadZone()) rStickCol = 0xff0000;

		debugDrawCircFill(posX + 190, nPosY, 25, 0x9d9d9d);
		debugDrawCircFill(posX + 190,  nPosY, 0 + (getJoyDeadZone() * 25), 0x904040);

		debugDrawCircFill(posX + 190 + (rAnaPos.x * 25), nPosY + (rAnaPos.y * 25), 5, rStickCol);
		debugText(posX + 165, nPosY + 30, rStickCol, -1, "X:%+1.3f\nY:%+1.3f", getJoyAnalogAxisValue(i, 2), getJoyAnalogAxisValue(i, 3));

		debugBar(posX + 70, nPosY - 25, getJoyTriggerValue(i, 0) * 100, 100, 0, (char*)" ", 2);
		debugBar(posX + 117, nPosY - 25, getJoyTriggerValue(i, 1) * 100, 100, 0, (char*)" ", 2);
		debugText(posX + 65, nPosY + 30, 0xffff00, -1, "  LT:     RT:");
		debugText(posX + 65, nPosY + 43, 0x00ffff, -1, "%+1.3f  %+1.3f", getJoyTriggerValue(i, 0), getJoyTriggerValue(i, 1));

	}


}

//! Sets the internal dead zone value for game controllers. (dead zone is the area where not input is detected). Value between 0 and 1. 0 = no deadzone. 1 = all deadzone. default is 0.3;
/**
 *  @param deadZoneValue	the value to set the deadzone threshold a number between 0 and 1 0 no deadzone... 1 deadzone everywhere
 *							default is 0.3
 *  
 */
void VoxieBox::setJoyDeadZone(double deadZoneValue)
{
	if (deadZoneValue < 0) joyDeadZone = 0;
	else if (deadZoneValue >= 1) joyDeadZone = 1.1;
	else {
		joyDeadZone = deadZoneValue;
	}
}
//! Set the internal deadzone for Space Navigators. (dead zone is the area where not input is detected)
/** 
 *  @param deadZoneValue new value to set deadzone. For Space Navigators between 0 and 350. default is 0.1
 *
 *
 */
void VoxieBox::setNavDeadZone(double deadZoneValue) {

	if (deadZoneValue < 0) navDeadZone = 0;
	else if (deadZoneValue >= 1) navDeadZone = 1.1;
	else {
		navDeadZone = deadZoneValue;
	}

}

//! returns the internal dead one value for the Space Navigator
double VoxieBox::getNavDeadZone()
{
	return navDeadZone;
}

//! set the orientation of the user's intended position when using a Space Nav
/**
 * @param spaceNavID		the ID number of the spaceNav you want to edit (0,1,2,3)
 * @param orientation		the orientation of the controller 0 = front,  1 = 180 ' (behind),  2 = 90 ' CCW (right side),  3 = 90 ' CW (left side)
 *                                     
 */               
void VoxieBox::setNavOrientation(int spaceNavID, int orientation)
{
	navOri[spaceNavID] = orientation % 4;
}


//! set the orientation of the user's intended position when using the mouse
/**
 * @param orientation		the orientation of the controller 0 = front,  1 = 180 ' (behind),  2 = 90 ' CCW (right side),  3 = 90 ' CW (left side)
 *
 */
void VoxieBox::setMouseOrientation( int orientation)
{
	mouseOri = orientation % 4;
}

//! returns the orientation set for a specific Space Nav
/**
 * @param spaceNavID the SpaceNav's ID of which one you want to return its orientation information
 * @return 0 = front (normal), 1 = 180 ' (behind), 2 = 90 ' CCW (right side), 3 = 90 ' CW (left side) 
 */
int VoxieBox::getNavOrientation(int spaceNavID)
{
	return navOri[spaceNavID];
}

//! returns the orientation set for the mouse
/**
 * @param spaceNavID the SpaceNav's ID of which one you want to return its orientation information
 * @return 0 = front (normal), 1 = 180 ' (behind), 2 = 90 ' CCW (right side), 3 = 90 ' CW (left side) 
 */
int VoxieBox::getMouseOrientation()
{
	return mouseOri;
}

//! returns the orientation set for a specific game controller
/**
 * @param controllerID the joy's ID of which one you want to return its orientation information
 * 
 * @return 0 = front (normal), 1 = 180 ' (behind), 2 = 90 ' CCW (right side), 3 = 90 ' CW (left side) 
 */
int VoxieBox::getJoyOrientation(int controllerID)
{
	return joyOri[controllerID];
}


//! Change the orientation of the users intended position when using a game controller. At what side are they facing towards the volumetric display.
/**
 * @param controllerID the ID number of the controller you want to adjust (0,1,2,3)
 * @param orientation of the controller 0 = front, 1 = 180 ' (behind), 2 = 90 ' CCW (right side), 3 = 90 ' CW (left side)
 *                                      
 *                                    
 */
void VoxieBox::setJoyOrientation(int controllerID, int orientation)
{
	joyOri[controllerID] = orientation % 4;
}

//! Returns the internal gamepad's dead zone value. Default is 0.3
/**
 * @return Returns the internal gamepad's dead zone value. Default is 0.3
 */
double VoxieBox::getJoyDeadZone()
{
	return joyDeadZone;
}

//! Returns a controllers analog trigger value. 0 = 0% pressed ... 1 = 100% pressed. Presented as a float 
/**
 *  @param controllerID		the ID number of the controller you want to adjust (0 = port 1,1 = port 2, 2 = port 3, 3 = port 4)
 *  @param joyTriggerCode	the trigger to check 0 = left, 1 = right. 
 *
 *  @returns a number between 0 and 1 which represents the analog's trigger value. 0 not pressed, 1 is fully pressed.
 */
float VoxieBox::getJoyTriggerValue(int controllerID, int joyTriggerCode)
{

	float f = 0;
	switch (joyTriggerCode) {
	default:
	case 0:
		f = vx[controllerID].lt;
		break;
	case 1:
		f = vx[controllerID].rt;
		break;
	}

	f = ((f / 127) * .5);
	if (f > 1) f = 1;

	if (enableJoyDeadZone == true) {
		if (f > 0.05) return f;
		else return 0;
	}
	else {
		return f;
	}

}



float VoxieBox::normaliseAnaStickToFloat(short input) {
		return ( input  / (MAXSHORT * .5) ) * .5;
}

float VoxieBox::normaliseNavToFloat(int input) {

	return  ((double(input)) / 700) * 2;
}


//! takes in and X and Y value and from an input and converts to the right orientation
/**
 * 0 = normal front facing
 * 1 = 180 ' flip backwards back facing
 * 2 = CCW 90' right side
 * 3 = CW 90'  left side
 */
//TODO these should be pointers and maybe can be using radians / degrees to rotate instead of force 90 
point2d VoxieBox::oriCorrection(int oriType, float xValue, float yValue)
{

	point2d returnVal = { xValue, yValue };

	switch (oriType)
	{
	default:
	case 0:
		break;
	case 1:
		returnVal.x = -returnVal.x;
		returnVal.y = -returnVal.y;
		break;
	case 2: 
		returnVal.x = yValue;
		returnVal.y = -xValue;		
		break;
	case 3:
		returnVal.x = -yValue;
		returnVal.y = xValue;
		break;


	}

	return returnVal;
}


int VoxieBox::joyDeadZoneCheck(float value) {

	if (fabs(value) > joyDeadZone) return 1;
	else return 0;

}


//! returns the analog axis (control stick) from a game controller. Value as an Point2d range between -1 and 1, 0 is centred.
/**
 * @param controllerID	the controller ID you want to check (0 = player 1, 1 = player 2... etc)
 * @param stick			0 for left stick, 1 for right stick
 *
 * @return the XY analog axis as a Point2d. Range between -1 and 1, 0 is centred. To get a single dimension @see getJoyAnalogAxisValue()
 *
 */
point2d VoxieBox::getJoyAnalogAxisValueP2D(int controllerID, int stick) {


	point2d a = { 0,0 };

	if (manualJoyManage == false )

	switch (stick) {
	default:
	case 0:
		a.x = normaliseAnaStickToFloat(vx[controllerID].tx0);
		a.y = normaliseAnaStickToFloat(vx[controllerID].ty0);
		break;
	case 1:
		a.x = normaliseAnaStickToFloat(vx[controllerID].tx1);
		a.y = normaliseAnaStickToFloat(vx[controllerID].ty1);
		break;

	}

	a.y = -a.y; // the voxiebox.dll has the y inverted for some reason?! 
	a = oriCorrection(getJoyOrientation(controllerID), a.x, a.y);

	if (enableJoyDeadZone == true) {
		if (joyDeadZoneCheck(a.x) || joyDeadZoneCheck(a.y)) return a;
		else {

			a.x = 0;
			a.y = 0;
			return a;
		}
	}
	else {
		return a;
	}



}

//! returns a single analog axis (control stick) from a game controller. Value as an float range between -1 and 1, 0 is centred.
/**
 * @param controllerID  the controller ID you want to check (0 = player 1, 1 = player 2... etc)
 * @param axis			the controller's Axis to check (vxInputTypes.h::ControllerAxis) 0 left stick x, 1 left stick y, 2 right stick x, 3 right stick y
 *
 * @return the single analog axis as a float. Range between -1 and 1, 0 is centred. To get both the X and Y values together @see getJoyAnalogAxisValueP2D()  
 */
float VoxieBox::getJoyAnalogAxisValue(int controllerID, int axis) {


	float f = 0;
	point2d a = { 0, 0 };

	switch (axis) {
	default:
	case JOY_LEFT_STICK_X:
	case JOY_LEFT_STICK_Y:
		a = getJoyAnalogAxisValueP2D(controllerID, 0);
		break;
	case JOY_RIGHT_STICK_X:
	case JOY_RIGHT_STICK_Y:
		a = getJoyAnalogAxisValueP2D(controllerID, 1);

		break;
	}

	switch (axis) {
		case JOY_LEFT_STICK_X:
		case JOY_RIGHT_STICK_X:
			f = a.x;
			break;
		case JOY_LEFT_STICK_Y:
		case JOY_RIGHT_STICK_Y:
			f = a.y;
			break;
	}
	
	if (enableJoyDeadZone == true) {
		if (joyDeadZoneCheck(f)) return f;
		return 0;
	}
	else {
		return f;
	}

}


//! Reports the state of the keyboard on to the secondary (touch) screen. Used to help with debugging/ 
/**
*	@param posX the X position to render the report
*   @param poxY the Y position to render the report
*/
void VoxieBox::reportKeyboard(int posX, int posY)
{
	if (manualKeyManage == true) {

		debugText(posX, posY, 0xFF0000, -1, "Keyboard Report - disabled (manual key manage mode is enabled) ");
		return;
	}
	int i = 0;
	int col = 0xFF0000;
	int k = 0;

	debugDrawBoxFill(posX, posY, posX + 225, posY + 190, 0x330000);

	// reporting the info
	debugText(posX, posY, 0xFFFFFF, -1, "            Keyboard Report");
	debugText(posX, posY + 12, 0x00FFFF, -1, "\n              %c", keyHistory.history[0].inputCodeRaw);
	debugText(posX, posY + 12, 0xFF0000, -1, "\nLast Pressed:    (Raw:%#08x) %#04x", keyHistory.history[0].inputCodeRaw, keyHistory.history[0].inputCode);

	if (voxie_keystat(KB_Shift_Left) || voxie_keystat(KB_Shift_Right)) 	debugText(posX , posY + 9, 0x00FF00, -1, "Shft");
	if (voxie_keystat(KB_Control_Left) || voxie_keystat(KB_Control_Right)) 	debugText(posX + 30, posY + 9, 0xFF00FF, -1, "Ctrl");
	if (voxie_keystat(KB_Alt_Left) || voxie_keystat(KB_Alt_Right)) 	debugText(posX + 60, posY + 9, 0x00FFFFF, -1, "Alt");
	
	//if (lastPressedKeyTime > 0) debugText(posX + 140, posY + 30, 0xff0000, -1, "%1.2f", time - lastPressedKeyTime);

	if (keyHistory.history[0].duration) debugText(posX + 140, posY + 30, 0xff0000, -1, "%1.2f", keyHistory.history[0].duration);
	
	debugText(posX, posY + 30, 0xffff00, -1, "   %c", keyHistory.history[0].inputCodeRaw );
	debugText(posX, posY + 30, 0xff0000, -1, "%d    %#04x (state : %d)", 1, keyHistory.history[0].inputCodeRaw >>8, voxie_keystat(keyHistory.history[0].inputCode));
	
	if (getKeyDownTime(keyHistory.history[0].inputCode) > 1) 	debugText(posX, posY + 30, 0xffffff, -1, "                               H");
	if (getKeyIsDown(keyHistory.history[0].inputCode) == 1) 	debugText(posX, posY + 30, 0x00ffff, -1, "                              D");
	if (getKeyOnDown(keyHistory.history[0].inputCode) == 1) 	debugText(posX, posY + 30, 0x00ff00, -1, "                             O");
	if (getKeyOnUp(keyHistory.history[0].inputCode) == 1) 		debugText(posX, posY + 30, 0xff00ff, -1, "                            U");



	for (i = 1; i < KEY_HISTORY_LENGTH; i++) {
		if (keyHistory.history[i].duration) debugText(posX + 140, posY + 30 + (10 * i), 0xff0000, -1, "%1.2f", keyHistory.history[i].duration);
		debugText(posX, posY + 30 + (10 * i), 0xFFFF00, -1, "   %c", keyHistory.history[i].inputCodeRaw);
		debugText(posX, posY + 30 + (10 * i), 0xFF0000, -1, "%d    %#04x (state : %d)", i + 1, keyHistory.history[0].inputCodeRaw >> 8, voxie_keystat(keyHistory.history[i].inputCode));
		if (getKeyDownTime(keyHistory.history[i].inputCode) > 1) 	debugText(posX, posY + 30 + (10 * i), 0xffffff, -1, "                               H");
		if (getKeyIsDown(keyHistory.history[i].inputCode) == 1) 	debugText(posX, posY + 30 + (10 * i), 0x00ffff, -1, "                              D");
		if (getKeyOnDown(keyHistory.history[i].inputCode) == 1) 	debugText(posX, posY + 30 + (10 * i), 0x00ff00, -1, "                             O");
		if (getKeyOnUp(keyHistory.history[i].inputCode) == 1) 		debugText(posX, posY + 30 + (10 * i), 0xff00ff, -1, "                            U");

	}


}


//! returns internal voxie_keyboard_history_t struct which holds the keyboard input history. setEnableLegacyKeyInput but be set to false. 
voxie_keyboard_history_t  VoxieBox::getKeyHistory()
{
	return keyHistory;
}


//! draw a DICOM file onto the volumetric display. - for help with this function call contact voxon photonics directly.
void VoxieBox::drawDicom(voxie_dicom_t *vd, const char *gfilnam, point3d *gp, point3d *gr, point3d *gd, point3d *gf, int *animn, int *loaddone) {

	voxie_drawdicom(&vf, vd, gfilnam, gp, gr, gd, gf, animn, loaddone);

}

//! same as VoxieBox::quitLoop() just with an alternative name - exits out the breath() loop.
void VoxieBox::endBreath() {

	quitLoop();
}




//! Call this to tell Voxiebox library you want to exit the breath() loop and quit the application.
/** 
* forces VoxieBox::breath() to return non-zero on its next call.
* 
* By default a clean exit is set to true (VoxieBox::setEnsureCleanExitOnQuitLoop())
* if VoxieBox::setEnsureCleanExitOnQuitLoop() is set to true. VoxieBox::endFrame() is called and GFX stored in memory is cleared.
*
*/
void VoxieBox::quitLoop() 
{

	if (enableCleanExit)	endFrame();
		
	voxie_quitloop();

	if (enableCleanExit) {
		freeGfx((char*)"*");
		
	}

}
//! Plays a sound (can be WAV, FLAC, MP3, M4A) 
/**
 *  @param	fileName		filename (WAV, FLAC, MP3 or M4A) of sound to play
 *  @param	sourceChannel	which channel of the audio file extract (0 left, 1 right, -1 stereo, -2 left + right)
 *  @param	volumeLeft		% of full volume to left  channel {0..100, although may exceed 100}
 *  @param	volumeRight		% of full volume to right channel {0..100, although may exceed 100}
 *  @param	playBackSpeed	frequency scaling. use 1.0 for default sample rate, 2.0 for 1 octave up, 0.5 for 1 octave down... etc
 *
 *  @return
 *    a handle int of 0..MAX if success. Use this as a handle passed to voxie_playsound_update().
 *   -1 if file not found or other error related to loading.
 *   -2 if none of the MAX audio voice slots were free. New sounds don't play if list is full.
 */
int VoxieBox::playSound(const char *fileName, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed)
{

	return voxie_playsound(fileName, sourceChannel, volumeLeft, volumeRight, playBackSpeed);

}


//! Update a currently playing sound 
/** Use a returned voxie::playSound value as the handle (an int that is assigned to that specific sound)
 *	update a sound is great for fades or adjusting volumes set the volperc0, or volperc1 
 *  use to a negative number to turn off the sound and free it from memory.
 *
 *  @param handle			the current sound to handleID to effect (use the int that gets returned when using VoxieBox::playSound() )
 *  @param sourceChannel	which channel of WAV or FLAC to extract (0 left, 1 right, -1 stereo, -2 left + right)
 *  @param volumeLeft		% of full volume to left  channel {0..100, although may exceed 100} set to -1 to turn off
 *  @param volumeRight		% of full volume to right channel {0..100, although may exceed 100} set to -1 to turn off
 *  @param playBackSpeed	frequency scaling. use 1.0 for default sample rate, 2.0 for 1 octave up, 0.5 for 1 octave down... etc
 *
 */
void VoxieBox::updateSound(int handleID, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed)
{

	voxie_playsound_update(handleID, sourceChannel, volumeLeft, volumeRight, playBackSpeed);

}

/** Updates a sound's play position to . Requires the audio's handle ID
 *  @param handleID		the handleID for the audio (when VoxieBox::playSound is called it returns an int value which is its handleID) 
 *	@param second		the time in seconds to move the sound position to. Can use the decimal for exact position
 *  @param seekType		the seek type SEEK_SET (0), SEEK_CUR(1), SEEK_END(2). Most cases SEEK_SET (value of 0). Works similar to the fseek() function
 */
void VoxieBox::updateSoundPosition(int handleID, double second, int seekType)
{
	voxie_playsound_seek(handleID, second, seekType);
}
//!  Define a custom audio callback function to play PCM / raw audio data
/**
 *  This function is used to play PCM / audio data (not a file). 
 *
 *	If used, this function will be called from a separate thread, every 3ms (for Voxiebox mode), 
 *  or every 20ms (for emulated mode).
 *	Format is set by Voxiebox library. To get sample rate and number of channels, read samprate and
 *	nchans from the voxie_wind_t structure after the call to voxie_init(). The sample format is
 *	always signed 32-bit integers, with the PCM located in the least significant 24 bits.
 *
 *  @param userplayfunc	the custom call back function
 *  @param *sampleBuffer	the sample audio buffer passed in from memory
 *  @param sampleRate		the sample rate of the audio (determined by the voxie_wind_t vw.playsamprate)
 *              
 *           @example // An example of an custom audio callback.
 *                     // Once called this function will always be called on a separate thread. Use a global variable to turn it on / off
 *					static int gPlaySound = -1; // if this is set to 0 it will mute. It will play when set to -1 
 *					static int frequencyToPlay = 26162; // middle C note
 *					static void simplemix (int *ibuf, int nsamps)
 *					{
 *						static int cnt[2]; 
 *						int i, c; // c is number of channels to play (vw.playnchans)
 *
 * 						for(i=0;i<nsamps;i++,ibuf+=vw.playnchans)
 *							for(c=min(vw.playnchans,2)-1;c>=0;c--)
 *							{ ibuf[c] = ((cnt[c]&(1<<20))-(1<<19))&gPlaySound; cnt[c] += frequencyToPlay; }
 *					}
 *
 *
 *		      Now that a custom audio callback has been defined. Call it within your VX program and it should play. 
 *	
 * Note : The playback function will continue to be called until stopped. To kill the call back overwrite the user function with a 0 by writing playPCMData(0) in your program.
 */
void VoxieBox::setAudioPlayCallBack(void(*userplayfunc)(int *sampleBuffer, int sampleRate))
{

	voxie_setaudplaycb(userplayfunc);

}




//!  Define a custom audio callback function to record PCM audio data (not a file) 
/** 
 *  this function is used to capture audio from the device. How to use this function is still being documented
 *  important voxie_wind_t variables to have set before recording.
 *  @param  userplayfunc	the custom call back function
 *  @param  recsamprate		recording sample rate - to use, must write before voxie_init()
 *  @param  recnchans		number of audio channels in recording callback
 *  
 */
void VoxieBox::setAudioRecordCallBack(void(*userrecfunc)(int *sampleBuffer, int sampleRate))
{
	voxie_setaudreccb(userrecfunc);
}

//! MountZip - extracts and loads a .zip file into memory.
/**
*  Once a zip has been mounted all file names which are withing the zip folder can be referenced as if they were in the local directory
*  Example you have a zip file with 'example.png' within it. Once you mount that zip you refer 'example.png' in your program to access that file
*
*  @param filename the path and filename of the zip file to mount. (path is relative) 
*/
void VoxieBox::mountZip(char * filename)
{
	voxie_mountzip(filename);
}

//! Causes a screen capture of the volumetric buffer to occur on the next frame. Captured as a PLY file
/**  
 *  Writes to the next available nonexistent numbered file: VOXIE0000.PNG, VOXIE0001.PNG, ..
 *  will output in the current .exe folder unless VoxieBox.ini contains "volcapdir=" variable
 *  where a custom output directory can be set (default for voxiebox.ini is "volcapdir=c:\voxon\media\MyCaptures\"
 */
void VoxieBox::captureVolumeAsPly()
{
	voxie_volcap(NULL, 2, 15); // 1 = PNG, 2 = PLY
}

//! Causes a screen capture of the volumetric buffer to occur on the next frame. Captured as a PNG file
/**  Writes to the next available nonexistent numbered file: VOXIE0000.PNG, VOXIE0001.PNG, ..
 *   will output in the current .exe folder unless VoxieBox.ini contains "volcapdir=" variable 
 *   where a custom output directory can be set (default for voxiebox.ini is "volcapdir=c:\voxon\media\MyCaptures\"
 */
void VoxieBox::captureVolumeAsPng()
{
	voxie_volcap(NULL, 1, 15); // 1 = PNG, 2 = PLY
}

//! Causes a capture of the volumetric buffer to occur on the next frame, or video capture to start /stop.
/** @param fileName the file name of the capture - if null writes to the next available nonexistent numbered file: VOXIE0000.PNG, VOXIE0001.PNG, ..
 *  @param volumeCaptureMode the capture mode to use. Off (Stop recording) = 0, Single PLY = 1, Single PNG = 2, Single REC = 3, Video Rec = 4, Single VCB = 5, Video VCB = 6; 
 *  @param targetVPS if 
 * Note : 	If fileName is NULL, writes to the next available nonexistent numbered file: VOXIE0000.PNG, VOXIE0001.PNG, ..
 */
void VoxieBox::captureVolume(const char *fileName, int volumeCaptureMode, int targetVPS = 15)
{

	voxie_volcap(fileName, volumeCaptureMode, targetVPS);


}
//! If volumetric recording is happening. call this function to stop.
void VoxieBox::captureVolumeStop()
{
	voxie_volcap(NULL, 0, 0);

}

//! returns a timestamp of the compile date of VxCpp.dll expressed as an __int64. (format: YYYYMMDDHHmmss)
/** 
 * 
 * Returns compile date and time of VxCpp.dll as a 64-bit int in this format:
 * year*1e10 + month*1e8 + day*1e6 + hour*1e4 + minute*1e2 + second
 * For example, April 8, 2024 at 16:38:44 would be: 20240408163844
 */ 
    
__int64 VoxieBox::getVxCppVersion()
{
	#define MONTH ((__DATE__[0]=='F') ?  2 : (__DATE__[0]=='S') ? 9 : (__DATE__[0]=='O') ? 10 : (__DATE__[0]=='N') ? 11 : \
						(__DATE__[0]=='D') ? 12 : (__DATE__[1]=='p') ? 4 : (__DATE__[0]=='A') ?  8 : (__DATE__[2]=='y') ?  5 : \
						(__DATE__[2]=='l') ?  7 : (__DATE__[2]=='r') ? 3 : (__DATE__[1]=='a') ?  1 : (__DATE__[1]=='u') ?  6 : 0)
		return (__DATE__[7] & 15) * 10000000000000I64 +
			(__DATE__[8] & 15) * 1000000000000I64 +
			(__DATE__[9] & 15) * 100000000000I64 +
			(__DATE__[10] & 15) * 10000000000I64 +
			MONTH * 100000000I64 +
			(__DATE__[4] & 15) * 10000000I64 +
			(__DATE__[5] & 15) * 1000000I64 +
			(__TIME__[0] & 15) * 100000I64 +
			(__TIME__[1] & 15) * 10000I64 +
			(__TIME__[3] & 15) * 1000I64 +
			(__TIME__[4] & 15) * 100I64 +
			(__TIME__[6] & 15) * 10I64 +
			(__TIME__[7] & 15);
}


//! returns a scrolling color as an RGB hexadecimal value 
/**
 * @param offSet	offset (any whole number) to offset the color cycle to a different period in the cycle 
 * @return color as an RGB hexadecimal value 
 */
int VoxieBox::scrollCol(int offSet = 0 )
{

	int input = (rainbowCounter + offSet) % COL_SCROLL_MAX;

	return PALETTE_COLOR[input];

}
//! Set the speed of the internal color scroller default is 0.1. Value is how long in seconds before a color change.
/**
 * @param speed		how long in seconds before a color change.
 *
 */
void VoxieBox::setColScrollSpeed(double speed)
{
	colScrollSpeed = speed;
}

//! Updates/"moves" a point from the current position towards the destination point. Returns 1 if currentPos collides with destinnationPos otherwise returns a 0.  
/** 
 *  
 @param currentPos point3d pointer to the current position 
 *  @param currentPos point3d to the destination position
 *  @param speed the speed in which the point is traveling
 *  @param accuracy the accuracy to determine if the current position has reached the destination Pos
 *
 *  @returns 1 if currentPos has arrived at the destinationPos (within the accuracy provided) otherwise 0
 */
int VoxieBox::moveToPos(point3d * currentPos, point3d destinationPos, float speed, float accuracy)
{
	float acc = accuracy; // accuracy

	if (!(accuracy > 0)) {
		acc = 0.005;
	}

	point3d pp, vel;
	float f;
	double dtim = deltaTime;

	if (currentPos->x < destinationPos.x + acc + (speed * dtim) && currentPos->x > destinationPos.x - acc - (speed * dtim) &&
		currentPos->y < destinationPos.y + acc + (speed * dtim) && currentPos->y > destinationPos.y - acc - (speed * dtim) &&
		currentPos->z < destinationPos.z + acc + (speed * dtim) && currentPos->z > destinationPos.z - acc - (speed * dtim)
		) {
		currentPos->x = destinationPos.x;
		currentPos->y = destinationPos.y;
		currentPos->z = destinationPos.z;
		return 1;

	}
	else {
		pp.x = destinationPos.x - currentPos->x;
		pp.y = destinationPos.y - currentPos->y;
		pp.z = destinationPos.z - currentPos->z;

		f = sqrt(double(pp.x) * double(pp.x) + double(pp.y) * double(pp.y) + double(pp.z) * double(pp.z)); // normalise vectors

		vel.x = (speed) * (pp.x / f) * dtim;
		vel.y = (speed) * (pp.y / f) * dtim;
		vel.z = (speed) * (pp.z / f) * dtim;

		currentPos->x += vel.x;
		currentPos->y += vel.y;
		currentPos->z += vel.z;

		return 0;
	}

}




// divides a RGB hexadecimal color value by a division number (use this as when using drawModel() the color values are a 4th of the range)
/**
 * @param color		the RGB hexadecimal color to scale down
 * @param divideAmount	the mount to divide the color value.
 *
 * @return the new scaled down color value as an RGB hexadecimal color value.
 */
int VoxieBox::colorHexDivide(int color, float divideAmount) {

	int b, g, r;

	b = (color & 0xFF);
	g = (color >> 8) & 0xFF;
	r = (color >> 16) & 0xFF;

	if (divideAmount == 0) return 0;

	b /= divideAmount;
	g /= divideAmount;
	r /= divideAmount;

	return (r << 16) | (g << 8) | (b);
}

//! Tweens a color to the destination color. good for fade outs or tweens. 
/**
 * @param color			the color value to tween.
 * @param speed			the speed or rate of the color change
 * @param destcolor		the destination color. The color to tween to.
 *
 * @return returns a new color value that is one step closer to the destination color. 
 */
int VoxieBox::tweenCol(int color, int speed, int destcolor) {

	int b, g, r;
	int bd, gd, rd;

	b = (color & 0xFF);
	g = (color >> 8) & 0xFF;
	r = (color >> 16) & 0xFF;
	bd = (destcolor & 0xFF);
	gd = (destcolor >> 8) & 0xFF;
	rd = (destcolor >> 16) & 0xFF;

	if (b > bd)	b -= speed;
	else if (b < bd) b += speed;
	if (r > rd)	r -= speed;
	else if (r < rd) r += speed;
	if (g > gd)	g -= speed;
	else if (g < gd) g += speed;

	if (r < 0x00) r = 0x00;
	if (r > 0xFF) r = 0xFF;
	if (g < 0x00) g = 0x00;
	if (g > 0xFF) g = 0xFF;
	if (b < 0x00) b = 0x00;
	if (b > 0xFF) b = 0xFF;

	return (r << 16) | (g << 8) | (b);

}

//! Brighten (saturate) a color by adding only RGB values if they are needed.
/**
 * @param color		the color value to brighten
 * @param amount	the amount of color to add range (0 - 255) each value is 1 hex value.
 *
 * @return the new brightened color value. 
 */
int VoxieBox::brightenCol(int color, int amount) {

	int b, g, r;
	int in_r = 0, in_b = 0, in_g = 0;

	b = (color & 0xFF);
	g = (color >> 8) & 0xFF;
	r = (color >> 16) & 0xFF;

	if (r > 0) in_r = 1;
	if (g > 0) in_g = 1;
	if (b > 0) in_b = 1;

	if (r < 0x00) r = 0x00;
	if (r > 0xFF) { r = 0xFF; }
	if (g < 0x00) g = 0x00;
	if (g > 0xFF) { g = 0xFF; }
	if (b < 0x00) b = 0x00;
	if (b > 0xFF) { b = 0xFF; }

	if (in_r == 1)	r += amount;
	if (in_g == 1)	g += amount;
	if (in_b == 1)	b += amount;

	if (r < 0x00) r = 0x00;
	if (r > 0xFF) r = 0xFF;
	if (g < 0x00) g = 0x00;
	if (g > 0xFF) g = 0xFF;
	if (b < 0x00) b = 0x00;
	if (b > 0xFF) b = 0xFF;

	return (r << 16) | (g << 8) | (b);

}

//! returns a random color based on the RANDOM_COLOR defined in vxDataTypes.h
int VoxieBox::randomCol() {

	return  RANDOM_COLOR[rand() % sizeof(RANDOM_COLOR) / sizeof(RANDOM_COLOR[0])];

}

//! Compare two point2d with a degree of accuracy. returns true if the two points are the same within the accuracy amount specified
/**
 *  @param a			pointer to the 1st point2d to compare
 *  @param b			pointer to the 2nd point2d to compare
 *	@param accuracy     the accuracy to compare the two points 
 *
 *  @return true if the two points are the same within the accuracy amount specified
 */
int VoxieBox::pointSame(point2d * a, point2d * b, point2d accuracy = { 0.001, 0.001 })
{
	if (a->x + accuracy.x >= b->x && a->x - accuracy.x <= b->x &&
		a->y + accuracy.y >= b->y && a->y - accuracy.y <= b->y ) return 1;

	return 0;
}


//! Compare two point3d with a degree of accuracy. returns true if the two points are the same within the accuracy amount specified
/**
 *  @param a			pointer to the 1st point3d to compare
 *  @param b			pointer to the 2nd point3d to compare
 *	@param accuracy     the accuracy to compare the two points
 *
 *  @return true if the two points are the same within the accuracy amount specified
 */
int VoxieBox::pointSame(point3d * a, point3d * b, point3d accuracy = { 0.001, 0.001, 0.001 })
{
	if (a->x + accuracy.x >= b->x && a->x - accuracy.x <= b->x &&
		a->y + accuracy.y >= b->y && a->y - accuracy.y <= b->y &&
		a->z + accuracy.z >= b->z && a->z - accuracy.z <= b->z ) return 1;

	return 0;
}



void VoxieBox::updateKeyboardHistory()
{

	int i = 0;
	int k = 0;
	voxie_keyboard_history_t okeyHistory = { 0 };

	// this for loop needs to happened before voxie_keyread as that will flush it
	for (i = 0; i < KEY_HISTORY_LENGTH; i++) {

			// >> 8)&0xFF only passes through the scancode....
		if (voxie_keystat(keyHistory.history[i].inputCode) && keyHistory.history[i].isHeld == true) {

			keyHistory.history[i].state = 3;
			keyHistory.history[i].duration = time - keyHistory.history[i].startTime;
		}
		else if (keyHistory.history[i].onUp == false && keyHistory.history[i].isHeld == true) {
			keyHistory.history[i].duration = 0;
			keyHistory.history[i].isHeld = false;
			keyHistory.history[i].onUp = true;
		}
		else {
			keyHistory.history[i].onUp = false;
			keyHistory.history[i].state = 0;
		}

	}

	while (k = voxie_keyread()) {
		// update the lastPressedKey list
		if (k != 0) {

			if (k == keyHistory.history[0].inputCodeRaw && keyHistory.history[0].isHeld == true) continue;

			memcpy(&okeyHistory, &keyHistory, sizeof(keyHistory));

			for (i = 1; i < KEY_HISTORY_LENGTH; i++) {

				keyHistory.history[i].inputCodeRaw		= okeyHistory.history[i - 1].inputCodeRaw;
				keyHistory.history[i].inputCode			= okeyHistory.history[i - 1].inputCode;
				keyHistory.history[i].startTime			= okeyHistory.history[i - 1].startTime;
				keyHistory.history[i].startLastPressed	= okeyHistory.history[i - 1].startLastPressed;
				keyHistory.history[i].duration			= okeyHistory.history[i - 1].duration;
				keyHistory.history[i].isHeld			= okeyHistory.history[i - 1].isHeld;
				keyHistory.history[i].onUp				= okeyHistory.history[i - 1].onUp;
				keyHistory.history[i].state				= okeyHistory.history[i - 1].state;

			}

			keyHistory.history[0].inputCodeRaw = k;
			keyHistory.history[0].inputCode = (k>>8)&0xFF;
			keyHistory.history[0].startLastPressed = keyHistory.history[0].startTime;
			keyHistory.history[0].startTime = time;
			keyHistory.history[0].duration	= 0;
			keyHistory.history[0].isHeld	= true;
			keyHistory.history[0].onUp		= false;
			keyHistory.history[0].state		= 1;

		}
	}


}


//! Updates all Space Nav inputs used internally to manage SpaceNav input states
/**
 * Set VoxieBox::setEnableLegacyNavInput() to true if you want to manage input behavior manually 
 *
 *
 */
void VoxieBox::updateNavInput() {
	
	int j = 0;
	int b = 0;
	int i = 1;
	if (manualNavManage == false) i = 350; // legacy values where between -350 and 350 this scales the new input value (between -1 and 1) to be correct
	for (navplays = 0; navplays < 4; navplays++) {

		onavbut[navplays] = nav[navplays].but;
		if (voxie_nav_read(navplays, &nav[navplays]) == 0) break; 

		navPos[navplays].x += (((getNavDirectionDeltaAxis(navplays, 0)) * i) * this->navSensitivity[navplays]) * deltaTime;
		navPos[navplays].y += (((getNavDirectionDeltaAxis(navplays, 1)) * i) * this->navSensitivity[navplays]) * deltaTime;
		if (invertZAxis)	navPos[navplays].z += -(((getNavDirectionDeltaAxis(navplays, 2)) * i) * this->navSensitivity[navplays]) * deltaTime; // inverted

		else				navPos[navplays].z += (((getNavDirectionDeltaAxis(navplays, 2)) * i) * this->navSensitivity[navplays]) * deltaTime; // normal

		if (enableNavClip) clipInsideVolume(&navPos[navplays]);
		b = getNavButtonState(navplays);
		for (j = 0; j < 4; j++) {

			if ((b >> j & 1) && (onavbut[navplays] >> j & 1)) { // b 1 && o 1 -- button held

				navHistory[navplays].history[j].duration = time - navHistory[navplays].history[j].startTime;
				navHistory[navplays].history[j].state = 1;
				navHistory[navplays].history[j].isHeld = true;
				navHistory[navplays].history[j].onUp = false;
			}

			if ((b >> j & 1) && !(onavbut[navplays] >> j & 1)) { // b 1 && o 0 -- button on

				navHistory[navplays].history[j].startLastPressed = navHistory[navplays].history[j].startTime;
				navHistory[navplays].history[j].startTime = time;
				navHistory[navplays].history[j].inputCodeRaw = j;
				navHistory[navplays].history[j].inputCode = j;
				navHistory[navplays].history[j].state = 3;
				navHistory[navplays].history[j].isHeld = false;
				navHistory[navplays].history[j].onUp = false;
				navHistory[navplays].history[j].duration = 0;
			}

			if (!(b >> j & 1) && (onavbut[navplays] >> j & 1)) { // b 0 && o 1 -- button up

				navHistory[navplays].history[j].inputCodeRaw = j;
				navHistory[navplays].history[j].inputCode = j;
				navHistory[navplays].history[j].state = 0;
				navHistory[navplays].history[j].isHeld = false;
				navHistory[navplays].history[j].onUp = true;
				navHistory[navplays].history[j].duration = 0;
			}

			if (!(b >> j & 1) && !(onavbut[navplays] >> j & 1)) { // b 0 && o 0 -- button off

				navHistory[navplays].history[j].inputCodeRaw = j;
				navHistory[navplays].history[j].inputCode = j;
				navHistory[navplays].history[j].state = 0;
				navHistory[navplays].history[j].isHeld = false;
				navHistory[navplays].history[j].onUp = false;
				navHistory[navplays].history[j].duration = 0;
			}

		}

	}

	// This ensures the spaceNavs which aren't detected aren't filling their values with garbage.
	for (int i = navplays; i < 4; i++) {

		nav[i].but = 0;
		nav[i].ax = 0;
		nav[i].ay = 0;
		nav[i].az = 0;
		nav[i].dx = 0;
		nav[i].dy = 0;
		nav[i].dz = 0;
		navPos[i].x = 0;
		navPos[i].y = 0;
		navPos[i].z = 0;

	}

}

//! Returns the previous button values. Used for a legacy way to create your own button functions. Useful if setEnableLegacyNavInput() is set to true. 
/**
 * @param spaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 * 
 * @return  the button values from the previous volume update. 
 */
int	VoxieBox::getNavPrevButtonState(int spaceNavID) 
{
	return onavbut[spaceNavID];
}
//! Return a point3d of the Nav's tracked position. Used primary when the Space Nav is being used as a cursor.
/** 
 * @param spaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *
 * @return a point3d of the internally tracked Space Nav position
 */
point3d VoxieBox::getNavPosition(int spaceNavID)
{
	return navPos[spaceNavID];
}


//! Returns the button values (presented as binary of each button) for the Space Navigator. 0 = none, 1 = left, 2 = right, 3 = both.
/**
 * @param SpaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 * 
 * @return bits 0 and 1 are the left and right buttons respectfully, 0 = no button pressed, 1 = left button pressed, 2 = right button pressed, 3 = both buttons pressed
 */
int VoxieBox::getNavButtonState(int spaceNavID) 
{
	return nav[spaceNavID].but;
}

//! Returns 1 if particular Space Nav's button is being pressed. Button codes are based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button. 
/**
 *  @param SpaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *  @param buttonCode   the button code to check based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button. 
 *
 *  @return 1 if particular button is currently being pressed otherwise 0.
 */
int VoxieBox::getNavButtonIsDown(int spaceNavID, int buttonCode)
{

	return ((nav[spaceNavID].but >> buttonCode) & 1);

}
//! Returns 1 if particular Space Nav's button has just been pressed. Button codes are based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button. 
/**
 *  @param SpaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *  @param buttonCode   the button code to check based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button.
 *
 *  @return 1 if particular button has just been pressed, otherwise 0. This function is 'on pressed' it will call 1 on being pressed and then 0 after the first instance of a press.
 */
int VoxieBox::getNavButtonOnDown(int spaceNavID, int buttonCode)
{
	return ((nav[spaceNavID].but >> buttonCode) & 1) && !((onavbut[spaceNavID] >> buttonCode) & 1);

}
//! Returns 1 if particular Space Nav's button has been just released from being held ('on up').
/**
 *  @param SpaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *  @param buttonCode   the button code to check based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button.

 *
 *  @return 1 if button is just released, otherwise 0
 */
int VoxieBox::getNavButtonOnUp(int spaceNavID, int buttonCode)
{

	int a = !((nav[spaceNavID].but >> buttonCode) & 1);
	int b = ((onavbut[spaceNavID] >> buttonCode) & 1);
	return a && b;

}
//! Returns in seconds how long a Space Nav's button has been held down, otherwise returns a 0. 
/**
 *  @param SpaceNavID	the ID number of the Space Nav to check (0 = 1st detected,  1 = 2nd detected etc)
 *  @param buttonCode   the button code to check based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button.

 *
 *  @return in seconds how long a Space Nav's button has been held down, otherwise returns a 0. 
 */
double VoxieBox::getNavButtonDownTime(int spaceNavID, int buttonCode) {

	return navHistory[spaceNavID].history[buttonCode].duration;

}

//! Returns 1 if particular Space Nav's buttonCode has been clicked twice within the double click threshold.
/**
 *  @param buttonCode the Space Nav's button code to check (0 = left, 1 = right, 2 = both)
 *
 *  @return 1 if double click has been registered otherwise return 0.
 *
 *  To adjust click hold threshold @see getNavDoubleClickThreshold()
 */
int VoxieBox::getNavDoubleClick(int spaceNavID, int buttonCode) {

	if (time - navHistory[spaceNavID].history[buttonCode].startLastPressed < mouseClickThreshold && getNavButtonOnDown(spaceNavID, buttonCode)) return true;
	return false;

}
//! Set the time between Space Nav clicks to register a 'double click' (which triggers a true setting for getNavDoubleClick())
/**
 * @param timeThreshold		the time in seconds for 2 clicks to register as a 'double click'
 * Note use with VoxieBox::getNavDoubleClick() to register double clicks.
 *
 * @see VoxieBox::getNavDoubleClick()
 * @see VoxieBox::getNavDoubleClickThreshold()
 */
void VoxieBox::setNavDoubleClickThreshold(double timeThreshold)
{
	navClickThreshold = timeThreshold;
}
//! @return returns the internal Space Nav double click threshold value. (Used to determine if a 'double click' has been detected)
double VoxieBox::getNavDoubleClickThreshold()
{
	return navClickThreshold;
}

//! returns the Space Nav's angle input delta as a point3d. Range between -1 and 1. 0 being no change. 
/**
 * @param spaceNavID	the ID number of the Space Nav to check (0 - 3)
 *
 * @return the angle delta (what angle input has been altered since the last volume update)  
 */
point3d VoxieBox::getNavAngleDelta(int spaceNavID)
{

	point3d pReturn{ nav[spaceNavID].ax , nav[spaceNavID].ay, nav[spaceNavID].az };
	point2d a = { 0 };

	if (manualNavManage == false) { // if using modern way convert the data to be between -1 and 1
		if (pReturn.x != 0)	pReturn.x = normaliseNavToFloat(int(pReturn.x));
		if (pReturn.y != 0)	pReturn.y = normaliseNavToFloat(int(pReturn.y));
		if (pReturn.z != 0)	pReturn.z = normaliseNavToFloat(int(pReturn.z));
	}


	a.x = pReturn.x;
	a.y = pReturn.y;
	a = oriCorrection(getNavOrientation(spaceNavID), a.x, a.y);
	pReturn.x = a.x;
	pReturn.y = a.y;

	if (enableNavDeadZone) {
		if (fabs(pReturn.x) > navDeadZone || fabs(pReturn.y) > navDeadZone || fabs(pReturn.z) > navDeadZone) return pReturn;
		else {
			pReturn.x = 0;
			pReturn.y = 0;
			pReturn.z = 0;
			return pReturn;
		}
	}
	else {
		return pReturn;
	}
	
}


//! returns the Space Nav's direction input delta as a point3d. Range between -1 and 1. 0 being no change. 
/**
 * @param spaceNavID	the ID number of the Space Nav to check (0 - 3)
 *
 * @return the direction delta (what direction input has been altered since the last volume update)
 */
point3d VoxieBox::getNavDirectionDelta(int spaceNavID)
{
	point3d pReturn{ nav[spaceNavID].dx , nav[spaceNavID].dy, nav[spaceNavID].dz };
	point2d a = { 0 };
	
	if (manualNavManage == false) { // if using modern way convert the data to be between -1 and 1
		if (pReturn.x != 0)	pReturn.x = normaliseNavToFloat(int(pReturn.x));
		if (pReturn.y != 0)	pReturn.y = normaliseNavToFloat(int(pReturn.y));
		if (pReturn.z != 0)	pReturn.z = normaliseNavToFloat(int(pReturn.z));
	}

	a.x = pReturn.x;
	a.y = pReturn.y;
	a = oriCorrection(getNavOrientation(spaceNavID), a.x, a.y);
	pReturn.x = a.x;
	pReturn.y = a.y;

	if (enableNavDeadZone) {
		if (fabs(pReturn.x) > navDeadZone || fabs(pReturn.y) > navDeadZone || fabs(pReturn.z) > navDeadZone) return pReturn;
		else {
			pReturn.x = 0;
			pReturn.y = 0;
			pReturn.z = 0;
			return pReturn;
		}
	}
	else {
		return pReturn;
	}

}
//! Returns a single axis of a Space Nav's direction delta (range is -1 to 1, 0 is centered). Axis 0 = x, 1 = y, 2 = z
/**
 *	@param spaceNavID the space Nav's id to fetch
 *	@param axis the axis to get the direction information 0 = x, 1 = y 2 = z;
 *
 *  @return Returns a single axis of a Space Nav's direction delta (range is -1 to 1, 0 is centered)
 */
float VoxieBox::getNavDirectionDeltaAxis(int spaceNavID, int axis)
{
	point3d a = getNavDirectionDelta(spaceNavID);

	switch (axis) {
	case 0:
	default:
		return a.x; 
		break;
	case 1:
		return a.y;
		break;
	case 2:
		return a.z;
		break;

	}
	
}

//! returns a single axis of a spaceNav's angle delta (range is -1 to 1, 0 is centered). Axis 0 = x, 1 = y, 2 = z
/**
 *	@param spaceNavID the space Nav's id to fetch
 *	@param axis the axis to get the angle information 0 = x, 1 = y 2 = z;
 *
 *  @return Returns a single axis of a Space Nav's angle delta (range is -1 to 1, 0 is centered)
 */
float VoxieBox::getNavAngleDeltaAxis(int spaceNavID, int axis)
{
	point3d a = getNavAngleDelta(spaceNavID);

	switch (axis) {
	case 0:
	default:
		return a.x;
		break;
	case 1:
		return a.y;
		break;
	case 2:
		return a.z;
		break;

	}
}

//! updates joystick inputs
void VoxieBox::updateJoyInput()
{

	int j = 0;
	int b = 0;
	for (vxnplays = 0; vxnplays < 4; vxnplays++)
	{
		ovxbut[vxnplays] = vx[vxnplays].but;
		if (!voxie_xbox_read(vxnplays, &vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1


		b = getJoyButtonState(vxnplays);

		// orientation fix for dpad buttons
		switch (getJoyOrientation(vxnplays)) {
		case 0: // standard orientation
		default:

			break;
		case 1:
			if ((b >> 0 & 1)) /* up button   */ { vx[vxnplays].but -= 1; vx[vxnplays].but += 2; } // up to down
			if ((b >> 1 & 1)) /* down button */ { vx[vxnplays].but -= 2; vx[vxnplays].but += 1; } // down to up
			if ((b >> 2 & 1)) /* left button */ { vx[vxnplays].but -= 4; vx[vxnplays].but += 8; } // left to right
			if ((b >> 3 & 1)) /* right button*/ { vx[vxnplays].but -= 8; vx[vxnplays].but += 4; } // right to left
			break;
		case 2:
			if ((b >> 0 & 1)) /* up button   */ { vx[vxnplays].but -= 1; vx[vxnplays].but += 4; } // up to left
			if ((b >> 1 & 1)) /* down button */ { vx[vxnplays].but -= 2; vx[vxnplays].but += 8; } // down to right
			if ((b >> 2 & 1)) /* left button */ { vx[vxnplays].but -= 4; vx[vxnplays].but += 2; } // left to down
			if ((b >> 3 & 1)) /* right button*/ { vx[vxnplays].but -= 8; vx[vxnplays].but += 1; } // right to up
			break;
		case 3:
			if ((b >> 0 & 1)) /* up button   */ { vx[vxnplays].but -= 1; vx[vxnplays].but += 8; } // up to right
			if ((b >> 1 & 1)) /* down button */ { vx[vxnplays].but -= 2; vx[vxnplays].but += 4; } // down to left
			if ((b >> 2 & 1)) /* left button */ { vx[vxnplays].but -= 4; vx[vxnplays].but += 1; } // left to up
			if ((b >> 3 & 1)) /* right button*/ { vx[vxnplays].but -= 8; vx[vxnplays].but += 2; } // right to down
			break;
		}

		


		for (j = 0; j < 16; j++) {

			if ( (b >> j & 1) && (ovxbut[vxnplays] >> j & 1) ) { // b 1 && o 1 -- button held

				joyHistory[vxnplays].history[j].duration = time - joyHistory[vxnplays].history[j].startTime;
				joyHistory[vxnplays].history[j].state = 1;
				joyHistory[vxnplays].history[j].isHeld = true;
				joyHistory[vxnplays].history[j].onUp = false;
			}

			if ((b >> j & 1) && !(ovxbut[vxnplays] >> j & 1)) { // b 1 && o 0 -- button on

				joyHistory[vxnplays].history[j].startLastPressed = joyHistory[vxnplays].history[j].startTime;
				joyHistory[vxnplays].history[j].startTime = time;
				joyHistory[vxnplays].history[j].inputCodeRaw = j;
				joyHistory[vxnplays].history[j].inputCode = j;
				joyHistory[vxnplays].history[j].state = 3;
				joyHistory[vxnplays].history[j].isHeld = false;
				joyHistory[vxnplays].history[j].onUp = false;
				joyHistory[vxnplays].history[j].duration = 0;
			}

			if ( !(b >> j & 1) && (ovxbut[vxnplays] >> j & 1)) { // b 0 && o 1 -- button up

				joyHistory[vxnplays].history[j].inputCodeRaw = j;
				joyHistory[vxnplays].history[j].inputCode = j;
				joyHistory[vxnplays].history[j].state = 0;
				joyHistory[vxnplays].history[j].isHeld = false;
				joyHistory[vxnplays].history[j].onUp = true;
				joyHistory[vxnplays].history[j].duration = 0;
			}

			if (!(b >> j & 1) && !(ovxbut[vxnplays] >> j & 1)) { // b 0 && o 0 -- button off

				joyHistory[vxnplays].history[j].inputCodeRaw = j;
				joyHistory[vxnplays].history[j].inputCode = j;
				joyHistory[vxnplays].history[j].state = 0;
				joyHistory[vxnplays].history[j].isHeld = false;
				joyHistory[vxnplays].history[j].onUp = false;
				joyHistory[vxnplays].history[j].duration = 0;
			}

		}
	}

	// This ensures the controllers which aren't detected aren't filling their values with garbage.
	for (int i = vxnplays; i < 4; i++) {

		vx[i].but = 0;
		vx[i].lt = 0;
		vx[i].rt = 0;
		vx[i].tx0 = 0;
		vx[i].tx1 = 0;
		vx[i].ty0 = 0;
		vx[i].ty1 = 0;

	}

}

//! Internal function for managing zip files. 
int VoxieBox::_kzaddstack(const char * fileName) {

	return kzaddstack(fileName);

}

//! Internal function for managing zip files. 
void VoxieBox::_kzuninit()
{
	kzuninit();
}

//! Internal function for managing zip files. 
void VoxieBox::_kzsetfil(FILE * fileName)
{
	kzsetfil(fileName);
}

//! Internal function for managing zip files. 
INT_PTR VoxieBox::_kzopen(const char * st)
{
	return kzopen(st);
}

//! Internal function for managing zip files. 
void VoxieBox::_kzfindfilestart(const char * st)
{
	kzfindfilestart(st);
}

//! Internal function for managing zip files. 
int VoxieBox::_kzfindfile(kzfind_t * find, kzfileinfo_t * fileinfo)
{
	return kzfindfile(find,fileinfo);
}

//! Internal function for managing zip files. 
unsigned int VoxieBox::_kzread(kzfile_t * kzfile, void * buffer, unsigned int leng)
{
	return kzread(kzfile, buffer, leng);
}

//! Internal function for managing zip files. 
unsigned int VoxieBox::_kzfilelength(kzfile_t * kzfile)
{
	return kzfilelength(kzfile);
}

//! Internal function for managing zip files. 
unsigned int VoxieBox::_kztell(kzfile_t * kzfile)
{
	return kztell(kzfile);
}

//! Internal function for managing zip files. 
int VoxieBox::_kzseek(kzfile_t * kzfile, int offset, int whence)
{
	return kzseek(kzfile, offset, whence);
}

//! Internal function for managing zip files. 
int VoxieBox::_kzgetc(kzfile_t * kzfile)
{
	return kzgetc(kzfile);
}

//! Internal function for managing zip files. 
int VoxieBox::_kzeof(kzfile_t * kzfile)
{
	return kzeof(kzfile);
}

//! Internal function for managing zip files. 
void VoxieBox::_kzclose(kzfile_t * kzfile)
{
	kzclose(kzfile);
}

//! Internal function for managing image files. 
int VoxieBox::_kpgetdim(const char * buffer, int nby, int * xsiz, int * ysiz)
{
	return kpgetdim(buffer, nby, xsiz, ysiz);
}

//! Internal function for managing image files. 
int VoxieBox::_kprender(const char * buffer, int nby, INT_PTR fptr, int bpl, int xsiz, int ysiz, int xoff, int yoff)
{
	return kprender(buffer, nby, fptr, bpl, xsiz, ysiz, xoff, yoff);
}

//! Internal function for managing zip files. 
void VoxieBox::_kpzload(const char * fileName, INT_PTR * fptr, INT_PTR * bpl, INT_PTR * xsiz, INT_PTR * ysiz)
{
	kpzload(fileName, fptr, bpl, xsiz, ysiz);
}

//! Internal function for clearing up touch input data
void VoxieBox::touchClear(touchInput_t * touchInputPtr, int index)
{
	
	touchInputPtr->tPoint[index].oposx = NULL;
	touchInputPtr->tPoint[index].oposy = NULL;
	touchInputPtr->tPoint[index].deltax = NULL;
	touchInputPtr->tPoint[index].deltay = NULL;
	touchInputPtr->tPoint[index].posx = NULL;
	touchInputPtr->tPoint[index].posy = NULL;
	touchInputPtr->tPoint[index].state = -1;
	touchInputPtr->tPoint[index].justPressed = false;
	touchInputPtr->tPoint[index].onUp = false;
	touchInputPtr->tPoint[index].active = false;
	touchInputPtr->tPoint[index].isDown = false;
	touchInputPtr->tPoint[index].inPinch = false;
	touchInputPtr->tPoint[index].startTime = 0;
	touchInputPtr->tPoint[index].lastUpdate = 0;


}

void VoxieBox::touchDraw()
{
	int x = -1, y = -1, i = 0, j = 0;

	touch.pinchActive = false;

	for (i = 0; i < touch.currentTouchNo; i++) {

		if (touch.tPoint[i].state == -1) continue;

		// work out radius of touch
		j = 5 + ((time - touch.tPoint[i].startTime) * 75);
		if (j > 30) j = 25;

		if (touch.tPoint[i].justPressed == true || touch.tPoint[i].isHeld) voxie_debug_drawcircfill(touch.tPoint[i].posx, touch.tPoint[i].posy, j, 0x9d9d9d);

		else voxie_debug_drawcirc(touch.tPoint[i].posx, touch.tPoint[i].posy, j, 0x9d9d9d);


		// limit pinch when there is only two contact points
		if (/* touch.currentTouchNo == 2 */ touch.pinchActive == false) { //  remove this if statement if you want the pinch to search through the finger index

			// draw pinch
			if (touch.currentTouchNo > 1 && x == -1 || touch.currentTouchNo > 1 && y == -1) {
				if (touch.tPoint[i].active == true && x == -1)		{	x = i; touch.pinch0Index = i; }
				else if (touch.tPoint[i].active == true && y == -1) {	y = i; touch.pinch1Index = i; }
				if (x != -1 && y != -1) {

					touch.pinchActive = true;
					touch.pinchLastUpdate = time;
					touch.tPoint[x].inPinch = true;
					touch.tPoint[y].inPinch = true;

					voxie_debug_drawline(touch.tPoint[x].posx, touch.tPoint[x].posy, touch.tPoint[y].posx, touch.tPoint[y].posy, 0x00ffff);

				}
			}
		}

		// draw pix and overlay
		voxie_debug_drawpix(touch.tPoint[i].posx, touch.tPoint[i].posy, 0x00ff00);
		voxie_debug_drawline(touch.tPoint[i].posx, touch.tPoint[i].posy, touch.tPoint[i].posx + touch.tPoint[i].deltax, touch.tPoint[i].posy + touch.tPoint[i].deltay, 0xff0000);

	}

}

//! Internal function for managing touch inputs
void VoxieBox::updateTouch()  
{
	//if (!touchUpdate) return;

	int i = 0, j = 0, x = 0, y = 0;
	point2d pp = { 0 };
	point2d distFrom = { 0 }, distTo = { 0 };

	touch.ogDeltaX = touch.gDeltaX;
	touch.ogDeltaY = touch.gDeltaY;

	touch.gDeltaX = 0;
	touch.gDeltaY = 0;

	x = touch.pinch0Index;
	y = touch.pinch1Index;

	if (touch.pinchActive) {

		distFrom.x = touch.tPoint[y].posx;
		distFrom.y = touch.tPoint[y].posy;

		distTo.x = touch.tPoint[x].posx;
		distTo.y = touch.tPoint[x].posy;

		touch.opinchDistance = touch.pinchDistance;

		// calculate 2D distances 
		pp.x = (distTo.x - distFrom.x);
		pp.y = (distTo.y - distFrom.y);

		touch.pinchDistance = sqrt((pp.x * pp.x) + (pp.y * pp.y));

		touch.opinchRotation = touch.pinchRotation;

		// calculate 2D angle
		touch.pinchRotation = atan2((distTo.y - distFrom.y), (distTo.x - distFrom.x));





		touch.opinch0.x = touch.tPoint[x].posx;
		touch.opinch0.y = touch.tPoint[x].posy;
		touch.opinch1.x = touch.tPoint[y].posx;
		touch.opinch1.y = touch.tPoint[y].posy;


		
		touch.pinchDistanceDelta = (touch.pinchDistance - touch.opinchDistance);
		touch.pinchRotationDelta = (touch.pinchRotation - touch.opinchRotation);

		if (fabs(touch.pinchDistanceDelta) * .01 > fabs(touch.pinchRotationDelta)) touch.pinchPriority = 1;
		if (fabs(touch.pinchDistanceDelta) * .01 < fabs(touch.pinchRotationDelta)) touch.pinchPriority = 2;
		//	else touch.pinchPriority = 0;

		if (touch.initPinch) {
			touch.pinchDistanceDelta = 0;
			touch.pinchRotationDelta = 0;
			touch.initPinch = false;
		}

	}
	else {
		touchPinchClear(&touch);
	}

	touch.currentTouchNo = 0;

	while (voxie_touch_read(&i, &x, &y, &j)) {

		touch.tPoint[i].oposx = touch.tPoint[i].posx;
		touch.tPoint[i].oposy = touch.tPoint[i].posy;

		touch.tPoint[i].posx = x;
		touch.tPoint[i].posy = y;

		if (j == 1) {
			touch.tPoint[i].oposx = -1;
			touch.tPoint[i].oposy = -1;
			touch.tPoint[i].startTime = this->time;
		}

		touch.tPoint[i].lastUpdate = this->time;
		touch.tPoint[i].state = j;
		touch.tPoint[i].active = true;

		touch.currentTouchNo++;
	}

	if (touch.currentTouchNo > TOUCH_MAX) touch.currentTouchNo = TOUCH_MAX;

	// update loop for touch controls
	for (i = 0; i < TOUCH_MAX; i++) {

		// add clear if  last update is long -- this relieves unexpected errors
		if (touch.tPoint[i].lastUpdate + TOUCH_TIME_OUT < this->time)  touchClear(&touch, i);

		touch.tPoint[i].justPressed = false;
		touch.tPoint[i].onUp = false;
		touch.tPoint[i].isDown = false;
		touch.tPoint[i].isHeld = false;
		touch.tPoint[i].inPinch = false;

		// state reminder; 1 == just pressed, 2 == off, 0 == down
		if (touch.tPoint[i].state == 1 || touch.tPoint[i].state == 0 && touch.tPoint[i].ostate == -1) touch.tPoint[i].justPressed = true;
		if (touch.tPoint[i].state == 2 && touch.tPoint[i].ostate <= 1) touch.tPoint[i].onUp = true;
		if (touch.tPoint[i].state == 0) touch.tPoint[i].isDown = true;

		// hold / held check
		if (touch.tPoint[i].state == 0 && this->time - touch.tPoint[i].startTime > touch.heldTime) touch.tPoint[i].isHeld = true;

		if (touch.tPoint[i].state == 0 && touch.tPoint[i].justPressed == false) {

			touch.tPoint[i].deltax = touch.tPoint[i].posx - touch.tPoint[i].oposx;
			touch.tPoint[i].deltay = touch.tPoint[i].posy - touch.tPoint[i].oposy;

			touch.gDeltaX += touch.tPoint[i].deltax;
			touch.gDeltaY += touch.tPoint[i].deltay;

		}

		touch.tPoint[i].ostate = touch.tPoint[i].state;

		if (touch.tPoint[i].state == 2 && touch.tPoint[i].onUp == false) { // 2 is off
			touchClear(&touch, i);
			continue;
		}

	}

}
//! internal function to help manage the pinch touch state
void VoxieBox::touchPinchClear(touchInput_t * touchInputPtr)
{
	if (touchInputPtr->pinchLastUpdate + 0.1 < this->time) {

		touchInputPtr->opinch0.x = 0;
		touchInputPtr->opinch0.y = 0;
		touchInputPtr->opinch1.x = 0;
		touchInputPtr->opinch1.y = 0;
		touchInputPtr->opinchDistance = 0;
		touchInputPtr->pinchDistance = 0;
		touchInputPtr->pinchDistanceDelta = 0;
		touchInputPtr->pinchRotationDelta = 0;
		touchInputPtr->initPinch = true;
		touchInputPtr->pinchRotation = 0;
		touchInputPtr->opinchRotation = 0;
		touchInputPtr->pinchActive = false;
		touchInputPtr->pinchPriority = -1;
		touchInputPtr->pinch0Index = 0;
		touchInputPtr->pinch1Index = 0;
	}
}
