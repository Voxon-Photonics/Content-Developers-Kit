#if 0
!if 1
#Visual C makefile:
VoxieFont.exe: VoxieFont.c voxiebox.h; cl /TP VoxieFont.c /Ox /MT /link user32.lib
	del VoxieFont.obj

!else

#GNU C makefile:
VoxieFont.exe: VoxieFont.c; gcc VoxieFont.c -o ark.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

/*
2D / 3D font test how to turn a 2d bitmap font and a 3d obj font unto the display

2019 9/8/2019 Matthew Vecchio / Ready Wolf

*/
static int WORLD_SCALE = 1;
static bool DEBUG_SPHERE = true;
static int gbstat = 0;

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

// these need to be accessible for the functions and the main programs
poltex_t vtext[5]; int mesh[5]; 
	float angleOFFSET = 1.5707963268f; // 90' offset due to where the objs face
// images for font2d
static  char *font2d[] =
{
	"2d0.png","2d1.png","2d2.png","2d3.png","2d4.png","2d5.png","2d6.png","2d7.png","2d8.png","2d9.png",
	"2dA.png", "2dB.png", "2dC.png", "2dD.png", "2dleftB.png", "2dRightB.png",
};

// load in font as OBJ
// This is all the chars turned into a an array which is linked to the char number 
static  char * font3d[] =
{
"BLANK 0","BLANK 1","BLANK 2","BLANK 3","BLANK 4","BLANK 5","BLANK 6","BLANK 7","BLANK 8","BLANK 9",
"BLANK 10","BLANK 11","BLANK 12","BLANK 13","BLANK 14","BLANK 15","BLANK 16","BLANK 17","BLANK 18","BLANK 19",
"BLANK 20","BLANK 21","BLANK 22","BLANK 23","BLANK 24","BLANK 25","BLANK 26","BLANK 27","BLANK 28","BLANK 29",
"BLANK 30","BLANK 31",

"BLANK SPACE","font/Exclamation_Mark.obj","BLANK COMMA",
"BLANK NUMBER", "BLANK DOLLAR", "BLANK PERCENT", "font/AND.obj","font/High_Comma.obj","font/Round_Bracket_Open.obj",
"font/Round_Bracket_Close.obj","BLANK STAR","BLANK PLUS","font/Comma.obj","font/Hyphen.obj","font/Dot.obj","font/Slash.obj",
"font/0.obj","font/1.obj","font/2.obj","font/3.obj","font/4.obj","font/5.obj","font/6.obj","font/7.obj","font/8.obj","font/9.obj",
"font/Double_Dot.obj","font/Semicolon.obj","font/Smaller_Than.obj","BLANK EQUALS","font/Greater_Than.obj","font/Question_Mark.obj",
"BLANK AT", 	

"font/A.obj","font/B.obj","font/C.obj","font/D.obj","font/E.obj","font/F.obj","font/G.obj","font/H.obj","font/I.obj",
"font/J.obj","font/K.obj","font/L.obj","font/M.obj","font/N.obj","font/O.obj","font/P.obj","font/Q.obj","font/R.obj",
"font/S.obj","font/T.obj","font/U.obj","font/V.obj","font/W.obj","font/X.obj","font/Y.obj","font/Z.obj",
"font/Round_Bracket_Open.obj","font/Slash.obj","font/Round_Bracket_Close.obj","BLANK SHIFT 6", "BLANK UNDERSCORE",
"font/High_Comma.obj",

"font/A.obj","font/B.obj","font/C.obj","font/D.obj","font/E.obj","font/F.obj","font/G.obj","font/H.obj","font/I.obj",
"font/J.obj","font/K.obj","font/L.obj","font/M.obj","font/N.obj","font/O.obj","font/P.obj","font/Q.obj","font/R.obj",
"font/S.obj","font/T.obj","font/U.obj","font/V.obj","font/W.obj","font/X.obj","font/Y.obj","font/Z.obj",  
"font/Round_Bracket_Open.obj","font/Slash.obj","font/Round_Bracket_Close.obj"  	
};

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

// calculates the angle to move plug in the position of both objects 
static float calcSeek(point3d seekFromPos,  point3d seekToPos ) {

return atan2((seekToPos.y  - seekFromPos.y), ( seekToPos.x - seekFromPos.x  )  );

}

