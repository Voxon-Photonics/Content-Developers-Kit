#pragma once
/** 
 * voxieBoxCPP.dll - wraps the Voxiebox.dll into a Voxiebox class for easy use with C++ and OO programming.
 *
 *
 */

#ifdef VOXIEBOXCPP_EXPORTS
#define EXTEND_API __declspec(dllexport)
#else
#define EXTEND_API __declspec(dllimport)
#endif


// include VoxieBox.DLL types 
#include "vxTypes.h"
#include "./input/input_types.h"
#include <memory>

class Ivoxiebox {
public:
	virtual ~Ivoxiebox() { ; }
	virtual bool Breath()		= 0;
	virtual void StartFrame()	= 0;
	virtual void EndFrame()		= 0;
	virtual void Quit()			= 0;
	virtual void Shutdown()		= 0;

	// New Methods
	virtual bool GetKey(Keys key, Key_Modifier modifier)		= 0;
	virtual bool GetKeyDown(Keys key, Key_Modifier modifier)	= 0;
	virtual void ClearInputBuffer()								= 0;
	virtual char GetInputBuffer()								= 0;

	// time
	virtual double DeltaTime()	 = 0;
	virtual double Time()		 = 0;
	virtual double AverageTime() = 0;

	// prints VPS to the secondary screen at posX and posY
	virtual void showVPS()										= 0;
	virtual void showVPS(int posX, int posY, int colour)		= 0;
	virtual void Display2D()									= 0;
	virtual void Display3D()									= 0;
	
	virtual void voxie_printalph_(point3d *p, point3d *r, point3d *d, int col, const char *fmt, ...) = 0;
	virtual void Debug(int x, int y, int fcol, int bcol, const char *fmt, ...) = 0;

	// get the aspectRatio of X,Y or Z
	virtual float  getAspectX() = 0;
	virtual float  getAspectY() = 0;
	virtual float  getAspectZ() = 0;
	virtual point3d getAspect() = 0;

	// Drawing

	virtual void DrawPlane(std::shared_ptr<tiletype> tile_type, poltex_t* poltex_ptr) = 0;
	   	 

	// Controller functions
	virtual void updateControllers()												= 0; 	
	virtual void reportControllers(point3d position)								= 0;
	virtual int  getButtonState(int controller_no)									= 0;
	virtual bool getButtonOnDown(int controller_no, int button_no)					= 0;
	virtual bool getButtonIsDown(int controller_no, int button_no)					= 0;
	virtual bool getButtonOnUp(int controller_no, int button_no)					= 0;
	virtual int  getAnalogAxis(int controller_no, ControllerAxis axis_name)			= 0;
	virtual int  getAnalogTrig(int controller_no, ControllerTrigger trigger_name)	= 0;
	virtual int  getHat(int controller_no)											= 0;
	virtual int  getNumberControllers()												= 0;

	// Mouse 
	virtual void updateMouse()														= 0;
	virtual float getMouseXDelta()													= 0;
	virtual float getMouseYDelta()													= 0;
	virtual float getMouseZDelta()													= 0;
	virtual point3d getMousePosition()												= 0;
	virtual int  getMouseButtonState()												= 0;
	virtual bool getMouseClickIsDown(MouseButton button)							= 0;
	virtual bool getMouseClickOnDown(MouseButton button)							= 0;
	virtual bool getMouseClickOnUp(MouseButton button)								= 0;
	virtual void reportMouse(point3d report_position)								= 0;


	// Misc
	virtual void setBorder(bool option)												= 0;

	// Legacy methods // these are the same as the functions from C / direct voxiebox.dll development plus a few useful methods
    // Legacy methods always start with a capital

