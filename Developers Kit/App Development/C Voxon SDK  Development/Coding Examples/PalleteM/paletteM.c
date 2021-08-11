// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#define WINDOWS_IGNORE_PACKING_MISMATCH
#define _CRT_SECURE_NO_WARNINGS
#if 0
!if 1



#Visual C makefile:
paletteM.exe: paletteM.c voxiebox.h; cl /TP paletteM.c /Ox /MT /link user32.lib
	del paletteM.obj

!else

#GNU C makefile:
paletteM.exe: paletteM.c; gcc paletteM.c -o paletteM.exe -pipe -O3 -s -m64

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

bool gIsCMK = false;

#define COLOUR_MAX 16
typedef struct { int value; char *name; } palette_t;
static palette_t palette[COLOUR_MAX];
int paletteNo = 0;

enum menu // menu names
{

	MENU_CMK_ON, MENU_RGB_ON, MENU_EXPORT,

};

static int menu_update (int id, char *st, double v, int how, void *userdata)
{

	switch(id)
	{
		case MENU_CMK_ON: 

			vw.usecol = 1; vw.ilacemode = 6; vw.sensemask[0] = 0xff0000; vw.outcol[0] = 0x00ffff;
			vw.sensemask[1] = 0x00ff00; vw.outcol[1] = 0xff00ff;
			vw.sensemask[2] = 0x0000ff; vw.outcol[2] = 0xffff00; voxie_init(&vw);
			voxie_init(&vw);
			gIsCMK = true;

		break;
		case MENU_RGB_ON:

			vw.usecol = 1; vw.ilacemode = 6; vw.sensemask[0] = 0xff0000; vw.outcol[0] = 0xff0000;
			vw.sensemask[1] = 0x00ff00; vw.outcol[1] = 0x00ff00;
			vw.sensemask[2] = 0x0000ff; vw.outcol[2] = 0x0000ff; voxie_init(&vw);
			voxie_init(&vw);
			gIsCMK = false;

		break;
		case MENU_EXPORT:
		{
			FILE *file;
			int i;
			file = fopen("Export.txt", "w");
			if (file == NULL) return false;
			if (gIsCMK == true)      fprintf(file, "Colour mode CMK \n" );
			else                fprintf(file, "Colour mode RGB \n" );

			for (i = 0; i < paletteNo; i++) {
				fprintf(file, "#DEFINE %s %06X \n", palette[i].name, palette[i].value  );
			}

			fclose(file);
		}
		break;
	}
	return(1);
}

	//Rotate vectors a & b around their common plane, by ang