//prints a string in 3d ori is orientation
void print3d (voxie_frame_t *vf, char *text,  point3d pos, float size, int col, int ori, bool centre ) 
{
/*
ori 0 = from the front
ori 1 = from the back
ori 2 = from the right
ori 3 = from the left
ori 4 = flip to be top down
ori 6 = use 360' angle
*/

	int i;
	float midChar, g;
	point3d dd,rr,ff,tpp = {0,0,0}, pp;
	float sizeX = size;
	float sizeY = size * 1.23;
	float sizeZ = size / 1.23;
	if (col == -1)  col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
	if (ori == 2 || ori == 3) {	
		sizeY = size; sizeX = size * 1.23;
		}
	rr.x = sizeX;  	 dd.x = 0;			ff.x = 0;
	rr.y = 0; 	 	 dd.y = sizeY; 		ff.y = 0;
	rr.z = 0; 		 dd.z = 0; 			ff.z = sizeZ;

	switch(ori) {
		case 4:
		case 0:
			break;
		case 1:
			rotvex(PI, &rr, &dd);
			size = -size;
		break;
		case 2:
			rotvex(-PI / 2, &rr, &dd);
			size = -size;
		break;		
		case 3:
			rotvex(PI / 2, &rr, &dd);
			size = size;
		break;
		default:
		case 6:
			g = calcSeek(pos, tpp) + angleOFFSET;
			rotvex(g,   &rr, &dd);
			pp.x = pos.x;
			pp.y = pos.y;
			pp.z = pos.z;
	
		break;

	}			

	// Debug sphere remove this if you don't want to see the sphere
	if (DEBUG_SPHERE) voxie_drawsph(vf, pos.x , pos.y, pos.z , .02 , 1, 0x00FF00);

	if (centre == true) {

		if (ori == 6) {
			midChar = int( strlen(text) / 2);
			for(i = 0; i < midChar; i++) {
				pp.x = pos.x -= size * cos (g);
				pp.y = pos.y -= size * sin (g); 
				pp.z = pos.z;
				g -= size - (size * .02);
			}
		}


		if (ori == 2 || ori == 3) {
				midChar = pos.y +  ( int( strlen(text) / 2) * size - (size * .02));
				pos.y -= midChar;
		} else if (ori == 0 || ori == 1 || ori == 4) {
				midChar = pos.x +  ( int( strlen(text) / 2) * size - (size * .02));
				pos.x -= midChar;
		}


	}
	
	// Debug sphere remove this if you don't want to see the sphere
	if (DEBUG_SPHERE) voxie_drawsph(vf, pos.x , pos.y, pos.z , .02 , 1, 0xFF0000);

	
	for (i = 0; i < strlen(text); i++)
	{
		if (ori == 6) {
			voxie_drawspr(vf,font3d[text[i]],&pp,&rr,&dd,&ff,col);
			
			pp.x = pos.x += size * cos (g);
			pp.y = pos.y += size * sin (g); 
			
			pp.z = pos.z;
			g += size - (size * .02);
			
		} else {
		
			if (ori == 4) voxie_drawspr(vf,font3d[text[i]],&pos,&rr,&ff,&dd,col);
			else voxie_drawspr(vf,font3d[text[i]],&pos,&rr,&dd,&ff,col);
			
			if (ori == 2 || ori == 3 ) 	pos.y += size - (size * .02); // line spacing
			else pos.x += size - (size * .02); // line spacing
		}

	}

}

// display a 5 digit score in bitmap font
void display2dNum(voxie_frame_t *vf, int value, float x, float y, float z, int ori, int col, float size) {
	vtext[0].col = 0; vtext[0].u = 0; vtext[0].v = 0;           
	vtext[1].col = 0; vtext[1].u = 1; vtext[1].v = 0;
	vtext[2].col = 0; vtext[2].u = 1; vtext[2].v = 1;
	vtext[3].col = 0; vtext[3].u = 0; vtext[3].v = 1;
	mesh[0] = 3; mesh[1] = 2; mesh[2] = 1; mesh[3] = 0; mesh[4] = -1;
	int const SCORE_SIZE = 6;

	if (col == -1)  col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
	float fontLetterSpace = size + (size *.10);
	int i = 0, j = 0;

	float sizeX = size;
	float sizeY = size;
	float yt;

	if (ori == 1) {sizeY = -sizeY; sizeX = -sizeX; fontLetterSpace = -fontLetterSpace; }
	if (ori == 2) {sizeX = -sizeX; fontLetterSpace = -fontLetterSpace; }
	if (ori == 3) {sizeY = -sizeY;  }

/*	// Ken's optimised version -- my old version is below -- using printf -- but doesn't put it in  the right position so will have to fix this later
	for(i=0;i<4;i++) { vtext[i].x = x + sizeX*vtext[i].u; vtext[i].y = y + sizeY*vtext[i].v*1.53; vtext[i].z = z; }
	char st[16]; sprintf(st,"%d",value);
	for(j=0;st[j];j++)
	{
			voxie_drawmeshtex(vf,font2d[st[j]-'0'],vtext,4,mesh,5,2,col);
		for(i=0;i<4;i++) vtext[i].x += fontLetterSpace;
	}
*/	


 // my old way of doing sizes
	for (j = 0; j < SCORE_SIZE; j++) {

		if (ori < 2) {
			vtext[0].x = x;					vtext[0].y = y ;							vtext[0].z = z ;
			vtext[1].x = x + sizeX; 		vtext[1].y = y ; 							vtext[1].z = z ;
			vtext[2].x = x + sizeX;	 		vtext[2].y = y + (sizeY * 1.53) ;			vtext[2].z = z ;
			vtext[3].x = x;	 				vtext[3].y = y + (sizeY * 1.53) ; 			vtext[3].z = z ;
		} else {
			vtext[0].y = x;					vtext[0].x = y ;							vtext[0].z = z ;
			vtext[1].y = x + sizeX; 		vtext[1].x = y ; 							vtext[1].z = z ;
			vtext[2].y = x + sizeX;	 		vtext[2].x = y + (sizeY * 1.53) ;			vtext[2].z = z ;
			vtext[3].y = x;	 				vtext[3].x = y + (sizeY * 1.53) ; 			vtext[3].z = z ;
		}

		switch (j) 
		{
			case 0:
			if ((value / 10000) % 10 > 0) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 10000) % 10],vtext,4,mesh,5,2,col);
				i = 1;
			}
			break;
			case 1:
			if ((value / 1000) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 1000) % 10],vtext,4,mesh,5,2,col);
				i = 1;
			}
			break;
			case 2:
			if ((value / 100) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 100) % 10],vtext,4,mesh,5,2,col);
				i = 1;
			}
			break;
			case 3:
			if ((value / 10) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 10) % 10],vtext,4,mesh,5,2,col);
				i = 1;
			}
			break;
			case 4:
			if (value % 10 > 0 || i == 1 || value == 0 )	voxie_drawmeshtex(vf,font2d[value % 10],vtext,4,mesh,5,2,col);
			break;
		}

		x += fontLetterSpace;

	}

}

static voxie_wind_t vw; // the voxie window struct which holds lots of Voxon settings.  




