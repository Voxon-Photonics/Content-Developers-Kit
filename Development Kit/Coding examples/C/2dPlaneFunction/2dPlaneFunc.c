#if 0
#MSVC cmdline makefile:
2dPlaneFunc.exe: 2dPlaneFunc.c voxiebox.h; cl /TP 2dPlaneFunc.c /Ox /GFy /MT /link user32.lib
	del 2dPlaneFunc.obj
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) < (b)) ? (a) : (b))
#endif

static voxie_wind_t vw;
static voxie_frame_t vf;

	//Rotate vectors a & b around their common plane, by ang in degrees
void rotate_vex (float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

	//! Renders a 2D quad (.png, .jpg... most image formats) onto the volumetric display. Must be called between voxie_frame_start() & voxie_frame_end().
	/**
	 * filename for texture
	 * pos = position = center of quad
	 * width = how wide
	 * height = how high
	 * hang = horizontal rotation around xy plane... in degrees (0 assumes facing towards the viewer, 180 facing back the box etc)...
	 * vang = vertical tilt (0 = horizontal plane, 90 = vertical plane)
	 * tilt = twist angle
	 * col = color
	 * u = the u value
	 * v = the v value
	 */
void drawQuad (char *filename, point3d *pos, float width, float height, float hang, float vang, float tilt, int col = 0x404040, float u = 1, float v = 1)
{
	poltex_t vt[4];
	point3d vx, vy;
	float f, g;
	int i, mesh[4+1] = {0,1,2,3,-1}, cols[4] = {0xff00ff,0x00ffff,0x0000ff,0xff00ff};

	hang *= PI/180.0;
	vang *= PI/180.0;
	tilt *= PI/180.0;

	vx.x = cos(hang);
	vx.y = sin(hang);
	vx.z = 0.f;

	f = cos(vang);
	vy.x =-vx.y*f;
	vy.y = vx.x*f;
	vy.z =-sin(vang);

	rotate_vex(tilt,&vx,&vy);

	voxie_drawsph(&vf,pos->x, pos->y, pos->z, .10, 0, 0x0000ff);
	for(i=0;i<4;i++)
	{
		if ((i+1)&2) { f = -width; vt[i].u = 0.f; } else { f = width; vt[i].u = u; }
		if (!(i&2))  { g =-height; vt[i].v = 0.f; } else { g =height; vt[i].v = v; }

		vt[i].x = vx.x*f + vy.x*g + pos->x;
		vt[i].y = vx.y*f + vy.y*g + pos->y;
		vt[i].z = vx.z*f + vy.z*g + pos->z;
		vt[i].col = 0;
		voxie_drawsph(&vf,vt[i].x,vt[i].y,vt[i].z, .10, 0, cols[i]);
	}
	voxie_drawmeshtex(&vf, filename, vt, 4, mesh, 5, 2, col);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in;
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	point3d pos = {0};
	float sizeX = .5, sizeY = .7, hang = 0, vang = 0, tilt = 0;
	int i = 0, debug = 1;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); return(-1); } // if there is an error this will create a standard Windows message box
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ return(-1); }

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; tim = voxie_klock(); dtim = tim - otim;

		if (voxie_keystat(0x1)) // esc key closes ap
		{
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}

		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1); // keys '[' and ']'
		if (i)
		{
			if (voxie_keystat(0x2a) || voxie_keystat(0x36))
				vw.emuvang = min(max(vw.emuvang + (float)i * dtim * 2.f, -PI * .5), 0.f); //Shift+[,]
			else if (voxie_keystat(0x1d) || voxie_keystat(0x9d))
				vw.emudist = min(max(vw.emudist - (float)i * dtim * 2048.f, 400.f), 4000.f); //Ctrl+[,]
			else
				vw.emuhang += (float)i * dtim * 2.f; //[,]
			voxie_init(&vw);
		}

//************************** INIT **************************
			//scaling and movement controls

			//Q,A,W,S,E,D keys change the size of the planes
		if (voxie_keystat(0x10)) sizeX += dtim*.5f; //Q
		if (voxie_keystat(0x1e)) sizeX -= dtim*.5f; //A
		if (voxie_keystat(0x11)) sizeY += dtim*.5f; //W
		if (voxie_keystat(0x1f)) sizeY -= dtim*.5f; //S

		if (voxie_keystat(0x13)) hang += dtim*64.f; //R
		if (voxie_keystat(0x14)) hang -= dtim*64.f; //T
		if (voxie_keystat(0x15)) vang += dtim*64.f; //Y
		if (voxie_keystat(0x23)) vang -= dtim*64.f; //H
		if (voxie_keystat(0x16)) tilt += dtim*64.f; //U
		if (voxie_keystat(0x17)) tilt -= dtim*64.f; //I

		if (voxie_keystat(0x35)) { hang = 0.f; vang = 0.f; tilt = 0.f; } // /

			//Arrows, - and + change the position of the planes
		if (voxie_keystat(0xcb)) pos.x -= dtim*.5f; //LEFT
		if (voxie_keystat(0xcd)) pos.x += dtim*.5f; //RIGHT
		if (voxie_keystat(0xc8)) pos.y -= dtim*.5f; //UP
		if (voxie_keystat(0xd0)) pos.y += dtim*.5f; //DOWN
		if (voxie_keystat(0x0c) || voxie_keystat(0x4a)) pos.z -= dtim*.5f; //- or numpad -
		if (voxie_keystat(0x0d) || voxie_keystat(0x4e)) pos.z += dtim*.5f; //+ or numpad +

//************************** DRAW **************************

		voxie_frame_start(&vf); //once a frame starts all Voxon graphical calls are loaded into the frame
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz);

		drawQuad("voxon.png", &pos, sizeX, sizeY, hang, vang, tilt, 0x404040, 1, 1);

//************************** DEBUG **************************

		if (debug) //if debug is set to 1 display these debug messages
		{
				//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

				//display VPS
			avgdtim += (dtim - avgdtim)*.1;

			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim);
			voxie_debug_print6x8_(30, 160, 0xff0000, -1, "pos %1.3f %1.3f %1.3f ", pos.x, pos.y, pos.z);
			voxie_debug_print6x8_(30, 180, 0xff0000, -1, "hang %1.3f vang %1.3f tilt %1.3f ", hang, vang, tilt);
			voxie_debug_print6x8_(30, 200, 0xff0000, -1, "Size %1.3f %1.3f ", sizeX, sizeY);
			voxie_debug_print6x8_(30, 280, 0x00ff00, -1, "Keys:\n\nArrows Move Pos\n-,+    Raise / Lower Pos\nR,T    Adjust Horiz ang\nY,H    Adjnst Vert ang\nU,I    Adjust Twist ang\n/      Reset angs\nQ,A    Adjust Xsize\nW,S    Adjust Ysize");
		}

		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
