#include "vxCPP.h"

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	if (!hVoxieDLL) return 1;
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, (const char*)R"(CreateVoxieBoxObject)");
	if (pEntryFunction == 0) return 1;
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
