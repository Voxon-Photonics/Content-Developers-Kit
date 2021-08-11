// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1
#Visual C makefile:
VoxieFontAdv.exe: VoxieFontAdv.c voxiebox.h; cl /TP VoxieFontAdv.c /Ox /MT /link user32.lib
	del VoxieFontAdv.obj

!else

#GNU C makefile:
VoxieFontAdv.exe: VoxieFontAdv.c; gcc VoxieFontAdv.c -o ark.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

/*
2D / 3D advanced font effects

2019 9/8/2019 Matthew Vecchio / Ready Wolf

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

enum {

FONT_EFFECT_SPIN_X, FONT_EFFECT_SPIN_Y, FONT_EFFECT_SPIN_Z,
FONT_EFFECT_SINE_Z, FONT_EFFECT_COLOUR_SWAP, FONT_EFFECT_COLOUR_ALT,
FONT_EFFECT_COLOUR_RANDOM, FONT_EFFECT_SECOND_COL, FONT_EFFECT_LETTERSPACE,
FONT_EFFECT_FLIPX, 


};

#define TEXTMAX 100
typedef struct { char * text; point3d pos, rotX, rotY, rotZ; float speed, size, var1; int col1, col2, col1Org, id; } text_t;
static text_t texts[TEXTMAX];
static int textNo = 0;

// these need to be accessible for the functions and the main programs
poltex_t vtext[5]; int mesh[5]; 

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

// function to tween one colour to another
int tweenCol(int colour, int speed, int destColour) {

	int b,g,r;
	int bd,gd,rd;

	b = (colour & 0xFF);	
	g = (colour >> 8) & 0xFF;	
	r = (colour >> 16) & 0xFF;
	bd = (destColour & 0xFF);	
	gd = (destColour >> 8) & 0xFF;	
	rd = (destColour >> 16) & 0xFF;

	if (b > bd)	b -= speed;
	else if (b < bd) b += speed;
	if (r > rd)	r -= speed;
	else if (r < rd) r += speed;
	if (g > gd)	g -= speed;
	else if (g < gd) g += speed;

	if (r < 0x00) r = 0x00;
	if (r > 0xFF) r = 0xFF;
	if (g < 0x00) g = 0x00;
	if (g > 0xFF) g = 0xFF;
	if (b < 0x00) b = 0x00;
	if (b > 0xFF) b = 0xFF;
	return (r << 16) | (g << 8) | (b);

}
// creates a text returns an int of the ID
int createText (char * text, point3d pos, float size, int col1, int col2) {


	float sizeX = size;
	float sizeY = size * 1.53;
	float sizeZ = size / 1.2;

	if (textNo < TEXTMAX) {
		texts[textNo].text = text;
		texts[textNo].size = size;
		texts[textNo].col1 = col1;
		texts[textNo].col1Org = col1;
		texts[textNo].col2 = col2;
		
		// create sizes
		texts[textNo].pos.x = pos.x;
		texts[textNo].pos.y = pos.y;
		texts[textNo].pos.z = pos.z;

		texts[textNo].rotX.x = sizeX;
		texts[textNo].rotX.y = 0;
		texts[textNo].rotX.z = 0;

		texts[textNo].rotY.x = 0;
		texts[textNo].rotY.y = sizeY;
		texts[textNo].rotY.z = 0;

		texts[textNo].rotZ.x = 0;
		texts[textNo].rotZ.y = 0;
		texts[textNo].rotZ.z = sizeZ;

		texts[textNo].id = textNo;
		



	textNo++;
	return textNo -1;
	} else {
	return 0;
	}
}


// a texts move to function returns turn when in position
bool printTextsMovTo (int textId, point3d moveTopos, float speed, double dtim) {

	point3d pp, vel;
	float f;

	if ( texts[textId].pos.x < moveTopos.x + 0.02 && texts[textId].pos.x > moveTopos.x - 0.02 &&
		 texts[textId].pos.y < moveTopos.y + 0.02 && texts[textId].pos.y > moveTopos.y - 0.02 &&
		 texts[textId].pos.z < moveTopos.z + 0.02 && texts[textId].pos.z > moveTopos.z - 0.02 
		 ) {

			texts[textId].pos.x = moveTopos.x;
			texts[textId].pos.y = moveTopos.y;
			texts[textId].pos.z = moveTopos.z;

			return true;
		 } 
	else {
		pp.x = moveTopos.x - texts[textId].pos.x;
		pp.y = moveTopos.y - texts[textId].pos.y;
		pp.z = moveTopos.z - texts[textId].pos.z;

		f = sqrt(pp.x*pp.x + pp.y*pp.y + pp.z*pp.z); // normilize vectors

		vel.x = 	(speed ) * (pp.x / f) * dtim;
		vel.y = 	(speed ) * (pp.y / f) * dtim;
		vel.z = 	(speed ) * (pp.z / f) * dtim;

		texts[textId].pos.x += vel.x;
		texts[textId].pos.y += vel.y;
		texts[textId].pos.z += vel.z;

		return false;
	}

}
// printing the texts
void printTexts (voxie_frame_t * vf, int textId, int effect, float speed, double tim, bool print) {

point3d pos = texts[textId].pos;
int i;
float size = texts[textId].size;
int col = texts[textId].col1;


	switch (effect)
	{
	case FONT_EFFECT_SPIN_X: // spin by X
		rotvex(speed, &texts[textId].rotX, &texts[textId].rotZ );
		break;
	case FONT_EFFECT_SPIN_Y: // spin by Y
		rotvex(speed, &texts[textId].rotX, &texts[textId].rotY );
	break;	
	case FONT_EFFECT_SPIN_Z: // spin by Z
		rotvex(speed, &texts[textId].rotZ, &texts[textId].rotY );
	break;	
	case FONT_EFFECT_COLOUR_SWAP:
		if (texts[textId].col1 != int(tim) && texts[textId].var1 != 1 )  texts[textId].col1 = tweenCol( texts[textId].col1, speed * 25, tim );
		if (texts[textId].col1 == int(tim) ) texts[textId].var1 = 1;
		else if ( texts[textId].col1 == texts[textId].col1Org && texts[textId].var1 == 1 ) texts[textId].var1 = 0;
		if (texts[textId].var1 == 1) texts[textId].col1 = tweenCol( texts[textId].col1, speed * 25, texts[textId].col1Org );
		break;
	case FONT_EFFECT_SECOND_COL:
		col = texts[textId].col2;
		break;	
	default:
		break;
	}




	for (int i = 0; i < strlen(texts[textId].text); i++)
	{

		switch (effect)
	{
		case FONT_EFFECT_SINE_Z: // spin by X
		pos.z = texts[textId].pos.z + cos(double(i + tim * speed)) / 10 ;
		break;
		case FONT_EFFECT_COLOUR_ALT:
		if (i % 2) col = texts[textId].col2;
		else col = texts[textId].col1;
		break;
		case FONT_EFFECT_COLOUR_RANDOM:
		col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change	
		break;
		case FONT_EFFECT_LETTERSPACE:
		pos.x += size - (size * .02 + (cos(tim + i) / 20)); // line spacing
		break;
		
	}
		
		
		
		

	if (print) {
		
		if (effect == FONT_EFFECT_FLIPX) voxie_drawspr(vf,font3d[texts[textId].text[i]],&pos,&texts[textId].rotX,&texts[textId].rotZ,&texts[textId].rotY,col);
		else 							 voxie_drawspr(vf,font3d[texts[textId].text[i]],&pos,&texts[textId].rotX,&texts[textId].rotY,&texts[textId].rotZ,col);
		
		pos.x += size - (size * .02); // line spacing
		}
	}


}

//prints a string in 3d
void print3d (voxie_frame_t *vf, char *text,  point3d pos, float size, int col, int view ) 
{
	int i;
	point3d dd,rr,ff;
	float sizeX = size;
	float sizeY = size * 1.53;
	float sizeZ = size / 1.2;
	if (col == -1)  col = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); // rainbow colour change
			
	rr.x = sizeX;  	 dd.x = 0;			ff.x = 0;
	rr.y = 0; 	   dd.y = sizeY; 		ff.y = 0;
	rr.z = 0; 		 dd.z = 0; 			ff.z = sizeZ;


	for (int i = 0; i < strlen(text); i++)
	{
		if (view == 0) voxie_drawspr(vf,font3d[text[i]],&pos,&rr,&dd,&ff,col);
		if (view == 1) voxie_drawspr(vf,font3d[text[i]],&pos,&rr,&ff,&dd,col);
		pos.x += size - (size * .02); // line spacing

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
				voxie_drawmeshtex(vf,font2d[(value / 10000) % 10],vtext,5,mesh,6,2,col);
				i = 1;
			}
			break;
			case 1:
			if ((value / 1000) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 1000) % 10],vtext,5,mesh,6,2,col);
				i = 1;
			}
			break;
			case 2:
			if ((value / 100) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 100) % 10],vtext,5,mesh,6,2,col);
				i = 1;
			}
			break;
			case 3:
			if ((value / 10) % 10 > 0 || i == 1) 
			{
				voxie_drawmeshtex(vf,font2d[(value / 10) % 10],vtext,5,mesh,6,2,col);
				i = 1;
			}
			break;
			case 4:
			if (value % 10 > 0 || i == 1 || value == 0 )	voxie_drawmeshtex(vf,font2d[value % 10],vtext,5,mesh,6,2,col);
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
	int i, j, k, font1;
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

	if (inited == 0) {
		inited = 1;
		pp.x = -.8;
		pp.y = -.8;
		pp.z = -0.2;
		createText("HOLO-TEXT!",pp, fontSizeX * 2.5, 0xff0000,0x00ff00 );
	
		pp.y += 0.2f;
		pp.x = -0.5;
		createText("revolution in 3D",pp, fontSizeX, 0xff00ff,0x00ff00 );

		pp.y += 0.2f;
		pp.x = -0.65;
		createText("watch me slowly change the colour of a font",pp, fontSizeX / 2, 0x00ff00,0x00ff00 );

		pp.y += 0.2f;
		pp.x = -0.4;
		createText("Alter letters",pp, fontSizeX, 0xff0000,0x0000ff );

		pp.y += 0.2f;
		pp.x = -0.4;
		createText("Random Color",pp, fontSizeX, 0xff0000,0x0000ff );

		pp.y += 0.2f;
		pp.x = -0.7;
		createText("Move Me",pp, fontSizeX, 0xff0000,0x0000ff );

		pp.y += 0.2f;
		pp.x = -0.9;
		font1 = createText("Letter Space Effect",pp, fontSizeX * 1.2, 0x00ff66,0x0000ff );

		pp.y += 0.2f;
		pp.x = -0.85;
		pp.z = 0.1;
		createText("Vertical Font Flip",pp, fontSizeX * 3.5, 0x1030ff,0x0000ff );

		rr.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
		rr.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
		rr.z = ((float)((rand()&32767)-16384)/16384.f*vw.aspz); 

	}

	voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz);

	printTexts(&vf, 0, FONT_EFFECT_SPIN_X, 1 * dtim, tim, false);

	printTexts(&vf, 1, FONT_EFFECT_SPIN_Z, 1 * dtim, tim, true);

	printTexts(&vf, 0, FONT_EFFECT_SINE_Z, 2, tim, true);

	printTexts(&vf, 2, FONT_EFFECT_COLOUR_SWAP, 5 * dtim, 0xff00ff, true);

	printTexts(&vf, 3, FONT_EFFECT_COLOUR_ALT, 0, tim, true);

	printTexts(&vf, 4, FONT_EFFECT_COLOUR_RANDOM, 5, tim, true); 

	printTexts(&vf, 5, FONT_EFFECT_SECOND_COL, 1 * dtim, tim, true); 

	printTexts(&vf, font1, FONT_EFFECT_LETTERSPACE, 1 * dtim, tim, true); 
	
	printTexts(&vf, font1, FONT_EFFECT_COLOUR_SWAP, 10 * dtim, 0x000000, false); 

	printTexts(&vf, 7, FONT_EFFECT_SPIN_Y, 2 * dtim, 0x000000, false); 	
	
	printTexts(&vf, 7, FONT_EFFECT_FLIPX, 10 * dtim, 0x000000, true); 

	if (printTextsMovTo(5, rr, .2, dtim)) {
		rr.x = (float)((rand()&32767)-16384)/16384.f*vw.aspx;
		rr.y = (float)((rand()&32767)-16384)/16384.f*vw.aspy;
		rr.z = ((float)((rand()&32767)-16384)/16384.f*vw.aspz); 
	}


	// final update loop for frame
	if (debug == 1) 
		{
			//draw wireframe box
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f, font1 %d", 1.0 / avgdtim, font1 );
			voxie_debug_print6x8_(30, 100, 0xff0000, -1, "An example on how to get bitmap and .OBJ fonts into VoxieBox");
			voxie_debug_print6x8_(30, 120, 0x00ff00, -1, "Press '2' and '3' to increase font size! Fontsize is %1.5f", fontSizeX);
				
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
