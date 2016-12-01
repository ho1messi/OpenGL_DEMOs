#ifndef __MC_MESH_H__
#define __MC_MESH_H__

#include "MC_Mesh_Base.h"

class NoFuncDifinationError
{

};

template <unsigned int N>
class MC_Mesh : public MC_Mesh_Base<N>
{
protected:
	float(*mF)(float, float, float);

public:
	MC_Mesh( float(*f)(float, float, float) );
	virtual ~MC_Mesh();

protected:
	float getValue(int x, int y, int z);
};

template <unsigned int N>
MC_Mesh<N>::MC_Mesh(float(*f)(float, float, float)) : MC_Mesh_Base(),
	mF(f)
{

}

template <unsigned int N>
MC_Mesh<N>::~MC_Mesh()
{

}

template <unsigned int N>
float MC_Mesh<N>::getValue(int x, int y, int z)
{
	float posX, posY, posZ;
	getPos(x, y, z, posX, posY, posZ);

	if (mF == NULL)
		throw NoFuncDifinationError();
	return (*mF)(posX, posY, posZ);
}

#endif//__MC_MESH_H__
