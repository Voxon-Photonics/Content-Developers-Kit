#if 0
kVoxieDrawMeshExt.exe: kVoxieDrawMeshExt.c voxiebox.h; cl /TP kVoxieDrawMeshExt.c /Ox /MT /link user32.lib
	del kVoxieDrawMeshExt.obj
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define PI atan(1.0)*4.0

/* voxie_drawmeshtex and voxie_drawmeshtex_ext examples
 * How to setup and draw a mesh from scratch. 
 * By Ken S and Mat V for Voxon.
 * 4 / 10 / 2023 
*/

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in;
	static voxie_wind_t vw;
	static voxie_frame_t vf;
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i, drawMode = 0/*0=voxie_drawmeshtex(), 1=voxie_drawmeshtex_ext()*/, vertexShadingFlag = 16;

	if (voxie_load(&vw) < 0) { MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); return (-1); }
	if (voxie_init(&vw) < 0) { return (-1); }
	int tex_ext_tri = (1<<31);
	//-----------------------------------
		//voxie_drawmeshtex() setup:
	poltex_t verticeArray[8]; int vertCount;
	int meshArray[40], meshCount;

		 //voxie_drawmeshtex() -- draw a diamond 

		 // define vertices
	vertCount = 0;
	verticeArray[vertCount].x =-0.3; verticeArray[vertCount].y =-0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //LTU
	verticeArray[vertCount].x =+0.3; verticeArray[vertCount].y =-0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 0; vertCount++; //RTU
	verticeArray[vertCount].x =+0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //RBU
	verticeArray[vertCount].x =-0.3; verticeArray[vertCount].y = 0.3; verticeArray[vertCount].z = 0.0; verticeArray[vertCount].col = 0xff0000; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 1; vertCount++; //LBU
	verticeArray[vertCount].x =   0; verticeArray[vertCount].y =   0; verticeArray[vertCount].z =-0.3; verticeArray[vertCount].col = 0xff00ff; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //top mid
	verticeArray[vertCount].x =   0; verticeArray[vertCount].y =   0; verticeArray[vertCount].z =-0.3; verticeArray[vertCount].col = 0xff00ff; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //top mid
	verticeArray[vertCount].x =   0; verticeArray[vertCount].y =   0; verticeArray[vertCount].z = 0.3; verticeArray[vertCount].col = 0x00ff00; verticeArray[vertCount].u = 0; verticeArray[vertCount].v = 0; vertCount++; //bot mid
	verticeArray[vertCount].x =   0; verticeArray[vertCount].y =   0; verticeArray[vertCount].z = 0.3; verticeArray[vertCount].col = 0x00ff00; verticeArray[vertCount].u = 1; verticeArray[vertCount].v = 1; vertCount++; //bot mid

		// draw mesh use a -1 to start a new face
	meshCount = 0;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 5; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1; //draw top spike
	meshArray[meshCount++] = 1; meshArray[meshCount++] = 4; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 4; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 3; meshArray[meshCount++] = 5; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 1; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1; //draw base
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 3; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 0; meshArray[meshCount++] = 7; meshArray[meshCount++] = 1; meshArray[meshCount++] = -1; //draw bot spike
	meshArray[meshCount++] = 1; meshArray[meshCount++] = 6; meshArray[meshCount++] = 2; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 2; meshArray[meshCount++] = 6; meshArray[meshCount++] = 3; meshArray[meshCount++] = -1;
	meshArray[meshCount++] = 3; meshArray[meshCount++] = 7; meshArray[meshCount++] = 0; meshArray[meshCount++] = -1;
	//-----------------------------------
		//voxie_drawmeshtex_ext() setup:
	point3d verticeXYZ[8]; int vXYZCount;
	uvcol_t verticeUVC[8]; int vUVCCount;
	inds_t indices[40]; int indCount;

		//voxie_drawmeshtex_ext() -- draw a diamond

		// define vertice positions and vertice U,V and color values spread over 2 different arrays.
	vXYZCount = 0; vUVCCount = 0;
	verticeXYZ[vXYZCount].x =-0.3; verticeXYZ[vXYZCount].y =-0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //LTU
	verticeXYZ[vXYZCount].x =+0.3; verticeXYZ[vXYZCount].y =-0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 0; vUVCCount++; //RTU
	verticeXYZ[vXYZCount].x =+0.3; verticeXYZ[vXYZCount].y = 0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //RBU
	verticeXYZ[vXYZCount].x =-0.3; verticeXYZ[vXYZCount].y = 0.3; verticeXYZ[vXYZCount].z = 0.0; vXYZCount++; verticeUVC[vUVCCount].col = 0xff0000; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 1; vUVCCount++; //LBU
	verticeXYZ[vXYZCount].x =   0; verticeXYZ[vXYZCount].y =   0; verticeXYZ[vXYZCount].z =-0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0xff00ff; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //top mid
	verticeXYZ[vXYZCount].x =   0; verticeXYZ[vXYZCount].y =   0; verticeXYZ[vXYZCount].z =-0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0xff00ff; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //top mid
	verticeXYZ[vXYZCount].x =   0; verticeXYZ[vXYZCount].y =   0; verticeXYZ[vXYZCount].z = 0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0x00ff00; verticeUVC[vUVCCount].u = 0; verticeUVC[vUVCCount].v = 0; vUVCCount++; //bot mid
	verticeXYZ[vXYZCount].x =   0; verticeXYZ[vXYZCount].y =   0; verticeXYZ[vXYZCount].z = 0.3; vXYZCount++; verticeUVC[vUVCCount].col = 0x00ff00; verticeUVC[vUVCCount].u = 1; verticeUVC[vUVCCount].v = 1; vUVCCount++; //bot mid

		// use the inds_t struct to set define the faces / indices the xyz and col values
		// the difference with using drawmeshtex_ext is that instead of using a -1 at the end of the face you add a 1 to the last bit of the 32 bit interger ( +(1<<31) ) to signitfy the start of a new face / polygon index loop.
		// this could be assigned to a variable aka int new_face = (1<<31);
	indCount = 0;
	indices[indCount++].xyzi = 0+(1<<31); indices[indCount++].xyzi = 5; indices[indCount++].xyzi = 1; //draw top spike
	indices[indCount++].xyzi = 1+(1<<31); indices[indCount++].xyzi = 4; indices[indCount++].xyzi = 2;
	indices[indCount++].xyzi = 2+(1<<31); indices[indCount++].xyzi = 4; indices[indCount++].xyzi = 3;
	indices[indCount++].xyzi = 3+(1<<31); indices[indCount++].xyzi = 5; indices[indCount++].xyzi = 0;
	indices[indCount++].xyzi = 0+(1<<31); indices[indCount++].xyzi = 1; indices[indCount++].xyzi = 2; //draw base
	indices[indCount++].xyzi = 2+(1<<31); indices[indCount++].xyzi = 3; indices[indCount++].xyzi = 0;
	indices[indCount++].xyzi = 0+(1<<31); indices[indCount++].xyzi = 7; indices[indCount++].xyzi = 1; //draw bot spike
	indices[indCount++].xyzi = 1+(1<<31); indices[indCount++].xyzi = 6; indices[indCount++].xyzi = 2;
	indices[indCount++].xyzi = 2+(1<<31); indices[indCount++].xyzi = 6; indices[indCount++].xyzi = 3;
	indices[indCount++].xyzi = 3+(1<<31); indices[indCount++].xyzi = 7; indices[indCount++].xyzi = 0;

	// this simple for loop just copies the indices indexes to the uvci values and set the 32nd bit to be corrected 
	for(i=0;i<indCount;i++) { indices[i].uvci = indices[i].xyzi&~(1<<31); }
	//-----------------------------------

	while (!voxie_breath(&in))
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim;

		if (voxie_keystat(0x1)) voxie_quitloop();

		i = (voxie_keystat(0x1b)&1) - (voxie_keystat(0x1a)&1); // keys '[' and ']'
		if (i)
		{
				  if (voxie_keystat(0x2a) | voxie_keystat(0x36)) vw.emuvang = min(max(vw.emuvang + (float)i * dtim * 2.0, -PI * .5), 0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d) | voxie_keystat(0x9d)) vw.emudist = max(vw.emudist - (float)i * dtim * 2048.0, 400.0); //Ctrl+[,]
			else                                                vw.emuhang += (float)i * dtim * 2.0; //[,]
			voxie_init(&vw);
		}

		if (voxie_keystat(0x02) == 1) drawMode = 0;
		if (voxie_keystat(0x03) == 1) drawMode = 1;
		if (voxie_keystat(0x06) == 1) vertexShadingFlag = 16;
		if (voxie_keystat(0x07) == 1) vertexShadingFlag = 0;

		voxie_frame_start(&vf);
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz);

		if (!drawMode) voxie_drawmeshtex    (&vf, "pattern.png", verticeArray,vertCount,
																					meshArray,meshCount, 2 + vertexShadingFlag, 0x404040);
					 else voxie_drawmeshtex_ext(&vf, "pattern.png", verticeXYZ,vXYZCount,
																					verticeUVC,vUVCCount,
																					indices,indCount, 2 + vertexShadingFlag, 0x404040);

			//Drawing A textured QUAD using drawmeshtex ext
		{
		float xf, yf, zf = 0, xSize = 0.5f, ySize = 0.5f;
		inds_t quadInd[5];
		uvcol_t quadUVC[4];
		point3d quadPos[4];

		xf = -0.9f; xSize = 0.5f;
		yf = -0.9f; ySize = 0.5f; zf = 0.f;
		quadPos[0].x = xf;           quadPos[0].y = yf;                   quadPos[0].z = zf;
		quadPos[1].x = xf + xSize;   quadPos[1].y = yf;                   quadPos[1].z = zf;
		quadPos[2].x = xf + xSize;   quadPos[2].y = yf + (ySize * 1.53);  quadPos[2].z = zf;
		quadPos[3].x = xf;           quadPos[3].y = yf + (ySize * 1.53);  quadPos[3].z = zf;
		quadUVC[0].u = 0.f;          quadUVC[0].v = 0.f;                  quadUVC[0].col = 0xff0000; //LEF-TOP
		quadUVC[1].u = 1.f;          quadUVC[1].v = 0.f;                  quadUVC[1].col = 0x00ff00; //RIG-TOP
		quadUVC[2].u = 1.f;          quadUVC[2].v = 1.f;                  quadUVC[2].col = 0x00ffff; //RIG-BOT
		quadUVC[3].u = 0.f;          quadUVC[3].v = 1.f;                  quadUVC[3].col = 0xffff00; //LEF-BOT

		quadInd[0].xyzi = 0+(1<<31); quadInd[1].xyzi = 1;                 quadInd[2].xyzi = 2;          quadInd[3].xyzi = 3;
		quadInd[0].uvci = 0;         quadInd[1].uvci = 1;                 quadInd[2].uvci = 2;          quadInd[3].uvci = 3;

		voxie_drawmeshtex_ext(&vf, "plane.png",quadPos,4,quadUVC,4,quadInd,4,2 + vertexShadingFlag,0x404040);
		}

		voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
		avgdtim += (dtim - avgdtim) * .1; 
		voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); //display VPS
		voxie_debug_print6x8_(30, 80, 0xffc080, -1, "ext: %d", drawMode); // display stats
		voxie_debug_print6x8_(30, 95, 0x00ff00, -1, "DrawmeshTex and DrawmeshTex_ext examples\n\nPress 1 & 2 to switch between draw types. \nPress 3 & 4 to toggle vertex shading"); //instructions 

		voxie_frame_end(); voxie_getvw(&vw);
	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif
