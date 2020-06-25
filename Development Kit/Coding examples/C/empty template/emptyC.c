#if 0
!if 1
#Visual C makefile:
empty.exe: emptyC.c voxiebox.h; cl /TP emptyC.c /Ox /MT /link user32.lib
	del emptyC.obj

!else

#GNU C makefile:
emptyC.exe: emptyC.c; gcc empty.c -o emptyC.exe -pipe -O3 -s -m64

!endif
!if 0
#endif
/*
Voxon empty template by Matthew Vecchio for Voxon 
this version has comments for an un commented version please use 'empty.c'

change 'emptyC.c' to what ever you want to call your file

How to make:

1. Run Visual studio command prompt
2. Navigate to the program's directory
3 type "nmake emptyC.c" 
*/
#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

static voxie_wind_t vw; // the voxie window struct which holds many hardware / software settings which can be changed during runtime see voxiebox.h for more info
static voxie_frame_t vf; // the voxie frame struct which holds frame data not much a user can modify.

//Rotate vectors a & b around their common plane, by ang
//this function is great for rotating points around it is also used to simply rotate the emulator's view using the '[' and ']' keys
static void rotvex(float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

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

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; // the voxie input struct used to check mouse inputs
	pol_t pt[3];
	// tim is a clock counts up from runtime
	// dtim is delta time used for timings between frames
	// otim is old tim 
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; // these are used to work mouse positions 
	point3d pp, rr, dd, ff; // point 3d are 3 dimentional points used in various graphic calls.
	voxie_xbox_t vx[4]; // this array holds the number of controllers plugged in
	int i;
	int ovxbut[4], vxnplays; //ovxbut is the previous frame's controller button states vxnplays is how many controllers are currently pluged in and detected
	int inited = 0; // and inited variable is used when initalise a program
	int debug = 1; // a simple flag to turn on and off debug modes

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{					
		return (-1);
	}

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. A whole volume is rendrered in a single breath
	{
		otim = tim; // update the old timer to the last timers count
		tim = voxie_klock(); // work out the timer's value
		dtim = tim - otim; // work out the delta time
		mousx += in.dmousx; // update the mouse movement 
		mousy += in.dmousy; // update the mouse movement 
		mousz += in.dmousz; // update the mouse movement 
		for (vxnplays = 0; vxnplays < 4; vxnplays++) // check through the gamepad input
		{
			ovxbut[vxnplays] = vx[vxnplays].but; // turn the old buttons

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
			break; //but, lt, rt, tx0, ty0, tx1, ty1
		}


		// for keystats (input scan codes for keys) use Ken's "Keyview" program!
		/*
		Common Scan codes

		0xc8:Up       0xd0:Down
		0xcb:Left     0xcd:Right
		0x2a:L.Shift  0x36:R.Shift
		0x1d:L.Ctrl   0x9d:R.Ctrl
		0x38:L.Alt    0xb8:R.Alt
		0xc9:PageUp   0xd1:PageDn
		0xc7:Home     0xcf:End
		0x52:'0' on keypad

		*/

		// put controls global keys here
		if (voxie_keystat(0x1)) // esc key closes ap
		{
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}

		// use voxie_keystat(scan code)) to test inputs from the keyboard use Ken's keyview program for scancodes

		// rotation keys // this adds using [ ] keys to rotate the image in emulation mode
	
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1); // keys '[' and ']'
		if (i)
		{
			if (voxie_keystat(0x2a) | voxie_keystat(0x36))
				vw.emuvang = min(max(vw.emuvang + (float)i * dtim * 2.0, -PI * .5), 0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d) | voxie_keystat(0x9d))
				vw.emudist = max(vw.emudist - (float)i * dtim * 2048.0, 400.0); //Ctrl+[,]
			else
				vw.emuhang += (float)i * dtim * 2.0; //[,]
			voxie_init(&vw);
		}

		voxie_frame_start(&vf); // start the voxieFrame - a voxieFrame is one horiztonal slice 

		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); // set the view of the screen usually use the voxie_window's aspect ratios

	/**************************
	*  START OF INNIT PHASE   *
	*                         *
	**************************/

		if (inited == 0)
		{
			inited = 1;
		}


	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	/**************************
	*  INPUT                  *
	*                         *
	**************************/

	/**************************
	*   LOGIC                 *
	*                         *
	**************************/

	/**************************
	*   DRAW                  *
	*                         *
	**************************/

	/**************************
	*   DEBUG                 *
	*                         *
	**************************/

		// final update loop for frame
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
	
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
		}

		voxie_frame_end(); // the end of the voxie frame everything outside of this call will not be part of the frame
	
		voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
