#pragma once
// Collision to String - create a collision box and if something is inside it return a message


#include <vxDataTypes.h>
#include <vxCpp.h>

class collisionBox
{
public:


	collisionBox();
	collisionBox(Ivoxiebox * voxiePtr, point3d LUTpos, point3d RDBpos, char* setReturnMessage, float setReturnValue );
	~collisionBox();
	
	char*		getMessage();
	void		setMessage(char * newMessage);
	void		setValue(float newValue);
	float		getValue();
	void		setLUTPos(point3d LUTpos);
	void		setRDBPos(point3d RDBpos);
	void		update(Ivoxiebox * voxiePtr, point3d LUTpos, point3d RDBpos, char* setReturnMessage, float setReturnValue);
	void		update(point3d LUTpos, point3d RDBpos, char* setReturnMessage, float setReturnValue);
	void		setPos(point3d LUTpos, point3d RDBpos);
	void		setIvoxiebox(Ivoxiebox * voxiePtr);
	void		setActive(bool option);
	bool		isActive();
	void		setColour(int colour);

	int			checkCollision(point3d cursor);
	void		draw();


private:


	float		returnValue = 0;
	char*		returnMessage = (char*)"return message";
	point3d		LUTpoint = { 0 };
	point3d		RDBpoint = { 0 };
	Ivoxiebox * voxie = NULL;
	bool		active = false;
	int			colour = 0x00ff00;


};

