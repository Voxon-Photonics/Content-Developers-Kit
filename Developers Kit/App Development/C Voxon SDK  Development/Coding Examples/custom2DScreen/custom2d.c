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
#define PI 3.14159265358979323

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


   	// tiletype - 2D array of pixels
   	//  f: pointer to top-left pixel
   	//  p: pitch - number of bytes per horizontal line (usually x*4 but may be higher or negative)
   	//x,y: image dimensions
  	//typedef struct { INT_PTR f, p, x, y; } tiletype;

 	//Hi-level image decode (uses kzfile_t internally):
	//extern int kpzload (const char *filnam, INT_PTR *fptr, INT_PTR *bpl, INT_PTR *xsiz, INT_PTR *ysiz);

	// how to create a tile type to show onto the 2D screen.
	tiletype crab2d; 													// declare new tiletype type
	crab2d.x = 16;														// get the image's x dimension (check the details in windows explorer)
	crab2d.y = 16;														// get the image's y dimension (check the details in windows explorer)
	crab2d.p = (crab2d.x<<2);											// define the pitch number of bytes per horizontal line (usually x*4 but may be higher or negative)
	crab2d.f = (INT_PTR)malloc(crab2d.p*crab2d.y);						// create pointer to 1st pixel
	kpzload("2dCrab0.png", &crab2d.f, &crab2d.p, &crab2d.x, &crab2d.y); // load the image into file memory.

	// how to create a tile type to show onto the 2D screen.
	tiletype balls; 													// make new tiletype type
	balls.x = 800;														// get the image's x Dimension
	balls.y = 600;														// get the image's y Dimension
	balls.p = (balls.x<<2);											    // define the pitch number of bytes per horizontal line (usually x*4 but may be higher or negative)
	balls.f = (INT_PTR)malloc(balls.p*balls.y);							// create pointer to 1st pixel
	kpzload("balls.jpg", &balls.f, &balls.p, &balls.x, &balls.y); 		// load the image into file memory pass in the pointers

	tiletype sampleImg;
	sampleImg.x = 272;
	sampleImg.y = 170;
	sampleImg.p = (sampleImg.x<<2);
	sampleImg.f = (INT_PTR)malloc(sampleImg.p*sampleImg.y);	
	kpzload("sample.png", &sampleImg.f, &sampleImg.p, &sampleImg.x, &sampleImg.y); 	

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;

		if (in.bstat == 1) mousPing = 0;

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		if (mousPing < 100 ) {
			mousPing++; 
		}


		/////////////////////////////////////////////////////////////////// DRAW

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
		
		// New Debug Draw Function - Draw texture to secondary screen. "voxie_debug_drawtile"
	   	//  src: source texture. NOTE: Alpha bytes (bits 24-31) of source pixels control what's drawn:
 	  	//        0: fully transparent (pixel is ignored)
	  	//    1-254: blending in between (NOTE: will render slower than 0 or 255)
 	  	//      255: fully opaque (pixel is copied to screen)
	   	//x0,y0: top-left corner in pixels. Tile must be fully within bounds of screen.
		//void voxie_debug_drawtile (tiletype *src, int x0, int y0);

		voxie_debug_drawtile (&crab2d, mousx, 500);
		voxie_debug_drawtile (&crab2d, 150, 150);
		voxie_debug_drawtile (&balls, 800, 400);
		voxie_debug_drawtile (&sampleImg, 850, 75);

		voxie_debug_print6x8_(30, 80, 0xffffff, -1, "Press 'F1' to show / disable the custom help background"); 	

		////////////////////////////////////////////////////////////////// DEBUG
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
