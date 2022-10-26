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

	//VoxieMenu an example of how to write a custom voxieMenu works for Voxon Apps
	//Code by Matthew Vecchio for Voxon 9/08/2019. Updated 1/10/2020

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
	MENU_UP, MENU_ONE, MENU_TWO, MENU_THREE, MENU_DOWN, MENU_VSLIDER_NAME, MENU_HSLIDER_NAME,
	MENU_GO, MENU_EDIT_DO_NAME, MENU_EDIT_NAME, MENU_TOGGLE_NAME, MENU_PICKFILE_NAME, MENU_LOAD, MENU_FILE_NAME, MENU_BROWSE,
	MENU_SAVE_FILE, MENU_SAVE_TEXTBOX,
	
}; 


// global values we are changing via the menu
float gYHeightValue = 0.0f, gVSliderValue = 0; 
int gMenuChoiceValue = 0, gChoiceColourValue = 0x00ff00;
int gHSliderValue = 0, gGoColour = 0xffffff;
int gToggleValue = 0;
char *gTestMessage = "Edit me using the menu!";

#define MAX_FILE 260 
char gFilepath[MAX_FILE];
char gFilecontents[1000];
char gMessageBox[5000];


// example on how to load a text file and put the text file into the global test message char array
bool loadTxt(char* filepath) {

	FILE *fp;
	int i;
	fp = fopen(filepath, "r");
    if (fp == NULL){
		MessageBox(0, filepath, "Error: can't open file", MB_OK); // if there is an error this will create a standard Windows message box
	    return false;
    }

	char dataBuff[1000]; // only load in the 1st 1000 characters
	while (fgets(dataBuff, 1000, fp) != NULL) // iterate through the file and put it into 'dataBuff' 

	strcpy(gFilecontents, dataBuff);
	
	fclose(fp);
    return true;

}




// example on how to save a text file
bool saveTxt(char* filePath, char * dataBuff) {
	FILE *fp;
    fp = fopen(filePath, "w");
 
    if(fp == NULL) {
       	MessageBox(0, filePath, "Error: can't open file", MB_OK); // if there is an error this will create a standard Windows message box
        return false;
    }
 
	fprintf(fp, gMessageBox);

    fclose(fp);
    return true;

}

