#if 0
!if 1
#Visual C makefile:
captureVCB.exe: captureVCB.c voxiebox.h; cl /TP captureVCB.c /Ox /MT /link user32.lib
	del captureVCB.obj

!else

#GNU C makefile:
captureVCB.exe: captureVCB.c; gcc captureVCB.c -o captureVCB.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

/*
 Demo to show how to capture the volume through the Voxon API
 7/9/2023 - Matthew Vecchio for Voxon
*/

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	static voxie_wind_t vw;
	static voxie_frame_t vf; 
	voxie_inputs_t in; 

	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i;
	int ovxbut[4], vxnplays; 
	int inited = 0;
	int debug = 1;
	const char * captureFilename = "capture.zip";
	int captureVPS = 15;
	int recording = 0;
	point3d textPos = { 0,0,0};
	point3d textWidth = {0.08,0 ,0};
	point3d textHeight = {0,0.08,0};




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

		// EXIT and EMU bindings

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


		if (voxie_keystat(0x02) == 1) {                                  voxie_volcap("mycap.ply",VOLCAP_FRAME_PLY   , 0);   } //1   :)
		if (voxie_keystat(0x03) == 1) {                                  voxie_volcap("mycap.rec",VOLCAP_FRAME_REC   , 0);   } //2   :)
		if (voxie_keystat(0x04) == 1) {                                  voxie_volcap("mycap.png",VOLCAP_FRAME_PNG   , 0);   } //3   :) (NOTE: must be in UPDN mode, else ignored!)
		if (voxie_keystat(0x05) == 1) {                                  voxie_volcap("mycap.vcb",VOLCAP_FRAME_VCB   , 0);   } //4   :) (NOTE: must be in UPDN mode, else ignored!)

		if (voxie_keystat(0x06) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.rec",VOLCAP_VIDEO_REC   ,15); } } //5   :)
		if (voxie_keystat(0x07) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.zip",VOLCAP_VIDEO_PLY   , 5); } } //6   Broken - probably never worked. Creates 0by mycap.zip and single voxie0000.ply
		if (voxie_keystat(0x08) == 1) { if (!recording) { recording = 1; voxie_volcap("mycap.zip",VOLCAP_VIDEO_VCBZIP, 5); } } //7   :) (NOTE: must be in UPDN mode, else ignored!)
		if (voxie_keystat(0x39) == 1) { recording = 0;                   voxie_volcap(          0,VOLCAP_OFF         , 0);   } //Spc :)


		if (inited == 0) {

			textPos.x = -0.8;
			textPos.y = 0.7;
			textPos.z = -vw.aspz + 0.01;
			
			inited = 1;
		}
 		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 

		voxie_drawsph(&vf,cos(tim * 4) * 0.3 - 0.4, sin(tim * 4) * 0.25, 0,.3,1,0x00ff00);
		voxie_drawsph(&vf,0, 0, cos(tim) * 0.05,.25,1,0xff0000);
		voxie_drawsph(&vf,sin(tim * 4) * 0.3 + 0.4, cos(tim * 4) * 0.25, 0,.3,1,0x0000ff);
		voxie_printalph_(&vf, &textPos, &textWidth, &textHeight, 0xffffff, "Capture VPS: %d\nRuntime: %1.2f \nFName: %s", captureVPS, tim, captureFilename);

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

			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f Recording Stat: %d\n\n\nPress 1 to capture 1 frame as .PLY\nPress 2 to capture 1 frame as .REC\nPress 3 to capture PNG frame (UPDN VX1 mode only, else ignored)\nPress 4 to capture VCB frame (UPDN VX1 mode only, else ignored)\nPress 5 for .REC video\nPress 6 for for .PLY video\nPress 7 for VCB video (UPDN VX1 mode only, else ignored)\nPress Space Bar to STOP recording\n", 1.0 / avgdtim, recording); 

		
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