	virtual int  Voxie_keystat(int)																= 0;
	virtual int  Voxie_init()																	= 0;
	virtual void DrawVox(float x, float y, float z, int colour)									= 0;
	virtual void DrawBox(float, float, float, float, float, float, int, int)					= 0;
	virtual void DrawSphere(float x, float y, float z, float radius, int fill, int col)			= 0;
	virtual void DrawLine(float, float, float, float, float, float, int)						= 0;
	virtual void DrawPolygon(pol_t*, int, int)													= 0;
	virtual void DrawMeshTex(char*, poltex_t*, int, int*, int, int, int)						= 0;
	virtual void DrawCone(float, float, float, float, float, float, float, float, int, int)		= 0;
	virtual void DrawSprite(const char*, point3d*, point3d*, point3d*, point3d*, int)			= 0;
	virtual void PrintAlpha(point3d*, point3d*, point3d*, int, const char*)						= 0;
	virtual void DrawCube(point3d*, point3d*, point3d*, point3d*, int, int)						= 0;
	virtual void DrawHeightMap(char*, point3d*, point3d*, point3d*, point3d*, int, int, int)	= 0;
};


class voxieBox : public Ivoxiebox {
	bool success = false;
public:
		
	// public access to the Voxie Window vw and Voxie Frame vf -- makes it easier for direct access
	static voxie_wind_t vw;
	voxie_frame_t vf;
	double tim;		// Legacy tim : time variable 
	double dtim; 	// Legacy dtim : delta time varible 

	voxieBox();
	~voxieBox();

	
	// New Methods
	bool GetKey(Keys key, Key_Modifier modifier = NONE);
	bool GetKeyDown(Keys key, Key_Modifier modifier = NONE);
	void ClearInputBuffer();
	char GetInputBuffer();

	void DrawPlane(std::shared_ptr<tiletype> tile_type, poltex_t* poltex_ptr);

	bool Breath();
	void StartFrame();
	void EndFrame();
	void Quit();
	
	// time
	double DeltaTime();
	double Time();
	double AverageTime();

	// prints VPS to the secondary screen at posX and posY
	void showVPS();
	void showVPS(int posX, int posY, int colour);


	void Display2D();
	void Display3D();


	void voxie_printalph_(point3d *p, point3d *r, point3d *d, int col, const char *fmt, ...);
	void Debug(int x, int y, int fcol, int bcol, const char *fmt, ...);

	void Shutdown();
	   
	// get the aspectRatio of X,Y or Z
	float getAspectX();
	float getAspectY();
	float getAspectZ();
	point3d getAspect();


	// Controller functions
	void updateControllers(); 	// updates controller inputs
	void reportControllers(point3d position); // debug for controllers lists all connected controllers. Set a position to display info
	int  getButtonState(int controller_no);
	bool getButtonOnDown(int controller_no, int button_no);
	bool getButtonIsDown(int controller_no, int button_no);
	bool getButtonOnUp(int controller_no, int button_no);
	int  getAnalogAxis(int controller_no, ControllerAxis axis_name);
	int  getAnalogTrig(int controller_no, ControllerTrigger trigger_name);
	int  getHat(int controller_no);
	int  getNumberControllers();

	// Mouse 
	void updateMouse(); // updates mouse inputs need to create a 3d point of the mouse X,Y,Z and pass it through
	float getMouseXDelta();
	float getMouseYDelta();
	float getMouseZDelta();
	point3d getMousePosition();
	int  getMouseButtonState();
	bool getMouseClickIsDown(MouseButton button);
	bool getMouseClickOnDown(MouseButton button);
	bool getMouseClickOnUp(MouseButton button);
	void reportMouse(point3d report_position); // reports a debug of mouse info


	// Misc

	/**
	 * draws a wireframe border around the box 
	 */
	void setBorder(bool option);

	 // Legacy methods // these are the same as the functions from C / direct voxiebox.dll development plus a few useful methods
	// Legacy methods always start with a capital


	int  Voxie_keystat(int);
	int  Voxie_init();
	void DrawVox(float x, float y, float z, int colour);
	void DrawBox(float, float, float, float, float, float, int, int);
	void DrawSphere(float x, float y, float z, float radius, int fill, int col);
	void DrawLine(float, float, float, float, float, float, int);
	void DrawPolygon(pol_t*, int, int);
	void DrawMeshTex(char*, poltex_t*, int, int*, int, int, int);
	void DrawCone(float, float, float, float, float, float, float, float, int, int);
	void DrawSprite(const char*, point3d*, point3d*, point3d*, point3d*, int);
	void PrintAlpha(point3d*, point3d*, point3d*, int, const char*);
	void DrawCube(point3d*, point3d*, point3d*, point3d*, int, int);
	void DrawHeightMap(char*, point3d*, point3d*, point3d*, point3d*, int, int, int);

private:

