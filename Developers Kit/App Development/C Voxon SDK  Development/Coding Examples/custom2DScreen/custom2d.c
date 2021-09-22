#if 0
!if 1
#Visual C makefile:
custom2d.exe: custom2d.c voxiebox.h; cl /TP custom2d.c /Ox /MT /link user32.lib
	del custom2d.obj

!else

#GNU C makefile:
custom2d.exe: custom2d.c; gcc custom2d.c -o custom2d.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

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

#define DEBUG 1

static int menu_update (int id, char *st, double v, int how, void *userdata)
{
 return 1;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	voxie_xbox_t vx[4]; 
	int i;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int mousPing = 0;;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	// To enable a custom background when you call 'voxie_menu_reset' pass in a filename of an image to set as background.
	// 1:1 pixel mapping
	// Press 'f1' to toggle on / off by default 'showhelp' in voxiebox.ini is set to '-1' which means the background is 
	// disabled on system launch when running on the emulator and enabled on hardware. Change this value to 1 to always 
	// be on or 0 to always be off on launch
	voxie_menu_reset(menu_update,0,"background.jpg");

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;

		if (in.bstat == 1) mousPing = 0;

		if (voxie_keystat(0x1)) // esc key closes app
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		
	
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

		/**************************
		*   DRAW                  *
		*                         *
		**************************/
		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

		// these are the various graphical calls you can make to the 2D screen. They must be called between 
		//voxie_frame_start() and voxie_frame_end() functions

		voxie_debug_drawcircfill(650,200,mousPing,0xff0000); // draws a filled circle
		voxie_debug_drawcirc(650,200,mousPing,0x00000); // draws a circle
		voxie_debug_drawcirc(650,200,mousPing - 1,0x303030);

		voxie_debug_drawrectfill(60,210,510,610,0x15006E); // draws a rectangle
		voxie_debug_drawrectfill(50,200,500,600,0x3000dd);

		voxie_debug_drawline(50,200,500,200,0x6030ff); // draws a line
		voxie_debug_drawline(50,200,50,600,0x6030ff);

		voxie_debug_drawhlin(mousx - 10, mousx + 10, mousy, 0x00ff00); // draws horizontal line
		voxie_debug_drawline(mousx, mousy - 9, mousx, mousy + 11, 0x00ff00); 
		voxie_debug_drawpix(mousx,mousy,0xffffff); // draws a single pixel

		voxie_debug_print6x8_(150,202,0xffff00,0x000000,"Title Message"); // types out a message
		voxie_debug_print6x8_(150,210,0xffffff,-1,"show int: %d, show float: %1.5f", mousPing, tim); // types out a message
		
		if (mousPing < 100 ) {
			mousPing++; 
		}

		voxie_debug_print6x8_(30, 80, 0xffffff, -1, "Press 'F1' to show / disable the custom background"); 	
		/**************************
		*   DEBUG                 *
		*                         *
		**************************/

#if DEBUG	
		//draw wireframe box around the edge of the screen
		voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

		//display VPS
		avgdtim += (dtim - avgdtim) * .1;
		voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 	
#endif

	voxie_frame_end(); 
	
	voxie_getvw(&vw);

	}


	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