int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	pol_t pt[3];

	// tim is a clock counts up from runtime
	// dtim is delta time used for timings between frames
	// otim is old tim 
	double tim = 0.0, otim, dtim, avgdtim = 0.0, dlyTim;
	int i, j, k, mousx = 0, mousy = 0, mousz = 0;
	
	point3d ss, tp, pp, rr, dd, ff;
	voxie_xbox_t vx[4]; // this array holds the number of controllers plugged in
	int score2d;

	int inited = 0, debug = 1, ovxbut[4], vxnplays; //ovxbut is the previous frame's controller button states vxnplays is how many controllers are currently pluged in and detected
	float f, g;
	float fontSizeX = 0.07;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK);
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						 /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/
		return (-1);
	}

	while (!voxie_breath(&in)) // Breath is the update loop
	{
		otim = tim;
		tim = voxie_klock();
		mousx += in.dmousx; mousy += in.dmousy; mousz += in.dmousz;
		gbstat = -(in.bstat != 0);	
		dtim = tim - otim; // the timer
		for (vxnplays = 0; vxnplays < 4; vxnplays++) // seting up game controllers
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays, &vx[vxnplays]))
				break; //but, lt, rt, tx0, ty0, tx1, ty1
		}

		// for keystats (input scan codes for keys) use Ken's "Keyview" program!
		if (voxie_keystat(0x1)) 	voxie_quitloop(); // esc key closes ap
	
		if (voxie_keystat(0x03) == 1 ) { fontSizeX += 0.01; } // press keyboard 2
		if (voxie_keystat(0x04) == 1 ) { fontSizeX -= 0.01; } // press keyboard 3

		if (voxie_keystat(0x05) == 1 ) { DEBUG_SPHERE = !DEBUG_SPHERE;} // press keyboard 4 to switch debug spheres
				
		if (dlyTim < tim) { score2d = rand() % 10000; dlyTim = tim + 0.12f; } 

		// rotation keys // this adds using [ ] keys to rotate the image in emulation mode
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1);
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


	// 2d Score test
	display2dNum(&vf, score2d,- 0.2,vw.aspy - (fontSizeX * 2),0.1,0,0xffff00,fontSizeX);

	display2dNum(&vf, score2d,.2,-vw.aspy + (fontSizeX * 2),0.1,1,0x00ffff,fontSizeX);

	display2dNum(&vf, score2d,.2, vw.aspx - (fontSizeX * 2),0.1,2,0xff0000,fontSizeX);

	display2dNum(&vf, score2d,-.2,-vw.aspx + (fontSizeX * 2),0.1,3,0x00ff00,fontSizeX);

	// pp's are used to get the right area
	pp.x = -vw.aspx + 0.3;
	pp.y = -vw.aspy + 0.3;
	pp.z = cos(tim) / 3; // make it go up and down
	// 3d Text test
	print3d(&vf, "Ready", pp, fontSizeX * 2, 0x00FFFF, 0, false );
	pp.x += (fontSizeX * 12);
	print3d(&vf, "Wolf", pp, fontSizeX * 2, 0xFF2255, 4, false);
	pp.z += 0.1;
	pp.x = -vw.aspy + 0.1;
	pp.y += 0.2;
	print3d(&vf, "Sample text just imagine!", pp, fontSizeX, 0xffffff, 0, false);
	pp.y += 0.2;
	print3d(&vf, ",,, your text here! <>", pp, fontSizeX, 0xFF1000, 0, false);
	pp.y += 0.4; pp.x += 0.4;
	print3d(&vf, "Hyper Colour!", pp, fontSizeX, -1, 4 , false); // pass thru colour "-1" for hyper colour function
	pp.z += 0.2; pp.x = -vw.aspx + 0.3;
	print3d(&vf, "Send negative 1 to set", pp, fontSizeX, 0xffffff, 4, false); // pass thru colour "-1" for hyper colour function
	pp.z += 0.2; pp.x = -vw.aspx + 0.3;
	print3d(&vf, "random colour value", pp, fontSizeX, 0x9d9d9d, 4, false); // pass thru colour "-1" for hyper colour function
	pp.z += 0.1; pp.y += 0.3; pp.x = -vw.aspx + 0.2;
	print3d(&vf, "Big Text", pp, fontSizeX * 3, 0x330090, 0, false);
	pp.y += 0.3;
	pp.x = 0;
	print3d(&vf, "This is really really tiny text cool huh? Centred Too!", pp, fontSizeX / 2, 0x903300, 0, true);

	pp.x = 0; pp.y = 0.8; pp.z = 0;
	print3d(&vf, "Player 1", pp, fontSizeX / 2, 0xffff00, 0, true);

	pp.x = 0; pp.y = -0.8; pp.z = 0;
	print3d(&vf, "Player 2", pp, fontSizeX / 2, 0x00ffff, 1, true);

	pp.x = 0.8; pp.y = 0; pp.z = 0;
	print3d(&vf, "Player 3", pp, fontSizeX / 2, 0xff0000, 2, true);

	pp.x = -0.8; pp.y = 0; pp.z = 0;
	print3d(&vf, "Player 4", pp, fontSizeX / 2, 0x00ff00, 3, true);


	ss.x = mousx * .001;
	ss.y = mousy * .001;
	ss.z = mousz * .0004;
	print3d(&vf, "Mouse", ss, fontSizeX / 2, 0x0000ff, 6, true);
/*
	pp.x = -sin(tim / 2) ;
	pp.y = -cos(tim / 2) ;
	pp.z = 0;
	print3d(&vf, "1, MAT  98630210", pp, fontSizeX, 0xffff00, 6, true);
*/
	pp.x = 0; //sin(tim / 2) / 2;
	pp.y = 0.2; //cos(tim / 2) / 2;
	pp.z = 0;
	print3d(&vf, "2, VEX  430210", pp, fontSizeX, 0x00ffff, 4, true);


	// final update loop for frame
	if (debug == 1) 
		{
			//draw wireframe box
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f ", 1.0 / avgdtim );
			voxie_debug_print6x8_(30, 100, 0xff0000, -1, "An example on how to get bitmap and .OBJ fonts into VoxieBox");
			voxie_debug_print6x8_(30, 120, 0x00ff00, -1, "Press '2' and '3' to increase font size! Fontsize is %1.5f", fontSizeX);
			voxie_debug_print6x8_(30, 140, 0x00ff00, -1, "Press '4' to view debug text spheres", fontSizeX);
			
				
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
