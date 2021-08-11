#pragma once

#include <vxDataTypes.h>
#include "vxCpp.h"
#include "vxInterfaces.h"

//! plot
class PlotPoint : public IDrawable, public IActive
{
public:
	PlotPoint();
	PlotPoint(IVoxieBox * voxiePTR, point3d pos, float radius, int fillMode, int col);
	~PlotPoint();


	void	update(point3d pos, float radius, int fillMode, int col);

	void	setIVoxieBox(IVoxieBox * voxiePtr);

	void	setHeldValue(double newValue);
	double	getHeldValue();
	void    setColour(int colour);
	int		getColour();

	void	setPos(point3d pos);
	void	setXPos(float x);
	void	setYPos(float y);
	void	setZPos(float z);

	point3d getPos();
	float	getXPos();
	float	getYPos();
	float	getZPos();

	void	setRadius(float radius);
	float	getRadius();

	void    setFillMode(int newFillMode);
	int		getFillMode();


	void    setFontSize(float newFontSize);
	void    setShowValue(bool option);
	void    setShowPos(bool option);

	void    draw();




private:

	IVoxieBox* voxie = NULL;
	int			colour = 0xffffff;
	point3d		pos = { 0 };
	float		radius = 0;
	double		heldValue = 0;
	int			fill = 0;
	bool		showPos = false;
	bool		showValue = false;
	float		fontSize = 0.05;
};

