// This source code is provided by the Voxon Developers Kit with an open-source license. You may use this code in your own projects with no restrictions.
#if 0
!if 1
#Visual C makefile:
fileIO.exe: fileIO.c voxiebox.h; cl /TP fileIO.c /Ox /MT /link user32.lib comdlg32.lib
	del fileIO.obj

!else

#GNU C makefile:
fileIO.exe: fileIO.c; gcc fileIO.c -o fileIO.exe -pipe -O3 -s -m64

!endif
!if 0



/* 
Program to demonstrates how to use a Voxie Menu to Save & Load a file.
It also shows how to intergrate a Windows dialog for Saving and Opening Files
Due to how the Voxon hardware works the windows dialogs need to be in a new thread otherwise the VX1 will stop the recipcating screen.
This demo allows you to open up various 3D file types and arrange them in the volume so there are a few examples of rotating and using OBJs

* Opens/Saves a file using a windows dialog
* Opens/Saves a file using a voxieText box
* 

*/




#endif
#include "voxiebox.h"
#include "commdlg.h" // used for dialogs
#include <process.h> // used for multithreading
#include <math.h>
#include <stdio.h>
#include <stdlib.h> // used for malloc 
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

enum // menu names
{
	MENU_OPEN_DIALOG_TXT, MENU_SAVE_DIALOG_TXT, MENU_EDIT_TEXT, MENU_FILE_NAME,
	MENU_SAVE_DIRECT, MENU_OPEN_DIRECT, MENU_OPEN_OBJ, MENU_SAVE_SCENE, MENU_LOAD_SCENE
	
}; 




#define MAX_FILE 260 //


char gTestMessage[1000];
char gFilepath[MAX_FILE];
bool gDialogOpen = false;

// simple 3d Object xRot, yRot, zRot used to remember rotation data 
typedef struct { point3d vel, pos, xRot, yRot, zRot, size; int col; char filename[MAX_FILE];  } obj3d_t;
#define OBJECT3D_MAX 100
int obj3dNo = 0;
static obj3d_t obj3ds[OBJECT3D_MAX];

// copies a 3D obj from the ID to a new occurance
void obj3DCopy( int id) {


	if (obj3dNo < OBJECT3D_MAX) {

		obj3ds[obj3dNo].pos.x = obj3ds[id].pos.x; 
		obj3ds[obj3dNo].pos.y = obj3ds[id].pos.y; 
		obj3ds[obj3dNo].pos.z = obj3ds[id].pos.z; 

		obj3ds[obj3dNo].vel.x = obj3ds[id].vel.x;
		obj3ds[obj3dNo].vel.y = obj3ds[id].vel.y;
		obj3ds[obj3dNo].vel.z = obj3ds[id].vel.z;

		obj3ds[obj3dNo].xRot.x = obj3ds[id].xRot.x; 
		obj3ds[obj3dNo].xRot.y = obj3ds[id].xRot.y; 
		obj3ds[obj3dNo].xRot.z = obj3ds[id].xRot.z; 

		obj3ds[obj3dNo].yRot.x = obj3ds[id].yRot.x; 
		obj3ds[obj3dNo].yRot.y = obj3ds[id].yRot.y; 
		obj3ds[obj3dNo].yRot.z = obj3ds[id].yRot.z; 

		obj3ds[obj3dNo].zRot.x = obj3ds[id].zRot.x; 
		obj3ds[obj3dNo].zRot.y = obj3ds[id].zRot.y; 
		obj3ds[obj3dNo].zRot.z = obj3ds[id].zRot.z;

		obj3ds[obj3dNo].col = obj3ds[id].col;
	
		obj3ds[obj3dNo].size.x = obj3ds[id].size.x;
		obj3ds[obj3dNo].size.y = obj3ds[id].size.y;
		obj3ds[obj3dNo].size.z = obj3ds[id].size.z;

		strcpy(obj3ds[obj3dNo].filename,obj3ds[id].filename);
		
		obj3dNo++;
	}


}

