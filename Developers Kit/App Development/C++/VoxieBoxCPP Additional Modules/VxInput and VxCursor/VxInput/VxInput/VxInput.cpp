#include "VxInput.h"

VxInput::VxInput(IVoxieBox* voxiePtr)
{
	this->voxiePtr = voxiePtr;
}

VxInput::~VxInput()
{
}

void VxInput::update()
{

	if (voxiePtr == nullptr || voxiePtr == 0) return;


	// code to clean out the action list 1st
	if (autoEmptyPool == true) {

		this->inputPool.clear();

	}

	// then add the update functions here ...
	
	updateMouse();
	updateKeyboard();

		

	 
}


void VxInput::updateMouse()
{

	inputActionMsg_t action = { 0 };
	point3d pos = voxiePtr->getMouseDelta();

	// write the update function for the mouse movement here...


	if (pos.x != 0 || pos.y != 0 || pos.z != 0) {

		action.inputAction = AX_CURSOR_MOVEMENT;
		action.point = pos;
	
		

		this->inputPool.push_back(action);

	}

	if (voxiePtr->getMouseButtonOnDown(0)) {

		action.inputAction = AX_CURSOR_FLASH;
		action.i = 10;
		action.st = "flash";

		this->inputPool.push_back(action);

	}


}

void VxInput::updateKeyboard()
{
	inputActionMsg_t action = { 0 };

	point3d movement = { 0 };

	if (voxiePtr->getKeyOnDown(KB_R)) {

		action.inputAction = AX_RESET_SPHERES;
		this->inputPool.push_back(action);
	
	}

	if (voxiePtr->getKeyOnDown(KB_F)) {

		action.inputAction = AX_CURSOR_FLASH;
		action.i = 10;
		action.st = "flash";

		this->inputPool.push_back(action);

	}

	if (voxiePtr->getKeyIsDown(KB_Arrow_Up)) movement.y = -1;
	if (voxiePtr->getKeyIsDown(KB_Arrow_Down)) movement.y = 1;
	if (voxiePtr->getKeyIsDown(KB_Arrow_Left)) movement.x = -1;
	if (voxiePtr->getKeyIsDown(KB_Arrow_Right)) movement.x = 1;


	if (movement.x != 0 || movement.y != 0 || movement.z != 0) {

		action.inputAction = AX_CURSOR_MOVEMENT;
		action.point = movement;
		action.st = "movement";
		this->inputPool.push_back(action);

	}

}

const std::vector<inputActionMsg_t> VxInput::getInput()
{
	return inputPool;
}

void VxInput::report(int posX, int posY)
{

	if (voxiePtr == nullptr || voxiePtr == 0) return;

	voxiePtr->debugText(posX, posY, 0xffffff, -1, "VxInput V1 REPORT");
	posY += 8;
	voxiePtr->debugText(posX, posY, 0xffffff, -1, "auto clear pool : %d", this->autoEmptyPool);

	int  i = 0;
	posY += 8;
	for (i = 0; i < inputPool.size(); i++) {

		voxiePtr->debugText(posX, posY, voxiePtr->randomCol(), -1, "action %d, id: %d p: {%1.2f,%1.2f,%1.2f} i: %d f: %1.2f d: %1.2f s : %s ", inputPool[i].inputAction, inputPool[i].id, inputPool[i].point.x, inputPool[i].point.y, inputPool[i].point.z, inputPool[i].i, inputPool[i].f, inputPool[i].d, inputPool[i].st.c_str());
		posY += 8;

	}


}

void VxInput::addInput(inputActionMsg_t inputAction)
{
	inputPool.push_back(inputAction);
}

void VxInput::flush()
{
	this->inputPool.clear();
}