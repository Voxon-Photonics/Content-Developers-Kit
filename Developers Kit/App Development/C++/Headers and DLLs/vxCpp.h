#pragma once
/** \mainpage VxCpp - VX++ "Friendly Voxon Development with classes"
 * \section intro_sec Introduction
 *
 *  VxCpp or (VX++) is a C++ wrapper for the Voxiebox SDK (Voxiebox SDK by Ken Silverman).
 *  The VxCpp.dll allows the Voxon API to be used as a 'voxiebox class' making Voxon application development
 *  compatible with object oriented principals.
 *
 *	The VoxieBox class includes all the functions from the Voxon API, extra helper functions and quality of life
 *  changes. The intention of this project is to make development for Voxon Applications easier.
 *  In a nutshell, VX++ offers "friendly Voxon development with classes".
 *
 *	VxCpp.dll does not replace the voxiebox.dll. VX applications that use VxCpp still require access to voxiebox.dll to work.
 *
 *  please note for users who have worked directly with the voxiebox.dll some of the original voxiebox.dll function names
 *  have been named differently, functions may behave differently (and call functions written in camelCase).
 *
 *  The compiled VxCpp.dll and associated files are released as public domain. However the source code for VxCpp / VX++ project
 *	is released with a CC BY licence. You may release your VX++ applications (even commercially) and distribute the compiled VxCpp.dll
 *  freely without accreditation. However, any adaption or code and built upon the source code from VxCpp requires accreditation
 *  for the original creators. (Matthew Vecchio & Voxon Photonics).
 *
 *  \section benefits_sec Benefits of Using the VX++ Framework
 *
 *
 *               VxCpp.Dll / Developing VX++ App benefits:
 *
 *                "Voxiebox as a class"
 *
 *                * Presents VoxieBox as a class so Voxon Applications can be made using OO principles
 *                * Supports all function calls from VoxieBox.h / VoxieBox.dll
 *
 *
 *	              "Quality of life features"
 *
 *                *  Embedded Hotkeys for rotation for emulator/simulator mode
 *                      (use '[' and ']' to rotate the view. R Ctrl + '[' or ']' zooms in or out R Shift + '[' or ']' adjust vertical view)
 *                *  Automatically detect Up/Down or Spinner hardware
 *                *  Easier Debuging Reporting for Voxie Window, Voxie Frame and all inputs types
 *                *  Extended input function checks for mouse, keyboard, SpaceNav and gamepads all inputs have functions to check for onDown(),
 *                      onUp(), duration(), and isDown()
 *                *  Internally managed Input for Game controllers and SpaceNav. More advanced Key stroke history.
 *                *  Various boolean function calls to turn on/off features such as 'drawBorder' 'clipMouse' or 'invertZAxis'
 *                *  Embedded cursor (for mouse or otherwise)
 *                *  DrawQuad() function to easily draw a textured Quad on the display.
 *                *  All quality of life features can be disabled / enabled independently.
 *
 *
 *                "Included helper functions"
 *
 *                *  Simple collision detection for sphere, box and 'is inside a box'
 *                *  Color management included palette for the display, functions to tween color to color, color scroll, get a random color,
 *                      color divide and color brighten.
 *                *  Simple Move To Point function for moving a position to another.
 * 
 *
 *
 *  \section developing_sec Developing with VxCpp.dll and making your own VX++ App
 *
 *  Developing with VxCpp.dll is similar to developing with the voxiebox.dll. A VX application needs to load in the voxiebox.dll
 *  (Ken's Voxon SDK library) a VX++ application loads in the voxiebox.dll by an entry function in VxCpp.dll "CreateVoxieBoxObject"
 *  calling this function creates a new 'VoxieBox' an object Once the VoxieBox object is created it can be accessed using an
 *  pointer to the IVoxieBox interface.
 *
 *  A simple hello world example...
 *
 *					// VX++ APP HELLO WORLD EXAMPLE
 *					// You may use this as a template to start creating your own volumetric applications.
 *					#include "VxCpp.h"
 *					int main(HINSTANCE hidistributenst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
 *					{
 *
 *						// load in the VxCpp.dll so we can create the VoxieBox object...
						HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
						if (!hVoxieDLL) return 1;
						CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
						if (pEntryFunction == 0) return 1;
						IVoxieBox* voxie = pEntryFunction();

 *						// variables to setup text positions to display 'Hello World' -- feel free to delete this
 *						point3d textPos		{ -0.5,	0, 0 };		// text positions x,y,z values
 *						point3d textWidth	{ 0.1,	0, 0 };		// text rVector  x,y,z ... the x value determines the width of the text the other values deal with rotating text
 *						point3d textHeight	{ 0, 0.15, 0 };	// text dVector  x,y,z ... the y value determines the height of the text the other values deal with rotating text
 *
 *						voxie->setBorder(true); // if true draws a border around the perimeter of the display
 *
 *						// Update loop for program -- breath() -- is a complete volume sweep. Called once per volume.
 *						while (voxie->breath())
 *						{
 *							voxie->startFrame();   // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the endFrame() function)
 *							voxie->drawText(&textPos, &textWidth, &textHeight, 0xffffff, "Hello World"); // draw text onto the volumetric display.
 *							textPos.z = cos(voxie->getTime()) / 5; // move the text's Z position over time using cos (creates a moving up and down effect)*
 *							voxie->debugText(35, 100, 0xffffff, -1, "Hello World On the Touch Screen!"); // draw text onto the secondary (touch) screen.
 *							voxie->showVPS(); //show VPS data and VxCpp.DLL version unto the touch screen.
 *							voxie->endFrame(); // the end of drawing to the volumetric display.
 *						}
 *
 *						voxie->quitLoop(); // quitLoop() stops the hardware from physically moving and ends the breath() loop
 *						delete voxie;
 *						return 0; 	// After the program quits the de-constructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
 *					}
 *
 *
 *  \section running_sec Running Applications written using VX++ / VxCpp
 *
 *
 *  A VX++ App (Voxon App developed with VxCpp.DLL) requires  access to the VxCpp.dll. To do this you can copy VxCpp.dll
 *  to the local directory, place it in the Voxon runtime directory (C:\Voxon\System\Runtime\) or set it up in your system's
 *  path variable.
 *
 *
 *	\section releases_sec Updating / Where to get newer versions of VX++ / VxCpp.dll
 *
 *  VxCpp / VX++ public release (with source code) is available as part of the Voxon Content Developers Kit which can be obtained
 *  freely at https://github.com/Voxon-Photonics/Content-Developers-Kit
 *
 *
 *	\section licencefg_sec Licence
 *
 *  The compiled VxCpp.dll is public domain any application you make using VxCpp.dll is yours.
 *  Applications made with VX++ and the VxCpp.dll can be distributed with your applications freely without any accreditation needed.
 *  However. VX++ / VxCpp.dll's source code is released with a Creative Commons By Attribution (CC BY) license.
 *  This means that licensees may copy, distribute, display and perform the work and make derivative works and remixes based on it
 *  only if they give the author (Matthew Vecchio / Voxon Photonics ) proper accreditation
 *
 *
 *  \section contact_sec Contact information
 *
 *	VxCpp / VX++ is maintained by Matthew Vecchio for Voxon Photonics. Contact me at Matt@Voxon.co for feature requests, code suggestions.
 *	bug reports, feedback, showing off cool stuff you have done or anything else
 *  you can contact me at Matt@Voxon.co
 *
 */

 /** \file VxCpp.h
  *
  * Header file for VxCpp.dll outlines the VoxieBox class and the IVoxieBox interface.
  * main header file for VX++ development. Requires vxDataTypes.h, vxInterfaces.gh and vxInputTypes.h to work.
  * @see VoxieBox::VoxieBox() @see IVoxieBox
  */

#ifdef VOXIEBOXCPP_EXPORTS
#define EXTEND_API __declspec(dllexport)
#else
#define EXTEND_API __declspec(dllimport)
#endif

  // include VoxieBox.DLL types 
#include "vxDataTypes.h"
#include "vxInputTypes.h"

#include <memory>
#include <vector>


#define PI 3.14159265358979323
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

//! Interface for VoxieBox class 
/**
*	Contains all the methods and attributes needed to encapsulate Voxiebox.dll into its the VoxieBox class
*   these methods and attributes are abstract and are overwritten by the VoxieBox class on runtime.
*
*   For detailed attributes and methods information see the member information on @see VoxieBox
*/
class IVoxieBox {
public:

	// De-constructor


		// De-constructor for VoxieBox automatically calls shutdown() and frees the .DLL from memory.
	virtual ~IVoxieBox() { ; }

	// Core Functions 


	//! A 'breath' is the 'update loop' for a VX application. A single breath is a single volume. breath() passes in the default voxie_window and input struct
	virtual int breath() = 0;
	//! Breath() function which can be used with custom input struct.   
	virtual int breath(voxie_inputs_t* input) = 0;
	//! Signifies the start of the volumetric frame. Call this once before all draw calls. Prepares the internal vf voxie_frame_t() voxel buffer to receive volumetric content.  
	virtual void startFrame() = 0;
	//! Signifies the end of a volumetric frame. Call this once after all draw calls. Sends the voxel buffer to the volumetric and secondary screen. 
	virtual void endFrame() = 0;
	//! Forces VoxieBox::breath() to return non-zero on its next call. Usually called when you want the program to quit.
	virtual void quitLoop() = 0;
	//! Frees the VoxieBox.DLL from memory and allows the file to be accessed by other processes
	virtual void shutdown() = 0;
	//! First time it is called it initialise the voxie_wind_t. Subsequent calls updates the VoxieBox.dll's voxie window struct
	virtual  int init() = 0;
	//! returns a pointer to the internal voxie_wind_t struct
	virtual voxie_wind_t* getVoxieWindow() = 0;
	//! returns a pointer to the internal voxie_frame_t struct
	virtual voxie_frame_t* getVoxieFrame() = 0;

	// Settings


