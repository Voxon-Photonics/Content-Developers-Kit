#if 0
!if 1
#Visual C makefile:
empty.exe: empty.c voxiebox.h; cl /TP empty.c /Ox /MT /link user32.lib
	del empty.obj

!else

#GNU C makefile:
empty.exe: empty.c; gcc empty.c -o empty.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

#define _CRT_SECURE_NO_WARNINGS

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

 
static voxie_wind_t vw;
static voxie_frame_t vf; 

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in;
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0;
	voxie_xbox_t vx[4];
	int i;
	int ovxbut[4], vxnplays;
	int inited = 0;
	int debug = 1;


	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, (char *)"Error: can't load voxiebox.dll", (char *)"", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) // first initalise of the (&vw) voxie_wind_t activates settings from voxiebox.ini and voxie_menu_0.ini 
	{
		return (-1);
	}
	// anything before the voxie_breath while loop is called once 
	// use this space to setup your program

	// Hello world text variables - safe to remove these variables
	point3d textPos = { -1,-1,0 };
	point3d textWidth = { .1, 0,0 };
	point3d textHeight = { 0, .15, 0 };
	int textcolor = 0xffffff;


	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		// update the timers. tim is current runtime and dtim is delta time.
		otim = tim;
		tim = voxie_klock();
		dtim = tim - otim;

		/**********INPUT***********/

		// update the mouse position by applying the deltas in this frame
		mousx += in.dmousx;
		mousy += in.dmousy;
		mousz += in.dmousz;

		// check through the xbox controllers
		for (vxnplays = 0; vxnplays < 4; vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
				break; //but, lt, rt, tx0, ty0, tx1, ty1
		}

		// check for a few keyboard presses...
		if (voxie_keystat(0x1)) // esc key closes ap
		{
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}

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

		/**********INIT***********/

		if (inited == 0) // this is a run once statement which might be useful for your coding
		{
			inited = 1;
		}

		/**********DRAW***********/

		voxie_frame_start(&vf); // start the voxie frame all graphical calls must be between frame_start and frame_end
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); // set the view - the 'camera' position to capture volumetric content (usually the confines of the aspect ration set in the voxie_window

		textPos.z = cos(tim) / 5; // move the text's Z position over time using cos (a moving up and down effect)
		voxie_printalph(&vf, &textPos, &textWidth, &textHeight, textcolor, (char*)"Hello World"); // actually draw / render the text

		/*********DEBUG**********/

		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim);
		}

		voxie_frame_end(); // end of the voxie frame 

		voxie_getvw(&vw); // update the local voxie_window data with the one being used by the DLL
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif

