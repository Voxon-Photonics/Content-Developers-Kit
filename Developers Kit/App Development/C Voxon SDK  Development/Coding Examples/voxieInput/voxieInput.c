#if 0
!if 1

	#Visual C makefile:
voxieInput.exe: voxieInput.c voxiebox.h; cl /TP voxieInput.c /Ox /MT /link user32.lib
	del voxieInput.obj

!else

	#GNU C makefile:
voxieInput.exe: voxieInput.c; gcc voxieInput.c -o voxieInput.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

	/*VoxieInput an input tester for Voxon Hardware.
	This code shows you how to create your own input calls
	& Tests inputs are working correctly for VX1

	Currently supports
	Direct X and JoyPosEx controller inputs
	Mouse Input
	Keyboard
	Space Navigator

	TODO:
	add leap motion controls

	

	// Matthew Vecchio 6/8/2019 for Voxon
	// last updated  5/8/2020
	*/

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif

bool SHOW_ALL = false;
int JOY_DEADZONE = 10000; // a value of deadzone (appears as red on the display) for the joystick counteract drift 
int mousx = 0, mousy = 0, mousz = 0;
static voxie_wind_t vw;
voxie_frame_t vf;
static int gbstat = 0, disp = 1;
static const int MAX_KEY_HISTORY = 10;
static int keyHistory[MAX_KEY_HISTORY];
static int inited = 0;


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
// scancodes for all VoxieBox.dll inputs
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


enum menuStates {

	MENU_TEST_JOY, MENU_TEST_MOUSE, MENU_TEST_KEYBOARD, MENU_TEST_SPACE_MOUSE,
	MENU_XINPUT, MENU_JOYGETPOSEX, MENU_FORCE_SHOW_ALL,

	STATE_JOY, STATE_MOUSE, STATE_KEYBOARD, STATE_SPACE_MOUSE, STATE_HOME
};

int testState = STATE_HOME;
bool choiceMade = false;

// clips the mouse so the pointer won't go out of the bounds forever....
static void mouseClip() {
	float	MOUSE_MAX_X = vw.aspx * 1000;
	float	MOUSE_MAX_Y = vw.aspy * 1000;
	float 	MOUSE_MAX_Z = vw.aspz * 2400;
	// mouse clipping
	if (mousx > MOUSE_MAX_X) mousx = MOUSE_MAX_X;
	if (mousx < -MOUSE_MAX_X) mousx = -MOUSE_MAX_X;
	if (mousy > MOUSE_MAX_Y) mousy = MOUSE_MAX_Y;
	if (mousy < -MOUSE_MAX_Y) mousy = -MOUSE_MAX_Y;
	if (mousz > MOUSE_MAX_Z) mousz = MOUSE_MAX_Z;
	if (mousz < -MOUSE_MAX_Z) mousz = -MOUSE_MAX_Z;
}

// draw a circle
static void drawCir( point3d pos, float rad, float resolution, int colour) {
float angle = 0;
float x,y;
while (angle < 2 * PI) {
// calculate x, y from a vector with known length and angle
    x = rad * cos (angle);
    y = rad * sin (angle);
    voxie_drawvox (&vf,x + pos.x,y + pos.y, pos.z, colour);
    angle += resolution;
	}

}

//Rotate vectors a & b around their common plane, by ang
static void rotvex (float ang, point3d *a, point3d *b) {
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}