	//! Toggles a colored border around the perimeter of the volumetric display. Set to false by default. default color is white.
	virtual void setBorder(bool option, int color = 0xffffff) = 0;
	//! Toggles the use of the 'esc' key to escape a Voxon program (by calling VoxieBox::quitLoop()). Set to true by default.
	virtual void setEnableExitOnEsc(bool option) = 0;
	//! Toggles mouse clipping (cursor / position is always inside the volume) when using the VoxieBox::getMousePosition or the VoxieBox::drawCursor() functions.  Set to false by default.
	virtual void setEnableMouseClipping(bool option) = 0;
	//! Toggles nav clipping (cursor / position is always inside the volume) when using the VoxieBox::getNavPosition or the VoxieBox::drawCursor() functions.  Set to false by default.
	virtual void setEnableNavClipping(bool option) = 0;
	//! Toggles when enabled and VoxieBox::quitLoop() is called the Voxiebox library is freed from memory. Set to true by default.
	virtual void setCleanExitOnQuitLoop(bool option) = 0;
	//! Sets VoxieBox class to use the XInput API to read joystick inputs (for modern game controllers).
	virtual void setJoyInputToXInput() = 0;
	//! Set VoxieBox class to use the DirectInput API to read joystick inputs (for older game controllers). Uses joyGetPosEx function.
	virtual void setJoyInputToDirectInput() = 0;
	//! Set to true to handle Joy input manually. Disables many of the joy input functions.	Set to false by default
	virtual void setEnableLegacyJoyInput(bool option) = 0;
	//! Set to true to handle Touch input manually. Disables many of the touch input functions.	Set to false by default
	virtual void setEnableLegacyTouchInput(bool option) = 0;
	//! Set to true to give access to the touch screen (if disabled touching the screen will always bring up the Voxie Menu)
	virtual void setEnableJoyDeadZone(bool option) = 0;
	//! Toggles a dead zone for Space Navigator. Set to true by default.
	virtual void setEnableNavDeadZone(bool option) = 0;
	//! Set to true to handle Nav input manually. Disables many of the nav input functions.	Set to false by default
	virtual void setEnableLegacyNavInput(bool option) = 0;
	//! Set to false to disable the VoxieBox class / VxCpp.dll to from managing gamepad / joy input handling. Set to false by default. 
	virtual void setEnableLegacyKeyInput(bool option) = 0;
	//! Inverts the Z axis. (by default - values are at the top of the screen and + values are below). Set to false by default.
	virtual void setInvertZAxis(bool option) = 0;

	// Time


		//! Returns in seconds the delta time(time between volumes) delta time is CPU speed dependent and can be used to make ensure timing is consistent between various computers / systems
	virtual double getDeltaTime() = 0;
	//! Returns the time (in seconds) from program execution till present.
	virtual double getTime() = 0;
	//! Returns the system's current VPS (volumes per second) - need to be +15 VPS for a up/down display to run smoothly
	virtual double getVPS() = 0;

	// Display


		//! Returns the internal voxie_wind_t's aspect X ratio
	virtual float	getAspectX() = 0;
	//! returns the internal voxie_wind_t's aspect Y ratio
	virtual float	getAspectY() = 0;
	//! returns the internal voxie_wind_t's aspect Z ratio
	virtual float	getAspectZ() = 0;
	//! returns the internal voxie_wind_t's aspect ratio values data as a point3d 
	virtual point3d getAspect() = 0;
	//! set the internal voxie_wind_t's aspect X ratio
	virtual void	setAspectX(float newAspectX) = 0;
	//! set the internal voxie_wind_t's aspect Y ratio
	virtual void	setAspectY(float newAspectY) = 0;
	//! set the internal voxie_wind_t's aspect Z ratio
	virtual void	setAspectZ(float newAspectZ) = 0;
	//! set the internal voxie_wind_t's aspect ratio values data as a point3d 
	virtual void	setAspect(point3d newAspect) = 0;
	//! Turns off the reciprocating screen and effectively makes the display a '2D' screen. (Works on Voxon hardware only)
	virtual void	setDisplay2D() = 0;
	//! Turns on the reciprocating screen / activates the volumetric display. (Works on Voxon hardware only)
	virtual void	setDisplay3D() = 0;
	//! Sets the 'view' for the volumetric display by default it is the aspect ratio of the voxie window but can be overridden
	virtual void	setView(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax) = 0;
	//! Same as setView() but using two point3d for coordinates instead of 6 floats. (LUT = left, up, top value, RDB = right, down, bottom value)
	virtual void    setView(point3d LUT, point3d RDB) = 0;
	//! Call after each setView() to mask off a plane of a specified thickness. Used for viewing a 'slice' of a scene.
	virtual void	setMaskPlane(float x0, float y0, float z0, float normVx, float normVy, float normVz) = 0;
	//! Changes the global normal vector (vw.normhax) for shading. Pass in three 0's to disable. 
	virtual void    setGlobalShader(float horizontalAngle, float verticalAngle, float amplitude) = 0;
	//! Flush all graphics commands on internal buffer, causing all graphics commands execute and complete.
	virtual void	flushGfx(void) = 0;
	//! Frees a filename from VoxieBox.dll internal cache (any filename passed to voxie_draweshtex(), voxie_drawmeshtex_ext(), voxie_drawheimap(), voxie_drawspr())
	virtual void	freeGfx(const char* fileName) = 0;
	//! Advanced utility function for keystone calibration (used by graphcalc and keystone calibration in voxiedemo)
	virtual void	setProject(int dispNum, int dir, float x, float y, int z, float* xo, float* yo) = 0;
	//! Set LEDs values on projector hardware. WARNING: Be careful not to overheat projector! Sending over 160 can be risky!
	virtual void	setLeds(int dispNum, int r, int g, int b) = 0;
	//! Exposes all the variables from the internal vw voxie_wind_t onto the secondary (touch) screen. For debugging
	virtual void	reportVoxieWind(int posX, int posY) = 0;
	//! Exposes all the variables from the internal vf voxie_frame_t onto the secondary (touch) screen. For debugging
	virtual	void    reportVoxieFrame(int posX, int posY) = 0;

	// Debug 


		//! Display text unto the secondary (touch) screen. Must be called within start and end frame functions. backgroundCol = -1 for transparent
	virtual void	debugText(int posX, int posY, int forgroundCol, int backgroundCol, const char* fmt, ...) = 0;
	//! Draws a vertical bar for help tracking variables onto the secondary (touch) screen. Must be called within the startFrame() endFrame() functions.
	virtual double	debugBar(int posX, int posY, double currentVal, double maxVal, double minVal, const char* text, int type) = 0;
	//! Displays volumes per second (volumetric equivalent of 'FPS') and version info on the secondary (touch) screen. Must be called within the startFrame() & endFrame() functions. Returns VPS value
	virtual double	showVPS(int posX = 15, int posY = 525) = 0;


	// Drawing Graphics to Secondary Screen

		//! Draws single pixel on the secondary (touch) screen. Must be called between startFrame() endFrame() functions.
	virtual void debugDrawPix(int x, int y, int col) = 0;
	//! Draws horizontal line on the secondary (touch) screen. Must be called between startFrame() endFrame() functions.
	virtual void debugDrawHLine(int xStartPos, int xEndPos, int y, int col) = 0;
	//! Draws a line on the secondary (touch) screen. Must be called between startFrame() & endFrame() functions.
	virtual void debugDrawLine(float xStartPos, float yStartPos, float xEndPos, float yEndPos, int col) = 0;
	//! Draw circle on the secondary (touch) screen. Must be called between startFrame() & endFrame() functions.
	virtual void debugDrawCircle(int xCenterPos, int yCenterPos, int radius, int col) = 0;
	//! Draw filled rectangle on the secondary (touch) screen.  Must be called between startFrame() & endFrame() functions. 
	virtual void debugDrawBoxFill(int xStartPos, int yStartPos, int xEndPos, int yEndPos, int col) = 0;
	//! Draw filled circle on the secondary (touch) screen.  Must be called between startFrame() & endFrame() functions. 
	virtual void debugDrawCircFill(int xCenterPos, int yCenterPos, int radius, int col) = 0;
	//! Draw a 2D texture to the secondary (touch) screen. Must be called between startFrame() & endFrame() functions. Sent through a tiletype_t pointer. 
	virtual void debugDrawTile(tiletype* source, int xpos, int ypos) = 0;

	//! Sets a custom font to be used as an internal tile type to be written to the secondary (touch) screen.
	virtual bool debugCustomFontSet(const char* fileName, int index = -1) = 0;
	//! Prints the custom bitmap font to the secondary (touch) screen. Must be called between startFrame() & endFrame() functions.
	virtual void debugCustomFont(int xpos, int ypos, int customFontIndex, const char* fmt, ...) = 0;


	//  Drawing Graphics to Volumetric Display

