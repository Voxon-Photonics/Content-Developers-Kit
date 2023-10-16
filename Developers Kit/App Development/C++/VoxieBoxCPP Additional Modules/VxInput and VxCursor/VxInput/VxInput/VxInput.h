#pragma once

#include "vxCpp.h"
#include <vector>
#include <string>

/** VxInput is a class that the developer can customize to turn all inputs into actions. 
 *  This way of managing inputs might appeal to some developers see the main example. The benefit here is that all inputs turn into an 'action' which can be easier to update. 
 *
 */

// put the  actions in here which the program wishes to do

enum input_action_description {

	AX_RESET_SPHERES,
	AX_CURSOR_FLASH,
	AX_CURSOR_CLICK,
	AX_CURSOR_MOVEMENT,


};


// an input action holds all the info you'll need to deceipiter (or add your own...)
typedef struct { input_action_description inputAction; int id, i; point3d point; float f; double d; std::string st; }  inputActionMsg_t;


class VxInput : public IVoxiePtr
{

public:

	VxInput(IVoxieBox* voxiePtr);
	~VxInput();

	void update();
	void updateMouse();
	void updateTouch();
	void updateSpaceNav();
	void updateKeyboard();
	void updateJoysticks();
	
	const std::vector<inputActionMsg_t> getInput();
	void addInput(inputActionMsg_t inputAction);
	void flush();
	void report(int posX, int posY);


private:

	std::vector<inputActionMsg_t>  inputPool; // input pool is public so its a bit easier to 
	bool autoEmptyPool = true;


};

