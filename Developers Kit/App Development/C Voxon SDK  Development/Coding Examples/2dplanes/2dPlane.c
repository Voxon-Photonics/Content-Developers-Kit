// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1
#Visual C makefile:
2dPlane.exe: 2dPlane.c voxiebox.h; cl /TP 2dPlane.c /Ox /MT /link user32.lib
	del 2dPlane.obj

!else

#GNU C makefile:
2dPlane.exe: 2dPlane.c; gcc 2dPlane.c -o 2dPlane.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

/* 2D planes are great from drawing 2D textures / billboards here is a example on how to render them on a volumetric display
 *
 * A demo by Matthew Vecchio for Voxon  19/05/2021
 * 
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

static voxie_wind_t vw;
static voxie_frame_t vf; 

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	point3d pp, rr, dd, ff; 
	int inited = 0; 
	int debug = 1;
	int i = 0;
	float x = 0,y = 0,z = 0;
	float sizeX = 1;
	float sizeY = 1;
	float sizeZ = 1;
	point3d posOfSet = {0,0,0};
	

	// for 2D planes you need an array of poltex_t for vertice data and an array for the mesh to be mapped to the vertice data
	poltex_t vtext[5]; // poltex_t contains vertice data which consists of positional data (x,y,z) texture data (u,v and colour value)
	int mesh[5]; // the mesh array how the vertices are connected together


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

		/**************************
		*  INIT                   *
		*                         *
		**************************/	

		// scaling and movement controls

		// Q , A , W , S , E & D keys change the size of the planes
		if (voxie_keystat(0x10)) sizeX += 0.01; // Q
		if (voxie_keystat(0x1e)) sizeX -= 0.01; // A
		if (voxie_keystat(0x11)) sizeY += 0.01; // W
		if (voxie_keystat(0x1f)) sizeY -= 0.01; // S
		if (voxie_keystat(0x12)) sizeZ += 0.01; // E
		if (voxie_keystat(0x20)) sizeZ -= 0.01; // D

		// Up, Down, Left, Right, - and +  change the position of the planes
		if (voxie_keystat(0xc8)) posOfSet.y += 0.01; // UP
		if (voxie_keystat(0xd0)) posOfSet.y -= 0.01; // UP
		if (voxie_keystat(0xcb)) posOfSet.x -= 0.01; // LEFT
		if (voxie_keystat(0xcd)) posOfSet.x += 0.01; // RIGHT
		if (voxie_keystat(0x0c) || voxie_keystat(0x4A) )  posOfSet.z -= 0.01; // - and numpad -
		if (voxie_keystat(0x0d) || voxie_keystat(0x4e) )  posOfSet.z += 0.01; // - and numpad -

		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		voxie_frame_start(&vf);  // once a frame starts all Voxon graphical calls are loaded into the frame


		// 2D plane -- along the X and Y
		/*  This is what we are drawing...
			0------1   
			|	   |
			|      |
			3------2 			<-- the numbers represent the vertice number in the vtext array


		*/


		// set the position of where the map is
		x = posOfSet.x + -.95;
		y = posOfSet.y + -.8;
		z = posOfSet.z + 0;
	
		// set colour, u and v values
		// we are setting col value to 0 as we are using a texture
		// the u and v values are the x and y dimensions of the texture. 
		// a 0 value is the start and a 1 value is the end. you can use decimal points or negative values
		// you can even have the texture repeat for example a value of '3' will repeat texture 3 times across that dimension
		// u is the x dimensions of the texture vertice 0 and 3 are on the left side and vertice 1 and 2 are on the right. 
		// v is the y dimensions of the texture vertice 0 and 1 are on the top and 3 and 2 are on the bottom 
		// note if you ever wanted to have the image mirrored on the X and Y reverse the U and V values 
		vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
		vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
		vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
		vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;

		// map / connect the vertices and create a mesh	

		/* 
			Mapping the mesh is how vertices  image is mapped across the vertice data.
			work a single direction around the vertices
			0 --> 1		  
			^	  |
			|	  v
			3 <-- 2

			when you have finished mapping the vertices put use a -1 to end the texture / mesh map

		*/

		mesh[0] = 1; mesh[1] = 2; mesh[2] = 3; mesh[3] = 0; mesh[4] = -1; // clockwise and showing the texture correctly

		// other ways to map the mesh
		//	mesh[0] = 3; mesh[1] = 1; mesh[2] = 2; mesh[3] = 0; mesh[4] = -1; // crossed X and showing the texture incorrectly
		//	mesh[0] = 1; mesh[1] = 3; mesh[2] = 2; mesh[3] = 0; mesh[4] = -1; // crossed Y and showing the texture incorrectly
		//	mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1; // counter-clockwise and showing the texture correctly 



		// set the vertices (3d points) and their distances between eachother ... 

		vtext[0].x = x;					vtext[0].y = y ;							vtext[0].z = z ;
		vtext[1].x = x + sizeX; 		vtext[1].y = y ; 							vtext[1].z = z ;
		vtext[2].x = x + sizeX;	 		vtext[2].y = y + (sizeY * 1.53) ;			vtext[2].z = z ;
		vtext[3].x = x;	 				vtext[3].y = y + (sizeY * 1.53) ; 			vtext[3].z = z ;

		voxie_drawmeshtex(&vf,"voxon.png",vtext,5,mesh,6,2,0x404040);


		// 2D plane -- along the X and Z 
		/*
		   0-----1   ** looking downward at 45' 
            \	  \     vertice 3 and 2 are actually 
             3-----2    exactly UNDERNEATH the 0 and 1 vertice

			** note : the numbers represent the vertice number in the vtext array
		*/
		/*
		you don't need to change the .col and .u and .v 
		you could render many 2D planes and just change the 
		vtext's xyz value... that is if you are happy with the texturs orientation
		*/

		/* 
		no need to change the mesh data either
		*/

		// set the position of the plane
		x = posOfSet.x + 0;
		y = posOfSet.y + -.9;
		z = posOfSet.z + -.4;


		// set the vertices 
		vtext[0].x = x;					vtext[0].y = y ;							vtext[0].z = z ;
		vtext[1].x = x + sizeX; 		vtext[1].y = y ; 							vtext[1].z = z ;
		vtext[2].x = x + sizeX;	 		vtext[2].y = y ;							vtext[2].z = z + sizeZ; 
		vtext[3].x = x;	 				vtext[3].y = y ; 							vtext[3].z = z + sizeZ;

		voxie_drawmeshtex(&vf,"voxon.png",vtext,5,mesh,6,2,0x404040);


		// 2D plane -- along the Y and Z 
		/*
		     0      looking downward at 45' 
             |\	    vertice 1 and 2 are actually 
			 | 1	underneath 0 and 3
		     | |
		     3 |
			  \2    <-- the numbers represent the vertice number in the vtext array
		*/
		/*
		you don't need to change the .col and .u and .v 
		you could render many 2D planes and just change the 
		vtext's xyz value
		*/

		/* 
		no need to change the mesh data either
		*/

		// set the position of the plane
		x = posOfSet.x + 0.5;
		y = posOfSet.y + -0.7;
		z = posOfSet.z + -0.4;

		// set the vertices 
		vtext[0].x = x;					vtext[0].y = y ;							vtext[0].z = z ;
		vtext[1].x = x; 				vtext[1].y = y ; 							vtext[1].z = z + sizeZ;
		vtext[2].x = x; 	 			vtext[2].y = y + (sizeY * 1.53) ;			vtext[2].z = z + sizeZ; 
		vtext[3].x = x;	 				vtext[3].y = y + (sizeY * 1.53) ; 			vtext[3].z = z ;

		// if we want texture to be upright (rotate the texture)  we have to edit the vtext's u and v values 
		// for this example 
		// we want vertice 0 and 1 to be the start of the X dimension for the texture which is U  
		// we want vertice 0 and 3 to be the start of the Y dimension for the texture which is V  
	
		vtext[0].u = 0; vtext[0].v = 0;           
		vtext[1].u = 0; vtext[1].v = 1;
		vtext[2].u = 1; vtext[2].v = 1;
		vtext[3].u = 1; vtext[3].v = 0;
	
		voxie_drawmeshtex(&vf,"voxon.png",vtext,5,mesh,6,2,0x404040);

		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 


		/**************************
		*   DEBUG                 *
		*                         *
		**************************/

	
		if (debug) // if debug is set to 1 display these debug messages
		{
			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
	
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 
			voxie_debug_print6x8_(30, 100, 0x00ff00, -1, "2D plane drawing demo (how to view .JPG and .PNG files on the VX1)\n\nSee 2dPlane.c for example code\n\nUse Keys Q,A,W,S,E,D to change the size of the plane\nUse the Arrow keys and - and + to move the positions"); 
			voxie_debug_print6x8_(30, 180, 0xff0000, -1, "sizeX : %1.2f, sizeY : %1.2f, sizeZ : %1.2f ", sizeX, sizeY, sizeZ); 

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
