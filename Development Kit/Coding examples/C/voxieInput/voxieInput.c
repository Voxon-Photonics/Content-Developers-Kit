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

	Currently supports
	Direct X and JoyPosEx controller inputs
	Mouse Input

	TODO:
	add keyboard
	add space mouse
	add leap motion controls
	add VX coin slot
	

	// Matthew Vecchio 6/8/2019 for Voxon
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
int JOY_DEADZONE = 10000;
int mousx = 0, mousy = 0, mousz = 0;
static voxie_wind_t vw;
voxie_frame_t vf;
static int gcnti[2], gbstat = 0, disp = 1;

enum {

	MENU_TEST_JOY, MENU_TEST_MOUSE,
	MENU_XINPUT, MENU_JOYGETPOSEX, MENU_FORCE_SHOW_ALL,

	STATE_JOY, STATE_MOUSE, STATE_KEY,
};

int testState = STATE_JOY;

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
	case MENU_TEST_JOY: testState = STATE_JOY; break;
	case MENU_TEST_MOUSE: testState = STATE_MOUSE; break;
	
	}

	return(1);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow) {
	
	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim;
	int i, x,y;
	point3d ss, joyC, joyP, pp, rr, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int ovxbut[4], vxnplays, col[4] = {0xffff00,0x00ffff,0xff00ff,0x00ff00}; // the standard joystick colours
	

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }


		voxie_menu_reset(menu_voxieJoy_update,0,0);
		voxie_menu_addtab("Settings",350,0,650,400);
		voxie_menu_additem("Test Input Type", 220,25, 64, 64,0                 ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 
		voxie_menu_additem("Game Pad"	, 	55,	50,120,50,	MENU_TEST_JOY ,MENU_BUTTON+1,1,0x808080,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Mouse"	, 	185,	50,120,50,	MENU_TEST_MOUSE ,MENU_BUTTON+2,0,0x808080,0.0,0.0,0.0,0.0,0.0);

		voxie_menu_additem("Game Pad Input Options"	, 	190,150, 64, 64,0                 ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 
		voxie_menu_additem("Xinput"					, 	55,	170,150,50,	MENU_XINPUT ,MENU_BUTTON+1,0,0x00A0A0,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("JoyPosEx"				, 	225,	170,170,50,	MENU_JOYGETPOSEX ,MENU_BUTTON+2,0,0xA00000,0.0,0.0,0.0,0.0,0.0);
		voxie_menu_additem("Force Show All"			, 	425,	170,170,50,	MENU_FORCE_SHOW_ALL ,MENU_BUTTON+3,0,0xA000A0,0.0,0.0,0.0,0.0,0.0);

		voxie_menu_additem("Voxon Input Test By Matthew Vecchio Voxon 28/5/2020"	, 	15,375, 0, 0,0                 ,MENU_TEXT    ,0             ,0x00FF80,0.0,0.0,0.0,0.0,0.0); 

	while (!voxie_breath(&in)) 	{ // a voxie breath is a one complete volume being rendered 
		otim = tim; tim = voxie_klock(); dtim = tim-otim; // the timer
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);
		for(vxnplays=0;vxnplays<4;vxnplays++){
			ovxbut[vxnplays] = vx[vxnplays].but;
			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1
		}
		// input controls 
		//voxie_keystat is used to detect key presses use Ken's 'keyview' to get the correct scancode
		
		if (voxie_keystat(0x1)) { voxie_quitloop(); }
		if (voxie_keystat(0x02) == 1) { testState = STATE_JOY;  	} // press 1 joystick
		if (voxie_keystat(0x03) == 1) { testState = STATE_MOUSE;  	} // press 2 mouse
		

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
	
 		pp.x = -0.950f; rr.x = 0.04f; dd.x = 0.015f; 
		pp.y = -0.910f;	rr.y = 0.0f; dd.y = 0.080f;
		pp.z = 0.270f; 	rr.z = 0.0f; dd.z = 0.0f;

		switch(testState) {
			case STATE_MOUSE: // mouse
			// how mouse is setup initalise
			// mousx = 256, mousy = 256, mousz = 0
			// during update
			// mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
			// in.but = button state
			// use (in.bstat&1) > (in.obstat&1) to determine just pressed 

			voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"Pos X%d, Y%d, Z%d Del X%d, Y%d, Z%d", mousx, mousy, mousz, in.dmousx, in.dmousy, in.dmousz);
			pp.y -= -0.120f;

			voxie_printalph_(&vf,&pp,&rr,&dd,0xffffff,"But Pressed %d But Just Pressed %d ", in.bstat, (in.bstat&1) > (in.obstat&1));
			pp.y -= -0.120f;

			x = 0;
			if (in.bstat == 1)  x = 1; // left click
			if (in.bstat == 2)  x = 2; // right click
			if (in.bstat == 3)  x = 3; // both buttons held down
			if (in.bstat == 4)  x = 3; // scroll wheel
						 
			voxie_drawsph(&vf, ( mousx * .001 ), ( mousy * .001 ), ( mousz * .0004 ), 0.05f, 1 , col[x]);	
			
			// clip the mouse so it can't scroll outside of the screen
			mouseClip();

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
			
			if (SHOW_ALL == true) y = 4;
			else y = vxnplays;

			if (y <= 0) voxie_printalph_(&vf,&pp,&rr,&dd,col[0],"No controllers found! vxnplays = %d", vxnplays);
				
			for (i = 0; i < y; i++) {

			voxie_debug_print6x8_(32,100 + (i * 20 ),col[i],-1,"J %d X%d Y%d:  Y: But :%04d ", i, vx[i].tx0, vx[i].ty0, vx[i].but );	
							
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
			if ((vx[i].but>>5)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xff00ff); 	// BACK
			if ((vx[i].but>>4)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffffff);	// START
			if ((vx[i].but>>12)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0x00ff00);	// A
			if ((vx[i].but>>13)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y + 0.2f, joyP.z , 0.05 , 1, 0xff0000);	// B
			if ((vx[i].but>>14)&1) voxie_drawsph(&vf,joyC.x - 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0x0000ff);	// X
			if ((vx[i].but>>15)&1) voxie_drawsph(&vf,joyC.x + 0.2f , joyC.y - 0.2f, joyP.z , 0.05 , 1, 0xffff00);	// Y

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

		} 
	
		voxie_debug_print6x8_(32,78,0xffc080,-1,"Press '1': JOYSTICK STICK '2' : MOUSE TEST");
		voxie_frame_end(); voxie_getvw(&vw);

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
