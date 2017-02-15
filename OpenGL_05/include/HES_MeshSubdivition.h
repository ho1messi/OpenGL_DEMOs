#ifndef __HES_MESH_SUBDIVITION_H__
#define __HES_MESH_SUBDIVITION_H__

#include "HES_Mesh.h"

typedef std::vector<HES_Mesh *> HES_Mesh_List;
typedef HES_Mesh_List::iterator HES_Mesh_Iterator;

class HES_MeshSubdivition
{
private:
	HES_Mesh_List mHESMeshList;
	HES_Mesh_Iterator mCurrentMeshI;

public:
	HES_MeshSubdivition();
	HES_MeshSubdivition(HES_Mesh * HESmesh);
	~HES_MeshSubdivition();
	void setMesh(HES_Mesh * HESmesh);

	void DooSabinSubdivition();
	inline void DooSabinVertices(HES_Face *face, std::vector<vec3> & verticesList);
	HES_Mesh * getCurrentMesh();
	HES_Mesh * lastMesh();
	HES_Mesh * nextMesh();
};

#endif//__HES_MESH_SUBDIVITION_H__