// create a function to update the menu when changes occur. This function gets called when a menu button is active or anythig on the menu is changed
// the ID is equal to the enum
// the v is the value 
static int menu_update (int id, char *st, double v, int how, void *userdata)
{
	switch(id)
	{
		case MENU_VSLIDER_NAME: gVSliderValue = v;
		break;
		case MENU_HSLIDER_NAME: gHSliderValue = (int)v;
		break;
		case MENU_EDIT_NAME:
		case MENU_EDIT_DO_NAME:		
			gTestMessage = st;
		break;
		case MENU_UP:   gYHeightValue -= 0.05f; 										break; 
		case MENU_ONE:  gMenuChoiceValue = 1;  gChoiceColourValue = 0xFF0000; 			break;
		case MENU_TWO:  gMenuChoiceValue = 2;  gChoiceColourValue = 0x00FF00;			break;
		case MENU_THREE: gMenuChoiceValue = 3; gChoiceColourValue = 0x0000FF; 			break;
		case MENU_DOWN:  gYHeightValue += 0.1f;  										break;
		case MENU_GO: gGoColour = ((rand()&1)<<7) + ((rand()&1)<<15) + ((rand()&1)<<23); break;
		case MENU_TOGGLE_NAME: gToggleValue = (int)v; 									break;

		// items under the 2nd tab
		case MENU_FILE_NAME: strcpy(gFilepath,st); 										break;
		case MENU_BROWSE: 
			strcpy(gFilepath,st); 
			// update item to display the correct filename on the menu
			voxie_menu_updateitem(MENU_FILE_NAME,st,0,0.0); 
		break;
		case MENU_LOAD: loadTxt(gFilepath);  											break;
		break;
		case MENU_SAVE_TEXTBOX:

 		strcpy(gMessageBox,st);

		break;
		case MENU_SAVE_FILE:
			saveTxt(gFilepath, gMessageBox);
		break;


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
	/* 
	Define / initiate the Menu - you only do this once. So put it after the voxie_init() call but before voxie_breath() you'll want to 
		make sure the initialization happens only once.
    
	//Add user menu tab (NOTE: there's only space for 2 more tabs on the 7" 1024x600 LCD screen)
	//   st: name of tab
	//  x,y: top-left corner of tab item area
	//xs,ys: size of tab item area
	void voxie_menu_addtab (char *st, int x, int y, int xs, int ys);

	//Add item to menu tab
	//   st: name of text/button/slider
	//  x,y: top-left corner offset relative to tab
	//xs,ys: size of item
	//   id: user-defined low integer (use enum to differentiate easily)
	// type: MENU_TEXT:     text (decoration only)
	//       MENU_LINE:     line (decoration only)
	//       MENU_BUTTON+3: push button (single button)
	//       MENU_BUTTON+1: push button (first in group - auto-depresses others so only 1 on)
	//       MENU_BUTTON  : push button (in middle of group)
	//       MENU_BUTTON+2: push button (last in group - auto-depresses others so only 1 on)
	//       MENU_gHSliderValue:  horizontal slider
	//       MENU_gVSliderValue:  vertical slider
	//       MENU_EDIT:     edit text box
	//       MENU_EDIT_DO:  edit text box, click next item on 'Enter'
	//       MENU_TOGGLE:   combo box (w/o the drop down). Useful for saving space in dialog.
	//                      Specify multiple strings in 'st' using \r as separator.
	//       MENU_PICKFILE: file selector. Specify type in 2nd string. Ex: "Browse\r*.kv6"
	//  col: color of item (typically 0xffffff)
	//    v: starting value
	//v0,v1: min,max range of values (respectively)
	//vstp0: minor step in values
	//vstp1: major step in values
	void voxie_menu_additem (char *st, int x, int y, int xs, int ys, int id, int type, int down,
							int col, double v, double v0, double v1, double vstp0, double vstp1);

	1 = is for the first button in the chain
	0 = is a standard button in the middle of a chain
	2 = is for the end of the chain
	3 = is for a single standard button

   */
	// Menu_button +(1 - 4) +1 for first, +2 for last, +3 for solo  
	// sider settings are after the colour value is starting starting value, lowest value, highest value, minimal adjustment, major adjustment
	// to perform a major adjustment hold down control as you change the setting
	
	voxie_menu_reset(menu_update,0, 0); // resets the menu tab
	voxie_menu_addtab("Test Name",		350,0,600,500); // add a tab to the top of the menu be careful not to add more than 2 they will be hidden on the VX1's display
	
	// add text
	voxie_menu_additem("Menu Text Example", 300,20, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text
	
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
	voxie_menu_additem("V Slider Text",	50,85,64,150,	MENU_VSLIDER_NAME ,MENU_VSLIDER ,0.5,0xFFFF80,.5,2.0,.1,.1,.3);

	// horizontal slider
	// sider settings are after the colour value is starting starting value, lowest value, highest valuer, minimal adjustment, major adjustment
	voxie_menu_additem("H Slider Text",	200,270,300,64,	MENU_HSLIDER_NAME ,MENU_HSLIDER ,5 ,0x808080,5.0,1.0,20.0,1.0,3.0);
	
	// How to input a string
	voxie_menu_additem("Menu Edit text",		50,350,500,50,	MENU_EDIT_NAME ,MENU_EDIT ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);
	
	// Edit is like Edit Do does the next action when you press enter in this case will hit the Go button
	voxie_menu_additem("Edit Do text",	50,410,400,50,	MENU_EDIT_DO_NAME ,MENU_EDIT_DO ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);
	
	// A button which is linked to Edit Do
	voxie_menu_additem("GO",		460,410,90,50,	MENU_GO,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);

	// A Toggle button use '\r' to seperate values each entry has its only value which gets passed to the menu_update function
	voxie_menu_additem("Toggle Button\rPress Me\rTo Change\rValues",		175,200,175,50,	MENU_TOGGLE_NAME,MENU_TOGGLE,0,0xA080A0,0.0,0.0,0.0,0.0,0.0);

	// add some more text
	voxie_menu_additem("Matthew Vecchio / ReadyWolf for Voxon 2020", 40,480, 64, 64,0, MENU_TEXT ,0 ,0xFF0000,0.0,0.0,0.0,0.0,0.0);

	// start a second tab - now all items de\fined after this point will go into this new table
	voxie_menu_addtab("Test File",		450,0,600,420); // add a tab to the top of the menu be careful not to add more than 2 they will be hidden on the VX1's display

	voxie_menu_additem("FILE PICKER", 250,20, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text


	voxie_menu_additem("This is a second tab which contains\na 'File Picker' Example\n\n\nchoose 'browse' to load up a .txt file!\nSee the code to edit the filters\nEnjoy!", 70,75, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text

	// another example of edit_do and a button
	voxie_menu_additem(gFilepath         , 20,250,440, 64,MENU_FILE_NAME,MENU_EDIT_DO ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Load"         ,480,250,100, 64,MENU_LOAD  ,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);

	// file picker demo.. use '\r' to sperate the name of the button and filter
	voxie_menu_additem("Browse File\r*.txt", 20,330,560, 64,MENU_BROWSE,MENU_PICKFILE,0,0x908070,0.0,0.0,0.0,0.0,0.0);

	//

	// start a second tab - now all items defined after this point will go into this new table
	voxie_menu_addtab("Save File",		450,0,600,420); // add a tab to the top of the menu be careful not to add more than 2 they will be hidden on the VX1's display

	voxie_menu_additem("SAVE FILE Example\n\nwrite a message in the text box & set a\nfile name.\n\nSaved Message :\n", 10,20, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text

	voxie_menu_additem(gMessageBox         , 20,150,550, 140,MENU_SAVE_TEXTBOX,MENU_EDIT ,0,0x908070,0.0,0.0,0.0,0.0,0.0);


	// another example of edit_do and a button
	
	voxie_menu_additem("Filename", 10,360, 64, 64,0                 ,MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); // adding menu text
	voxie_menu_additem(gFilepath         , 140,330,300, 64,MENU_FILE_NAME,MENU_EDIT_DO ,0,0x908070,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("SAVE"         ,480,330,100, 64,MENU_SAVE_FILE  ,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);



	while (!voxie_breath(&in)) 
	{
		otim = tim; tim = voxie_klock(); dtim = tim-otim; 
		for(vxnplays=0;vxnplays<4;vxnplays++)
		{
			ovxbut[vxnplays] = vx[vxnplays].but;

			if (!voxie_xbox_read(vxnplays,&vx[vxnplays])) break; //but, lt, rt, tx0, ty0, tx1, ty1

		}

		if (voxie_keystat(0x1)) { voxie_quitloop(); }


		// !IMPORTANT! -- this is how to updatie the menu item's state via code
		// also how to press buttons via code you can use this to overide settings

		// pressing keyboard number '1' will be the same as pushing '1' on the voxieMenu
		if (voxie_keystat(0x02) == 1 ) {  // pressing keyboard 1
			// use voxie_menu_updateitem to update the apperance of a button
			voxie_menu_updateitem(MENU_ONE,"1",1,0);
			voxie_menu_updateitem(MENU_TWO,"2",0,0);
			voxie_menu_updateitem(MENU_THREE,"3",0,0);
			
			// to perform the same action as pressing the button you can just call the function direct
			// menu_update(int id, char *st, double v, int how, void *userdata)
			menu_update(MENU_ONE,"",1,0,NULL);
			// this is the sound the menu usually plays -- note this is not a relative path
			voxie_playsound("C:/voxon/system/runtime/typekey.flac",-1,100,100,1);  

		} 
		// pressing keyboard number '2' will be the same as pushing '2' on the voxieMenu
		if (voxie_keystat(0x03) == 1 ) {  // pressing keyboard 2
			// use voxie_menu_updateitem to update the apperance of a button
			voxie_menu_updateitem(MENU_ONE,"1",0,0);
			voxie_menu_updateitem(MENU_TWO,"2",1,0);
			voxie_menu_updateitem(MENU_THREE,"3",0,0);
			
			// to perform the same action as pressing the button you can just call the function direct
			menu_update(MENU_TWO,"",1,0,NULL);
			// this is the sound the menu usually plays -- note this is not a relative path
			voxie_playsound("C:/voxon/system/runtime/typekey.flac",-1,100,100,1);

		} 
		// pressing keyboard number '3' will be the same as pushing '3' on the voxieMenu
		if (voxie_keystat(0x04) == 1 ) {  // pressing keyboard 3
			// use voxie_menu_updateitem to update the apperance of a button
			voxie_menu_updateitem(MENU_ONE,"1",0,0);
			voxie_menu_updateitem(MENU_TWO,"2",0,0);
			voxie_menu_updateitem(MENU_THREE,"3",1,0);
			
			// to perform the same action as pressing the button you can just call the function direct
			menu_update(MENU_THREE,"",1,0,NULL);
			// this is the sound the menu usually plays -- note this is not a relative path
			voxie_playsound("C:/voxon/system/runtime/typekey.flac",-1,100,100,1);

		} 




		voxie_frame_start(&vf);

		voxie_setview(&vf,-vw.aspx,-vw.aspy,-vw.aspz,vw.aspx,vw.aspy,vw.aspz);
		
		pp.x = -vw.aspx; 			rr.x = 0.075f; dd.x = 0;
		pp.y = -vw.aspy + gYHeightValue;	rr.y = 0; dd.y = 0.1f;
		pp.z = cos(tim) / 10; 		rr.z = 0; dd.z = 0;
		voxie_printalph_(&vf,&pp,&rr,&dd,gChoiceColourValue,"Choice:%d  gHSliderValue:%d ", gMenuChoiceValue, gHSliderValue );
	 	pp.y += 0.2;
		voxie_printalph_(&vf,&pp,&rr,&dd,gChoiceColourValue,"gVSliderValue:%1.5f ", gVSliderValue );
		pp.y += 0.2;
		voxie_printalph_(&vf,&pp,&rr,&dd,gChoiceColourValue,"Toggle value:%d", gToggleValue );
		pp.y += 0.2;
		voxie_printalph_(&vf,&pp,&rr,&dd,gChoiceColourValue,gTestMessage );
		pp.y += 0.2;
		rr.x *= .60; dd.y *= .60; // resize text so its a smaller	
		voxie_printalph_(&vf,&pp,&rr,&dd,gChoiceColourValue, gFilecontents );


		pp.x = -0.4 ; pp.y += 0.2; 
		rr.x = 0.8 ; rr.y = 0.0f; rr.z = 0.0f;
		dd.x = 0.0f; dd.y = 0.2f; dd.z = 0.0f;
		ff.x = 0.0;  ff.y = 0.0f; ff.z = 0.2f;

		voxie_drawcube(&vf,&pp, &rr, &dd, &ff, 2, gGoColour);	

		voxie_debug_print6x8_(30,68,0xffc080,-1,"VPS %5.1f",1.0/avgdtim);	
		voxie_debug_print6x8_(30,100,0xffc080,-1,"VoxieMenu a sample program to learn how to make menus with VoxieBox.dll");	
		avgdtim += (dtim-avgdtim)*.1;
		voxie_drawbox(&vf,-vw.aspx+1e-3,-vw.aspy+1e-3,-vw.aspz,+vw.aspx-1e-3,+vw.aspy-1e-3,+vw.aspz,1,0xffffff);

		if (debug == 1) 
		{
			voxie_debug_print6x8_(30,150,0xffffff, -1, "MessageBox : %s", gMessageBox);	
		}

		voxie_frame_end(); voxie_getvw(&vw);

	}

	voxie_uninit(0); //Close window and unload voxiebox.dll
	return(0);
}

#if 0
!endif
#endif
