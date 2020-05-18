#if 0
!if 1

	#Visual C makefile:
voxienew.exe: voxiemenu.c voxiebox.h; cl /TP voxiemenu.c /Ox /MT /link user32.lib
	del voxiemenu.obj

!else

	#GNU C makefile:
voxienew.exe: voxiemenu.c; gcc voxiemenu.c -o voxiemenu.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

	//VoxieMenu an example of how the in built menu works for C. Requires voxiebox.dll to be in path.
	//NOTE:voxiebox.dll is compiled as 64-bit mode. Be sure to compile for 64-bit target or else voxie_load() will fail.
	//To compile, type "nmake voxiesimp.c" at a VC command prompt, or set up in environment using hints from makefile above.
	//Code by Matthew Vecchio for Voxon 9/08/2019

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif

static voxie_wind_t vw;
// create an enum to stare the values of all the menu names 
enum // menu names
{
	//Generic names
	MENU_UP, MENU_ONE, MENU_TWO, MENU_THREE, MENU_DOWN, MENU_VSLIDERNAM, MENU_HSLIDERNAM,
	MENU_GO, MENU_EDITDO_ID, MENU_EDIT_ID, 
	
	//Other
	MENU_AUTOROTATEOFF, MENU_AUTOROTATEX, MENU_AUTOROTATEY, MENU_AUTOROTATEZ, MENU_AUTOROTATESPD,
	MENU_FRAME_PREV, MENU_FRAME_NEXT,
	MENU_SOL0, MENU_SOL1, MENU_SOL2, MENU_SOL3,
	MENU_HINT, MENU_DIFFICULTY, MENU_SPEED,
	MENU_ZIGZAG, MENU_SINE, MENU_HUMP_LEVEL,
	MENU_DISP_CUR, MENU_DISP_END=MENU_DISP_CUR+MAXDISP-1,
}; 
static float yHeight = 0.0f, vSlider = 0; 
static int menuChoice = 0, choiceCol = 0x00ff00;
static int hSlider = 0, goColour = 0xffffff;
char *testMessage = "Edit me using the menu!";

