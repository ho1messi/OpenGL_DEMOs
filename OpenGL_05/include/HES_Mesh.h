#ifndef __MY_MESH_H__
#define __MY_MESH_H__

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <string>
#include <vector>
#include <map>

#include "..\third_party\include\gl\glew.h"
#include "..\third_party\include\glm\glm.hpp"

using std::string;
using glm::vec3;
using glm::cross;
using glm::normalize;

class HES_Vertex;
class HES_Edge;
class HES_Face;

typedef std::vector<HES_Vertex *>	HES_Vertex_List;
typedef std::vector<HES_Edge *>		HES_Edge_List;
typedef std::vector<HES_Face *>		HES_Face_List;

typedef HES_Vertex_List::iterator	HES_Vertex_Iterator;
typedef HES_Edge_List::iterator		HES_Edge_Iterator;
typedef HES_Face_List::iterator		HES_Face_Iterator;

typedef std::pair<HES_Vertex *, HES_Vertex *> HES_Vertex_Pair;

class HES_Vertex
{
public:
	HES_Edge *edge;
	vec3 pos;
	vec3 normal;

	unsigned int id;

public:
	HES_Vertex(const vec3 & pos);
	HES_Vertex(const float posX, const float posY, const float posZ);
};

class HES_Edge
{
public:
	HES_Vertex *vertex;
	HES_Edge *pair;
	HES_Edge *next;
	HES_Edge *prev;
	HES_Face *face;

public:
	HES_Edge();
};

class HES_Face
{
public:
	HES_Edge *edge;
	vec3 normal;
	int valence;

public:
	HES_Face();
};

class HES_Mesh
{
public:
	HES_Vertex_List	*	mHESVerticesList;
	HES_Edge_List	*	mHESEdgesList;
	HES_Face_List	*	mHESFacesList;

	std::map<HES_Vertex_Pair, HES_Edge *> mEdgeMap;

protected:
	std::vector<vec3>			mVertices;
	std::vector<unsigned int>   mIndices;
	std::vector<vec3>			mNormal;

	bool mUpdateVerticesFlag;

	vec3 mMaxPos, mMinPos;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	const float HES_DRAW_LINE_WIDTH;
	static const unsigned HES_PRIMITIVE_RESTART_INDEX = 0xFFFF;

public:
	HES_Mesh();
	virtual ~HES_Mesh();
	void readFromObj(const char * fin);
	void writeToFile(const char * fout);

	vec3		 *	getVerticesPos();
	unsigned int *  getVertexIndices();

	HES_Vertex * insertVertex(const vec3 & pos);
	HES_Vertex * insertVertex(const float posX, const float posY, const float posZ);
	HES_Edge   * insertEdge(HES_Vertex * vertexS, HES_Vertex * vertexE);
	HES_Face   * insertFace(HES_Vertex_List & verticesList);
	HES_Face   * insertFace(std::vector<int> & verticesIdList);

	bool isBorder(HES_Edge * edge);
	bool isBorder(HES_Vertex * vertex);

	void setupMesh();
	void drawMeshLine();
	void drawMeshFace();

protected:
	inline void updateMinMaxPos(const vec3 & pos);

	void updateVertices();
	void updateVerticesNormal();
	void updateVerticesPos();
	void updateVerticesIndices();

	void checkFlags();
};

//VertexPos & operator * (const VertexPos & vertexPosition, )

#endif//__MY_MESH_H__
