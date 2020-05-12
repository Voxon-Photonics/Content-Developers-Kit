#if 0
!if 1
#Visual C makefile:
starField.exe: starField.c voxiebox.h; cl /TP starField.c /Ox /MT /link user32.lib
	del starField.obj

!else

/* A simple star field simulator by Matt Vecchio / Ready Wolf for Voxon 
feel free to use the code in your examples
*/ 


#GNU C makefile:
starField.exe: starField.c; gcc starField.c -o starField.exe -pipe -O3 -s -m64

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


typedef struct { point3d pos; float rad; int col, fill; } star_t;
#define STARMAX 100
static star_t stars[STARMAX];
static int starNo = 0;

//Rotate vectors a & b around their common plane, by ang

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
	voxie_inputs_t in; 
	pol_t pt[3];
		double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	voxie_xbox_t vx[4]; 
	int i;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a single breath renders the whole volume
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 
		for (vxnplays = 0; vxnplays < 4; vxnplays++) 
		{
			ovxbut[vxnplays] = vx[vxnplays].but; 

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
			break; //but, lt, rt, tx0, ty0, tx1, ty1
		}


		if (voxie_keystat(0x1)) // esc key closes app
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

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

	/**************************
	*  START OF INNIT PHASE   *
	*                         *
	**************************/

		if (inited == 0)
		{
			for(i =  0; i < STARMAX - 1; i++)	stars[i].pos.z = vw.aspz + 1;
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

	//draw stars
	for (i = 0; i < STARMAX - 1; i++ ) {	
		stars[i].pos.z += dtim / 50; // the vertical movement of the stars
		// vw.aspx is max X usually 1.0
		// vw.aspy is max Y usually 1.0
		// vw.aspx is max Z usually 0.4
		voxie_drawsph(&vf,stars[i].pos.x,stars[i].pos.y,stars[i].pos.z,stars[i].rad,stars[i].fill,stars[i].col);

		if (stars[i].pos.z > vw.aspz) {
			stars[i].pos.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
			stars[i].pos.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
			stars[i].pos.z = ((float)((rand()&32767)-16384)/16384.f*vw.aspz) ;
		//	stars[i].pos.z = (float)((rand()&32767)-16384)/16384.f*(vw.aspz - (vw.aspz / 3)) - (vw.aspz / 2); // cull data to the top third of the screen


			stars[i].rad = fabs((float)(((rand() & 32767) - 16384) / 16384.f) / 70) ;
			
			if (i % 2) stars[i].col = 0x00ffff; // if index is odd
			else stars[i].col = 0xffffff; // if index is even
			if (rand() % 3 == 1)  stars[i].fill = 0;
			else stars[i].fill = 1;

			//stars[i].col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // random colour

		}	
	}

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
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
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
