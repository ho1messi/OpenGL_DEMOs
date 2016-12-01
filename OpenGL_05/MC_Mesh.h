#ifndef __MC_MESH_H__
#define __MC_MESH_H__

#include "MC_Mesh_Base.h"

class MC_Mesh : public MC_Mesh_Base
{
protected:
	float(*mF)(float, float, float);

public:
	MC_Mesh(int numOfCubes);
	MC_Mesh(int numOfCubes, float(*f)(float, float, float) );
	virtual ~MC_Mesh();

protected:
	float getValue(int x, int y, int z);
};

MC_Mesh::MC_Mesh(int numOfCubes) : MC_Mesh_Base(numOfCubes)
{

}

MC_Mesh::MC_Mesh(int numOfCubes, float(*f)(float, float, float)) : MC_Mesh_Base(numOfCubes)
{
	mF = f;
}

MC_Mesh::~MC_Mesh()
{

}

float MC_Mesh::getValue(int x, int y, int z)
{
	float posX, posY, posZ;
	getPos(x, y, z, posX, posY, posZ);

	if (mF != NULL)
		return (*mF)(posX, posY, posZ);
	return f2(posX, posY, posZ);
}

#endif//__MC_MESH_H__