	static HINSTANCE hvoxie;

	voxie_inputs_t in;
	point3d mouse;

	// Game controllers details
	voxie_xbox_t controller_input[MAX_CONTROLLERS];
	int old_button_controller_data[MAX_CONTROLLERS];
	int no_controllers = MAX_CONTROLLERS;

	int display_volume;

	double time;
	double oldTime;
	double deltaTime;
	double averageTime;

	double version = 0.01;

	int plane_indices[5] = { 3, 2, 1, 0, -1 };

	// draws a border around the outside of the voxieFrame
	bool drawBorder = false;

#pragma region InternalFunctions
	int voxie_load(voxie_wind_t *vw);
	void voxie_uninit(int mode);
#pragma endregion

#pragma region DLL_Functions
	void(__cdecl *voxie_loadini_int)(voxie_wind_t *vw);
	void(__cdecl *voxie_getvw)(voxie_wind_t *vw);
	int(__cdecl *voxie_init)(voxie_wind_t *vw);
	void(__cdecl *voxie_uninit_int)(int);
	void(__cdecl *voxie_mountzip)(char *fnam);
	void(__cdecl *voxie_free)(char *fnam);
	HWND(__cdecl *voxie_gethwnd)(void);
	int(__cdecl *voxie_breath)(voxie_inputs_t *);
	void(__cdecl *voxie_quitloop)(void);
	double(__cdecl *voxie_klock)(void);
	int(__cdecl *voxie_keystat)(int);
	int(__cdecl *voxie_keyread)(void);
	int(__cdecl *voxie_xbox_read)(int id, voxie_xbox_t *vx);
	void(__cdecl *voxie_xbox_write)(int id, float lmot, float rmot);
	int(__cdecl *voxie_nav_read)(int id, voxie_nav_t *nav);
	void(__cdecl *voxie_menu_reset)(int(*menu_update)(int id, char *st, double val, int how, void *userdata), void *userdata, char *bkfilnam);
	void(__cdecl *voxie_menu_addtab)(char *st, int x, int y, int xs, int ys);
	void(__cdecl *voxie_menu_additem)(char *st, int x, int y, int xs, int ys, int id, int type, int down, int col, double v, double v0, double v1, double vstp0, double vstp1);
	void(__cdecl *voxie_menu_updateitem)(int id, char *st, int down, double v);
	void(__cdecl *voxie_touch_custom)(const touchkey_t *touchkey, int num);
	void(__cdecl *voxie_doscreencap)(void);
	void(__cdecl *voxie_setview)(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1);
	void(__cdecl *voxie_setmaskplane)(voxie_frame_t *vf, float x0, float y0, float z0, float nx, float ny, float nz);
	int(__cdecl *voxie_frame_start)(voxie_frame_t *vf);
	void(__cdecl *voxie_frame_end)(void);
	void(__cdecl *voxie_setleds)(int id, int r, int g, int b);
	void(__cdecl *voxie_drawvox)(voxie_frame_t *vf, float fx, float fy, float fz, int col);
	void(__cdecl *voxie_drawbox)(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col);
	void(__cdecl *voxie_drawlin)(voxie_frame_t *vf, float x0, float y0, float z0, float x1, float y1, float z1, int col);
	void(__cdecl *voxie_drawpol)(voxie_frame_t *vf, pol_t *pt, int n, int col);
	//void (__cdecl *voxie_drawmesh   )(voxie_frame_t *vf, point3dcol_t *vt, int vtn, int *mesh, int meshn, int fillmode, int col); //use voxie_drawmeshtex() instead
	void(__cdecl *voxie_drawmeshtex)(voxie_frame_t *vf, char *fnam, poltex_t *vt, int vtn, int *mesh, int meshn, int flags, int col);
	void(__cdecl *voxie_drawsph)(voxie_frame_t *vf, float fx, float fy, float fz, float rad, int issol, int col);
	void(__cdecl *voxie_drawcone)(voxie_frame_t *vf, float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col);
	int(__cdecl *voxie_drawspr)(voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col);
	int(__cdecl *voxie_drawspr_ext)(voxie_frame_t *vf, const char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int col, float forcescale, float fdrawratio, int flags);
	void(__cdecl *voxie_printalph)(voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, int col, const char *st);
	void(__cdecl *voxie_drawcube)(voxie_frame_t *vf, point3d *p, point3d *r, point3d *d, point3d *f, int fillmode, int col);
	float(__cdecl *voxie_drawheimap)(voxie_frame_t *vf, char *fnam, point3d *p, point3d *r, point3d *d, point3d *f, int colorkey, int reserved, int flags);
	void(__cdecl *voxie_drawdicom)(voxie_frame_t *vf, voxie_dicom_t *vd, const char *gfilnam, point3d *gp, point3d *gr, point3d *gd, point3d *gf, int *animn, int *loaddone);
	void(__cdecl *voxie_debug_print6x8)(int x, int y, int fcol, int bcol, const char *st);
	void(__cdecl *voxie_debug_drawpix)(int x, int y, int col);
	void(__cdecl *voxie_debug_drawhlin)(int x0, int x1, int y, int col);
	void(__cdecl *voxie_debug_drawline)(float x0, float y0, float x1, float y1, int col);
	void(__cdecl *voxie_debug_drawcirc)(int xc, int yc, int r, int col);
	void(__cdecl *voxie_debug_drawrectfill)(int x0, int y0, int x1, int y1, int col);
	void(__cdecl *voxie_debug_drawcircfill)(int x, int y, int r, int col);
	int(__cdecl *voxie_playsound)(const char *fnam, int chan, int volperc0, int volperc1, float frqmul);
	void(__cdecl *voxie_playsound_update)(int handle, int chan, int volperc0, int volperc1, float frqmul);
	void(__cdecl *voxie_setaudplaycb)(void(*userplayfunc)(int *samps, int nframes));
	void(__cdecl *voxie_setaudreccb)(void(*userrecfunc)(int *samps, int nframes));
	int(__cdecl *voxie_rec_open)(voxie_rec_t *vr, char *fnam, int flags);
	int(__cdecl *voxie_rec_play)(voxie_rec_t *vr, int domode);
	void(__cdecl *voxie_rec_close)(voxie_rec_t *vr);
	//High-level (easy) picture loading function:
	void(__cdecl *kpzload)(const char *, INT_PTR *, int *, int *, int *);
	//Low-level PNG/JPG functions:
	int(__cdecl *kpgetdim)(const char *, int, int *, int *);
	int(__cdecl *kprender)(const char *, int, INT_PTR, int, int, int, int, int);
	//Ken's ZIP functions:
	int(__cdecl *kzaddstack)(const char *);
	void(__cdecl *kzuninit)();
	void(__cdecl *kzsetfil)(FILE *);
	INT_PTR(__cdecl *kzopen)(const char *);
	void(__cdecl *kzfindfilestart)(const char *);
	int(__cdecl *kzfindfile)(char *);
	int(__cdecl *kzread)(void *, int);
	int(__cdecl *kzfilelength)();
	int(__cdecl *kzseek)(int, int);
	int(__cdecl *kztell)();
	int(__cdecl *kzgetc)();
	int(__cdecl *kzeof)();
	void(__cdecl *kzclose)();
#pragma endregion

};