// create a function to update the menu when changes occur. This function gets called when a menu button is active or anythig on the menu is changed
// the ID is equal to the enum
// the v is the value 
static int menu_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_VSLIDERNAM: vSlider = v;
		break;
		case MENU_HSLIDERNAM: hSlider = (int)v;
		break;
		case MENU_EDIT:
		case MENU_EDIT_DO:
		testMessage = st;
		break;
		case MENU_UP:   yHeight -= 0.05f; break; 
		case MENU_ONE:  menuChoice = 1;  choiceCol = 0xFF0000; break;
		case MENU_TWO:  menuChoice = 2;  choiceCol = 0x00FF00;break;
		case MENU_THREE: menuChoice = 3; choiceCol = 0x0000FF; break;
		case MENU_DOWN:  yHeight += 0.1f;  break;
		case MENU_GO: goColour = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); break;
	}
	return(1);
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_frame_t vf;
	voxie_inputs_t in;
	pol_t pt[3];
	double d, tim = 0.0, otim, dtim,  avgdtim = 0.0;
	int i, mousx = 256, mousy = 256, mousz = 0;
	point3d ss, pp, rr, dd, ff, pos = {0.0,0.0,0.0}, inc = {0.3,0.2,0.1};
	voxie_xbox_t vx[4];
	int debug = 1, ovxbut[4], vxnplays;
	
	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
		{ MessageBox(0,"Error: can't load voxiebox.dll","",MB_OK); return(-1); }
	if (voxie_init(&vw) < 0) //Start video and audio.
		{ /*MessageBox(0,"Error: voxie_init() failed","",MB_OK);*/ return(-1); }
	// Define / initiate the Menu - you only do this once. So put it somewhere in the code that will run only once
    /*
	Here you define how you want to change the menu

	when you add a tab ("Text / Name", starting X position of box, starting Y posiion of box, X size, Y size)

	when adding an item the constructor basically works like this (" Text ", X position, Y position, X size, Y size, enum name (can just be an int), button state, colour, slider default value, slider min value, slider max, slider small increment amount, slider large increment )

	Button State determines if any buttons are chained together...

	1 = is for the first button in the chain
	0 = is a standard button in the middle of a chain
	2 = is for the end of the chain
	3 = is for a single standard button

	item types:

	MENU_BUTTON = a typical button whih can be presed ( button state is 0 when unpressed and 1 when pressed);
	MENU_TEXT = displays text on the screen
	MENU_hSlider = horizontal slider
	MENU_VSLIDER = vertical slider
	MENU_EDIT = editable text box
	MENU_EDIT_DO = editable text box and does the action after you enter details


   */
	// Menu_button +(1 - 4) +1 for first, +2 for last, +3 for solo  
	// sider settings are after the colour value is starting starting value, lowest value, highest valuer, minimal adjustment, major adjustment
	// to perform a major adjustment hold down control as you change the setting
	
	voxie_menu_reset(menu_update,0, 0); // resets the menu tab
	voxie_menu_addtab("Test Name",		350,0,600,500); // add a tab to the top of the menu be careful not to add to many as they will be hidden on the VX1
	
	// add text
	voxie_menu_additem("Menu Text Test", 220,20, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text
	
	// single button (MENU_BUTTON+3)
	voxie_menu_additem("Up"    ,	400, 50,75,50,	MENU_UP   ,MENU_BUTTON+3,0,0x808000,0.0,0.0,0.0,0.0,0.0);
	
	// group of butons (first button MENU_BUTTON+1, middle button just MENU_BUTTON, last button MENU_BUTTON+2)
	voxie_menu_additem("1"  	, 	300,110,75,50,	MENU_ONE ,MENU_BUTTON+1,0,0xFF0000,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("2"		,	400,110,75,50,	MENU_TWO,MENU_BUTTON,1,0x00FF00,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("3"		 ,	500,110,75,50,	MENU_THREE,MENU_BUTTON+2,0,0x0000FF,0.0,0.0,0.0,0.0,0.0);

	// single button
	voxie_menu_additem("Down"  ,	400,170,75,50,	MENU_DOWN ,MENU_BUTTON+3,0,0x008080,0.0,0.0,0.0,0.0,0.0);
	
	// vertical slider the values at the end after the colour value are l
	// sider settings are after the colour value is starting starting value, lowest value, highest valuer, minimal adjustment, major adjustment
	voxie_menu_additem("V Slider",	75,85,64,150,	MENU_VSLIDERNAM ,MENU_VSLIDER ,0.5,0xFFFF80,.5,2.0,.1,.1,.3);

	// horizontal slider
	// sider settings are after the colour value is starting starting value, lowest value, highest valuer, minimal adjustment, major adjustment
	voxie_menu_additem("H Slider",	200,270,300,64,	MENU_HSLIDERNAM ,MENU_HSLIDER ,5 ,0x808080,5.0,1.0,20.0,1.0,3.0);
	
	// How to input a string
	voxie_menu_additem("Edit this text",		50,350,500,50,	MENU_EDIT_ID ,MENU_EDIT ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);
	
	// Edit is like Edit Do does the next action when you press enter in this case will hit the Go button
	voxie_menu_additem("Edit Do",	50,410,400,50,	MENU_EDITDO_ID ,MENU_EDIT_DO ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);
	
	// A button which is linked to Edit Do
	voxie_menu_additem("GO",		460,410,90,50,	MENU_GO,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);
	
	// add some more text
	voxie_menu_additem("Matthew Vecchio / ReadyWolf for Voxon 2019", 40,480, 64, 64,0, MENU_TEXT ,0 ,0xFF0000,0.0,0.0,0.0,0.0,0.0);
			
	while (!voxie_breath(&in)) 
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; 
		for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1

		}

		if (voxie_keystat(0x1)) { voxie_quitloop(); }

		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
		
		pp.x = -0.900f ; 			rr.x = 0.075f; dd.x = 0;
		pp.y = -0.01f + yHeight;	rr.y = 0; dd.y = 0.1f;
		pp.z = cos(tim) / 10; 		rr.z = 0; dd.z = 0;
		voxie_printalph_(&vf,&pp,&rr,&dd,choiceCol,"Choice:%d  HSlider:%d ", menuChoice, hSlider );
	 	pp.y += 0.2;
		voxie_printalph_(&vf,&pp,&rr,&dd,choiceCol,"VSlider:%1.5f ", vSlider );
		pp.y += 0.2;
		voxie_printalph_(&vf,&pp,&rr,&dd,choiceCol,testMessage );


		pp.x = -0.4 ; pp.y = -0.4 + yHeight; 
		rr.x = 0.8 ; rr.y = 0.0f; rr.z = 0.0f;
		dd.x = 0.0f; dd.y = 0.2f; dd.z = 0.0f;
		ff.x = 0.0;  ff.y = 0.0f; ff.z = 0.2f;

		voxie_drawcube(&vf,&pp, &rr, &dd, &ff, 2, goColour);	


		if (debug == 1) 
		{
			avgdtim += (dtim-avgdtim)*.1;
			voxie_debug_print6x8_(30,68,0xffc080,-1,"VPS %5.1f",1.0/avgdtim);	
			voxie_debug_print6x8_(30,100,0xffc080,-1,"VoxieMenu a sample program to learn how to make menus with VoxieBox.dll");	
			voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);
		}

		voxie_frame_end(); voxie_getvw(&vw);

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
