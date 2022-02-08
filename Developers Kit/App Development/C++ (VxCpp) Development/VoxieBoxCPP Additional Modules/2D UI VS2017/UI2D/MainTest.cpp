#include "vxCPP.h"
#include "UI2D.h"

// WINDOW2D -- DONE
// BUTTON2D -- DONE
// VSLIDER2D -- DONE
// HSLIDER2D -- DONE
// PINCHBOX2D -- DONE

// GROUPBOX -- like a window but can manage text elements and 
// WINDOW + TEXTBOX + BUTTON array

// MESSAGE BOX  :: TYPE OF GROUPBOX

// TODO: 
// GROUP -- uses a vector to 'add' IUI2D elements together - so they can all be moved together
// TEXTBOX (/w Editable text) 
// MESSAGE BOX

// Test Class for 2DUI elements - By Matthew Vecchio for Voxon November 2021. -- Feel free to use this code in your own programs.

int main(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	HINSTANCE _Notnull_ hVoxieDLL = LoadLibrary("vxCpp.dll");
	CREATE_VOXIEBOX pEntryFunction = (CREATE_VOXIEBOX)GetProcAddress(hVoxieDLL, "CreateVoxieBoxObject");
	IVoxieBox* voxie = pEntryFunction();

	voxie->setEnableTouchInput(true);

	Window2D	box		(voxie, 1000, 300, 200, 500, 0x800000);
	VSlider2D	vslide	(voxie, 300, 300, 200, 10, 0, 1, 0x0000ff);
	HSlider2D	hslide	(voxie, 600, 300, 200, 10, 0, 1, 0x00ff00);
	Button2D	button	(voxie, (char*)"test", 800, 100, 200, 200, 0x309090);
	PinchBox2D	pinch	(voxie, 100, 600, 400, 400, 0x303030);
	Element2D	element1(voxie, 50, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element2(voxie, 160, 120, 100, 100, 0x303000, 0xffffff);
	Element2D	element3(voxie, 270, 120, 100, 100, 0x003030, 0xffffff);
	Element2D	element4(voxie, 270, 120, 100, 100, 0x303030, 0xffffff);
	Element2D	element5(voxie, 270, 120, 100, 100, 0x300030, 0xffffff);
	Element2D	element6(voxie, 270, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element7(voxie, 50, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element8(voxie, 160, 120, 100, 100, 0x303000, 0xffffff);
	Element2D	element9(voxie, 270, 120, 100, 100, 0x003030, 0xffffff);
	Element2D	element10(voxie, 270, 120, 100, 100, 0x303030, 0xffffff);
	Element2D	element11(voxie, 270, 120, 100, 100, 0x300030, 0xffffff);
	Element2D	element12(voxie, 270, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element13(voxie, 270, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element14(voxie, 50, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element15(voxie, 160, 120, 100, 100, 0x303000, 0xffffff);
	Element2D	element16(voxie, 270, 120, 100, 100, 0x003030, 0xffffff);
	Element2D	element17(voxie, 270, 120, 100, 100, 0x303030, 0xffffff);
	Element2D	element18(voxie, 270, 120, 100, 100, 0x300030, 0xffffff);
	Element2D	element19(voxie, 270, 120, 100, 100, 0x003000, 0xffffff);
	Element2D	element20(voxie, 270, 120, 100, 100, 0x003000, 0xffffff);
	int x = 0;
	int y = 0;
	int z = 0;
	float rot = 10;
	float dist = 10;

	x = 2;

	element1.setMessage((char*)"ele 1 %d", x);
	element2.setMessage((char*)"ele 2 %d", x);
	element3.setMessage((char*)"ele 3 %d", x);
	element4.setMessage((char*)"ele 4 %d", x);
	element5.setMessage((char*)"ele 5 %d", x);
	element6.setMessage((char*)"ele 6 %d", x);

	element7.setMessage((char*)"ele 7 %d", x);
	element8.setMessage((char*)"ele 8 %d", x);
	element9.setMessage((char*)"ele 9 %d", x);
	element10.setMessage((char*)"ele 10 %d", x);
	element11.setMessage((char*)"ele 11 %d", x);
	element12.setMessage((char*)"ele 12 %d", x);

	element13.setMessage((char*)"ele 13 %d", x);
	element14.setMessage((char*)"ele 14 %d", x);
	element15.setMessage((char*)"ele 15 %d", x);
	element16.setMessage((char*)"ele 16 %d", x);
	element17.setMessage((char*)"ele 17 %d", x);
	element18.setMessage((char*)"ele 18 %d", x);
	element19.setMessage((char*)"ele 19 %d", x);
	element20.setMessage((char*)"ele 20 %d", x);



	box.setTitle((char*)"My Window %d", x - 1);
	box.setEnableTitle(true);
	box.autoArrangeElementsStyle(1);
	box.addElement(&element1);
	box.addElement(&element2);
	box.addElement(&element3);
	box.addElement(&element4);
	box.addElement(&element5);
	box.addElement(&element6);
	box.addElement(&element7);
	box.addElement(&element8);
	box.addElement(&element9);
	box.addElement(&element10);
	box.addElement(&element11);
	box.addElement(&element12);
	box.addElement(&element14);
	box.addElement(&element15);
	box.addElement(&element16);
	box.addElement(&element17);
	box.addElement(&element18);
	box.addElement(&element19);
	box.addElement(&element20);
	   
	box.setElementSpacing(110, 110);
	
	box.setShowElements(true);

	while (voxie->breath())
	{
		voxie->startFrame();

		if (voxie->getKeyOnDown(KB_1)) vslide.setMinValue(rand() % 100);
		if (voxie->getKeyOnDown(KB_2)) vslide.setMaxValue(rand() % 100);
		if (voxie->getKeyOnDown(KB_3)) vslide.setValue(15);
		
		vslide.draw();
		box.draw();
		hslide.draw();
		button.draw();
		pinch.draw();
		
		button.setImage((char*)"sample.png", 272, 170, -25, 0);
		
		x = box.isTouched();
		y = vslide.isTouched();
		z = hslide.isTouched();

		if (button.getButtonState() == TOUCH_STATE_JUST_PRESSED) {
			voxie->debugText(button.getPosX(), button.getPosY(), 0xff0000, -1, "Pressed");	
		}

		if (button.getButtonState() == TOUCH_STATE_ON_UP) {
			voxie->debugText(button.getPosX(), button.getPosY(), 0x00ff00, -1, "On Up");
		}

		box.update();
		vslide.update();
		hslide.update();
		button.update();
		pinch.update();
						
		rot += pinch.getPinchRotationDelta() * 100;
		dist += pinch.getPinchDistanceDelta();

		voxie->debugDrawCircFill(500, 500, (int)rot, 0xffff00);
		voxie->debugDrawCircFill(800, 500, (int)dist, 0x00ffff);
		
		voxie->debugText(30, 85, 0x00ff00, -1, "pinch minR %d rel Pos X %d Y %d | DX %d DY %d, Rot %1.2f Dis %1.2f", pinch.getMinPinchResponse(), pinch.getRelPositionX(), pinch.getRelPositionY(),
			pinch.getGlobalDeltaX(), pinch.getGlobalDeltaY(), pinch.getPinchRotationDelta(), pinch.getPinchDistanceDelta());

		voxie->debugText(30, 75, 0xffffff, -1, "box touch %d vslide touch %d hslide touch %d but is touched %d state %d pinch %d ", x, y, z, button.isTouched(), button.getButtonState(), pinch.isTouched());
		
		voxie->showVPS();
		voxie->endFrame();
	}

	voxie->quitLoop();
	delete voxie;
	return 0;
}
