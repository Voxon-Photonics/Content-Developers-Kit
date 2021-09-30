#include "vxCPP.h"
#include "VxBeeper.h"
/**
 * VxBeeper test. 
 * VxBeeper demonstrates how to use the custom audio playback function for Voxon Development.
 * the VoxieBox::setAudioPlayCallBack allows a function to be called automatically to play back 
 * 
 *
 */


VxBeeper * beeperPtr; // make a pointer to reference the actual beeper

// A custom audio callback that just calls the better to handle it
// Once called this function will always be called on a separate thread. Use a global variable to turn it on / off
static void playBeeperMix(int* sampleBuffer, int sampleRate)
{

	beeperPtr->playMix(sampleBuffer, sampleRate); // use the pointer to play the mix of the actual object.
}



int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{

	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (hVoxieDLL == NULL) return 0;
	CREATE_VOXIEBOX pEntryFunction = pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();
	

	voxie->setAudioPlayCallBack(playBeeperMix); // initialise beeper
	VxBeeper beeper(voxie);
	beeperPtr = &beeper;

	while (voxie->breath())
	{
		voxie->startFrame();   
		voxie->showVPS();

		if (voxie->getKeyOnDown(KB_A)) beeper.playNote(C4, 1, 1.2, 0);
		if (voxie->getKeyOnDown(KB_S)) beeper.playNote(G5, 1, 1, 0);
		if (voxie->getKeyOnDown(KB_D)) beeper.playNote(E5, 2, 1, 0);
		if (voxie->getKeyOnDown(KB_W)) beeper.playNote(E4, 1, 4, 0);
		voxie->endFrame(); 
		
		
		
	}

	voxie->quitLoop(); 
	delete voxie;
	return 0;   
}