	//! Renders a single voxel onto the volumetric using a 3 floats for coordinates. Must be called between startFrame() & endFrame() functions.
	virtual void drawVox(float x, float y, float z, int col) = 0;
	//! Renders a single voxel onto the volumetric using a single point3d for coordinates. Must be called between startFrame() & endFrame() functions.
	virtual void drawVox(point3d pos, int col) = 0;
	//! Renders a rectangle / box on the volumetric display using 3 floats for coordinates. 0 position is LEFT, UP, TOP, 1 position is RIGHT, DOWN, BOTTOM. fillmode = 0 dots, 1 lines, 2 surfaces, 3 solid
	virtual void drawBox(float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col) = 0;
	//! Renders a rectangle / box on the volumetric display using 2 point3ds for coordinates. fillmode = 0 dots, 1 lines, 2 surfaces, 3 solid. Must be called between startFrame() & endFrame() functions.
	virtual void drawBox(point3d posLeftUpTop, point3d posRightDownBottom, int fillmode, int col) = 0;
	//! Renders a cube onto the volumetric display using specified vectors. fillmode = 0 dots, 1 lines, 2 surfaces, 3 solid
	virtual void drawCube(point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int fillmode, int col) = 0;
	//! Renders a sphere on the volumetric display using 3 floats. fillmode = 0 surface, 1 filled. Must be called between startFrame() & endFrame() functions.
	virtual void drawSphere(point3d pos, float radius, int fillmode, int col) = 0;
	//! Renders a sphere on the volumetric display using 3 floats. fillmode = 0 surface, 1 filled. Must be called between startFrame() & endFrame() functions.
	virtual void drawSphere(float x, float y, float z, float radius, int fillmode, int col) = 0;
	//! Renders a line on the volumetric display using 6 floats for coordinates. Must be called between startFrame() & endFrame() functions.
	virtual void drawLine(float xStartPos, float yStartPos, float zStartPos, float xEndPos, float yEndPos, float zEndPos, int col) = 0;
	//! Renders a line on the volumetric display using point3d for coordinates. Must be called between startFrame() & endFrame() functions.
	virtual void drawLine(point3d startPos, point3d endPos, int col) = 0;
	//!Renders a filled polygon. Assumes points are in loop order and coplanar. Must be called between startFrame() & endFrame() functions.
	virtual void drawPoly(pol_t* polArray, int ptCount, int col) = 0;
	//! Draws a mesh from a list of vertices can render in as dots, lines, polygons, or filled mesh.
	virtual void drawMesh(const char* fileName, poltex_t* verticeList, int verticeNum, int* meshList, int meshNum, int flags, int col) = 0;
	//! Extended version of drawMesh draw by separately passing in the XYZ, UVC and Indices to render in as dots, lines, polygons, or filled mesh.
	virtual void drawMeshExt(const char* fileNam, point3d* VerticeXYZArray, int verticeXYZNum, uvcol_t* verticcUVCArray, int verticeUVCNum,  inds_t* indiceArray, int indiceNum, int flags, int col) = 0;
	//! Renders a cone shape on the volumetric display with rounded ends (also capable of rendering a cylinder/sphere) using 6 floats for coordinates
	virtual void drawCone(float xStartPos, float yStartPos, float zStartPos, float startRadius, float xEndPos, float yEndPos, float zEndPos, float endRadius, int fillmode, int col) = 0;
	//! Renders a cone shape on the volumetric display with rounded ends (also capable of rendering a cylinder/sphere) using 2 point3ds for coordinates
	virtual void drawCone(point3d startPos, float startRadius, point3d endPos, float endRadius, int fillmode, int col) = 0;
	//! Renders 3D model (.obj, .ply, .stl, .kv6). Displays a filename mesh onto the volumetric display.
	virtual int  drawModel(const char* fileName, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int col) = 0;
	//! Returns the extents of a 3D model (.obj, .kv6, .stl etc...)
	virtual int drawModelGetExtents(const char* charbuff, extents_t* extentsPtr, int flags) = 0;
	//! Renders 3D model (.obj, .ply, .stl, .kv6) Displays a filename mesh onto the volumetric display. Extended from VoxieBox:drawModel
	virtual int  drawModelExt(const char* fileName, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int col, float forcescale, float fdrawratio, int flags) = 0;
	//! Renders a 2D textured (.png, .jpg... most image formats) quad (plane) onto the volumetric display. Useful to rendering 2D textures. Must be called between startFrame() & endFrame() functions.
	virtual void drawQuad(const char* filename, point3d* pos, float width, float height, float hAng, float vAng, float twist, int col, float uValue, float vValue) = 0;
	//! Renders a heightmap (.jpg, .png or tiletype data) onto the volumetric display
	virtual float drawHeightMap(char* fileName, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int colorKey, int reserved, int flags) = 0;
	//! Renders a string (printf-style) unto the volumetric display. Must be called between startFrame() & endFrame() functions.
	virtual void drawText(point3d* pos, point3d* rVector, point3d* dVector, int col, const char* fmt, ...) = 0;
	//! Renders a string (printf-style) unto the volumetric display additional parameter for font width. Must be called between startFrame() & endFrame() functions.
	virtual void drawTextExt(point3d* pos, point3d* rVector, point3d* dVector, float size, int col, const char* fmt, ...) = 0;

	//! Simple version to render a string (printf-style) unto the volumetric display. Must be called between startFrame() & endFrame() functions.
	virtual void drawTextSimp(point3d* pos, float textWidth, float textHeight, float hang, float vang, float tilt, int col, const char* fmt, ...) = 0;
	//! Renders  a DICOM file unto the volumetric display
	virtual void drawDicom(voxie_dicom_t* vd, const char* gfilnam, point3d* gp, point3d* gr, point3d* gd, point3d* gf, int* animn, int* loaddone) = 0;

	// Keyboard

		//! Returns a key's input state (0 = no press, 1 = just pressed, 3 = held down) see vxInputTypes.h::Keys for scancodes
	virtual int		getKeyState(int scancode) = 0;
	//! Returns 1 if the scancode's key is held down (pressed). For scancodes see vxInputTypes.h::Keys
	virtual int		getKeyIsDown(int scancode) = 0;
	//! Returns 1 if the scancode's key is just pressed. Requires VoxieBox::setEnableLegacyKeyInput() set to false to work. For scancodes see vxInputTypes.h::Keys 
	virtual int		getKeyOnDown(int scancode) = 0;
	//! Returns 1 if the scancode's key is just released. Requires VoxieBox::setEnableLegacyKeyInput() set to false to work. For scancodes see vxInputTypes.h::Keys 
	virtual int		getKeyOnUp(int scancode) = 0;
	//! Return the time (in seconds) The scancode's key has been held down for requires VoxieBox::setEnableLegacyKeyInput() to be set to false to work
	virtual double  getKeyDownTime(int scancode) = 0;
	//! Returns buffered ASCII keyboard input. Requires setEnableLegacyKeyInput is set to true.
	virtual int		getKeyStream() = 0;
	//! Similar to getKeyStream() but returns only the scancode of the keystate of a streaming state. Returns 0 if no more keys are in the stream. Requires setEnableLegacyKeyInput is set to true.
	virtual int		getKeyStreamScanCode() = 0;
	//! returns internal voxie_keyboard_history_t struct which holds the keyboard input history.  
	virtual voxie_keyboard_history_t getKeyHistory() = 0;
	//! Reports the state of the keyboard on to the secondary (touch) screen. Used to help with debugging.
	virtual void	reportKeyboard(int posX, int posY) = 0;

	// Mouse

		//! Returns the internal mouse's X delta movement. Reads from internal (in) voxie_input_t.
	virtual int				getMouseXDelta() = 0;
	//! Returns the internal mouse's Y delta movement. Reads from the internal (in) voxie_input_t.
	virtual int				getMouseYDelta() = 0;
	//! Returns the internal mouse's Z delta movement. Reads from the internal (in) voxie_input_t.
	virtual int				getMouseZDelta() = 0;
	//! Returns the internal mouse's X,Y and Z delta movements as a point3d. Reads from the internal (in) voxie_input_t.
	virtual point3d			getMouseDelta() = 0;
	//! Returns the internal mouse's X,Y and Z position as a point3d 
	virtual point3d			getMousePosition() = 0;
	//! Overrides internal mouse position with a new point3d position
	virtual void			setMousePosition(point3d newPos) = 0;
	//! Set the Mouse's X and Y sensitivity. 0.001 is default. (0.0001 = low sensitive, 0.9 = extremely sensitive) Mouse movements work in integers the amount is scaled down
	virtual void			setMouseXYSensitivity(float newAmount) = 0;
	//! Set the Mouse's Z sensitivity. 0.0005 is default. (0.0001 = low sensitive, 0.9 = extremely sensitive) Mouse movements work in integers the amount is scaled down
	virtual void			setMouseZSensitivity(float newAmount) = 0;
	//! Set the time between mouse clicks to register a 'double click' (which triggers a true setting for getMouseDoubleClick())
	virtual void			setMouseDoubleClickThreshold(double timeThreshold) = 0;
	//! Returns the mouse double click threshold. (how quickly 2 mouse clicks signify a 'double click'). Presented in seconds.
	virtual double			getMouseDoubleClickThreshold() = 0;
	//! Returns the current mouse XY sensitivity
	virtual float			getMouseXYSensitivity() = 0;
	//! Returns the current mouse Z sensitivity
	virtual float			getMouseZSensitivity() = 0;
	//! Returns the internal mouse button state. Reads from the internal (in) voxie_input_t.
	virtual int				getMouseButtonState() = 0;
	//! Returns the internal mouse previous button state. Reads from the internal (in) voxie_input_t.
	virtual int				getMousePrevButtonState() = 0;
	//! Returns the state of mouse's buttonCode. 1 if currently pressed down and 0 if not being pressed. (buttonCodes are 0 for left, 1 for right, 2 for center buttons) 
	virtual int				getMouseButtonIsDown(int buttonCode) = 0;
	//! Returns the state of mouse's buttonCode. returns 1 if button has been pressed during this update frame otherwise 0 if not just pressed. (buttonCodes are 0 for left, 1 for right, 2 for centre buttons) 
	virtual int				getMouseButtonOnDown(int buttonCode) = 0;
	//! checks to see if desired MoustButton has been just released ('on up'). (buttonCodes are 0 for left, 1 for right, 2 for center buttons) 
	virtual int				getMouseButtonOnUp(int buttonCode) = 0;
	//! Returns the mount of time (in seconds) a mouse's button has been pressed. (buttonCodes are 0 for left, 1 for right, 2 for center buttons)  
	virtual double			getMouseButtonDownTime(int buttonCode) = 0;
	//! Returns true if particular buttonCode has been clicked twice within the double click threshold.
	virtual int				getMouseDoubleClick(int buttonCode) = 0;
	//! Reports mouse input state information onto secondary (touch) screen.
	virtual void			reportMouse(int posX, int posY, bool showCursor = true) = 0;
	//! returns the internal (in) voxie_inputs_t struct.
	virtual voxie_inputs_t	getMouseState() = 0;
	//! Override the internal voxie_input_t mouse state. 
	virtual void			setMouseState(voxie_inputs_t newMouse) = 0;
	//! Draws a cursor unto the volumetric display at the position specified.  
	virtual void			drawCursor(point3d* pos, int inputType, int inputID, int col) = 0;
	//! Change the orientation of the users intended position when using the mouse. At what side are they facing towards the volumetric display?
	virtual void			setMouseOrientation(int orientation) = 0;
	//! Returns the orientation set for the mouse
	virtual int				getMouseOrientation() = 0;


	// Touch Controls

		//! Add custom touch keys. (enable touch keyboard under 'Misc' menu tab) 
	virtual void			AddTouchKeyboardLayout(const touchkey_t* touchkey, int  sizeOfArray) = 0;
	//! Read touch inputs - This function should be called in a while loop until 0 is returned
	virtual int				touchManualRead(int* touchIndex, int* xVal, int* yVal, int* packetState) = 0;


