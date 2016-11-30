#ifndef __PC_MESH_H__
#define __PC_MESH_H__

#include "RBF_Func.h"
#include "MC_Mesh.h"

//¼Ì³Ð×Ô MC_Mesh Àà
template <unsigned int N>
class PC_Mesh : public MC_Mesh
{
private:
	RBF_Func *func;

public:
	PC_Mesh();
	PC_Mesh(const string & path);
	virtual ~PC_Mesh();

private:
	float getValue(float x, float y, float z);
};

template <unsigned int N>
PC_Mesh<N>::PC_Mesh()
{

}

template <unsigned int N>
PC_Mesh<N>::PC_Mesh(const string & path) :
	func(path)
{

}

template <unsigned int N>
PC_Mesh<N>::~PC_Mesh()
{

}

template <unsigned int N>
float PC_Mesh<N>::getValue(float x, float y, float z)
{ 

}

#endif // !__PC_MESH_H__