//  Factory function that will return the new object instance.
extern "C" /*Important for avoiding Name decoration*/
{
	EXTEND_API Ivoxiebox* _cdecl CreateVoxieBoxObject();
	
};

// Function Pointer Declaration of CreateMathObject() [Entry Point Function]
typedef Ivoxiebox* (*CREATE_VOXIEBOX) ();

// button codes for Joypads
enum JOY_BUTTON_CODES
{
	Joy_DPad_Up = 0,
	Joy_DPad_Down = 1,
	Joy_DPad_Left = 2,
	Joy_DPad_Right = 3,
	Joy_Start = 4,
	Joy_Back = 5,
	Joy_Left_Thumb = 6,
	Joy_Right_Thumb = 7,
	Joy_Left_Shoulder = 8,
	Joy_Right_Shoulder = 9,
	Joy_A = 12,
	Joy_B = 13,
	Joy_X = 14,
	Joy_Y = 15
};
// scancodes for all VoxieBox.dll Keyboard inputs
enum KB_SCAN_CODES
{
	KB_ = 0x00,
	KB_Escape = 0x01,
	KB_1 = 0x02,
	KB_2 = 0x03,
	KB_3 = 0x04,
	KB_4 = 0x05,
	KB_5 = 0x06,
	KB_6 = 0x07,
	KB_7 = 0x08,
	KB_8 = 0x09,
	KB_9 = 0x0A,
	KB_0 = 0x0B,