static void rotvex(float ang, point3d *a, point3d *b)
{
	float f, c, s;
	int i;

	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f * c + b->x * s; b->x = b->x * c - f * s;
	f = a->y; a->y = f * c + b->y * s; b->y = b->y * c - f * s;
	f = a->z; a->z = f * c + b->z * s; b->z = b->z * c - f * s;
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
	int debug = 1;
	float f,g;
	int colR = 0x40,colG = 0x40,colB = 0x40, colVal = 0;
	const float cubeScale = .25f;
	int cursor = 0;
	int cursorCol = 0xFFFFFF;
	int cursorYmov = -1;
	double cursorColDly = 0;
	double colSpeed = 0.00001;
	double colAmount = 1;



	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}

	if (vw.nblades > 0 ) { //Check if a VX1 or Spinner display.
	vw.clipshape = 1;
	}

	if (voxie_init(&vw) < 0) //Start video and audio.
	{
		return (-1);
	}

	voxie_menu_reset(menu_update,0,0);

	voxie_menu_addtab((char*)"Palette", 512, 0, 400, 150);
	voxie_menu_additem((char*)"Colour Switch", 25, 25, 64, 64, 0, MENU_TEXT, 0, 0x00FF80, 0.0, 0.0, 0.0, 0.0, 0.0); // adding menu text
	voxie_menu_additem((char*)"CMK", 25, 75, 60, 50,  MENU_CMK_ON, MENU_BUTTON + 1, 0, 0x808080, 0.0, 0.0, 0.0, 0.0, 0.0);
	voxie_menu_additem((char*)"RGB", 100, 75, 60, 50, MENU_RGB_ON, MENU_BUTTON + 2, 1, 0x808080, 0.0, 0.0, 0.0, 0.0, 0.0);

	voxie_menu_additem((char*)"Export", 250, 25, 100, 100, MENU_EXPORT, MENU_BUTTON + 3, 0, 0x8080E0, 0.0, 0.0, 0.0, 0.0, 0.0);

		// create colours
	palette[0].name = "COLOUR_ONE";     palette[0].value = 0xFF0000;
	palette[1].name = "COLOUR_TWO";   palette[1].value = 0x00FF00;
	palette[2].name = "COLOUR_THREE";    palette[2].value = 0x0000FF;
	palette[3].name = "COLOUR_FOUR";  palette[3].value = 0xFFFF00;
	palette[4].name = "COLOUR_FIVE";    palette[4].value = 0x00FFFF;
	palette[5].name = "COLOUR_SIX"; palette[5].value = 0xFF00FF;
	palette[6].name = "COLOUR_SEVEN";   palette[6].value = 0xFFFFFF;
	paletteNo = 7;

	/**************************
	*  START OF UPDATE LOOP   *
	*                         *
	**************************/

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendrered in a single breath
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

		voxie_frame_start(&vf);
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz);

		/**************************
		*  INPUT                  *
		*                         *
		**************************/

		if (voxie_keystat(0xcb) == 1 && cursor > 0) cursor--;            // move cursor left
		if (voxie_keystat(0xcd) == 1 && cursor < paletteNo-1) cursor++;  // move cursor right
		if (voxie_keystat(0xc8) == 1 && cursor - cursorYmov >= 0           && cursorYmov != -1 ) cursor -= cursorYmov; // move cursor up
		if (voxie_keystat(0xd0) == 1 && cursor + cursorYmov <= paletteNo-1 && cursorYmov != -1 ) cursor += cursorYmov; // move cursor down

		// space bar apply colour
		if (voxie_keystat(0x39) == 1) { palette[cursor].value = colVal; cursorCol = 0xFF0000; cursorColDly = tim +  colSpeed * 10; }


		f = colAmount;
		if (voxie_keystat(0x2a)) { f = colAmount * 3;}

		if (voxie_keystat(0x10) && colR < 0xFF && cursorColDly < tim) { colR += f; cursorColDly = tim + colSpeed; } // Q red colour up
		if (voxie_keystat(0x11) && colG < 0xFF && cursorColDly < tim) { colG += f; cursorColDly = tim + colSpeed; } // W green colour up
		if (voxie_keystat(0x12) && colB < 0xFF && cursorColDly < tim) { colB += f; cursorColDly = tim + colSpeed; } // E blue colour up
		if (voxie_keystat(0x1e) && colR > 0x00 && cursorColDly < tim) { colR -= f; cursorColDly = tim + colSpeed; } // A red colour down
		if (voxie_keystat(0x1f) && colG > 0x00 && cursorColDly < tim) { colG -= f; cursorColDly = tim + colSpeed; } // S green colour down
		if (voxie_keystat(0x20) && colB > 0x00 && cursorColDly < tim) { colB -= f; cursorColDly = tim + colSpeed; } // D blue colour down

		if (colR > 0xFF) colR = 0xFF;
		if (colG > 0xFF) colG = 0xFF;
		if (colB > 0xFF) colB = 0xFF;
		if (colR < 0x00) colR = 0x00;
		if (colB < 0x00) colG = 0x00;
		if (colB < 0x00) colB = 0x00;




		/**************************
		*   LOGIC                 *
		*                         *
		**************************/
		colVal = (colR << 16) | (colG << 8) | (colB);

		if (cursorColDly < tim) { cursorCol = 0xFFFFFF; }

		/*
		b = (colour & 0xFF);
		g = (colour >> 8) & 0xFF;
		r = (colour >> 16) & 0xFF;
		return (r << 16) | (g << 8) | (b);
		*/


		/**************************
		*   DRAW                  *
		*                         *
		**************************/

		// draw cubes
		pp.x = -vw.aspx + (cubeScale);
		pp.y = -vw.aspy + (cubeScale);
		pp.z = -0.02;

		rr.x = cubeScale;
		rr.y = 0;
		rr.z = 0;

		dd.x = 0;
		dd.y = cubeScale;
		dd.z = 0;
		
		ff.x = 0;
		ff.y = 0;
		ff.z = cubeScale;
		
		g = cubeScale *.75; // cursor size
		cursorYmov = -1;

		for (i = 0; i < paletteNo; i++) {

				// draw cursor
			if (cursor == i)
			{
				voxie_drawbox(&vf,pp.x - g, pp.y - g, pp.z - g, pp.x + g, pp.y + g, pp.z + g, 1, cursorCol);
				//voxie_drawspr(&vf, "cube.obj", &pp, &rr, &dd, &ff,  colVal);
			} //else {
				voxie_drawspr(&vf, "cube.obj", &pp, &rr, &dd, &ff,  palette[i].value);
				//voxie_drawspr(&vf, "caco.kv6", &pp, &rr, &dd, &ff,  palette[i].value);
				//voxie_drawsph(&vf,pp.x, pp.y, pp.z, cubeScale*.75f, 0, palette[i].value);
			//}

			pp.x += (cubeScale * 1.5);
			if (pp.x + (cubeScale) > vw.aspx)
			{
				pp.x = -vw.aspx + (cubeScale);
				pp.y += cubeScale * 1.5;
				if (cursorYmov == -1) cursorYmov = i + 1;
			}
		}

		pp.x = -vw.aspx + (cubeScale);
		pp.y = vw.aspy - (cubeScale);
		pp.z =  0;
		voxie_drawsph(&vf, pp.x, pp.y, pp.z, cubeScale /2, 1, palette[cursor].value);
		pp.x += cubeScale * 1.5;
		voxie_drawbox(&vf, pp.x - cubeScale * .5, pp.y - cubeScale * .5, pp.z - cubeScale * .5, pp.x + cubeScale * .5, pp.y + cubeScale * .5, pp.z + cubeScale * .5, 3, palette[cursor].value);
		pp.x += cubeScale * 1.5;
		voxie_drawbox(&vf, pp.x - cubeScale * .5, pp.y - cubeScale * .5, pp.z - cubeScale * .5, pp.x + cubeScale * .5, pp.y + cubeScale * .5, pp.z + cubeScale * .5, 2, palette[cursor].value);
		//voxie_drawcube(&vf, &pp, &rr, &dd, &ff, 2, palette[cursor].value);
		pp.x += cubeScale * 1.5;
		voxie_drawcone(&vf,pp.x,pp.y + cubeScale *.5,pp.z,cubeScale  *.5,pp.x, pp.y  - cubeScale *.5, pp.z, cubeScale * 0.1, 2, palette[cursor].value);
		pp.x += cubeScale;
		f = cubeScale * .5;
		rr.x = f / 2;
		rr.y = 0;
		rr.z = 0;
		dd.x = 0;
		dd.y = f;
		dd.z = 0;
		voxie_printalph(&vf,&pp, &rr, &dd, palette[cursor].value, "text");


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
			voxie_debug_print6x8_(30, 68, 0xFFFFFF, -1, "VPS %5.1f    R = %x G = %x B = %x ", 1.0 / avgdtim, colR, colG, colB);
			voxie_debug_print6x8_(30, 78, 0xFFFFFF, -1, "Press Q - W - E to increase colour | Press A - S - D to decrease colour value");
			voxie_debug_print6x8_(30, 88, 0xFFFFFF, -1, "Press Space Bar to Assign new colour");
			
			voxie_debug_print6x8_(300, 68, colVal, -1, "*******************");
			for (i = 0; i < paletteNo; i++)
			{
				voxie_debug_print6x8_(100, 100 + (i * 15), palette[i].value, -1, "%d Colour %s  %06X", i, palette[i].name, palette[i].value );
				if (i == cursor) voxie_debug_print6x8_(30, 100 + (i * 15), 0xffffff, -1, "-->" );
			}
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