// create 3D object
void obj3DCreate(char filename[MAX_FILE], point3d pos, point3d size, int col ) {

	if (obj3dNo < OBJECT3D_MAX) {

		obj3ds[obj3dNo].pos.x = pos.x;
		obj3ds[obj3dNo].pos.y = pos.y;
		obj3ds[obj3dNo].pos.z = pos.z;

		obj3ds[obj3dNo].vel.x = 0;
		obj3ds[obj3dNo].vel.y = 0;
		obj3ds[obj3dNo].vel.z = 0;

		obj3ds[obj3dNo].xRot.x = size.x;
		obj3ds[obj3dNo].xRot.y = 0;
		obj3ds[obj3dNo].xRot.z = 0;

		obj3ds[obj3dNo].yRot.x = 0;
		obj3ds[obj3dNo].yRot.y = size.y;
		obj3ds[obj3dNo].yRot.z = 0;

		obj3ds[obj3dNo].zRot.x = 0;
		obj3ds[obj3dNo].zRot.y = 0;
		obj3ds[obj3dNo].zRot.z = size.z;

		obj3ds[obj3dNo].col = col;
	
		obj3ds[obj3dNo].size.x = size.x;
		obj3ds[obj3dNo].size.y = size.y;
		obj3ds[obj3dNo].size.z = size.z;

		strcpy(obj3ds[obj3dNo].filename,filename);
		
		obj3dNo++;
	}

}


void loadObj3D() {

	point3d pp = {0,0,0}, size = {1,1,1};

	//check if file exention is legitmate
	int i = 0;
	int result = 0;

	// check if file is of the right exention
	for (i = 0; i < MAX_FILE; ++i) {
		if (gFilepath[i] == '\0'
		 	&& tolower(gFilepath[i - 1]) == 'j' 
			&& tolower(gFilepath[i - 2]) == 'b'
			&& tolower(gFilepath[i - 3]) == 'o'
			&& gFilepath[i - 4] == '.')  {	
			result = 1;
			break;
		}
		
		if (gFilepath[i] == '\0'
		 	&& gFilepath[i - 1] == '6' 
			&& tolower(gFilepath[i - 2]) == 'v'
			&& tolower(gFilepath[i - 3]) == 'k'
			&& gFilepath[i - 4] == '.')  {	
			result = 1;
			break;
		}


		if (gFilepath[i] == '\0'
		 	&& tolower(gFilepath[i - 1]) == 'l' 
			&& tolower(gFilepath[i - 2]) == 't'
			&& tolower(gFilepath[i - 3]) == 's'
			&& gFilepath[i - 4] == '.')  {	
			result = 1;
			break;
		}

		if (gFilepath[i] == '\0'
		 	&& (gFilepath[i - 1]) == 'y' 
			&& (gFilepath[i - 2]) == 'l'
			&& (gFilepath[i - 3]) == 'p'
			&& gFilepath[i - 4] == '.')  {	
			result = 1;
			break;
		}

					
	}

	if (result != 0) {
		obj3DCreate(gFilepath, pp, size, 0x404040);
	} else {
			MessageBox(0, "File must be .obj, .kv6, .ply or .stl", "Error: invalid file", MB_OK); 
	}
	
}







// draw 3d object
void obj3DDraw() {
	int i = 0;
	point3d pp, dd, rr, ff;
	for(i = 0; i < obj3dNo; i++) {

		pp.x = obj3ds[i].pos.x; 							pp.y = obj3ds[i].pos.y; 						pp.z = obj3ds[i].pos.z;
		rr.x = obj3ds[i].xRot.x * obj3ds[i].size.x; 		rr.y = obj3ds[i].xRot.y * obj3ds[i].size.y; 	rr.z = obj3ds[i].xRot.z * obj3ds[i].size.z;
		dd.x = obj3ds[i].yRot.x * obj3ds[i].size.x; 		dd.y = obj3ds[i].yRot.y * obj3ds[i].size.y; 	dd.z = obj3ds[i].yRot.z * obj3ds[i].size.z;
		ff.x = obj3ds[i].zRot.x * obj3ds[i].size.x; 		ff.y = obj3ds[i].zRot.y * obj3ds[i].size.y; 	ff.z = obj3ds[i].zRot.z * obj3ds[i].size.z;

		voxie_drawspr(&vf, (char*)obj3ds[i].filename, &pp, &rr, &dd, &ff, obj3ds[i].col);
	}

}




