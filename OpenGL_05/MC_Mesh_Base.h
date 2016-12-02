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

template <unsigned int N>
class MC_Mesh_Base
{
private:
	float points[N + 1][N + 1][N + 1];

	MC_Vertex_Pair_Map	mVertexPairIdMap;
	MC_Vertex_Pair_List mNewVerticesPairList;
	MC_Triangle_List	mNewTrianglesList;

	HES_Mesh *mesh;

	const float distance;

public:
	MC_Mesh_Base();
	virtual ~MC_Mesh_Base();

	HES_Mesh * getMesh();

protected:
	void loadMesh();
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

template <unsigned int N>
MC_Mesh_Base<N>::MC_Mesh_Base() :
	distance(2.0f / N), mesh(NULL)
{

}

template <unsigned int N>
MC_Mesh_Base<N>::~MC_Mesh_Base()
{
	if (mesh)
		delete mesh;
}

template <unsigned int N>
HES_Mesh * MC_Mesh_Base<N>::getMesh()
{
	loadMesh();
	return this->mesh;
}

template <unsigned int N>
void MC_Mesh_Base<N>::loadMesh()
{
	mesh = new HES_Mesh();

	initPoints();
	calculateNewTriangles();
	createNewMesh();
}

template <unsigned int N>
void MC_Mesh_Base<N>::initPoints()
{
	for (int i = 0; i <= N; i++)
		for (int j = 0; j <= N; j++)
			for (int k = 0; k <= N; k++)
				points[i][j][k] = getValue(i, j, k);
}

template <unsigned int N>
void MC_Mesh_Base<N>::calculateNewTriangles()
{
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			for (int k = 0; k < N; k++)
				checkCube(i, j, k);
}

template <unsigned int N>
void MC_Mesh_Base<N>::checkCube(int x, int y, int z)
{
	unsigned char cube = 0x00;
	const int *triangles = NULL;

	if (points[x][y][z] < 0.0f)
		cube |= 0x01;
	if (points[x + 1][y][z] < 0.0f)
		cube |= 0x02;
	if (points[x + 1][y][z + 1] < 0.0f)
		cube |= 0x04;
	if (points[x][y][z + 1] < 0.0f)
		cube |= 0x08;
	if (points[x][y + 1][z] < 0.0f)
		cube |= 0x10;
	if (points[x + 1][y + 1][z] < 0.0f)
		cube |= 0x20;
	if (points[x + 1][y + 1][z + 1] < 0.0f)
		cube |= 0x40;
	if (points[x][y + 1][z + 1] < 0.0f)
		cube |= 0x80;

	triangles = triangleTable[cube];
	addNewTriangles(triangles, x, y, z);
}

template <unsigned int N>
void MC_Mesh_Base<N>::addNewTriangles(const int * triangles, int x, int y, int z)
{
	while (*triangles != -1)
	{
		addNewTriangle(triangles, x, y, z);
		triangles += 3;
	}
}

template <unsigned int N>
MC_Triangle * MC_Mesh_Base<N>::addNewTriangle(const int * edges_ptr, int x, int y, int z)
{
	int pos[6];
	const int *newVertexPos;
	MC_Triangle *newTriangle = new MC_Triangle;

	for (int i = 0; i < 3; i++)
	{
		newVertexPos = edgeTable[*edges_ptr];
		pos[0] = x + newVertexPos[0];
		pos[1] = y + newVertexPos[1];
		pos[2] = z + newVertexPos[2];
		pos[3] = x + newVertexPos[3];
		pos[4] = y + newVertexPos[4];
		pos[5] = z + newVertexPos[5];

		newTriangle->vertex[i] = addNewVertex(pos);

		edges_ptr++;
	}

	mNewTrianglesList.push_back(newTriangle);
	return newTriangle;
}

template <unsigned int N>
MC_Vertex_Pair * MC_Mesh_Base<N>::addNewVertex(int * pos)
{
	unsigned int vertexIndex1, vertexIndex2;
	MC_Vertex_Pair *newVertex;

	vertexIndex1 = pointToIndex(pos[0], pos[1], pos[2]);
	vertexIndex2 = pointToIndex(pos[3], pos[4], pos[5]);

	if (vertexIndex1 < vertexIndex2)
		newVertex = new MC_Vertex_Pair(vertexIndex1, vertexIndex2);
	else
		newVertex = new MC_Vertex_Pair(vertexIndex2, vertexIndex1);

	mNewVerticesPairList.push_back(newVertex);

	return newVertex;
}

template <unsigned int N>
void MC_Mesh_Base<N>::createNewMesh()
{
	createNewVertices();
	createNewFaces();
}

template <unsigned int N>
void MC_Mesh_Base<N>::createNewVertices()
{
	vec3 vertexPos;
	vec3I tempPos1, tempPos2;
	MC_Vertex_Pair_List_Iter newVertexPairI = mNewVerticesPairList.begin();
	for (int numOfVertices = 0; newVertexPairI != mNewVerticesPairList.end(); newVertexPairI++)
	{
		MC_Vertex_Pair newVertexPair = **newVertexPairI;

		if (mVertexPairIdMap[**newVertexPairI] == NULL)
		{
			mVertexPairIdMap[**newVertexPairI] = ++numOfVertices;

			indexToPoint((*newVertexPairI)->first, tempPos1);
			indexToPoint((*newVertexPairI)->second, tempPos2);
			getPos(tempPos1, tempPos2, vertexPos);

			mesh->insertVertex(vertexPos);
		}
	}

	mNewVerticesPairList.clear();
}

template <unsigned int N>
void MC_Mesh_Base<N>::createNewFaces()
{
	std::vector<int> verticesIdList;
	MC_Vertex_Pair *vertexPair;

	MC_Triangle_List_Iter newTriangleListI = mNewTrianglesList.begin();
	for (; newTriangleListI != mNewTrianglesList.end(); newTriangleListI++)
	{
		verticesIdList.clear();

		vertexPair = (*newTriangleListI)->vertex[0];
		verticesIdList.push_back(mVertexPairIdMap[*vertexPair] - 1);
		vertexPair = (*newTriangleListI)->vertex[1];
		verticesIdList.push_back(mVertexPairIdMap[*vertexPair] - 1);
		vertexPair = (*newTriangleListI)->vertex[2];
		verticesIdList.push_back(mVertexPairIdMap[*vertexPair] - 1);

		mesh->insertFace(verticesIdList);
	}

	mVertexPairIdMap.clear();
	mNewTrianglesList.clear();
}

template <unsigned int N>
unsigned int MC_Mesh_Base<N>::pointToIndex(int x, int y, int z)
{
	int numOfPoints = N + 1;
	return static_cast<unsigned int>(x * numOfPoints * numOfPoints + y * numOfPoints + z);
}

template <unsigned int N>
unsigned int MC_Mesh_Base<N>::pointToIndex(vec3I & pos)
{
	int numOfPoints = N + 1;
	return static_cast<unsigned int>(pos.x * numOfPoints * numOfPoints + pos.y * numOfPoints + pos.z);
}

template <unsigned int N>
void MC_Mesh_Base<N>::indexToPoint(unsigned int index, vec3I & pos)
{
	int numOfPoints = N + 1;
	pos.x = index / numOfPoints / numOfPoints;
	pos.y = (index / numOfPoints) % numOfPoints;
	pos.z = index % numOfPoints;
}

template <unsigned int N>
void MC_Mesh_Base<N>::getPos(int x, int y, int z, float & posX, float & posY, float & posZ)
{
	posX = -1.0f + x * distance;
	posY = 1.0f - y * distance;
	posZ = 1.0f - z * distance;
}

template <unsigned int N>
void MC_Mesh_Base<N>::getPos(const vec3I & point, vec3 & pos)
{
	pos.x = -1.0f + point.x * distance;
	pos.y = 1.0f - point.y * distance;
	pos.z = 1.0f - point.z * distance;
}

template <unsigned int N>
void MC_Mesh_Base<N>::getPos(const vec3I & lastPos1, const vec3I & lastPos2, vec3 & newPos)
{
	vec3 tempPos1, tempPos2;
	getPos(lastPos1, tempPos1);
	getPos(lastPos2, tempPos2);

	float p1, p2;
	p1 = fabs(points[lastPos1.x][lastPos1.y][lastPos1.z]);
	p2 = fabs(points[lastPos2.x][lastPos2.y][lastPos2.z]);

	newPos = (tempPos1 * p2 + tempPos2 * p1) / (p1 + p2);
}



#endif//__MC_MESH_BASE_H__