	KB_A = 0x1E,
	KB_B = 0x30,
	KB_C = 0x2E,
	KB_D = 0x20,
	KB_E = 0x12,
	KB_F = 0x21,
	KB_G = 0x22,
	KB_H = 0x23,
	KB_I = 0x17,
	KB_J = 0x24,
	KB_K = 0x25,
	KB_L = 0x26,
	KB_M = 0x32,
	KB_N = 0x31,
	KB_O = 0x18,
	KB_P = 0x19,
	KB_Q = 0x10,
	KB_R = 0x13,
	KB_S = 0x1F,
	KB_T = 0x14,
	KB_U = 0x16,
	KB_V = 0x2F,
	KB_W = 0x11,
	KB_X = 0x2D,
	KB_Y = 0x15,
	KB_Z = 0x2C,

	KB_Alt_Left = 0x38,
	KB_Alt_Right = 0xB8,
	KB_Backspace = 0x0E,
	KB_CapsLock = 0x3A,
	KB_Comma = 0x33,
	KB_Control_Left = 0x1D,
	KB_Control_Right = 0x9D,
	KB_Delete = 0xD3,
	KB_Divide = 0x35,
	KB_Dot = 0x34,
	KB_End = 0xCF,
	KB_Enter = 0x1C,
	KB_Equals = 0x0D,
	KB_Home = 0xC7,
	KB_Insert = 0xD2,
	KB_Minus = 0x0C,
	KB_NumLock = 0x45,
	KB_PageDown = 0xD1,
	KB_PageUp = 0xC9,
	KB_Pause = 0xC5,
	KB_PrintScreen = 0xB7,
	KB_SecondaryAction = 0xDD,
	KB_SemiColon = 0x27,
	KB_ScrollLock = 0x46,
	KB_Shift_Left = 0x2A,
	KB_Shift_Right = 0x36,
	KB_SingleQuote = 0x28,
	KB_Space = 0x39,
	KB_SquareBracket_Open = 0x1A,
	KB_SquareBracket_Close = 0x1B,
	KB_Tab = 0x0F,
	KB_Tilde = 0x29,
	//BackSlash = 0x2B, (Owned by VX1)

	KB_F1 = 0x3B,
	KB_F2 = 0x3C,
	KB_F3 = 0x3D,
	KB_F4 = 0x3E,
	KB_F5 = 0x3F,
	KB_F6 = 0x40,
	KB_F7 = 0x41,
	KB_F8 = 0x42,
	KB_F9 = 0x43,
	KB_F10 = 0x44,
	KB_F11 = 0x57,
	KB_F12 = 0x58,

	KB_NUMPAD_Divide = 0xB5,
	KB_NUMPAD_Multiply = 0x37,
	KB_NUMPAD_Minus = 0x4A,
	KB_NUMPAD_Plus = 0x4E,
	KB_NUMPAD_Enter = 0x9C,

	KB_NUMPAD_0 = 0x52,
	KB_NUMPAD_1 = 0x4F,
	KB_NUMPAD_2 = 0x50,
	KB_NUMPAD_3 = 0x51,
	KB_NUMPAD_4 = 0x4B,
	KB_NUMPAD_5 = 0x4C,
	KB_NUMPAD_6 = 0x4D,
	KB_NUMPAD_7 = 0x47,
	KB_NUMPAD_8 = 0x48,
	KB_NUMPAD_9 = 0x49,
	KB_NUMPAD_Dot = 0x53,

	KB_ARROW_Up = 0xC8,
	KB_ARROW_Down = 0xD0,
	KB_ARROW_Left = 0xCB,
	KB_ARROW_Right = 0xCD

	
};