	//! Enables the touch screen to be used as a input device. Set this to true if you want to use the touch screen in your own VX applications 
	virtual void			setEnableTouchInput(bool option) = 0;
	//! Returns the touch movement delta of the X position by index register. Use index register -1 for global delta 
	virtual int				getTouchDeltaX(int index) = 0;
	//! Returns the touch movement delta of the Y position by index register. Use index register -1 for global delta 
	virtual int				getTouchDeltaY(int index) = 0;
	//! Returns the touch X position by index register. Run using a for loop with TOUCH_MAX_INPUT to check all inputs 
	virtual int				getTouchPosX(int index) = 0;
	//! Returns the touch Y position by index register. Run using a for loop with TOUCH_MAX_INPUT to check all inputs 
	virtual int				getTouchPosY(int index) = 0;
	//! Returns the touch state requires the touch point index number. (0 = no touch, 1 = touch is down, 2 = is held, 3 = just pressed, 4 = on up)
	virtual int				getTouchState(int index) = 0;
	//! Returns in radians the touch rotation delta when a pinch is active range is usually 0.2 and 0.01
	virtual float			getTouchRotationDelta() = 0;
	//! Returns the touch rotation delta when a pinch is active range is usually 0.2 and 0.01
	virtual float			getTouchDistanceDelta() = 0;
	//! Returns a positive number if a touch event happens within the collision box (0 = no touch, 1 = touch is down, 2 = is held, 3 = just pressed, 4 = on up)
	virtual int				getTouchPressState(point2d TLpos, point2d BRpos, bool drawCollision = false) = 0;
	//! Returns a positive number if a touch event happens within the collision circle (0 = no touch, 1 = touch is down, 2 = is held, 3 = just pressed, 4 = on up)
	virtual int				getTouchPressStateCir(point2d CirPos, float radius, bool drawCollision = false) = 0;
	//! Returns a the internal touchInput_t pointer  
	virtual touchInput_t*	getTouchInputStruct() = 0;
	//! Updates the internal touchInput_t struct
	virtual void			setTouchInputStruct(touchInput_t* newTouchInput) = 0;
	//! Sets the touch sensitivity value 1 is default. the higher the value the more sensitive.  
	virtual void			setTouchSensitivity(float newSensitivityValue) = 0;
	//! Returns the touch sensitivity setting/
	virtual float			getTouchSensitivity() = 0;
	//! Disables / enables the touches focus pinch. 
	virtual void			enableTouchFocusPinch(bool choice) = 0;
	//! Returns the touch point index if a touch event happens within the collision box 
	virtual int				getTouchPressIndex(point2d TLpos, point2d BRpos, bool drawCollision = false) = 0;
	//! Returns the touch point index if a touch event happens within the collision circle
	virtual int				getTouchPressIndexCir(point2d CirPos, float radius, bool drawCollision = false) = 0;
	//! Returns the number of current touch presses / points 
	virtual int				getTouchCurrentNoPress() = 0;
	//! Reports the state of the touch input on to the secondary (touch) screen.
	virtual void			reportTouch(int posX, int posY) = 0;

	// Game Controllers



		//!	Returns theg ame controller's input state as voxie_xbox_t structure. Use the controllerID to determine which one (0 = 1st, 1 = 2nd etc...)
	virtual voxie_xbox_t getJoyStateRaw(int controllerID) = 0;
	// Updates the game controller's input state. Used to manage game controller inputs when setEnableLegacyJoyInput() is set to true
	virtual int			updateJoyStateRaw(int controllerID, voxie_xbox_t* vx) = 0;
	//! returns the game controllers button state. See vxInputTypes::JoyButtonCodes for details. 
	virtual int			getJoyButtonState(int controllerID) = 0;
	//! Activates vibration motors in XBox controller. XInput controllers only. To stop Be sure to call again with leftMotorSpeed=0.f, rightMotorSpeed=0.f!
	virtual void		setJoyVibrate(int controllerID, float leftMotorSpeed, float rightMotorSpeed) = 0;
	//! Returns 1 if controller's button is pressed down. See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes  
	virtual int			getJoyButtonIsDown(int controllerID, int joyButtonCode) = 0;
	//! Returns 1 if controller's button is just released.  See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes.
	virtual int			getJoyButtonOnUp(int controllerID, int joyButtonCode) = 0;
	//! Returns 1 if controller's button is just pressed. See vxInputTypes::JoyButtonCodes for reference on Joy Button Codes.
	virtual int			getJoyButtonOnDown(int controllerID, int joyButtonCode) = 0;
	//! Returns a value in seconds of how long a controller's button has been held down for. Requires setEnableLegacyJoyInput() to be set to false to work.  
	virtual double		getJoyButtonDownTime(int controllerID, int joyButtonCode) = 0;
	//! Returns a single analog axis (control stick) from a game controller. Value as an float range between -1 and 1, 0 is centered.
	virtual float		getJoyAnalogAxisValue(int controllerID, int axis) = 0;
	//! Returns the analog axis (control stick) from a game controller. Value as an Point2d range between -1 and 1, 0 is centered.
	virtual point2d		getJoyAnalogAxisValueP2D(int controllerID, int stick) = 0;
	//! Returns a controllers analog trigger value. 0 = 0% pressed ... 1 = 100% pressed. Presented as a float 
	virtual float		getJoyTriggerValue(int controllerID, int joyTriggerCode) = 0;
	//! Returns the number of USB game controllers detected by the system.
	virtual int			getJoyNum() = 0;
	//! Reports the state of the game controllers on to the secondary (touch) screen.
	virtual void		reportJoy(int posX, int posY) = 0;
	//! Sets the internal dead zone value for game controllers. (dead zone is the area where not input is detected). Value between 0 and 1. 0 = no deadzone. 1 = all deadzone. default is 0.3;
	virtual void		setJoyDeadZone(double deadZoneValue) = 0;
	//! Returns the internal gamepad's dead zone value
	virtual double		getJoyDeadZone() = 0;
	//! Change the orientation of the users intended position when using a game controller. At what side are they facing towards the volumetric display.
	virtual void		setJoyOrientation(int controllerID, int orientation) = 0;
	//! Returns the orientation set for a specific game controller
	virtual int			getJoyOrientation(int controllerID) = 0;


	// Space Mouse

		//!	Returns the 3DConnexion Space Navigator controller state as voxie_nav_t structure. Use the SpaceNavID to determine which one (0 = 1st, 1 = 2nd etc...)
	virtual voxie_nav_t	getNavStateRaw(int spaceNavID) = 0;
	//!	Copies 3DConnexion Space Navigator controller state to voxie_nav_t structure. Used to updated SpaceNavs if setEnableLegacyNavInput() is set to true.
	virtual int			updateNavStateRaw(int spaceNavID, voxie_nav_t* nav) = 0;
	//! Returns the number of SpaceNavs detected by the system. Note : SpaceNavs are only detected after first movement.
	virtual int		    getNavNum() = 0;
	//! Reports all SpaceNav input state onto the secondary (touch screen)
	virtual void		reportNav(int posX, int posY, bool showCursor = true) = 0;
	// Returns the button values (presented as binary of each button) for the Space Navigator. 0 = none, 1 = left, 2 = right, 3 = both.
	virtual int			getNavButtonState(int spaceNavID) = 0;
	// Returns the previous button values. Used for a legacy way to create your own button functions. Useful if setEnableLegacyNavInput() is set to true. 
	virtual int			getNavPrevButtonState(int spaceNavID) = 0;
	//! Returns the Space Nav's angle input delta as a point3d. Range between -1 and 1. 0 being no change. 
	virtual point3d		getNavAngleDelta(int spaceNavID) = 0;
	//! Returns the Space Nav's direction input delta as a point3d. Range between -1 and 1. 0 being no change. 
	virtual point3d		getNavDirectionDelta(int spaceNavID) = 0;
	//! Returns a point3d of the Nav's tracked position. Used primary when the Space Nav is being used as a cursor.
	virtual point3d		getNavPosition(int spaceNavID) = 0;
	//! Returns a single axis of a Space Nav's direction delta (range is -1 to 1, 0 is centered). Axis 0 = x, 1 = y, 2 = z
	virtual float		getNavDirectionDeltaAxis(int spaceNavID, int axis) = 0;
	//! Returns a single axis of a spaceNav's angle delta (range is -1 to 1, 0 is centered). Axis 0 = x, 1 = y, 2 = z
	virtual float		getNavAngleDeltaAxis(int spaceNavID, int axis) = 0;
	//! Returns 1 if particular Space Nav's button is being pressed. Button codes are based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button. 
	virtual int			getNavButtonIsDown(int spaceNavID, int buttonCode) = 0;
	//! Returns 1 if particular Space Nav's button has been just released from being held ('on up').
	virtual int			getNavButtonOnUp(int spaceNavID, int buttonCode) = 0;
	//! Returns 1 if particular Space Nav's button has just been pressed. Button codes are based on vxInputTypes.h::NavButton(). 0 = left button, 1 = right button. 
	virtual int			getNavButtonOnDown(int spaceNavID, int buttonCode) = 0;
	//! Returns in seconds how long a Space Nav's button has been held down, otherwise returns a 0. 
	virtual double		getNavButtonDownTime(int spaceNavID, int buttonCode) = 0;
	//! Set the time between Space Nav clicks to register a 'double click' (which triggers a true setting for getNavDoubleClick())
	virtual void		setNavDoubleClickThreshold(double timeThreshold) = 0;
	//! Returns the internal Space Nav double click threshold value. (Used to determine if a 'double click' has been detected)
	virtual double		getNavDoubleClickThreshold() = 0;
	//! Returns 1 if particular Space Nav's buttonCode has been clicked twice within the double click threshold.
	virtual int			getNavDoubleClick(int spaceNavID, int buttonCode) = 0;
	//! Set the internal deadZone for Space Navigators
	virtual void		setNavDeadZone(double deadZoneValue) = 0;
	//! Set the Space Nav's sensitivity. Used to manage the Space Nav being used as a cursor. 0.0025 is default. Higher values increase sensitivity  
	virtual void		setNavSensitivity(int spaceNavID, double newAmount) = 0;
	//! @return returns the current Space Nav internal sensitivity setting
	virtual double		getNavSensitivity(int spaceNavID) = 0;
	//! returns the internal dead one value for the Space Navigator
	virtual double		getNavDeadZone() = 0;
	//! set the orientation of the user's intended position when using a Space Nav
	virtual void		setNavOrientation(int spaceNavID, int orientation) = 0;
	//! returns the orientation set for a specific Space Nav
	virtual int			getNavOrientation(int spaceNavID) = 0;


	// Menus

		//! Function used to reset the voxie menu and set a new custom menu update function (the menu which is on the secondary touch screen)
	virtual void menuReset(int(*menu_update)(int id, char* st, double val, int how, void* userdata), void* userdata, char* bgImageFileName) = 0;
	//! Add a custom menu tab on secondary (touch) screen  menu (NOTE: there's only space for 2 more tabs on the 7" 1024x600 LCD screen)
	virtual void menuAddTab(const char* st, int x, int y, int xs, int ys) = 0;
	//! Add an item to a menu tab
	virtual void menuAddItem(const char* st, int x, int y, int xSize, int ySize, int id, int type, int state, int col, double startingVal, double minVal, double maxVal1, double minStepVal, double majStepVal) = 0;
	//! Once the menu is created, use this function to update a string, button status, or slider value
	virtual void updateMenu(int id, const char* st, int state, double v) = 0;


