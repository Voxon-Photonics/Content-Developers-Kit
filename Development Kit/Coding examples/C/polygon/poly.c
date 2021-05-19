#if 0
!if 1
#Visual C makefile:
poly.exe: poly.c voxiebox.h; cl /TP poly.c /Ox /MT /link user32.lib
	del poly.obj

!else

#GNU C makefile:
poly.exe: poly.c; gcc poly.c -o poly.exe -pipe -O3 -s -m64

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

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	point3d pp, rr, dd, ff; 
	int i;

	int debug = 1;
	float f;
	int fillmode = 2;
	const float cubeScale = .25f;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 

		/**************************
		*  INPUT                  *
		*                         *
		**************************/	

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


		// press 0,1,2,3 to change fillmode type	
		if (voxie_keystat(0x02) == 1) fillmode = 1; // kb 1
		if (voxie_keystat(0x03) == 1) fillmode = 2; // kb 2
		if (voxie_keystat(0x04) == 1) fillmode = 3; // kb 3
		if (voxie_keystat(0x0b) == 1) fillmode = 0; // kb 0
		

		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 


		poltex_t vt[4]; int mesh[16], i = 0;
		vt[0].x =-0.4; vt[0].y =-0.8; vt[0].z =-0.2; vt[0].col = 0xff0000;
		vt[1].x =-0.4; vt[1].y =-0.4; vt[1].z =+0.2; vt[1].col = 0x00ff00;
		vt[2].x =+0.4; vt[2].y =-0.8; vt[2].z =+0.2; vt[2].col = 0x0000ff;
		vt[3].x =+0.4; vt[3].y =-0.4; vt[3].z =-0.2; vt[3].col = 0xff00ff;
		mesh[i++] = 0; mesh[i++] = 1; mesh[i++] = 2; mesh[i++] = -1; /*-1 = end of polygonal facet*/
		mesh[i++] = 1; mesh[i++] = 0; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 2; mesh[i++] = 1; mesh[i++] = 3; mesh[i++] = -1;
		mesh[i++] = 0; mesh[i++] = 2; mesh[i++] = 3; mesh[i++] = -1;
		voxie_drawmeshtex(&vf,0,vt,4,mesh,i,16+fillmode,0x404040); /* (for colour of vertices to work needd to have fill mode 16+ (0010000b) ) */

		voxie_drawbox(&vf, -0.9, 0, -0.4, -0.5, 0.4,0,fillmode, 0xff0000 );
	
		pp.x = -0.4; 	pp.y = 0; 		pp.z = -0.4;
		rr.x = 	0.4; 	rr.y = 0; 		rr.z = 	0;
		dd.x = 	0; 		dd.y = 0.4;		dd.z = 	0;
		ff.x = 	0; 		ff.y = 0; 		ff.z = 	0.4;

		voxie_drawcube(&vf, &pp, &rr, &dd, &ff, fillmode, 0x00ff00);

		pp.x = +0.3; 	pp.y = 0.2; 	pp.z =  -0.2;
		rr.x = 	0.4; 	rr.y = 0; 		rr.z = 	0;
		dd.x = 	0; 		dd.y = 0.4;		dd.z = 	0;
		ff.x = 	0; 		ff.y = 0; 		ff.z = 	0.4;

		voxie_drawspr(&vf, "cube.obj", &pp, &rr, &dd, &ff,  0x0000ff);
	
		voxie_drawsph(&vf, 0.8, 0.2, -0.2, .15,fillmode, 0xffff00 );

		voxie_drawlin(&vf, -0.8, -0.2, -0.4, 0.8, -0.2, 0.4,0x00ffff );

		voxie_drawcone(&vf, -0.8, 0.7, 0.2, 0.1, 0.8, 0.7, -0.2, 0.2, fillmode, 0xff00ff );


		/**************************
		*   DEBUG                 *
		*                         *
		**************************/

	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
	
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f ", 1.0 / avgdtim, fillmode); 
			voxie_debug_print6x8_(30, 128, 0x00ff80, -1, "Polygon and primative shape demo press 0 - 3 to change fillmode", 1.0 / avgdtim, fillmode); 
			voxie_debug_print6x8_(30, 138, 0x00ffff, -1, "fillmode : %d",  fillmode); 
			voxie_debug_print6x8_(30, 148, 0x00ff80, -1, "This demo also shows how to use vertice colouring"); 

			voxie_debug_print6x8_(30, 168, 0xffff80, -1, "what is being shown on the screen from top left to bottom right:\nvoxie_drawmeshtex\nvoxie_drawlin\nvoxie_drawbox\nvoxie_drawcube\nvoxie_drawspr (cube.obj)\nvoxie_drawsph\nvoxie_drawcone "); 



		}

		voxie_frame_end(); 
	
		voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