// requires to include #include "commdlg.h" and to link comdlg32.lib
void saveFileDialog() {  

	OPENFILENAME ofn;       	// common dialog box structure
	char szFile[MAX_FILE];      // buffer for file name
	//HWND hwnd;              	// owner window
	//HANDLE hf;              	// file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // hwnd
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)==TRUE) {

	}
			
	// update gFilepath with the new filename
	strcpy(gFilepath,ofn.lpstrFile );


}


// requires to include #include "commdlg.h" and to link comdlg32.lib
void openFileDialog() {

	OPENFILENAME ofn;       	// common dialog box structure
	char szFile[MAX_FILE];      // buffer for file name
	//HWND hwnd;            	// owner window  -- not needed for us
	//HANDLE hFile;          	// file handle  -- not needed for us

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // hwnd
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn) == TRUE) {
		// you could create a proper handle for the file
		// but we will just copy the file path to our global filepath
		strcpy(gFilepath, szFile);
	}


	
}


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

	strcpy(gTestMessage, dataBuff);
	
	fclose(fp);
    return true;

}


bool saveTxt(char* filename, char* message) {

	FILE* fp;
	int i;

	// check  to see if the file name is empty
	if (filename == NULL || filename[0] == '\0') return false;

	char _filename[MAX_FILE];
	
	/*
	// add the .txt extension 
	strcpy(_filename, filename);
	char* fileType = (char*)".txt";
	strcat(_filename, fileType);
	*/

	fp = fopen (_filename,"w");

	fprintf (fp, message);
	fclose(fp);

 	return true;

}

void obj3DDebug(float x, float y) {
	int i = 0;
	voxie_debug_print6x8_(x, y, 0xffffff, -1, "obj3dNo = %d", obj3dNo); 

	for (i = 0; i < obj3dNo; i++) {

		voxie_debug_print6x8_(x, y + 12 + (i * 12), obj3ds[i].col, -1, "%s col: %x | Pos %1.2f %1.2f %1.2f | Vel %1.2f %1.2f %1.2f | XRot %1.2f %1.2f %1.2f | YRot %1.2f %1.2f %1.2f | ZRot %1.2f %1.2f %1.2f",
		obj3ds[i].filename, obj3ds[i].col,
		obj3ds[i].pos.x, obj3ds[i].pos.y, obj3ds[i].pos.z,
		obj3ds[i].vel.x, obj3ds[i].vel.y, obj3ds[i].vel.z,
		obj3ds[i].xRot.x, obj3ds[i].xRot.y, obj3ds[i].xRot.z,
		obj3ds[i].yRot.x, obj3ds[i].yRot.y, obj3ds[i].yRot.z,
		obj3ds[i].zRot.x, obj3ds[i].zRot.y, obj3ds[i].zRot.z ); 

	} 

}

// save the scene data as a .scn file
bool saveScene() {
	
	// save trac struct 
	FILE* f;
	int i;
	INT64 j;
	char newFile[MAX_FILE];
	saveFileDialog();

	if (gFilepath == NULL || gFilepath[0] == '\0') return false;

	strcpy(newFile, gFilepath);

	// add the .scn extention 
	for (i = 0; i < MAX_FILE; ++i) {
		if (newFile[i] == '\0') {
			newFile[i] = '.';
			newFile[i + 1] = 's';
			newFile[i + 2] = 'c';
			newFile[i + 3] = 'n';
			newFile[i + 4] = '\0';
			break;		
		}
	}

	// work out how many active objs are in the array
	j = obj3dNo;

	// save a file with all the obj in them
	f = fopen(newFile, "wb");
	fwrite(obj3ds, sizeof(obj3d_t) * j, 1, f);
	if (f != NULL) 	fclose(f);

	return true;

}