	// Sound 

	//! Plays a sound (can be WAV, FLAC, MP3, M4A)
	virtual int  playSound(const char* fileName, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed) = 0;
	//! Updates or adjusts a currently playing sound. Used for muting a current sound or adjusting settings.  
	virtual void updateSound(int handleID, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed) = 0;
	//! Updates or  currently playing sound. to a new position. Seek types are SEEK_SET (0), SEEK_CUR (1) & SEEK_END (2) - values based on fseek()  
	virtual void updateSoundPosition(int handleID, double second, int seekType = SEEK_SET) = 0;
	//! Define a custom audio callback function to play PCM raw audio data
	virtual void  setAudioPlayCallBack(void(*userplayfunc)(int* sampleBuffer, int sampleRate)) = 0;
	//! Define a custom audio callback function to record PCM audio data - see VXBeeper demo (under modules) for a demo on how to use this function
	virtual void  setAudioRecordCallBack(void(*userrecfunc)(int* sampleBuffer, int sampleRate)) = 0;


	// Misc

	//! Extracts and loads a .zip file into memory
	virtual void		mountZip(const char* fileName) = 0;
	//! Causes a screen capture of the volumetric buffer to occur on the next frame. Captured as a PLY file
	virtual void		captureVolumeAsPly() = 0;
	//! Causes a screen capture of the volumetric buffer to occur on the next frame. Captured as a PNG file
	virtual void		captureVolumeAsPng() = 0;
	//! Allows capture and recording of volumetric screen.  a screen capture of the volumetric buffer to occur on the next frame. Captures
	virtual void		captureVolume(const char* filnam, int volcap_mode, int target_vps) = 0;
	//! returns a time stamp of the compile date of VxCpp.dll expressed as an __int64. (format: YYYYMMDDHHmmss)
	virtual __int64		getVxCppVersion() = 0;
	//! Returns the handle HINSTANCE of the VoxieBox.DLL which might be need access by other DLLs (such as DemView_API)
	virtual HINSTANCE	getVoxieBoxDLLHandle() = 0;


	// Utilities

	//! Returns a scrolling color as an RGB hexadecimal value 
	virtual int		scrollCol(int offSet = 0) = 0;
	//! Set the speed of the internal color scroller default is 0.1. Value is how long in seconds before a color change.
	virtual void	setColScrollSpeed(double speed) = 0;
	//! Clips a point to ensures it is within the volumes display's bounds. 
	virtual int		clipInsideVolume(point3d* pos, float radius = 0) = 0;
	//! Compare two point3d with a degree of accuracy. returns true if the two points are the same within the accuracy amount specified
	virtual int		pointSame(point3d* a, point3d* b, point3d accuracy) = 0;
	//! Compare two point2d with a degree of accuracy. returns true if the two points are the same within the accuracy amount specified
	virtual int		pointSame(point2d* a, point2d* b, point2d accuracy) = 0;
	//! Divides (dims) a RGB hexadecimal color value by a division number (use this as when using drawModel() the color values are a 4th of the range)
	virtual int		colorHexDivide(int color, float divideAmount) = 0;
	//! Tweens a color to the destination color. good for fade outs or tweens. 
	virtual int		tweenCol(int color, int speed, int destcolor) = 0;
	// Brighten (saturate) a color by adding only RGB values if they are needed.
	virtual int		brightenCol(int color, int amount) = 0;
	//! Returns a random color based on the RANDOM_COLOR defined in vxDataTypes.h
	virtual int		randomCol() = 0;
	//! Returns a random position within the VX display.
	virtual point3d	randomPos() = 0;


	//! Sphere collision check. Check if two spheres are touching. Returns 1 if collision is found otherwise returns 0 
	virtual int		sphereCollideChk(point3d* sphereAPos, double sphereARadius, point3d* sphereBPos, double sphereBRadius, bool showCollisionBox = false) = 0;
	//! Within box collision check 2D. Checks if a X Y position is within a collision box. two boxes are touching. Returns 1 if collision is found otherwise returns 0
	virtual int		boxInsideCollideChk2D(point2d* TLpos, point2d* BRpos, point2d* collisionPos, bool showCollisionBox = false) = 0;
	//! Box collision check. Check if two boxes are touching. Returns 1 if collision is found otherwise returns 0
	virtual int		boxCollideChk(point3d* LUTpos1, point3d* RDBpos1, point3d* LUTpos2, point3d* RDBpos2, bool showCollisionBox = false) = 0;
	//! Within box collision check. Check if a position is inside a collision box.  boxTLU = Top, Left, Up, boxBRD = bottom, right, down.
	virtual int		boxInsideCollideChk(point3d* LUTpos, point3d* RDBpos, point3d collisionPos, bool showCollisionBox = false) = 0;
	//! Within circle collision check. Check if a position is inside a collision circle. 
	virtual int		ciricle2DChk(point2d* circlePos, float radius, point2d* collisionPos, int showCollisionBox = 0) = 0;

	// TODO: Cube to Cube check - checks two cubes (with rotation vectors) are colliding 


	//! Updates/"moves" a point from the current position towards the destination point. Returns 1 if currentPos collides with destinnationPos otherwise returns a 0.  
	virtual int		moveToPos(point3d* currentPos, point3d destinationPos, float speed, float accuracy) = 0;
	//! Rotate two point3d vectors a & b around their common plane, by angle expressed in radians
	virtual void	rotVex(float angInRadians, point3d* a, point3d* b) = 0;
	//! Rotate two point3d vectors a & b around their common plane, by angle expressed in degrees 
	virtual void	rotVexD(float angInDegrees, point3d* a, point3d* b) = 0;
	//! Rotate two point2d vectors a & b around their common plane, by angle expressed in radians
	virtual void	rotVex(float angInRadians, point2d* a, point2d* b) = 0;
	//! Rotate two point2d vectors a & b around their common plane, by angle expressed in degrees 
	virtual void	rotVexD(float angInDegrees, point2d* a, point2d* b) = 0;
	//! Helper function for VoxieBox::DrawMeshExt copies indices.xyz values to indices.uvc values 
	virtual void	indicesCpyXYZ_2_UVC(inds_t * indiceArray, int indiceCount) = 0;





	//Ken's ZIP functions:

		// Ken's file handlers -- these are internal functions used for decoding images and managing zipped files. 
		/* "These functions are used internally by voxiebox.dll and shared externally mainly to save exe space.
			They are used by voxiedemo for various purposes. These functions can be used to load images from the
			following image formats :
			JPG, PNG, GIF, PCX, TGA, BMP, DDS, CEL
			And they can be used as generic file loading functions for the following archive formats :
			ZIP, TAR, GZ, GRP, and directories." - Ken S.
		*/

	//! Internal zip file management function - global function (these not multi-thread safe!)
	virtual int					_kzaddstack(const char* fileName) = 0;
	//! Internal zip file management function -  global function (these not multi-thread safe!)
	virtual void				_kzuninit() = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual kzfind_t*			_kzfindfilestart(const char* st) = 0;   //pass wild-card string
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access @return 1:found, 0:~found, NOTE:keep calling until ret 0 else mem leak
	virtual int					_kzfindfile(kzfind_t* find, kzfileinfo_t* fileinfo) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual kzfile_t *			_kzsetfil(FILE* fileName) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual kzfile_t *			_kzopen(const char*) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual kzfile_t*			_kzopen_ext(const char*, const char*, const char*) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual unsigned int		_kzread(kzfile_t* kzfile, void* buffer, unsigned int leng) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual unsigned int		_kzfilelength(kzfile_t* kzfile) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual unsigned int		_kztell(kzfile_t* kzfile) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual int					_kzseek(kzfile_t* kzfile, int offset, int whence) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual int					_kzgetc(kzfile_t* kzfile) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual int					_kzeof(kzfile_t* kzfile) = 0;
	//! Internal zip file management function. All 'kz...' functions are to do with Ken's zip file access
	virtual void				_kzclose(kzfile_t* kzfile) = 0;
	//! Internal image decoder  function. All 'kp...' functions are to do with Ken's picture library (Low-level PNG/JPG decoding functions)
	virtual int					_kpgetdim(const char* buffer, int nby, int* xsiz, int* ysiz) = 0;
	//! Internal image decoder  function. All 'kp...' functions are to do with Ken's picture library (Low-level PNG/JPG decoding functions)
	virtual int					_kprender(const char* buffer, int nby, INT_PTR fptr, int bpl, int xsiz, int ysiz, int xoff, int yoff) = 0;
	//! Internal image decoder  function. All 'kp...' functions are to do with Ken's picture library (High-level (easy) picture loading function)
	virtual int					_kpzload(const char* fileName, INT_PTR* fptr, INT_PTR* bpl, INT_PTR* xsiz, INT_PTR* ysiz) = 0;




};


#include "vxInterfaces.h"

/** \class VoxieBox wraps the Voxiebox.dll into a VoxieBox class.
*  The VoxieBox class is all the voxiebox.dll functions, helper functions and internal variables to manage a Voxon volumetric application.
*  The VoxieBox class can be interfaced with the IVoxieBox and used a pointer reference This allows the VoxieBox class to be used
*  with OO principals and C++ development easier for Voxon applications.
*/
class VoxieBox : public IVoxieBox {
	bool success = false;			//!< boolean to check if the VoxieBox constructor is a success (only successful on first instance to use singleton approach)
public:

	VoxieBox();
	~VoxieBox();

	// Core Functions 
	int breath();
	int breath(voxie_inputs_t* input);
	void startFrame();
	void endFrame();
	void endBreath();
	void quitLoop();
	void shutdown();
	int  init();
	voxie_wind_t* getVoxieWindow();
	voxie_frame_t* getVoxieFrame();



	// Settings
	void setBorder(bool option, int color = 0xffffff);
	void setEnableExitOnEsc(bool option);
	void setEnableNavClipping(bool option);
	void setEnableMouseClipping(bool option);
	void setCleanExitOnQuitLoop(bool option);
	void setEnableLegacyKeyInput(bool option);
	void setEnableLegacyJoyInput(bool option);
	void setEnableLegacyNavInput(bool option);
	void setEnableLegacyTouchInput(bool option);
	void setInvertZAxis(bool option);
	void setEnableJoyDeadZone(bool option);
	void setEnableNavDeadZone(bool option);


	// Time
	double getDeltaTime();
	double getTime();
	double getVPS();