// the menu update this is where the variables change when a button is pressed in the menu
static int menu_voxieJoy_update (int id, char *st, double v, int how, void *userdata) {
	switch(id)
	{
	case MENU_XINPUT:  vw.usejoy = 1; voxie_init(&vw); break;
	case MENU_JOYGETPOSEX:  vw.usejoy = 0; voxie_init(&vw); break;
	case MENU_FORCE_SHOW_ALL: SHOW_ALL = true; break;
	case MENU_TEST_JOY: testState = STATE_JOY; choiceMade = true; inited = 0; break;
	case MENU_TEST_MOUSE: testState = STATE_MOUSE; choiceMade = true; inited = 0; break;
	case MENU_TEST_KEYBOARD: testState = STATE_KEYBOARD; choiceMade = true; inited = 0; break;
	case MENU_TEST_SPACE_MOUSE: testState = STATE_SPACE_MOUSE; choiceMade = true; inited = 0; break;
	
	
	}

	return(1);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow) {
	
	voxie_inputs_t in; // needed for mouse
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim, avgdtim;
	int i,j = 0,k,x,y,z;
	float f,g,h, objScale = 1;
	int lastPressedKey = 0;
	point3d ss, joyC, joyP, pp, rr, dd, ff, opp, orr, odd, off, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1}, xRot, yRot, zRot;
	int col[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00}; // the standard joystick colours
	const int MOUSE_POINT_MAX = 5000;
	point3d mousePoints[MOUSE_POINT_MAX]; 
	int mousePointIndex = 0;
	double  autoSelectDelay = 3;
	



	// Varibles needed for Joystick input initalise Joysticks
	int ovxbut[4], vxnplays;
	voxie_xbox_t vx[4];

	//	Variables needed for Space Navigator
	int onavbut[4];
	voxie_nav_t nav[4] = {0};
	


	

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }


		voxie_menu_reset(menu_voxieJoy_update,0,0);
		voxie_menu_addtab("Settings",350,0,650,400);
		voxie_menu_additem("Test Input Type", 220,25, 64, 64,0                			 ,MENU_TEXT    ,0 ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 
		voxie_menu_additem("Game Pad"	, 	55,		50,120,50,	MENU_TEST_JOY 			,MENU_BUTTON+1,1,0x808080,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Mouse"		, 	185,	50,120,50,	MENU_TEST_MOUSE 		,MENU_BUTTON+2,0,0x808080,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Keyboard"	, 	315,	50,120,50,	MENU_TEST_KEYBOARD 		,MENU_BUTTON+2,0,0x808080,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Space Nav"	, 	445,	50,120,50,	MENU_TEST_SPACE_MOUSE 	,MENU_BUTTON+2,0,0x808080,0.0,0.0,0.0,0.0,0.0);

		voxie_menu_additem("Game Pad Input Options"	, 	190,150, 64, 64,0                 ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 
		voxie_menu_additem("Xinput"					, 	55,	170,150,50,	MENU_XINPUT ,MENU_BUTTON+1,0,0x00A0A0,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("JoyPosEx"				, 	225,	170,170,50,	MENU_JOYGETPOSEX ,MENU_BUTTON+2,0,0xA00000,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Force Show All"			, 	425,	170,170,50,	MENU_FORCE_SHOW_ALL ,MENU_BUTTON+3,0,0xA000A0,0.0,0.0,0.0,0.0,0.0);

		voxie_menu_additem("Voxon Input Test By Matthew Vecchio Voxon 5/8/2020"	, 	15,375, 0, 0,0                 ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 

	while (!voxie_breath(&in)) 	{ // a voxie breath is a one complete volume being rendered 
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);
		
		
		for(vxnplays=0;vxnplays<4;vxnplays++){ //check through controllers' inputs
			ovxbut[vxnplays] = vx[vxnplays].but;
			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1
		} 


		for(i = vxnplays; i < 4; i++) {
			ovxbut[i] = vx[i].but;
			vx[i].but = 0;
			vx[i].hat = 0;
			vx[i].lt = 0;
			vx[i].rt = 0;
			vx[i].tx0 = 0;
			vx[i].tx1 = 0;
			vx[i].ty0 = 0;
			vx[i].ty1 = 0;
			
		}

		for(i=0;i<4;i++) { // check through space Nav
			onavbut[i] = nav[i].but; voxie_nav_read(i,&nav[i]);
		}

		if (autoSelectDelay < tim && testState == STATE_HOME && choiceMade == false) {
			testState = STATE_JOY; 				choiceMade = true; inited = 0;
		} 

	

		//Key Presses Menu Controls 
		//voxie_keystat is used to detect key presses use Ken's 'keyview' to get the correct scancode or see the enum VX_KB_SCANCODES
		
		if (voxie_keystat(KB_Escape)) { voxie_quitloop(); }
		if (voxie_keystat(KB_1) == 1) { testState = STATE_JOY; 				choiceMade = true; inited = 0;	 } // press 1 joystick
		if (voxie_keystat(KB_2) == 1) { testState = STATE_MOUSE;  			choiceMade = true; inited = 0;  } // press 2 mouse
		if (voxie_keystat(KB_3) == 1) { testState = STATE_KEYBOARD; j = 0; 	choiceMade = true; inited = 0;  } // press 3 keyboard
		if (voxie_keystat(KB_4) == 1) { testState = STATE_SPACE_MOUSE; 		choiceMade = true; inited = 0;  } // press 4 on keyboard 
	

		// a few things to setup on first run when changing input type
		if (inited == 0) {
			inited = 1;

			switch(testState) {
				case STATE_KEYBOARD:
					j = 0;
				break;
				case STATE_MOUSE:
				case STATE_SPACE_MOUSE:
	
					for (i = 0; i < MOUSE_POINT_MAX; i++) {
						mousePoints[i].x = vw.aspx * 2;
						mousePoints[i].y = vw.aspy * 2;
						mousePoints[i].z = vw.aspz * 2;
					}
					mousePointIndex = 0;
				
					objScale = 1;
					opp.x = 0; orr.x = 0.5; odd.x = 0; 	 off.x = 0;
					opp.y = 0; orr.y = 0; 	odd.y = 0.5; off.y = 0;
					opp.z = 0; orr.z = 0; 	odd.z = 0; 	 off.z = 0.5;
				break;


			}


		}

		
		

		i = (voxie_keystat(0x1b)&1)-(voxie_keystat(0x1a)&1);

		if (i) {
			if (voxie_keystat(0x2a)|voxie_keystat(0x36))      vw.emuvang = min(max(vw.emuvang+(float)i*dtim*2.0,-PI*.5),0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d)|voxie_keystat(0x9d)) vw.emudist = max(vw.emudist-(float)i*dtim*2048.0,400.0); //Ctrl+[,]
			else                                              vw.emuhang += (float)i*dtim*2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);

		//draw wireframe box
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

		// set up font size	
 		pp.x = -0.950f; rr.x = 0.04f; dd.x = 0.0f; 
		pp.y = -0.910f;	rr.y = 0.0f; dd.y = 0.080f;
		pp.z = 0.270f; 	rr.z = 0.0f; dd.z = 0.0f;

		switch(testState) {
			case STATE_HOME:
				rr.x = 0.05;
				dd.y = 0.1;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"\n\nVoxie Input Test\n\n\nUse the Touch menu to select test or...\n\nPress '1' for Joystick / Gamepad test\nPress '2' for Mouse test\nPress '3' for Keyboard test\nPress '4' for SpaceNav test", x, x>>8, voxie_keystat(x>>8));

			break;
			case STATE_KEYBOARD: 
				// setup slightly larger font size
				pp.x = -0.950f; rr.x = 0.045f; dd.x = 0.01f; 
				pp.y = -0.910f;	rr.y = 0.0f; dd.y = 0.08f;
				pp.z = 0.270f; 	rr.z = 0.0f; dd.z = 0.0f;

				/* There are two ways you can read keyboard input using a voxie_keyread() within a while look or using voxie_keystat(scan key) 
				and reading in the key's state

				WARNING : once calling voxie_keyread() it clears out the scan code so you can only use it once in your code otherwise you won't get the key pressings

				// using voxie_keyread 

				while (key = voxie_keyread()) //get buffered key (low 8 bits is ASCII code, next 8 bits is keyboard scancode - for more exotic keys)
				
				{
					if ((char)key == 27) { voxie_quitloop(); } //ESC:quit
					if (((char)key == '-') || ((char)key == '_')) { targvps = fmaxf(targvps-1.f,  3.f); }
					if (((char)key == '+') || ((char)key == '=')) { targvps = fminf(targvps+1.f,120.f); }
					if ((char)key == ',') { palind--; if (palind < 0) palind = sizeof(paltab)/sizeof(palcur)-1; }
					if ((char)key == '.') { palind++; if (palind >= sizeof(paltab)/sizeof(palcur)) palind = 0; }
					if (((char)key == 'I') || ((char)key == 'i')) { ctrldir = -ctrldir; }
					if (((char)key == '/') || ((char)key == ' ')) { resetposori = 1; }
					if (((char)key == ';') || ((unsigned char)(key>>8) == 0xc9)) { funcid--; if (funcid < 0) funcid = sizeof(getvox_funcs)/sizeof(getvox_funcs[0])-1; resetposori = 1; } //PGUP
					if (((char)key == '\'') || ((unsigned char)(key>>8) == 0xd1)) { funcid++; if (funcid >= sizeof(getvox_funcs)/sizeof(getvox_funcs[0])) funcid = 0; resetposori = 1; } //PGDN
				}

				// using voxie_keystat

					0x1e is scan code for the keyboard key 'a'
					voxie_keystat() return a 0 if the key has not been pressed, 1 if the key has just been pressed (was not pressed in the previous voxie_breath()) or 3 if it is held down

					if (voxie_keystat(0x1e)) { ...  } // if Keyboard A is pressed down excute code
					if (voxie_keystat(0x1e) == 3 ) { ...  } // same thing if Keyboard key 'a' is pressed down excute code
					if (voxie_keystat(0x1e) == 1 ) { ...  } // if Keyboard key 'a' has just been pressed down excute code -- this only works if it happens BEFORE a while() loop voxie_keyread()


				*/
			
			
				voxie_debug_print6x8_(32,200,0xff0000,-1,"voxie_keyread() = %d", y);	
					
				while (y = voxie_keyread()) {
					
					// add special keys here
					// special keys bitshift by 8 (>>8) to key a key's scancode 
					if (y>>8 == KB_Backspace && j > 0 ) { keyHistory[j - 1] = 0; j--;    }
					
					else { // usual flow
					
						x = y;
						keyHistory[j] = x;
						j++;
						
						if (j > MAX_KEY_HISTORY) { 
							j = 0;
							for (i = 0; i < MAX_KEY_HISTORY; i++) keyHistory[i] = 0;	
						}
					}
					
				}


				// to get the scancode from voxie_keyread() bit shift up >>8 the return value					
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"'%c' Pressed (ScanCode = %#04x) State = %d ", x, x>>8, voxie_keystat(x>>8));
				
				pp.y -= -0.120f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0x00ff00,"   index: %d | Type Something | Choice %d", j, k);
				pp.y -= -0.120f;

				// typing 
				pp.x = -0.990f;
				for (i = 0; i < MAX_KEY_HISTORY; i++) {
					if (keyHistory[i] == 0) continue;
					pp.x += (.04);
					voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"%c", keyHistory[i]);
				}

				// history
				pp.x = -0.950f;
				for (i = 0; i < MAX_KEY_HISTORY; i++) {
					if (keyHistory[i] == 0) break;
					pp.y -= -0.120f;
					voxie_printalph_(&vf,&pp,&rr,&dd,col[i % 4],"%d. %c Key Last Pressed (ScanCode = %#04x)", i, keyHistory[i], (keyHistory[i] >> 8) & 0xFF );
				}

				// keyreading choice using while loop
				pp.y -= -0.120f;

				// key reading using if statement
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"press 'a' 'b' or 'c' to make a choice");
				pp.y -= -0.120f;
				pp.x = 0 - 0.05;
				if (voxie_keystat(KB_A)) 		{ voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"a Held"); k = 1;}
				if (voxie_keystat(KB_B) == 1)   { voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"b Just pressed == 1"); k = 2;} // because of the voxie_keyread while loop this button won't register 
				if (voxie_keystat(KB_C) == 3) 	{ voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"c Held using == 3"); k = 3;}
				



				break;
			case STATE_SPACE_MOUSE:
				//int nav[i].but = the button state (compare with onavbut[i] for just pressed)
				//float nav[i].ax,ay,az = x,y,z floats of the angle infomation
				//float nav[i].dx,dy,dy = x,y,z floats  of the direction infomation
				//
				// NOTE:
				// the X Y Z values are right hand rule / coordinates systemvalues (not the same as VoxieBox.dll standards which is left hand rule / coordinates)



				for(i=0;i<4;i++) {
					voxie_debug_print6x8_(32,200 + ( i * 20),col[i],-1,"No %d Button : %d | Angle X :%1.3f, Y :%1.3f Z :%1.2f | Direction  X :%1.2f, Y:%1.2f, Z:%1.2f |", i, nav[i].but, nav[i].ax, nav[i].ay, nav[i].az, nav[i].dx, nav[i].dy, nav[i].dz);	
				}


				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Angle    : X%1.1f, Y%1.1f, Z%1.1f", nav[0].ax, nav[0].ay, nav[0].az);
				pp.y -= -0.120f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Direction: X%1.1f, Y%1.1f, Z%1.1f", nav[0].dx, nav[0].dy, nav[0].dz);
				pp.y -= -0.120f;
				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"But Pressed %d But Just Pressed %d ", nav[0].but, nav[0].but != onavbut[0]);
				pp.y -= -0.120f;

				




				//******************************************************
				// using the Space Nav as a cursor

				// space mouse sensitivity change this value to edit 				
				g = 0.00005; 

				pos.x += (nav[0].dx * (g * vw.aspx));
				pos.y += (nav[0].dy * (g * vw.aspy));
				pos.z += (nav[0].dz * (g * vw.aspz));

				// clip the space nav cursor to be within the volume
				if (pos.x > vw.aspx) 	pos.x = vw.aspx;
				if (pos.x < -vw.aspx) 	pos.x = -vw.aspx;
				if (pos.y > vw.aspy) 	pos.y = vw.aspy;
				if (pos.y < -vw.aspy) 	pos.y = -vw.aspy;
				if (pos.z > vw.aspz) 	pos.z = vw.aspz;
				if (pos.z < -vw.aspz) 	pos.z = -vw.aspz;
				
				
				//draw sphere for where the space nav cursor is
				voxie_drawsph(&vf, pos.x, pos.y, pos.z, 0.05f, 1 , col[nav[0].but]);	

					// click button to drop a point 
				// Left click for pressed
				// Right click for hold
				if (nav[0].but == 1 && nav[0].but != onavbut[0] || nav[0].but == 2) {

					mousePoints[mousePointIndex].x = pos.x;
					mousePoints[mousePointIndex].y = pos.y;
					mousePoints[mousePointIndex].z = pos.z;
					mousePointIndex++;
					if (mousePointIndex > MOUSE_POINT_MAX) mousePointIndex = 0;

				}

				// draw mouse points
				for (i = 0; i < MOUSE_POINT_MAX; i++) voxie_drawsph(&vf, mousePoints[i].x, mousePoints[i].y, mousePoints[i].z , 0.05f, 0 , 0xFF0000);	

				/******************************************************/
				// Space Mouse using the angle to rotate an object


				// rotate sensitivity  edit to adjust
				f = 0.0125;
				
				// using the space mouse to rotate an object
				if (nav[0].az > 0 || nav[0].az < 0) { rotvex((f * nav[0].az) * dtim, &orr, &odd); } // yaw left and right
				if (nav[0].ay > 0 || nav[0].ay < 0) { rotvex((f * nav[0].ay) * dtim, &odd, &off); } // pitch forward and back
				if (nav[0].ax > 0 || nav[0].ax < 0) { rotvex((f * nav[0].ax) * dtim, &orr, &off); } // roll the object

				// press both buttons to reset position
				if (nav[0].but == 3) {
					opp.x = 0; orr.x = 0.5; odd.x = 0; 	 off.x = 0;
					opp.y = 0; orr.y = 0; 	odd.y = 0.5; off.y = 0;
					opp.z = 0; orr.z = 0; 	odd.z = 0; 	 off.z = 0.5;
				} 

				// edit scale of object by pressing buttons
				if (nav[0].but == 1) {
					objScale += 0.01;
					objScale = min(objScale, 3);
				}
				if (nav[0].but == 2) {
					objScale -= 0.01;
					objScale = max(objScale, -3);
				}

				/* update the models position based the cursors location
				opp.x = pos.x;
				opp.y = pos.y;
				opp.z = pos.z;
				*/
				// before we draw the model combine the rotation and the scale 
				xRot.x = orr.x * objScale;
				xRot.y = orr.y * objScale;
				xRot.z = orr.z * objScale;
				yRot.x = odd.x * objScale;
				yRot.y = odd.y * objScale;
				yRot.z = odd.z * objScale;
				zRot.x = off.x * objScale;
				zRot.y = off.y * objScale;
				zRot.z = off.z * objScale;
	
				voxie_drawspr(&vf, "Muscle_Coupe.obj", &opp, &xRot, &yRot, &zRot, 0x404040);

				break;
			case STATE_MOUSE: // mouse
				// how mouse is setup initalise
				// mousx = 256, mousy = 256, mousz = 0
				// during update
				// mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
				// in.but = button state
				// use (in.bstat&1) > (in.obstat&1) to determine just pressed 

				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Pos X%d, Y%d, Z%d Del X%d, Y%d, Z%d", mousx, mousy, mousz, in.dmousx, in.dmousy, in.dmousz);
				pp.y -= -0.120f;

				voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"But Pressed %d But Just Pressed %d ", in.bstat, in.bstat != in.obstat);
				pp.y -= -0.120f;

				x = 0;
				if (in.bstat == 1)  x = 1; // left click
				if (in.bstat == 2)  x = 2; // right click
				if (in.bstat == 3)  x = 3; // both buttons held down
				if (in.bstat == 4)  x = 3; // scroll wheel
				
				// mouse sensitivity edit to adjust
				g = .001;
							
				voxie_drawsph(&vf, ( mousx * (g * vw.aspx )), ( mousy * (g * vw.aspy ) ), ( mousz * (g * vw.aspz ) ), 0.05f, 1 , col[x]);	
				
				// clip the mouse so it can't scroll outside of the screen
				mouseClip();

				// click button to drop a point 
				// Left click for pressed
				// Right click for hold
				if (in.bstat == 0 && in.obstat == 1 || in.bstat == 2) {

					mousePoints[mousePointIndex].x = mousx * (g * vw.aspx );
					mousePoints[mousePointIndex].y = mousy * (g * vw.aspy );
					mousePoints[mousePointIndex].z = mousz * (g * vw.aspz );
					mousePointIndex++;
					if (mousePointIndex > MOUSE_POINT_MAX) mousePointIndex = 0;

				}

				// draw mouse points
				for (i = 0; i < MOUSE_POINT_MAX; i++) {

					voxie_drawsph(&vf, mousePoints[i].x, mousePoints[i].y, mousePoints[i].z , 0.05f, 0 , 0xFF0000);	


				}


			break;
			case STATE_JOY: // joystick input test
			
				// vxnplays = the number of plugged in controllers
				// button codes Xbox 360 standard
				// vw[i].but>>0 = up
				// vw[i].but>>1 = down
				// vw[i].but>>2 = left
				// vw[i].but>>3 = right
				// vw[i].but>>4 = start
				// vw[i].but>>5 = back
				// vw[i].but>>6 = left analog stick button
				// vw[i].but>>7 = right analog stick button
				// vw[i].but>>8 = left trigger
				// vw[i].but>>9 = right trigger
				// vw[i].but>>10 = not used
				// vw[i].but>>11 = not used
				// vw[i].but>>12 = Green A button
				// vw[i].but>>13 = Red B button
				// vw[i].but>>14 = Blue X button
				// vw[i].but>>15 = Yellow Y button
				// vw[i].tx0 = value is analog left stick X
				// vw[i].ty0 = value is analog left stick Y
				// vw[i].tx1 = value is analog right stick X
				// vw[i].ty1 = value is analog right  stick Y
				// vx[i].hat = the hat direction
				// vw[i].lt = value is left analog trigger
				// vw[i].rt = value is left analog trigger

				// example of how to do 'on down' check
				// (vx[0].but&~ovxbut[0]&(1 <<12)) // if true the green A button is down for controller 1 

				// example of how to do 'is down' check
				// (vx[i].but>>12)&1 // if true the green A button is down for controller 1 
							
				// Keyboard overide for controls - if you ever wanted to use a keyboard as a controller
				if (voxie_keystat(KB_Backspace)) 	vx[i].but += 32; // press kb backspace for joy btn Back
				if (voxie_keystat(KB_Enter)) 		vx[i].but += 16; // press kb enter for joy btn start
				if (voxie_keystat(KB_ARROW_Up)) 	vx[i].but += 1;  // press kb up for joy Dpad up
				if (voxie_keystat(KB_ARROW_Down)) 	vx[i].but += 2;  // press kb down for joy Dpad down
				if (voxie_keystat(KB_ARROW_Left)) 	vx[i].but += 4;  // press kb left for joy Dpad Left 
				if (voxie_keystat(KB_ARROW_Right)) 	vx[i].but += 8;  // press kb right for joy Dpad Right
				if (voxie_keystat(KB_Space)) 		vx[i].but += 4096;  // press kb space for joy Dpad 'A'
					
				if (SHOW_ALL == true) y = 4;
				else y = vxnplays;
				
				if (y <= 0) voxie_printalph_(&vf,&pp,&rr,&dd,col[0],"No controllers found! vxnplays = %d", vxnplays);
					
				for (i = 0; i < y; i++) {

				voxie_debug_print6x8_(32,200 + (i * 20 ),col[i],-1,"J %d X%d Y%d:  Y: But :%04d || %d ", i, vx[i].tx0, vx[i].ty0, vx[i].but, vx[i].but );	
								
				voxie_printalph_(&vf,&pp,&rr,&dd,col[i],"J %d X%d Y%d X2 %d Y2 %d", i, vx[i].tx0, vx[i].ty0, vx[i].tx1, vx[i].ty1);
				pp.y -= -0.120f;

				voxie_printalph_(&vf,&pp,&rr,&dd,col[i],"%04d 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d", 
				vx[i].but,(vx[i].but>>0)&1, (vx[i].but>>1)&1, (vx[i].but>>2)&1, (vx[i].but>>3)&1, (vx[i].but>>4)&1, (vx[i].but>>5)&1, (vx[i].but>>6)&1, (vx[i].but>>7)&1 );
				
				pp.y -= -0.120f;
				voxie_printalph_(&vf,&pp,&rr,&dd,col[i],"8:%d 9:%d 10:%d 11:%d 12:%d 13:%d HAT:%d", 
				(vx[i].but>>8)&1, (vx[i].but>>9)&1, (vx[i].but>>10)&1, (vx[i].but>>11)&1, (vx[i].but>>12)&1, (vx[i].but>>13)&1, vx[i].hat);
				pp.y -= -0.120f;
				
				voxie_printalph_(&vf,&pp,&rr,&dd,col[i],"14:%d 15:%d LT%d RT%d JP:%d, %d, %d, %d", 
				(vx[i].but>>14)&1, (vx[i].but>>15)&1, vx[i].lt,  vx[i].rt, 
				(vx[i].but&~ovxbut[i]&(1 <<12)), (vx[i].but&~ovxbut[i]&(1 <<13)), (vx[i].but&~ovxbut[i]&(1 <<14)), (vx[i].but&~ovxbut[i]&(1 <<15)) );
				pp.y -= -0.120f;

				joyC.x = .73;
				joyC.y = -.75 + ( i * 0.5);
				joyC.z = 0;
				// draw circle for joystick
				drawCir( joyC, 0.2, 	.02f, 0xFFFFFF);

				/* Example of using raw values for button codes
				if ((vx[i].but>>5)&1)  voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xff00ff); 	// BACK
				if ((vx[i].but>>4)&1)  voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffffff);	// START
				if ((vx[i].but>>12)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0x00ff00);	// A
				if ((vx[i].but>>13)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0xff0000);	// B
				if ((vx[i].but>>14)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0x0000ff);	// X
				if ((vx[i].but>>15)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffff00);	// Y
				*/

				// Example of on to use the enum button codes.
				if ((vx[i].but>>Joy_Back)&1)  voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xff00ff); 	// BACK
				if ((vx[i].but>>Joy_Start)&1)  voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffffff);	// START
				if ((vx[i].but>>Joy_A)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0x00ff00);	// A
				if ((vx[i].but>>Joy_B)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0xff0000);	// B
				if ((vx[i].but>>Joy_X)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0x0000ff);	// X
				if ((vx[i].but>>Joy_Y)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffff00);	// Y
			

				// vibrate controls (controller no, float for left motor 0..1, float for right motor 0..1)
				if ((vx[i].but>>12)&1 || (vx[i].but>>13)&1 ) voxie_xbox_write(i, cos(tim), 0);
				else if ((vx[i].but>>14)&1 || (vx[i].but>>15)&1) voxie_xbox_write(i, 0, cos(tim));
				else voxie_xbox_write(i, 0, 0);
			
				
				joyP.x = vx[i].tx0;
				joyP.y = vx[i].ty0;
				joyP.z = -0.1f;
				
				// testing if the joystick is with in the deadzone
				if (joyP.x > JOY_DEADZONE || joyP.x < -JOY_DEADZONE || joyP.y > JOY_DEADZONE || joyP.y < -JOY_DEADZONE  ) // outside of deadzone
				voxie_drawsph(&vf,joyP.x *.000005 + joyC.x, -joyP.y *.000005 + joyC.y, joyP.z , 0.05 , 1, col[i]);
				else voxie_drawsph(&vf,joyP.x *.000005 + joyC.x, -joyP.y *.000005 + joyC.y, joyP.z , 0.05 , 1, 0xff0000);		


				voxie_drawcone (&vf, joyP.x *.000005 + joyC.x, -joyP.y *.000005 + joyC.y, joyP.z, 0.02,
								joyC.x , joyC.y, joyC.z, 0.02, 1, 0xFFFFFF);
				
			
			}
				break;

		} 
		//display menu commands on the VX1 touch screen
		voxie_debug_print6x8_(32,78,0xff0080,-1,"Voxie Input Tester Menu\n\n Press '1' : JOYSTICK STICK\n Press '2' : MOUSE TEST\n Press '3' : KEYBOARD\n Press '4' : SPACE MOUSE");

		//display VPS
		avgdtim += (dtim - avgdtim) * .1;
		voxie_debug_print6x8_(32, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
	
		voxie_frame_end(); voxie_getvw(&vw);

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
