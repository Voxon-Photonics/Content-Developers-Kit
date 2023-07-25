#if 0
!if 1
#Visual C makefile:
VoxieDrawMesh.exe: VoxieDrawMesh.c voxiebox.h; cl /TP VoxieDrawMesh.c /Ox /MT /link user32.lib
	del VoxieDrawMesh.obj

!else

#GNU C makefile:
VoxieDrawMesh.exe: VoxieDrawMesh.c; gcc VoxieDrawMesh.c -o VoxieDrawMesh.exe -pipe -O3 -s -m64

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

/*
 Example of using Voxie_DrawMeshTex

*/

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	static voxie_wind_t vw;
	static voxie_frame_t vf; 


	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i;
	int inited = 0; 
	int debug = 1;


	// mesh variables
	poltex_t verticeArray[6];
	int meshArray[40];
	int fillMode = 2; // 0 dots, 1 = lines, 2 = surface, 3 = solid

	int meshCount = 0;
	int vertCount = 0; 
	int vertexShadingFlag = 16;

	int totalMeshNo = 0;
	int totalVerticeNo = 0;
	int meshDrawMode = 0;


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

		/**************************
		*  INIT                   *
		*                         *
		**************************/	

		if (inited == 0)
		{
			// mesh for a diamond -- with vertex coloring
			verticeArray[vertCount].x = -0.3; verticeArray[vertCount].y = -0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; // LTU
			vertCount++;
			verticeArray[vertCount].x = 0.3; verticeArray[vertCount].y = -0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000;  verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 0; //  RTU
			vertCount++;
			verticeArray[vertCount].x = +0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000;  verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; // RBU
			vertCount++;
			verticeArray[vertCount].x = -0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000;  verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 1;// LBU

			vertCount++;
			// top middle point
			verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = -0.3; verticeArray[vertCount].col = 0xff00ff;  verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; // top middle

			vertCount++;
			// bottom middle point
			verticeArray[vertCount].x = 0; verticeArray[vertCount].y = 0; verticeArray[vertCount].z = 0.3; verticeArray[vertCount].col = 0x00ff00; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; // bottom middle

			// make up the triangles for the mesh

			//draw top spike
			meshArray[meshCount++] = 0; meshArray[meshCount++] = 4; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 1; meshArray[meshCount++] = 4; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 2; meshArray[meshCount++] = 4; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 3; meshArray[meshCount++] = 4; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;

			// draw the base
			meshArray[meshCount++] = 0; meshArray[meshCount++] = 1; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 2; meshArray[meshCount++] = 3; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1; 

			// draw the bottom spike

			meshArray[meshCount++] = 0; meshArray[meshCount++] = 5; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1; 
			meshArray[meshCount++] = 1; meshArray[meshCount++] = 5; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 2; meshArray[meshCount++] = 5; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
			meshArray[meshCount++] = 3; meshArray[meshCount++] = 5; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;

			// workout counts and draw mode...
			totalVerticeNo = vertCount + 1;
			totalMeshNo = meshCount;
			meshDrawMode = vertexShadingFlag + fillMode; // remove the vertexShadingFlag variable to just show the texture



			inited = 1;
		}


		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 


		// send a blank char * "" as a texture name for no texture. 
		voxie_drawmeshtex(&vf, "pattern4.png", verticeArray, totalVerticeNo, meshArray, totalMeshNo,meshDrawMode, 0x404040); // color 0x404040 uses the model's natural colors


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
			voxie_debug_print6x8_(30, 78, 0xffc080, -1, "vertices : %d mesh : %d", totalVerticeNo, totalMeshNo); 
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
