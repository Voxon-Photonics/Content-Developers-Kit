// VS2017 Template for C++.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>

#define PI 3.14159265358979323
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "voxieBoxCPP.h"


int main()
{

	// load in the voxieBoxCPP.dll
	HINSTANCE hVoxieDLL = LoadLibrary("voxieBoxCPP.dll");

	if (hVoxieDLL == NULL) {
		MessageBox(0, (char *)"Error: can't load voxieBoxCPP.dll", (char *)"", MB_OK);
		return (-1);
	}

	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	Ivoxiebox* voxie = pEntryFunction();
	// anything before voxie->Breath is called once 
	// use this space to setup your program
	

	// toggle show on (draws a wireframe of the volume
	voxie->setBorder(true);

	// Hello world text variables - safe to remove these variables
	point3d textPos = { -1,-1,0 };
	point3d textWidth = { .1, 0,0 };
	point3d textHeight = { 0, .15, 0 };
	int textColour = 0xffffff;


	// Breath is the update loop
	while (voxie->Breath())
	{
		voxie->StartFrame(); // The start of drawing the Voxieframe (all voxie draw calls need to be within this and the EndFrame() function

		textPos.z = cos(voxie->Time()) / 5;
		voxie->PrintAlpha(&textPos, &textWidth, &textHeight, textColour, "Hello World");

		voxie->showVPS();

		voxie->EndFrame();
	}

	// Quit stops the hardware from physically moving 
	voxie->Quit();
	return 0;
	// After the program quits the deconstructor for voxiebox frees the DLLs from memory if you wanted to do this manually call the voxie->Shutdown()
}