bool loadScene() {
	int i = 0, j = 0;
	long eof;
	int result = 0;

	openFileDialog();

	if (gFilepath == NULL || gFilepath[0] == '\0') return false;


	// check if file is of the right exention
	for (i = 0; i < MAX_FILE; ++i) {
		if (gFilepath[i] == '\0'
		 	&& tolower(gFilepath[i - 1]) == 'n' 
			&& tolower(gFilepath[i - 2]) == 'c'
			&& tolower(gFilepath[i - 3]) == 's'
			&& gFilepath[i - 4] == '.')  {	
			result = 1;
			break;
		}
	}


	if (result == 0) {

			MessageBox(0, "File must be a valid .scn file", "Error: invalid file", MB_OK); 
			return false;
	}

	FILE* f;
	if (!(f = fopen(gFilepath, "rb"))) return false;
		
	
	else {
		f = fopen(gFilepath, "rb");
		

		// seek to end of file
		fseek(f, 0, SEEK_END);
		eof = ftell(f);
		rewind(f);


		// work how many 3d objs are in the file
		obj3dNo = eof / sizeof(obj3d_t);
				
		obj3d_t* _obj3ds = (obj3d_t*)malloc(eof);
		fread(_obj3ds, sizeof(obj3d_t) * obj3dNo, 1, f);
			   
		memcpy(obj3ds, _obj3ds,eof);
		free(_obj3ds);

		return true;
	}
}

// A function to run as a thread needs void *param
void openFileInThread(void *param) {
	gDialogOpen = true; // HAK these dialog opens is to prevent voxiebox from stealing the clicks
	openFileDialog();
	loadObj3D();
	gDialogOpen = false;
	_endthread(); // anything after the _endthread(); will not execute 

}

// A function to run as a thread needs void *param
void saveSceneInThread(void *param) {
	gDialogOpen = true; // HAK these dialog opens is to prevent voxiebox from stealing the clicks
	saveScene();
	gDialogOpen = false;
	_endthread();

}

// A function to run as a thread needs void *param
void loadSceneInThread(void *param) {
	gDialogOpen = true; // HAK these dialog opens is to prevent voxiebox from stealing the clicks
	loadScene();
	gDialogOpen = false;
	_endthread();

}


//Rotate 3D vectors a & b around their common plane, by ang
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


