#include "MC_Mesh_Base.h"

MC_Mesh_Base::MC_Mesh_Base(int numOfCubes) :
	distance(2.0f / numOfCubes), mNumOfCubes(numOfCubes)
{
	mesh = new HES_Mesh();

	initPoints();
	calculateNewTriangles();
	createNewMesh();
}

MC_Mesh_Base::MC_Mesh_Base(int numOfCubes, float(*f)(float, float, float)) :
	distance(2.0f / numOfCubes), mNumOfCubes(numOfCubes)
{
	mesh = new HES_Mesh();

	initPoints();
	calculateNewTriangles();
	createNewMesh();
}

MC_Mesh_Base::~MC_Mesh_Base()
{
	if (mesh)
		delete mesh;
}

HES_Mesh * MC_Mesh_Base::getMesh()
{
	return this->mesh;
}

void MC_Mesh_Base::initPoints()
{
	for (int i = 0; i <= mNumOfCubes; i++)
		for (int j = 0; j <= mNumOfCubes; j++)
			for (int k = 0; k <= mNumOfCubes; k++)
				points[i][j][k] = getValue(i, j, k);
}

void MC_Mesh_Base::calculateNewTriangles()
{
	for (int i = 0; i < mNumOfCubes; i++)
		for (int j = 0; j < mNumOfCubes; j++)
			for (int k = 0; k < mNumOfCubes; k++)
				checkCube(i, j, k);
}

void MC_Mesh_Base::checkCube(int x, int y, int z)
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

void MC_Mesh_Base::addNewTriangles(const int * triangles, int x, int y, int z)
{
	while (*triangles != -1)
	{
		addNewTriangle(triangles, x, y, z);
		triangles += 3;
	}
}

MC_Triangle * MC_Mesh_Base::addNewTriangle(const int * edges_ptr, int x, int y, int z)
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

MC_Vertex_Pair * MC_Mesh_Base::addNewVertex(int * pos)
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

void MC_Mesh_Base::createNewMesh()
{
	createNewVertices();
	createNewFaces();
}

void MC_Mesh_Base::createNewVertices()
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

void MC_Mesh_Base::createNewFaces()
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

unsigned int MC_Mesh_Base::pointToIndex(int x, int y, int z)
{
	int numOfPoints = mNumOfCubes + 1;
	return static_cast<unsigned int>(x * numOfPoints * numOfPoints + y * numOfPoints + z);
}

unsigned int MC_Mesh_Base::pointToIndex(vec3I & pos)
{
	int numOfPoints = mNumOfCubes + 1;
	return static_cast<unsigned int>(pos.x * numOfPoints * numOfPoints + pos.y * numOfPoints + pos.z);
}

void MC_Mesh_Base::indexToPoint(unsigned int index, vec3I & pos)
{
	int numOfPoints = mNumOfCubes + 1;
	pos.x = index / numOfPoints / numOfPoints;
	pos.y = (index / numOfPoints) % numOfPoints;
	pos.z = index % numOfPoints;
}

void MC_Mesh_Base::getPos(int x, int y, int z, float & posX, float & posY, float & posZ)
{
	posX = -1.0f + x * distance;
	posY = 1.0f - y * distance;
	posZ = 1.0f - z * distance;
}

void MC_Mesh_Base::getPos(const vec3I & point, vec3 & pos)
{
	pos.x = -1.0f + point.x * distance;
	pos.y = 1.0f - point.y * distance;
	pos.z = 1.0f - point.z * distance;
}

void MC_Mesh_Base::getPos(const vec3I & lastPos1, const vec3I & lastPos2, vec3 & newPos)
{
	vec3 tempPos1, tempPos2;
	getPos(lastPos1, tempPos1);
	getPos(lastPos2, tempPos2);

	float p1, p2;
	p1 = fabs(points[lastPos1.x][lastPos1.y][lastPos1.z]);
	p2 = fabs(points[lastPos2.x][lastPos2.y][lastPos2.z]);

	newPos = (tempPos1 * p2 + tempPos2 * p1) / (p1 + p2);
}
/*
float MC_Mesh_Base<N>::getValue(int x, int y, int z)
{
float posX, posY, posZ;
getPos(x, y, z, posX, posY, posZ);

return f2(posX, posY, posZ);
}
*/
void multiply(float & x, float & y, float & z, float p)
{
	x *= p;
	y *= p;
	z *= p;
}

float f1(float x, float y, float z)
{
	return x * x + y * y + z * z - 1.0f;
}

float f2(float x, float y, float z)
{
	multiply(x, y, z, 10.0f);
	return x * x + y * y - z * z - 1.0f;
}

float f3(float x, float y, float z)
{
	multiply(x, y, z, 1.3f);
	float temp = powf(x, 2.0f) + 2.25f * powf(z, 2.0f) + powf(y, 2.0f) - 1;
	return powf(temp, 3.0f) - powf(x, 2) * powf(y, 3.0f)
		- 0.1125f * powf(z, 2) * powf(y, 3.0f);
}

float f4(float x, float y, float z)
{
	multiply(x, y, z, 2.0f);
	return powf(x * x + y * y - 3.0f, 3.0f)
		+ powf(z * z * z - 2.0f, 2.0f);
}