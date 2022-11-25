#include "VxStateManger.h"

VxStateManager::VxStateManager()
{
	this->currentState = this->intitalState;
}

VxStateManager::~VxStateManager()
{
}

void VxStateManager::setState(VxState state)
{
	this->currentState = state;
}

VxState VxStateManager::getState()
{
	return currentState;
}

VxState VxStateManager::getPreviousState()
{
	return previousState;
}

/* updates the current state -> any processing that needs to be done etc*/

VxState VxStateManager::update()
{

	previousState = currentState;

	// put any processes here that need to be done...


	return currentState;
}
