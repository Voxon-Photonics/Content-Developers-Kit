#include "VxMoxie.h"

VxMoxie::VxMoxie()
{
}

VxMoxie::VxMoxie(IVoxieBox* voxiePtr)
{
	this->setIVoxieBoxPtr(voxiePtr);
}

VxMoxie::~VxMoxie()
{
}

void VxMoxie::setIVoxieBoxPtr(IVoxieBox* voxiePtr)
{
	this->voxiePtr = voxiePtr;
}

IVoxieBox* VxMoxie::getIVoxieBoxPtr()
{
	return this->voxiePtr;
}



void VxMoxie::setCam(point3d* pos, point3d* rVector, point3d* dVector, point3d* fVector)
{
	this->cPos.x = pos->x; 			this->cPos.y = pos->y; 			this->cPos.z = pos->z;
	this->cRVec.x = rVector->x; 	this->cRVec.y = rVector->y; 	this->cRVec.z = rVector->z;
	this->cDVec.x = dVector->x; 	this->cDVec.y = dVector->y; 	this->cDVec.z = dVector->z;
	this->cFVec.x = fVector->x; 	this->cFVec.y = fVector->y; 	this->cFVec.z = fVector->z;
}

void VxMoxie::xformPos(point3d* p)
{
	point3d t; t.x = p->x - cPos.x; t.y = p->y - cPos.y; t.z = p->z - cPos.z;
	p->x = t.x * cRVec.x + t.y * cDVec.x + t.z * cFVec.x;
	p->y = t.x * cRVec.y + t.y * cDVec.y + t.z * cFVec.y;
	p->z = t.x * cRVec.z + t.y * cDVec.z + t.z * cFVec.z;
}

void VxMoxie::xformOri(point3d* r, point3d* d, point3d* f, point3d* nr, point3d* nd, point3d* nf)
{

	nr->x = r->x * cRVec.x + r->y * cDVec.x + r->z * cFVec.x;
	nr->y = r->x * cRVec.y + r->y * cDVec.y + r->z * cFVec.y;
	nr->z = r->x * cRVec.z + r->y * cDVec.z + r->z * cFVec.z;
	nd->x = d->x * cRVec.x + d->y * cDVec.x + d->z * cFVec.x;
	nd->y = d->x * cRVec.y + d->y * cDVec.y + d->z * cFVec.y;
	nd->z = d->x * cRVec.z + d->y * cDVec.z + d->z * cFVec.z;
	nf->x = f->x * cRVec.x + f->y * cDVec.x + f->z * cFVec.x;
	nf->y = f->x * cRVec.y + f->y * cDVec.y + f->z * cFVec.y;
	nf->z = f->x * cRVec.z + f->y * cDVec.z + f->z * cFVec.z;

}

void VxMoxie::setCamPos(float x, float y, float z)
{
	this->cPos.x = x;
	this->cPos.y = y;
	this->cPos.z = z;
}

void VxMoxie::setCamPos(point3d* pos)
{
	this->cPos.x = pos->x;
	this->cPos.y = pos->y;
	this->cPos.z = pos->z;


}


