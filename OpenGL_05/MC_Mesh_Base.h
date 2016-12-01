#ifndef __MC_MESH_BASE_H__
#define __MC_MESH_BASE_H__

#include "HES_Mesh.h"

#include <map>
#include <list>
#include <cmath>

using std::powf;
using std::fabs;
using std::sqrtf;

struct vec3I
{
	int x;
	int y;
	int z;
};

typedef std::pair<unsigned int, unsigned int>	MC_Vertex_Pair;

struct MC_Triangle
{
	MC_Vertex_Pair * vertex[3];
};

typedef std::list<MC_Triangle *>				MC_Triangle_List;
typedef std::list<MC_Vertex_Pair *>				MC_Vertex_Pair_List;
typedef std::map<MC_Vertex_Pair, int>			MC_Vertex_Pair_Map;

typedef MC_Triangle_List::iterator				MC_Triangle_List_Iter;
typedef MC_Vertex_Pair_List::iterator			MC_Vertex_Pair_List_Iter;

extern const int edgeTable[12][6];
extern const int triangleTable[256][16];


inline void multiply(float & x, float & y, float & z, float p);
inline float f1(float x, float y, float z);
inline float f2(float x, float y, float z);
inline float f3(float x, float y, float z);
inline float f4(float x, float y, float z);


class MC_Mesh_Base
{
private:
	float ***points;

	MC_Vertex_Pair_Map	mVertexPairIdMap;
	MC_Vertex_Pair_List mNewVerticesPairList;
	MC_Triangle_List	mNewTrianglesList;

	HES_Mesh *mesh;

	const float distance;
	const int mNumOfCubes;

public:
	MC_Mesh_Base(int numOfCubes);
	MC_Mesh_Base(int numOfCubes, float(*f)(float, float, float) );
	virtual ~MC_Mesh_Base();

	HES_Mesh * getMesh();

protected:
	void initPoints();
	void calculateNewTriangles();
	inline void checkCube(int x, int y, int z);
	inline void addNewTriangles(const int * triangles_ptr, int x, int y, int z);
	inline MC_Triangle *	addNewTriangle(const int * edges_ptr, int x, int y, int z);
	inline MC_Vertex_Pair * addNewVertex(int * pos);

	void createNewMesh();
	void createNewVertices();
	void createNewFaces();

	inline unsigned int pointToIndex(int x, int y, int z);
	inline unsigned int pointToIndex(vec3I & pos);
	inline void 		indexToPoint(unsigned int index, vec3I & pos);

	inline void			getPos(int x, int y, int z, float & posX, float & posY, float & posZ);
	inline void			getPos(const vec3I & point, vec3 & pos);
	inline void			getPos(const vec3I & lastPos1, const vec3I & lastPos2, vec3 & newPos);

	virtual float getValue(int x, int y, int z) = 0;
};

#endif//__MC_MESH_BASE_H__