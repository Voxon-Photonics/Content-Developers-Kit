#include "vxCPP.h"

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();
	
	while (voxie->breath())
	{
		voxie->startFrame();   
		voxie->showVPS();
		voxie->endFrame(); 
	}

	voxie->quitLoop(); 
	delete voxie;
	return 0;   
}
