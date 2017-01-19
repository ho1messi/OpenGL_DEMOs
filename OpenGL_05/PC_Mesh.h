#ifndef __PC_MESH_H__
#define __PC_MESH_H__

#include "RBF_Func.h"
#include "MC_Mesh_Base.h"

template <unsigned int N>
class PC_Mesh : public MC_Mesh_Base<N>
{
private:
	RBF_Func func;

public:
	PC_Mesh();
	PC_Mesh(const string & path);
	virtual ~PC_Mesh();
	void drawNormals();

protected:
	virtual float getValue(int x, int y, int z);
};

template <unsigned int N>
PC_Mesh<N>::PC_Mesh() : MC_Mesh_Base<N>()
{

}

template <unsigned int N>
PC_Mesh<N>::PC_Mesh(const string & path) : MC_Mesh_Base<N>(),
	func(path)
{

}

template <unsigned int N>
PC_Mesh<N>::~PC_Mesh()
{

}

template <unsigned int N>
void PC_Mesh<N>::drawNormals()
{
	func.drawNormals();
}

template <unsigned int N>
float PC_Mesh<N>::getValue(int x, int y, int z)
{ 
	float posX, posY, posZ;
	getPos(x, y, z, posX, posY, posZ);

	return func.func(posX, posY, posZ);
}

#endif // !__PC_MESH_H__