void VxMoxie::zoomCamIn(float amount)
{

	//apply offset pos for rot&zoom around pivot
	float f = normalise();
	cPos.x += (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y += (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z += (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	f = pow(amount, voxiePtr->getDeltaTime());

	cRVec.x *= f; cRVec.y *= f; cRVec.z *= f; 
	cDVec.x *= f; cDVec.y *= f; cDVec.z *= f; 
	cFVec.x *= f; cFVec.y *= f; cFVec.z *= f;

	//remove offset pos for rot&zoom around pivot
	f = normalise();
	cPos.x -= (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y -= (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z -= (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;


	scaleUpdate();
}

void VxMoxie::zoomCamOut(float amount)
{

	//apply offset pos for rot&zoom around pivot
	float f = normalise();
	cPos.x += (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y += (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z += (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	amount = 0.5;
	f = pow(amount, voxiePtr->getDeltaTime());

	cRVec.x *= f; cRVec.y *= f; cRVec.z *= f; 
	cDVec.x *= f; cDVec.y *= f; cDVec.z *= f; 
	cFVec.x *= f; cFVec.y *= f; cFVec.z *= f;

	f = normalise();
	cPos.x -= (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y -= (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z -= (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	scaleUpdate();
}

void VxMoxie::setZoom(float amount)
{

	//apply offset pos for rot&zoom around pivot
	float f = normalise();
	cPos.x += (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y += (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z += (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	f = amount;

	cRVec.x *= f; cRVec.y *= f; cRVec.z *= f; cDVec.x *= f; cDVec.y *= f; cDVec.z *= f; cFVec.x *= f; cFVec.y *= f; cFVec.z *= f;

	f = normalise();
	//* better to forget the pivot...
	cPos.x -= (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y -= (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z -= (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;



	scaleUpdate();
}

void VxMoxie::resetRotation()
{
	cRVec.x = scale; cRVec.y = 0;		cRVec.z = 0;
	cDVec.x = 0;	cDVec.y = scale;	cDVec.z = 0;
	cFVec.x = 0;	cFVec.y = 0;		cFVec.z = scale;


}

void VxMoxie::scaleUpdate() {

	scale = sqrt((double)cRVec.x * (double)cRVec.x + (double)cRVec.y * (double)cRVec.y + (double)cRVec.z * (double)cRVec.z); //precalc for spheres & cones - only works if axes are orthogonal ;P
}

float VxMoxie::normalise() {
	return 1.f / (cRVec.x * cRVec.x + cRVec.y * cRVec.y + cRVec.z * cRVec.z);
}

void VxMoxie::rotCam(float xAmount, float yAmount, float zAmount)
{

	float f = 0;
	xAmount *= 1.5;
	yAmount *= 1.5;
	zAmount *= 1.5;

	xAmount = (xAmount * movementSpeed) * voxiePtr->getDeltaTime();
	yAmount = (yAmount * movementSpeed) * voxiePtr->getDeltaTime();
	zAmount = (zAmount * movementSpeed) * voxiePtr->getDeltaTime();

	//offset pos for rot&zoom around pivot
	f = normalise();
	cPos.x += (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y += (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z += (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	//Transpose so we can prepare for rotation
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f;
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;

	voxiePtr->rotVex((xAmount), &cRVec, &cDVec); // rotate xAmount
	voxiePtr->rotVex((yAmount), &cDVec, &cFVec); // rotate yAmount
	voxiePtr->rotVex((zAmount), &cRVec, &cFVec); // rotate zAmount

	//Transpose
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f;
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;

	//restore pos for rot&zoom around pivot
	f = normalise();

	cPos.x -= (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y -= (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z -= (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

}
void VxMoxie::setRotation(float xRad, float yRad, float zRad, bool isDegrees = false)
{
	float f = 0;

	if (isDegrees) {
		xRad = xRad * (PI / 180);
		yRad = yRad * (PI / 180);
		zRad = zRad * (PI / 180);
	}


	//offset pos for rot&zoom around pivot
	f = normalise();
	cPos.x += (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y += (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z += (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;

	//Transpose so we can prepare for rotation
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f;
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;

	voxiePtr->rotVex((xRad), &cRVec, &cDVec); // rotate xAmount
	voxiePtr->rotVex((yRad), &cDVec, &cFVec); // rotate yAmount
	voxiePtr->rotVex((zRad), &cRVec, &cFVec); // rotate zAmount

	//Transpose
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f;
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;

	//restore pos for rot&zoom around pivot
	f = normalise();

	cPos.x -= (piv.x * cRVec.x + piv.y * cRVec.y + piv.z * cRVec.z) * f;
	cPos.y -= (piv.x * cDVec.x + piv.y * cDVec.y + piv.z * cDVec.z) * f;
	cPos.z -= (piv.x * cFVec.x + piv.y * cFVec.y + piv.z * cFVec.z) * f;
}
void VxMoxie::moveCam(float xAmount, float yAmount, float zAmount)
{
	float f = 0;

	xAmount = (xAmount * movementSpeed) * voxiePtr->getDeltaTime();
	yAmount = (yAmount * movementSpeed) * voxiePtr->getDeltaTime();
	zAmount = (zAmount * movementSpeed) * voxiePtr->getDeltaTime();

	f = normalise();

	cPos.x += (xAmount * cRVec.x + yAmount * cRVec.y + zAmount * cRVec.z) * f;
	cPos.y += (xAmount * cDVec.x + yAmount * cDVec.y + zAmount * cDVec.z) * f;
	cPos.z += (xAmount * cFVec.x + yAmount * cFVec.y + zAmount * cFVec.z) * f;

}
void VxMoxie::movePivot(float xAmount, float yAmount, float zAmount)
{
	xAmount = (-xAmount * movementSpeedPiv) * voxiePtr->getDeltaTime();
	yAmount = (-yAmount * movementSpeedPiv) * voxiePtr->getDeltaTime();
	zAmount = (-zAmount * movementSpeedPiv) * voxiePtr->getDeltaTime();

	piv.x += xAmount;
	piv.y += yAmount;
	piv.z += zAmount;

	if (isScrollPivotEnabled) {

		xAmount = min(max(piv.x, -voxiePtr->getAspectX() + .25f), voxiePtr->getAspectX() - .25f) - piv.x;
		yAmount = min(max(piv.y, -voxiePtr->getAspectY() + .25f), voxiePtr->getAspectY() - .25f) - piv.y;
		zAmount = min(max(piv.z, -voxiePtr->getAspectZ() + .05f), voxiePtr->getAspectZ() - .05f) - piv.z;

		//scroll camera so pivot on border of bounds (and adjust pivot point so overall position doesn't change here)
		float f = normalise();
		cPos.x -= (xAmount * cRVec.x + yAmount * cRVec.y + zAmount * cRVec.z) * f;
		cPos.y -= (xAmount * cDVec.x + yAmount * cDVec.y + zAmount * cDVec.z) * f;
		cPos.z -= (xAmount * cFVec.x + yAmount * cFVec.y + zAmount * cFVec.z) * f;

		piv.x += xAmount;
		piv.y += yAmount;
		piv.z += zAmount;
	}

}
/*
void VxMoxie::setRadRotCamX(float radians)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	float f = 0;
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;
	voxiePtr->rotVex(radians, &cRVec, &cDVec);

	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;
}

void VxMoxie::setRadRotCamY(float radians)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	float f = 0;
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;

	voxiePtr->rotVex(radians, &cDVec, &cFVec);

	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;
}

void VxMoxie::setRadRotCamZ(float radians)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	float f = 0;
	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;
	voxiePtr->rotVex(radians, &cRVec, &cFVec);

	f = cRVec.y; cRVec.y = cDVec.x; cDVec.x = f; //Transpose
	f = cRVec.z; cRVec.z = cFVec.x; cFVec.x = f;
	f = cDVec.z; cDVec.z = cFVec.y; cFVec.y = f;
}
*/
void VxMoxie::transposeMovCam(float x, float y, float z)
{
	float f = 1.f / (cRVec.x * cRVec.x + cRVec.y * cRVec.y + cRVec.z * cRVec.z);
	cPos.x += (x * cRVec.x + y * cRVec.y + z * cRVec.z) * f;
	cPos.y += (x * cDVec.x + y * cDVec.y + z * cDVec.z) * f;
	cPos.z += (x * cFVec.x + y * cFVec.y + z * cFVec.z) * f;
}

point3d VxMoxie::getCamPos()
{
	return this->cPos;
}

float VxMoxie::getCamPosX()
{
	return cPos.x;
}

float VxMoxie::getCamPosY()
{
	return cPos.y;
}

float VxMoxie::getCamPosZ()
{
	return cPos.z;
}

point3d VxMoxie::getCamRVec()
{
	return this->cRVec;
}

point3d VxMoxie::getCamDVec()
{
	return this->cDVec;
}

point3d VxMoxie::getCamFVec()
{
	return cFVec;
}

void VxMoxie::setMovementSpeed(float newValue)
{
	movementSpeed = newValue;
}

float VxMoxie::getMovementSpeed()
{
	return movementSpeed;
}

void VxMoxie::setMovementSpeedPiv(float newValue)
{
	movementSpeed = newValue;
}

float VxMoxie::getMovementSpeedPiv()
{
	return movementSpeedPiv;
}

float VxMoxie::getScale()
{
	return this->scale;
}

void VxMoxie::setScale(float newValue)
{
	this->scale = newValue;


}

void VxMoxie::setPivot(point3d pos)
{
	this->piv = pos;
}

point3d VxMoxie::getPivot()
{
	return piv;
}

void VxMoxie::drawPivot(float rad, int fillMode, int col)
{
	voxiePtr->drawSphere(this->piv, rad, fillMode, col);

}

void VxMoxie::drawVox(float x, float y, float z, int col)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	point3d t; t.x = x; t.y = y; t.z = z; this->xformPos(&t);
	voxiePtr->drawVox(t.x, t.y, t.z, col);

}

void VxMoxie::drawVox(point3d pos, int col)
{
	drawVox(pos.x, pos.y, pos.z, col);
}

void VxMoxie::drawSphere(float x, float y, float z, float rad, int type, int col)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	point3d t; t.x = x; t.y = y; t.z = z; this->xformPos(&t);
	voxiePtr->drawSphere(t.x, t.y, t.z, rad * scale, type, col);
}

void VxMoxie::drawSphere(point3d pos, float rad, int type, int col)
{
	this->drawSphere(pos.x, pos.y, pos.z, rad, type, col);
}

void VxMoxie::drawLine(float x0, float y0, float z0, float x1, float y1, float z1, int col)
{
	point3d p0, p1;
	p0.x = x0; p0.y = y0; p0.z = z0; this->xformPos(&p0);
	p1.x = x1; p1.y = y1; p1.z = z1; this->xformPos(&p1);
	voxiePtr->drawLine(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, col);
}

void VxMoxie::drawLine(point3d pos0, point3d pos1, int col)
{
	this->drawLine(pos0.x, pos0.y, pos0.z, pos1.x, pos1.y, pos1.z, col);

}

void VxMoxie::drawCube(point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int fillmode, int col)
{
	point3d np, nr, nd, nf;

	np.x = pos->x; np.y = pos->y; np.z = pos->z;
	this->xformPos(&np);
	nr.x = pos->x + rVec->x; nr.y = pos->y + rVec->y; nr.z = pos->z + rVec->z;
	this->xformPos(&nr);
	nr.x -= np.x; nr.y -= np.y; nr.z -= np.z;
	nd.x = pos->x + dVec->x; nd.y = pos->y + dVec->y; nd.z = pos->z + dVec->z;
	this->xformPos(&nd);
	nd.x -= np.x; nd.y -= np.y; nd.z -= np.z;
	nf.x = pos->x + fVec->x; nf.y = pos->y + fVec->y; nf.z = pos->z + fVec->z;
	this->xformPos(&nf);
	nf.x -= np.x; nf.y -= np.y; nf.z -= np.z;

	voxiePtr->drawCube(&np, &nr, &nd, &nf, fillmode, col);


}

void VxMoxie::drawBox(point3d LTU, point3d RBD, int fillmode, int col)
{
	this->drawBox(LTU.x, LTU.y, LTU.z, RBD.x, RBD.y, RBD.z, fillmode, col);
}

void VxMoxie::drawBox(float x0, float y0, float z0, float x1, float y1, float z1, int fillmode, int col)
{
	point3d p[4];
	p[0].x = x0; p[0].y = y0; p[0].z = z0;

	this->xformPos(&p[0]);
	p[1].x = x1; p[1].y = y0; p[1].z = z0;

	this->xformPos(&p[1]);
	p[1].x -= p[0].x; p[1].y -= p[0].y; p[1].z -= p[0].z;
	p[2].x = x0; p[2].y = y1; p[2].z = z0;

	this->xformPos(&p[2]);
	p[2].x -= p[0].x; p[2].y -= p[0].y; p[2].z -= p[0].z;
	p[3].x = x0; p[3].y = y0; p[3].z = z1;

	this->xformPos(&p[3]);
	p[3].x -= p[0].x; p[3].y -= p[0].y; p[3].z -= p[0].z;

	voxiePtr->drawCube(&p[0], &p[1], &p[2], &p[3], fillmode, col);

}

float VxMoxie::drawHeightMap(char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int colorkey, int reserved, int flags)
{
	point3d np = *pos, nr, nd, nf; this->xformPos(&np); this->xformOri(rVec, dVec, fVec, &nr, &nd, &nf);
	return voxiePtr->drawHeightMap(fileName, &np, &nr, &nd, &nf, colorkey, reserved, flags);

}


void VxMoxie::drawDicom(voxie_dicom_t* vd, const char* gfilnam, point3d* p, point3d* r, point3d* d, point3d* f, int* animn, int* loaddone)
{
	point3d np = *p, nr, nd, nf; this->xformPos(&np); this->xformOri(r, d, f, &nr, &nd, &nf);
	voxiePtr->drawDicom(vd, gfilnam, &np, &nr, &nd, &nf, animn, loaddone);
}

void VxMoxie::drawCone(float x0, float y0, float z0, float r0, float x1, float y1, float z1, float r1, int fillmode, int col)
{

	point3d p0, p1;
	p0.x = x0; p0.y = y0; p0.z = z0; this->xformPos(&p0);
	p1.x = x1; p1.y = y1; p1.z = z1; this->xformPos(&p1);

	voxiePtr->drawCone(p0, r0 * scale, p1, r1 * scale, fillmode, col);

}

void VxMoxie::drawCone(point3d pos0, float r0, point3d pos1, float r1, int fillmode, int col)
{
	this->drawCone(pos0.x, pos0.y, pos0.z, r0, pos1.x, pos1.y, pos1.z, r1, fillmode, col);

}

void VxMoxie::drawPoly(pol_t* pt, int n, int col)
{
	pol_t* npt;
	if (n <= 4096) { npt = (pol_t*)_alloca(n * sizeof(pol_t)); }
	else { npt = (pol_t*)malloc(n * sizeof(pol_t)); }
	memcpy(npt, pt, n * sizeof(pol_t));
	for (int i = n - 1; i >= 0; i--)
	{
		point3d t; t.x = npt[i].x; t.y = npt[i].y; t.z = npt[i].z; this->xformPos(&t);
		npt[i].x = t.x; npt[i].y = t.y; npt[i].z = t.z;
	}
	voxiePtr->drawPoly(npt, n, col);
	if (n > 4096) free(npt);

}

void VxMoxie::drawMesh(char* fileName, poltex_t* verticeList, int verticeNum, int* meshList, int meshNum, int flags, int col)
{
	poltex_t* nvt; // new vertcie List with translation
	if (verticeNum <= 4096) { nvt = (poltex_t*)_alloca(verticeNum * sizeof(poltex_t)); }
	else { nvt = (poltex_t*)malloc(verticeNum * sizeof(poltex_t)); }
	memcpy(nvt, verticeList, verticeNum * sizeof(poltex_t));
	for (int i = verticeNum - 1; i >= 0; i--)
	{
		point3d t; t.x = nvt[i].x; t.y = nvt[i].y; t.z = nvt[i].z; this->xformPos(&t);
		nvt[i].x = t.x; nvt[i].y = t.y; nvt[i].z = t.z;
	}
	voxiePtr->drawMesh(fileName, nvt, verticeNum, meshList, meshNum, flags, col);
	if (verticeNum > 4096) free(nvt);


}

int VxMoxie::drawModel(const char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int col)
{
	point3d np = *pos, nr, nd, nf;
	this->xformPos(&np);
	this->xformOri(rVec, dVec, fVec, &nr, &nd, &nf);
	return voxiePtr->drawModel(fileName, &np, &nr, &nd, &nf, col);
}

int VxMoxie::drawModelExt(const char* fileName, point3d* pos, point3d* rVec, point3d* dVec, point3d* fVec, int col, float forcescale, float fdrawratio, int flags)
{
	point3d np = *pos, nr, nd, nf;
	this->xformPos(&np);
	this->xformOri(rVec, dVec, fVec, &nr, &nd, &nf);
	return voxiePtr->drawModelExt(fileName, &np, &nr, &nd, &nf, col, forcescale, fdrawratio, flags);
}

void VxMoxie::drawText(point3d* pos, point3d* rVec, point3d* dVec, int col, const char* st)
{
	point3d np, nr, nd;
	np.x = pos->x; np.y = pos->y; np.z = pos->z;
	this->xformPos(&np);
	nr.x = pos->x + rVec->x; nr.y = pos->y + rVec->y; nr.z = pos->z + rVec->z;
	this->xformPos(&nr); nr.x -= np.x; nr.y -= np.y; nr.z -= np.z;
	nd.x = pos->x + dVec->x; nd.y = pos->y + dVec->y; nd.z = pos->z + dVec->z;
	this->xformPos(&nd); nd.x -= np.x; nd.y -= np.y; nd.z -= np.z;

	voxiePtr->drawText(&np, &nr, &nd, col, st);


}

void VxMoxie::drawTextExt(point3d* p, point3d* r, point3d* d, int col, const char* fmt, ...)
{
	va_list arglist;
	char st[1024];

	if (!fmt) return;
	va_start(arglist, fmt);
#if defined(_WIN32)
	if (_vsnprintf((char*)&st, sizeof(st) - 1, fmt, arglist)) st[sizeof(st) - 1] = 0;
#else
	if (vsprintf((char*)&st, fmt, arglist)) st[sizeof(st) - 1] = 0; //FUK:unsafe!
#endif
	va_end(arglist);

	this->drawText(p, r, d, col, st);


}

void VxMoxie::report(int posx, int posy, bool showPivot)
{
	if (voxiePtr == nullptr || voxiePtr == 0) return;

	voxiePtr->debugText(posx, posy, voxiePtr->scrollCol(1), -1, (char*)"VxMoxie Debug **");
	posy += 8;
	voxiePtr->debugText(posx, posy, 0xff0000, -1, (char*)"Camera Pos X:%1.3f , Y:%1.3f, Z X:%1.3f", cPos.x, cPos.y, cPos.z);
	posy += 8;
	voxiePtr->debugText(posx, posy, 0xff7F00, -1, (char*)"Camera RVec X:%1.3f , Y:%1.3f, Z X:%1.3f", cRVec.x, cRVec.y, cRVec.z);
	posy += 8;
	voxiePtr->debugText(posx, posy, 0xffff00, -1, (char*)"Camera DVec X:%1.3f , Y:%1.3f, Z X:%1.3f", cDVec.x, cDVec.y, cDVec.z);
	posy += 8;
	voxiePtr->debugText(posx, posy, 0x00ff00, -1, (char*)"Camera FVec X:%1.3f , Y:%1.3f, Z X:%1.3f", cFVec.x, cFVec.y, cFVec.z);
	posy += 8;
	voxiePtr->debugText(posx, posy, 0x00ffff, -1, (char*)"Scale %1.3f | Movement Speed %1.3f", scale, movementSpeed);
	posy += 8;
	voxiePtr->debugText(posx, posy, 0x7F00ff, -1, (char*)"Piv X %1.3f , Y: %1.3f, Z: %1.3f Pivot Scroll: %d", piv.x, piv.y, piv.z, this->isScrollPivotEnabled);

	if (showPivot) drawPivot(0.05, 1, 0xffff00);



}

void VxMoxie::setEnableScrollPivot(bool newValue)
{
	this->isScrollPivotEnabled = newValue;
}

point3d VxMoxie::voxie2MoxiePos(point3d pos)
{
	point3d pp = { pos.x, pos.y, pos.z };
	if (voxiePtr == nullptr || voxiePtr == 0) return pp;
	xformPos(&pp);
	return pp;

}