static int menu_update (int id, char *st, double v, int how, void *userdata)
{
	

	switch(id)
	{
		case MENU_FILE_NAME: 
			strcpy(gFilepath,st);
			break;
		case MENU_SAVE_DIALOG_TXT: 
			saveFileDialog(); // this isnt within a thread - on real VX hardware it will crash if the dialog is open for too long
			saveTxt(gFilepath, gTestMessage);
			break;
		case MENU_SAVE_DIRECT: 
			saveTxt(gFilepath, gTestMessage);
			break;
		case MENU_OPEN_DIALOG_TXT: 
			openFileDialog(); // this isnt within a thread - on real VX hardware it will crash if the dialog is open for too long 
			loadTxt(gFilepath);
			break; 
		case MENU_OPEN_DIRECT:
			loadTxt(gFilepath);
			break;
		case MENU_EDIT_TEXT:
			strcpy(gTestMessage,st);
			break;
		case MENU_OPEN_OBJ:
			if (gDialogOpen == false) _beginthread(openFileInThread,0,NULL); 	// start a new thread		
		break;
		case MENU_SAVE_SCENE:
			if (gDialogOpen == false) _beginthread(saveSceneInThread,0,NULL); 	
		break;
		case MENU_LOAD_SCENE:
			if (gDialogOpen == false) _beginthread(loadSceneInThread,0,NULL); 	
		break;
	
		
	}
	return(1);
}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int mousx = 256, mousy = 256, mousz = 0; 
	point3d pp, rr, dd, ff; 
	int i;
	int ovxbut[4], vxnplays; 
	int inited = 0; 
	int debug = 1;
	int currentObj = 0;
	float f = 0, g = 0;
	int cr = 0,cg = 0,cb = 0;

	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	voxie_menu_reset(menu_update,0, 0); // resets the menu tab
	voxie_menu_addtab("FileIO",		350,0,500,500); // add a tab to the top of the menu be careful not to add to many as they will be hidden on the VX1
	
	voxie_menu_additem("Test Message", 200,25, 64, 64,0,	MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); 

	voxie_menu_additem("Test Message",	50, 50,400,100,	 MENU_EDIT_TEXT,MENU_EDIT ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("Save Dialog Txt",		50,155,200,50,	 MENU_SAVE_DIALOG_TXT,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Open Dialog Txt",		250,155,200,50,	 MENU_OPEN_DIALOG_TXT,MENU_BUTTON+3,0,0x0880FF,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("File Name",	50, 260,400,50,	 MENU_FILE_NAME,MENU_EDIT_DO ,0 ,0x808080,0.0,0.0,0.0,0.0,0.0);

	voxie_menu_additem("S/O File Name Direct", 160,240, 64, 64,0,	MENU_TEXT    ,0             ,0xFF0000,0.0,0.0,0.0,0.0,0.0); 
	voxie_menu_additem("Save Direct",		50,300,200,50,	 MENU_SAVE_DIRECT,MENU_BUTTON+3,0,0x08FF80,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Open Direct",		250,300,200,50,	 MENU_OPEN_DIRECT,MENU_BUTTON+3,0,0x0880FF,0.0,0.0,0.0,0.0,0.0);


	voxie_menu_additem("Save Scene",		50,440,200,50,	 MENU_SAVE_SCENE,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);
	voxie_menu_additem("Load Scene",		250,440,200,50,	 MENU_LOAD_SCENE,MENU_BUTTON+3,0,0x808080,0.0,0.0,0.0,0.0,0.0);


	voxie_menu_additem("Load 3D Model",		50,375,400,50,	 MENU_OPEN_OBJ,MENU_BUTTON+3,0,0xFF8080,0.0,0.0,0.0,0.0,0.0);


	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{

	/**************************
	*  INPUT                  *
	*                         *
	**************************/

		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 
		mousx += in.dmousx; 
		mousy += in.dmousy; 
		mousz += in.dmousz; 
			if (voxie_keystat(0x1)) // esc key closes ap
		{
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}
	
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1); // keys '[' and ']' to rotate the emulator view
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

		// Input controls for 3D Models
		f = 0.5; // speed of movement

		// adjust scale 
		if (voxie_keystat(0x0d)) { // kb +
				obj3ds[currentObj].size.x += f * dtim;
				obj3ds[currentObj].size.y += f * dtim;
				obj3ds[currentObj].size.z += f * dtim;		
			}
		if (voxie_keystat(0x0c)) { // kb -
				obj3ds[currentObj].size.x -= f * dtim;
				obj3ds[currentObj].size.y -= f * dtim;
				obj3ds[currentObj].size.z -= f * dtim;		
			}

		// extract the separate colour values from the current Obj's colour
		cb = (obj3ds[currentObj].col & 0xFF);	
		cg = (obj3ds[currentObj].col >> 8) & 0xFF;	
		cr = (obj3ds[currentObj].col >> 16) & 0xFF;

		if (voxie_keystat(0x10) && cr < 0xFF) { // kb Q increase red
			cr += 1;
		}
		if (voxie_keystat(0x1e) && cr > 0x00) { // kb A decrease red
			cr -= 1;	
		}
		if (voxie_keystat(0x11) && cg < 0xFF) { // kb W increase green
			cg += 1;
		}
		if (voxie_keystat(0x1f) && cg > 0x00) { // kb S decrease green
			cg -= 1;	
		}
		if (voxie_keystat(0x12) && cb < 0xFF) { // kb E increase blue
			cb += 1;
		}
		if (voxie_keystat(0x20) && cb > 0x00) { // kb D decrease blue
			cb -= 1;	
		}

		// combine the three colour values into a single hex number.
		obj3ds[currentObj].col = (cr << 16) | (cg << 8) | (cb);

		if (voxie_keystat(0x2a)) { // if left shift is held rotate

			if (voxie_keystat(0xc8)) { // arrow up rotate Y 
				rotvex( f * dtim, &obj3ds[currentObj].xRot, &obj3ds[currentObj].zRot);
			}
			if (voxie_keystat(0xd0)) { // arrow down rotate Y
				rotvex( -f * dtim, &obj3ds[currentObj].xRot, &obj3ds[currentObj].zRot);
			}
			if (voxie_keystat(0xcb)) { // arrow left  rotate X
				rotvex( f * dtim, &obj3ds[currentObj].xRot, &obj3ds[currentObj].yRot);
			}
			if (voxie_keystat(0xcd)) { // arrow right  rotate X
				rotvex( -f * dtim, &obj3ds[currentObj].xRot, &obj3ds[currentObj].yRot);
			}
		


		} else {
			// adjust position of the model

			if (voxie_keystat(0xc8)) { // arrow up 
				obj3ds[currentObj].pos.y -= f * dtim;
			}
			if (voxie_keystat(0xd0)) { // arrow down 
				obj3ds[currentObj].pos.y += f * dtim;
			}
			if (voxie_keystat(0xcb)) { // arrow left 
				obj3ds[currentObj].pos.x -= f * dtim;
			}
			if (voxie_keystat(0xcd)) { // arrow right 
				obj3ds[currentObj].pos.x += f * dtim;
			}
			if (voxie_keystat(0x36)) { // right shift 
				obj3ds[currentObj].pos.z -= f * dtim;
			}
			if (voxie_keystat(0x9d)) { // right ctrl 
				obj3ds[currentObj].pos.z += f * dtim;
			}
		

		}


		//Rotate Z 
		if (voxie_keystat(0x33)) { // kb <
			rotvex( f * dtim, &obj3ds[currentObj].yRot, &obj3ds[currentObj].zRot);
		}
		if (voxie_keystat(0x34)) { // kb > 
			rotvex( -f * dtim, &obj3ds[currentObj].yRot, &obj3ds[currentObj].zRot);
		}
		// kb  tab change focus
		if (voxie_keystat(0x0f) == 1) { 
			currentObj++;
			if (currentObj > obj3dNo) currentObj = 0;
		}
		// kb space to copy OBJ
		if (voxie_keystat(0x39) == 1 && obj3dNo > 0) { 
			obj3DCopy(currentObj);
		}

		// kb backspace to delete last created moddel
		if (voxie_keystat(0x0e) == 1 && obj3dNo > 0) {
			obj3dNo--;
		}
	

		/**************************
		*   DRAW                  *
		*                         *
		**************************/


	    voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 


		// draw cursor
		if (currentObj < obj3dNo) {
			g = .2;
			voxie_drawbox(&vf, 	obj3ds[currentObj].pos.x - g, obj3ds[currentObj].pos.y - g, obj3ds[currentObj].pos.z - g,
								obj3ds[currentObj].pos.x + g, obj3ds[currentObj].pos.y + g, obj3ds[currentObj].pos.z + g, 
								1, 0xFF0000	  );
		}

		// update menu test message (if it has been loaded in)
		voxie_menu_updateitem(MENU_EDIT_TEXT, gTestMessage,0,0);
		// draw 3D objects	
		obj3DDraw();

		/**************************
		*   DEBUG                 *
		*                         *
		**************************/
	
		if (debug == 1) // if debug is set to 1 display these debug messages
		{
			// debug 3d objects
			obj3DDebug(30,220);

			//draw wireframe box around the edge of the screen
			voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);
	
			//display VPS
			avgdtim += (dtim - avgdtim) * .1;
			voxie_debug_print6x8_(30, 68, 0xffc080, -1, "VPS %5.1f, %s || %s || current OBJ %d", 1.0 / avgdtim, gFilepath, gTestMessage, currentObj); 

			voxie_debug_print6x8_(30, 90, 0x00ffff, -1, "File IO - a demo to show how to load/save files + OBJ loading and manipulation\nBy Matthew Vecchio/Ready Wolf for Voxon 8/10/2020"); 

			voxie_debug_print6x8_(30, 110, 0xff00ff, -1, "3D Object manipulation\nOnly Keyboard is supported\n\n- / + (not on the numpad) : Adjust scale\nArrow Keys                : Move position\nLeft Shift + Arrow Keys   : Rotate model (Y and X)\n< > Keys                  : Rotate Model (Z)\nSpace Bar                 : Copy current selected model\nTab                       : Switch focus of current model\nBackspace                 : Delete last created model\nQ / A                     : Adjust red colour in model (.OBJ and .KV6 only)\nW / S                     : Adjust green colour in model (.OBJ and .KV6 only)\nE / D                     : Adjust blue colour in model (.OBJ and .KV6 only)\n"); 
			
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