	// Display
	float	getAspectX();
	float	getAspectY();
	float	getAspectZ();
	point3d getAspect();
	void	setAspectX(float newAspectX);
	void	setAspectY(float newAspectY);
	void	setAspectZ(float newAspectZ);
	void	setAspect(point3d newAspect);
	void	setDisplay2D();
	void	setDisplay3D();
	void	setView(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax);
	void 	setView(point3d LUT, point3d RDB);
	void	setMaskPlane(float x0, float y0, float z0, float nx, float ny, float nz);
	void    setGlobalShader(float horizontalAngle, float verticalAngle, float amplitude);
	void	flushGfx();
	void	freeGfx(const char* fileName);
	void	setProject(int dispNum, int dir, float x, float y, int z, float* xo, float* yo);
	void	setLeds(int dispNum, int r, int g, int b);
	void	reportVoxieWind(int posX, int posY);
	void    reportVoxieFrame(int posX, int posY);
	void	reportVoxieWind(int posX, int posY, voxie_wind_t* VW);
	void	reportVoxieFrame(int posX, int posY, voxie_frame_t* VF);

	// Debug 
	void	debugText(int x, int y, int fcol, int bcol, const char* fmt, ...);
	double	debugBar(int posX, int posY, double currentVal, double maxVal, double minVal, const char* text, int type);
	double	showVPS(int posX, int posY);
		

	// Drawing Graphics to Secondary Screen
	void debugDrawPix(int x, int y, int col);
	void debugDrawHLine(int xStartPos, int xEndPos, int y, int col);
	void debugDrawLine(float xStartPos, float yStartPos, float xEndPos, float yEndPos, int col);
	void debugDrawCircle(int xCenterPos, int yCenterPos, int radius, int col);
	void debugDrawBoxFill(int xStartPos, int yStartPos, int xEndPos, int yEndPos, int col);
	void debugDrawCircFill(int xCenterPos, int yCenterPos, int radius, int col);
	void debugDrawTile(tiletype* source, int xpos, int ypos);
	bool debugCustomFontSet(const char* fileName, int index = -1);
	void debugCustomFont(int xpos, int ypos, int customFontIndex, const char* fmt, ...);

