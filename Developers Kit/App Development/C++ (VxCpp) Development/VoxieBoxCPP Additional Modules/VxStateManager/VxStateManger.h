#pragma once


enum VxState { // put in your states here

	SETUP,
	MAIN,
	EXIT

};


/* A Class to manage a state. 
 * Customize this class as you see fit to help manage your application. 
 * By Matthew Vecchio for Voxon 
 * 
 * Edit the update function to manage where the program is at. 
 */
class VxStateManager
{
	public:

		VxStateManager();
		~VxStateManager();

		void		setState(VxState state);
		VxState		getState();
		VxState		getPreviousState();
		

		VxState update();

	private: 

		VxState intitalState = SETUP;
		VxState currentState = SETUP;
		VxState previousState = SETUP;




};