	//  Drawing Graphics to Volumetric Display
	void drawVox(point3d pos, int col);
	void drawVox(float x, float y, float z, int col);
	void drawBox(point3d posLeftUpTop, point3d posRightDownBottom, int fillmode, int col);
	void drawBox(float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
	void drawCube(point3d* p, point3d* r, point3d* d, point3d* f, int fillmode, int col);
	void drawSphere(point3d pos, float radius, int fillmode, int col);
	void drawSphere(float x, float y, float z, float radius, int fillmode, int col);
	void drawLine(point3d startPos, point3d endPos, int col);
	void drawLine(float xStartPos, float yStartPos, float zStartPos, float xEndPos, float yEndPos, float zEndPos, int col);
	void drawPoly(pol_t*, int, int);
	void drawMesh(const char* fileNam, poltex_t* verticeList, int verticeNum, int* meshList, int meshNum, int flags, int col);
	void drawMeshExt(const char* fileNam, point3d* VerticeXYZArray, int verticeXYZNum, uvcol_t* verticcUVCArray, int verticeUVCNum,  inds_t* indiceArray, int indiceNum, int flags, int col);


	void drawCone(float xStartPos, float yStartPos, float zStartPos, float startRadius, float xEndPos, float yEndPos, float zEndPos, float endRadius, int fillmode, int col);
	void drawCone(point3d startPos, float startRadius, point3d endPos, float endRadius, int fillmode, int col);
	int  drawModel(const char* filename, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int col);
	int  drawModelExt(const char* filename, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int col, float forcescale, float fdrawratio, int flags);
	int  drawModelGetExtents(const char* charbuff, extents_t* extentsPtr, int flags);
	void drawQuad(const char* filename, point3d* pos, float width, float height, float hang, float vang, float tilt, int col = 0x404040, float uValue = 1, float vValue = 1);
	float drawHeightMap(char* fileName, point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector, int colorKey, int reserved, int flags);
	void drawText(point3d* pos, point3d* rVector, point3d* dVector, int col, const char* fmt, ...);
	void drawTextExt(point3d* pos, point3d* rVector, point3d* dVector, float size, int col, const char* fmt, ...);

	void drawTextSimp(point3d* pos, float textWidth, float textHeight, float hang, float vang, float tilt, int col, const char* fmt, ...);
	void drawDicom(voxie_dicom_t* vd, const char* gfilnam, point3d* gp, point3d* gr, point3d* gd, point3d* gf, int* animn, int* loaddone);


	// Keyboard
	int							getKeyState(int scancode);
	int							getKeyIsDown(int scancode);
	int							getKeyOnDown(int scancode);
	double						getKeyDownTime(int scancode);
	int							getKeyOnUp(int scancode);
	int							getKeyStream();
	int							getKeyStreamScanCode();
	void						reportKeyboard(int posX, int posY);
	voxie_keyboard_history_t	getKeyHistory();


	// Mouse
	int				getMouseXDelta();
	int				getMouseYDelta();
	int				getMouseZDelta();
	point3d			getMouseDelta();
	point3d			getMousePosition();
	void			setMousePosition(point3d newPos);
	void			setMouseXYSensitivity(float newAmount);
	void			setMouseZSensitivity(float newAmount);
	void			setMouseDoubleClickThreshold(double timeThreshold);
	double			getMouseDoubleClickThreshold();
	float			getMouseZSensitivity();
	float			getMouseXYSensitivity();
	int				getMouseButtonState();
	int				getMousePrevButtonState();
	int			    getMouseButtonIsDown(int buttonCode);
	int				getMouseButtonOnDown(int buttonCode);
	int				getMouseButtonOnUp(int buttonCode);
	double			getMouseButtonDownTime(int buttonCode);
	int				getMouseDoubleClick(int buttonCode);
	void			reportMouse(int posX, int posY, bool showCursor = true);
	voxie_inputs_t	getMouseState();
	void			setMouseState(voxie_inputs_t newMouse);
	void			drawCursor(point3d* pos, int inputType, int inputID, int col);
	void			setMouseOrientation(int orientation);
	int				getMouseOrientation();


	// Custom Touch Controls
	void	AddTouchKeyboardLayout(const touchkey_t* touchkey = default_touchkey, int  sizeOfArray = sizeof(default_touchkey) / sizeof(touchkey_t));
	int		touchManualRead(int* touchIndex, int* xVal, int* yVal, int* packetState);

	void			setEnableTouchInput(bool option);
	void			setDrawTouchInput(bool option);
	int				getTouchDeltaX(int index = -1);
	int				getTouchDeltaY(int index = -1);
	int 			getTouchPosX(int index);
	int 			getTouchPosY(int index);
	int				getTouchState(int index);
	float			getTouchRotationDelta();
	float			getTouchDistanceDelta();
	int				getTouchPressState(point2d TLpos, point2d BRpos, bool drawCollision = false);
	int				getTouchPressStateCir(point2d CirPos, float radius, bool drawCollision = false);
	int				getTouchPressIndex(point2d TLpos, point2d BRpos, bool drawCollision = false);
	int				getTouchPressIndexCir(point2d CirPos, float radius, bool drawCollision = false);



	touchInput_t* getTouchInputStruct();
	void			setTouchInputStruct(touchInput_t* newTouchInput);
	void			setTouchSensitivity(float newSensitivityValue);
	float			getTouchSensitivity();
	void			enableTouchFocusPinch(bool choice);
	int				getTouchCurrentNoPress();
	void			reportTouch(int posX, int posY);


	// Game Controllers
	voxie_xbox_t getJoyStateRaw(int controllerID);
	int			updateJoyStateRaw(int controllerID, voxie_xbox_t* vx);
	void		setJoyInputToXInput();
	void		setJoyInputToDirectInput();
	int			getJoyButtonState(int controllerID);
	int			getJoyButtonIsDown(int controllerID, int joyButtonCode);
	int			getJoyButtonOnDown(int controllerID, int joyButtonCode);
	int			getJoyButtonOnUp(int controllerID, int joyButtonCode);
	float		getJoyAnalogAxisValue(int controllerID, int axis);
	point2d		getJoyAnalogAxisValueP2D(int controllerID, int stick);
	void		setJoyVibrate(int controllerID, float leftMotorSpeed, float rightMotorSpeed);
	int			getJoyNum();
	double		getJoyButtonDownTime(int controllerID, int joyButtonCode);
	void		reportJoy(int posX, int posY);
	void		setJoyDeadZone(double deadZoneValue);
	double		getJoyDeadZone();
	float		getJoyTriggerValue(int controllerID, int joyTriggerCode);
	void		setJoyOrientation(int controllerID, int orientation);
	int			getJoyOrientation(int controllerID);


	// Space Mouse
	voxie_nav_t	getNavStateRaw(int spaceNavID);
	int			updateNavStateRaw(int spaceNavID, voxie_nav_t* nav);
	int		    getNavNum();
	void		reportNav(int posX, int posY, bool showCursor = true);
	int			getNavButtonState(int spaceNavID);
	int			getNavPrevButtonState(int spaceNavID);
	point3d		getNavPosition(int spaceNavID);
	point3d		getNavAngleDelta(int spaceNavID);
	point3d		getNavDirectionDelta(int spaceNavID);
	float		getNavDirectionDeltaAxis(int spaceNavID, int axis);
	float		getNavAngleDeltaAxis(int spaceNavID, int axis);
	int			getNavButtonIsDown(int spaceNavID, int buttonCode);
	int			getNavButtonOnUp(int spaceNavID, int buttonCode);
	int			getNavButtonOnDown(int spaceNavID, int buttonCode);
	double		getNavButtonDownTime(int spaceNavID, int buttonCode);
	int			getNavDoubleClick(int spaceNavID, int buttonCode);
	void		setNavDoubleClickThreshold(double timeThreshold);
	double		getNavDoubleClickThreshold();
	void		setNavDeadZone(double deadZoneValue);
	void		setNavSensitivity(int spaceNavID, double newAmount);
	double		getNavSensitivity(int spaceNavID);
	double		getNavDeadZone();
	void		setNavOrientation(int spaceNavID, int orientation);
	int			getNavOrientation(int spaceNavID);


	// Menus
	void menuReset(int(*menu_update)(int id, char* st, double val, int how, void* userdata), void* userdata, char* bgImageFileName);
	void menuAddTab(const char* st, int x, int y, int xs, int ys);
	void menuAddItem(const char* st, int x, int y, int xSize, int ySize, int id, int type, int state, int col, double startingVal, double minVal, double maxVal1, double minStepVal, double majStepVal);
	void updateMenu(int id, const char* st, int state, double v);


	// Sound 
	int  playSound(const char* fileName, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed);
	void updateSound(int handleID, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed);
	void updateSoundPosition(int handleID, double second, int seekType = SEEK_SET);
	void setAudioPlayCallBack(void(*userplayfunc)(int* sampleBuffer, int sampleRate));
	void setAudioRecordCallBack(void(*userrecfunc)(int* sampleBuffer, int sampleRate));


	// Misc
	void		mountZip(const char* fileName);
	void		captureVolumeAsPly();
	void		captureVolumeAsPng();
	void		captureVolume(const char* fileName, int volCapMode, int targetVPS);
	void		captureVolumeStop();
	__int64		getVxCppVersion();
	int			scrollCol(int offset = 0);
	void		setColScrollSpeed(double speed);
	HINSTANCE	getVoxieBoxDLLHandle();

	// Utilities 
	int			clipInsideVolume(point3d* pos, float radius = 0);
	int			pointSame(point3d* a, point3d* b, point3d accuracy);
	int			pointSame(point2d* a, point2d* b, point2d accuracy);
	int			colorHexDivide(int color, float divideAmount);
	int			tweenCol(int color, int speed, int destcolor);
	int			brightenCol(int color, int amount);
	int			randomCol();
	point3d		randomPos();
	int			sphereCollideChk(point3d* sphereAPos, double sphereARadius, point3d* sphereBPos, double sphereBRadius, bool showCollisionBox = false);
	int			boxInsideCollideChk(point3d* LUTpos, point3d* RDBpos, point3d collisionPos, bool showCollisionBox = false);
	int			boxInsideCollideChk2D(point2d* TLpos, point2d* BRpos, point2d* collisionPos, bool showCollisionBox = false);
	int			ciricle2DChk(point2d* circlePos, float radius, point2d* collisionPos, int showCollisionBox = 0);
	int			boxCollideChk(point3d* LUTpos1, point3d* RDBpos1, point3d* LUTpos2, point3d* RDBpos2, bool showCollisionBox = false);
	int			moveToPos(point3d* currentPos, point3d destinationPos, float speed, float accuracy);
	void		rotVex(float angInRaidans, point3d* a, point3d* b);
	void		rotVexD(float angInDegrees, point3d* a, point3d* b);
	void		rotVex(float angInRaidans, point2d* a, point2d* b);
	void		rotVexD(float angInDegrees, point2d* a, point2d* b);
	void		indicesCpyXYZ_2_UVC(inds_t * indiceArray, int indiceCount);


	// Ken's file handlers -- these are internal functions used for decoding images and managing zipped files. 
	/* "These functions are used internally by voxiebox.dll and shared externally mainly to save exe space.
		They are used by voxiedemo for various purposes. These functions can be used to load images from the
		following image formats :
		JPG, PNG, GIF, PCX, TGA, BMP, DDS, CEL
		And they can be used as generic file loading functions for the following archive formats :
		ZIP, TAR, GZ, GRP, and directories." - Ken S.
	*/

	//Internal functions:

	//Global functions (these 2 not multi-thread safe!):
	//Global functions (these 2 not multi-thread safe!):
	int					_kzaddstack(const char* fileName);
	void				_kzuninit();

	kzfind_t* _kzfindfilestart(const char* st);   //pass wildcard string
	int					_kzfindfile(kzfind_t* find, kzfileinfo_t* fileinfo); //returns 1:found, 0:~found, NOTE:keep calling until ret 0 else mem leak ;P

	kzfile_t*			_kzsetfil(FILE* fileName);
	kzfile_t*			_kzopen(const char* st);
	kzfile_t*			 _kzopen_ext(const char*, const char*, const char*);
	unsigned int		_kzread(kzfile_t* kzfile, void* buffer, unsigned int leng);
	unsigned int		_kzfilelength(kzfile_t* kzfile);
	unsigned int		_kztell(kzfile_t* kzfile);
	int					_kzseek(kzfile_t* kzfile, int offset, int whence);
	int					_kzgetc(kzfile_t* kzfile);
	int					_kzeof(kzfile_t* kzfile);
	void				_kzclose(kzfile_t* kzfile);

	//Low-level PNG/JPG decoding functions:
	int					_kpgetdim(const char* buffer, int nby, int* xsiz, int* ysiz);
	int					_kprender(const char* buffer, int nby, INT_PTR fptr, int bpl, int xsiz, int ysiz, int xoff, int yoff);
	//High-level (easy) picture loading function:
	int				_kpzload(const char* fileName, INT_PTR* fptr, INT_PTR* bpl, INT_PTR* xsiz, INT_PTR* ysiz);




private:

	voxie_wind_t			vw;										//!< the internal oxie_wind_t "voxie window" struct @see vxDataType::voxie_wind_t				
	voxie_frame_t			vf;										//!< the internal oxie_wind_t "voxie frame" struct @see vxDataType::voxie_frame_t	
	bool					customFontInit[20] = { 0 };				//!< internal bool to determine if a customfont has been loaded into memory
	int						customFontLastIndex = 0;				//!< internal int to manage the last valid loaded customFont
	tiletype				customFont[20];							//!< the tileType that holds the internal custom 2D Font -- can store up to 20
	static HINSTANCE		hvoxie;									//!< the voxie handle instance
	voxie_inputs_t			in;										//!< the internal input state @see vxDataTypes::voxie_inputs_t
	touchInput_t			touch;									//!< the struct that manages all the advanced touch input. 
	point3d					mousePos = { 0,0,0 };					//!< the internal mouse position used for mouse clipping and displaying a mouse cursor. Use VoxieBox::getMousePosition() to return values
	point3d					navPos[4] = { 0 };						//!< the 4 SpaceNav position used for clipping and nav cursor

	double mouseXYSensitivity = 0.001;								//!< the internal setting for the mouse's X and Y sensitivity. Can be adjusted using VoxieBox::setMouseXYSensitivity()
	double mouseZSensitivity = 0.0005;								//!< the internal setting for the mouse's Z sensitivity. Can be adjusted using VoxieBox::setMouseZSensitivity()
	double mouseClickThreshold = 0.5;								//!< the internal setting for the mouse's double click threshold (how much time between clicks to register a 'double click'). Can be adjusted using VoxieBox::setMouseDoubleClickThreshold()
	double startLastPressedMouse[3] = { -100, -100, -100 };			//!< timers to work out the last pressed mouse value. Used internally.
	double durationLastPressedMouse[3] = { 0 };						//!< timers to determine the last pressed mouse duration. Used internally.

	double  navSensitivity[4] = { 0.0025, 0.0025, 0.0025, 0.0025 };	//!< the internal setting for the spaceNav's sensitivity Can be adjusted using VoxieBox::setNavSensitivity()
	int		onavbut[4] = { 0 };										//!< the old (previous volumes) button state for the spaceNav. Used internally.

	int		joyOri[4] = { 0,0,0,0 };								//!< the orientation settings for the USB game controllers can be adjusted using VoxieBox::setJoyOrientation() and VoxieBox::getJoyOrientation()
	int		navOri[4] = { 0,0,0,0 };								//!< the orientation settings for the Space Nav controllers can be adjusted using VoxieBox::setNavOrientation() and VoxieBox::getNavOrientation()
	int		mouseOri = 0;

	bool	invertZAxis = false;									//!< internal bool for inverting the Z axis. For internal use.


	int		navplays = 0;											//!< stores total number of Space Nav's detected 
	double	navClickThreshold = 0.5;								//!< the internal setting for the Space Nav's double click threshold (how much time between clicks to register a 'double click'). Can be adjusted using VoxieBox::setNavDoubleClickThreshold()
	void	updateNavInput();										//!< Internal function that updates all the Space Nav inputs

	int		display_volume;

	voxie_keyboard_history_t keyHistory = { 0 };					//!< Internal struct which holds all the keyboard's keystroke data. Can be accessed VoxieBox::Get
	void					 updateKeyboardHistory();				//!< Internal function that updates and logs all the keyboard keystrokes

	int		colScrollcolor = PALETTE_COLOR[rainbowCounter];			//!< Internal color scroll color
	double	colScrollSpeed = 0.1;									//!< Internal color scroll speed.
	double	rainbowTim = 0;											//!< Timer to manage the color scroller
	int		rainbowCounter = 0;										//!< Counter to manage the internal color scroller

	double	joyDeadZone = 0.3;										//!< Internal value for the USB Game Controller's dead zone.
	double	navDeadZone = 0.1;										//!< Internal value for the Space Nav's dead zone.
	int		vxnplays = 0;											//!< Internal value for the number of Gamepad's conncted vxnplays is a legacy name
	int		ovxbut[4];												//!< Internal value for the USB Game Controller's previous button state
	void	updateJoyInput();										//!< Internal function to update the USB Game Controllers inputs. Accessed only when VoxieBox::setEnableLegacyJoyInput() is set to true.
	int		joyDeadZoneCheck(float value);							//!< Internal function to check if USB Game Controller is within its deadzone. 

	voxie_nav_t				nav[4] = { 0 };							//!< Internal Space Nav structs array for each possible Space Nav
	voxie_xbox_t			vx[4] = { 0 };							//!< Internal USB Game Controller structs array for each possible controller
	voxie_nav_history_t		navHistory[4] = { 0 };					//!< Internal history struck to manage advanced Nav button states (like button duration) based on the struct set out in vxDataTypes.h 
	voxie_xbox_history_t	joyHistory[4] = { 0 };					//!< Internal history struck to manage advanced Joy button states (like button duration) based on the struct set out in vxDataTypes.h

	double time;													//!< Internal variable to manage time
	double oldTime;													//!< Internal variable to manage previous volume's time. Used to calculate deltatime
	double deltaTime;												//!< Internal delta time the time difference between volume updates.
	double averageTime;												//!< Internal average time value to work out the average update time.

	bool touchUpdate = false;									//!< internal variable to manage touch updates 
	bool touchIsDrawing = true;									//!< internal variable to manage touch drawing
	int currentActiveTouches = 0;								//!< internal variable to manage current actve touches

	bool drawBorder = false;								//!< Internal bool to determine whether to draw a border around the edge of the display. 
	int drawBorderCol = 0xffffff;							//!< Internal int value to store the border drawn color 
	bool enableEscQuit = true;									//!< Internal bool to determine whether to map the 'Esc' key to quit the VX program.
	bool enableMouseClip = false;								//!< Internal bool to determine whether to clip the internal mouse position. 
	bool enableNavClip = false;								//!< Internal bool to determine whether to clip the internal Space Nav position. 
	bool enableCleanExit = true;									//!< Internal bool to determine whether to clean up memory when qutting the breath() loop. 
	bool manualJoyManage = false;								//!< Internal bool to determine whether to use modern or legacy game controller input management.
	bool manualNavManage = false;								//!< Internal bool to determine whether to use modern or legacy Space Nav input management.
	bool manualKeyManage = false;								//!< Internal bool to determine whether to use modern or legacy keyboard input management.
	bool manualTouchManage = false;								//!< Internal bool to determine whether to use modern or legacy touch input management.

	bool enableJoyDeadZone = true;									//!< Internal bool to determine whether to enable game controller dead zones or not 
	bool enableNavDeadZone = true;									//!< Internal bool to determine whether to enable Space Nav dead zones or not 

	float	   normaliseAnaStickToFloat(short input);				//!< Internal function to convert the VoxieBox.dll's controller input to the scale of -1 to 1.
	float	   normaliseNavToFloat(int input);						//!< Internal function to convert the VoxieBox.dll's Space Nav input to the scale of -1 to 1.

	point2d		oriCorrection(int oriType, float xValue, float yValue);		//!< fixes an XY value to the prescribed orientation 
	void		updateMousePosition();										//!< Internal function that updates mouse input
	void		updateTouch();												//!< Internal function that updates inputTouch_t struct		
	void		touchPinchClear(touchInput_t* touchInputPtr);				//!< Internal function clearing out the touch pinch state
	void		touchClear(touchInput_t* touchInputPtr, int index);		//!< Internal function clearing out the touch finger state
	void		touchDraw();												//!< Internal function for drawing touch input presses onto the touch screen


#pragma region InternalFunctions
	int voxie_load(voxie_wind_t* vw);
	void voxie_uninit(int mode);
#pragma endregion

#pragma region DLL_Functions
	void(__cdecl* voxie_loadini_int)(voxie_wind_t* vw);
	void(__cdecl* voxie_getvw)(voxie_wind_t* vw);
	int(__cdecl* voxie_init)(voxie_wind_t* vw);
	void(__cdecl* voxie_uninit_int)(int);
	void(__cdecl* voxie_mountzip)(const char* fnam);
	void(__cdecl* voxie_free)(const char* fnam);
	__int64(__cdecl* voxie_getversion)(void);
	HWND(__cdecl* voxie_gethwnd)(void);
	int(__cdecl* voxie_breath)(voxie_inputs_t*);
	void(__cdecl* voxie_quitloop)(void);
	double(__cdecl* voxie_klock)(void);
	int(__cdecl* voxie_keystat)(int);
	int(__cdecl* voxie_keyread)(void);
	int(__cdecl* voxie_xbox_read)(int id, voxie_xbox_t* vx);
	void(__cdecl* voxie_xbox_write)(int id, float lmot, float rmot);
	int(__cdecl* voxie_nav_read)(int id, voxie_nav_t* nav);
	int(__cdecl* voxie_touch_read)(int* id, int* xVal, int* yVal, int* packetState);
	void(__cdecl* voxie_menu_reset)(int(*menu_update)(int id, char* st, double val, int how, void* userdata), void* userdata, char* bkfilnam);
	void(__cdecl* voxie_menu_addtab)(const char* st, int x, int y, int xs, int ys);
	void(__cdecl* voxie_menu_additem)(const char* st, int x, int y, int xs, int ys, int id, int type, int down, int col, double v, double v0, double v1, double vstp0, double vstp1);
	void(__cdecl* voxie_menu_updateitem)(int id, const char* st, int down, double v);
	void(__cdecl* voxie_touch_custom)(const touchkey_t* touchkey, int num);
	void(__cdecl* voxie_volcap)(const char* fileName, int volcap_mode, int targetVps);
	void(__cdecl* voxie_setview)(voxie_frame_t* vf, float x0, float y0, float z0, float x1, float y1, float z1);
	void(__cdecl* voxie_setmaskplane)(voxie_frame_t* vf, float x0, float y0, float z0, float nx, float ny, float nz);
	void(__cdecl* voxie_setnorm)(float nx, float ny, float nz);
	int(__cdecl* voxie_frame_start)(voxie_frame_t* vf);
	void(__cdecl* voxie_flush)(void);
	void(__cdecl* voxie_frame_end)(void);
	void(__cdecl* voxie_setleds)(int id, int r, int g, int b);
	void(__cdecl* voxie_project)(int, int, float, float, int, float*, float*);
	void(__cdecl* voxie_drawvox)(voxie_frame_t* vf, float fx, float fy, float fz, int col);
	void(__cdecl* voxie_drawbox)(voxie_frame_t* vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
	void(__cdecl* voxie_drawlin)(voxie_frame_t* vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
	void(__cdecl* voxie_drawpol)(voxie_frame_t* vf, pol_t* pt, int n, int col);
	void(__cdecl* voxie_drawmeshtex)(voxie_frame_t* vf, const char* fnam, poltex_t* vt, int vtn, int* mesh, int meshn, int flags, int col);
	void(__cdecl* voxie_drawmeshtex_ext)(voxie_frame_t* vf, const char* texnam,  point3d* xyz, int xyzn,  uvcol_t* uvc, int uvcn,  inds_t* inds, int ninds, int flags, int col);
	void(__cdecl* voxie_drawsph)(voxie_frame_t* vf, float fx, float fy, float fz, float rad, int issol, int col);
	void(__cdecl* voxie_drawcone)(voxie_frame_t* vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
	int(__cdecl* voxie_drawspr_getextents)(const char*, extents_t*, int);
	int(__cdecl* voxie_drawspr)(voxie_frame_t* vf, const char* fnam, point3d* p, point3d* r, point3d* d, point3d* f, int col);
	int(__cdecl* voxie_drawspr_ext)(voxie_frame_t* vf, const char* fnam, point3d* p, point3d* r, point3d* d, point3d* f, int col, float forcescale, float fdrawratio, int flags);
	void(__cdecl* voxie_printalph)(voxie_frame_t* vf, point3d* p, point3d* r, point3d* d, int col, const char* st);
	void(__cdecl* voxie_printalph_ext)(voxie_frame_t* vf, point3d* p, point3d* r, point3d* d, float rad, int col, const char* st);
	void(__cdecl* voxie_drawcube)(voxie_frame_t* vf, point3d* p, point3d* r, point3d* d, point3d* f, int fillmode, int col);
	float(__cdecl* voxie_drawheimap)(voxie_frame_t* vf, char* fnam, point3d* p, point3d* r, point3d* d, point3d* f, int colorkey, int reserved, int flags);
	void(__cdecl* voxie_drawdicom)(voxie_frame_t* vf, voxie_dicom_t* vd, const char* gfilnam, point3d* gp, point3d* gr, point3d* gd, point3d* gf, int* animn, int* loaddone);
	void(__cdecl* voxie_debug_print6x8)(int x, int y, int fcol, int bcol, const char* st);
	void(__cdecl* voxie_debug_drawpix)(int x, int y, int col);
	void(__cdecl* voxie_debug_drawhlin)(int x0, int x1, int y, int col);
	void(__cdecl* voxie_debug_drawline)(float x0, float y0, float x1, float y1, int col);
	void(__cdecl* voxie_debug_drawcirc)(int xc, int yc, int r, int col);
	void(__cdecl* voxie_debug_drawrectfill)(int x0, int y0, int x1, int y1, int col);
	void(__cdecl* voxie_debug_drawcircfill)(int x, int y, int r, int col);
	void(__cdecl* voxie_debug_drawtile)(tiletype* src, int x0, int y0);
	int(__cdecl* voxie_playsound)(const char* fileName, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed);
	void(__cdecl* voxie_playsound_update)(int handle, int sourceChannel, int volumeLeft, int volumeRight, float playBackSpeed);
	void(__cdecl* voxie_playsound_seek)(int handle, double secs, int seekType);


	void(__cdecl* voxie_setaudplaycb)(void (*userplayfunc)(int* samps, int nframes));
	void(__cdecl* voxie_setaudreccb)(void(*userrecfunc)(int* samps, int nframes));
	int(__cdecl* voxie_rec_open)(voxie_rec_t* vr, char* fnam, char* port, int flags);
	int(__cdecl* voxie_rec_play)(voxie_rec_t* vr, int domode);
	void(__cdecl* voxie_rec_close)(voxie_rec_t* vr);

	//High-level (easy) picture loading function:
	int(__cdecl* kpzload)(const char* fileName, INT_PTR* fptr, INT_PTR* bpl, INT_PTR* xsiz, INT_PTR* ysiz);
	//Low-level PNG/JPG functions:
	int(__cdecl* kpgetdim)(const char* buffer, int nby, int* xsiz, int* ysiz);
	int(__cdecl* kprender)(const char* buffer, int nby, INT_PTR fptr, int bpl, int xsiz, int ysiz, int xoff, int yoff);
	//Ken's ZIP functions:
	int(__cdecl* kzaddstack)(const char* fileName);
	void(__cdecl* kzuninit)();
	kzfile_t* (__cdecl* kzsetfil)(FILE* fileName);
	kzfile_t* (__cdecl* kzopen)(const char*);
	kzfile_t* (__cdecl* kzopen_ext)(const char*, const char*, const char*);
	kzfind_t* (__cdecl* kzfindfilestart)(const char* st);
	int(__cdecl* kzfindfile)(kzfind_t* find, kzfileinfo_t* fileinfo);
	int(__cdecl* kzread)(kzfile_t* kzfile, void* buffer, unsigned int leng);
	int(__cdecl* kzfilelength)(kzfile_t* kzfile);
	int(__cdecl* kzseek)(kzfile_t* kzfile, int offset, int whence);
	int(__cdecl* kztell)(kzfile_t* kzfile);
	int(__cdecl* kzgetc)(kzfile_t* kzfile);
	int(__cdecl* kzeof)(kzfile_t* kzfile);
	void(__cdecl* kzclose)(kzfile_t* kzfile);


#pragma endregion

};

//! 'Factory' function that will return the new object's instance.
extern "C"
{
	EXTEND_API IVoxieBox* _cdecl CreateVoxieBoxObject();

};

//! Function pointer declaration of CreateVoxieBoxObject(). (the entry point function used to create the VoxieBox object)
typedef IVoxieBox* (*CREATE_VOXIEBOX) ();


