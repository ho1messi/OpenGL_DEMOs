#include "..\include\HES_Mesh.h"

#include <fstream>
#include <sstream>
#include <iomanip>

HES_Vertex::HES_Vertex(const vec3 & pos) :
	pos(pos), edge(NULL), normal(), id(0U)
{

}

HES_Vertex::HES_Vertex(const float posX, const float posY, const float posZ) :
	pos( { posX, posY, posZ } ), edge(NULL), normal(), id(0U)
{

}

HES_Edge::HES_Edge() :
	vertex(NULL), pair(NULL), next(NULL), prev(NULL), face(NULL)
{

}

HES_Face::HES_Face() :
	edge(NULL), normal(), valence(0)
{

}

HES_Mesh::HES_Mesh() :
	mHESVerticesList(NULL), mHESEdgesList(NULL), mHESFacesList(NULL), mEdgeMap(),
	mMaxPos({ -INFINITY, -INFINITY, -INFINITY }), mMinPos({ INFINITY, INFINITY, INFINITY }),
	mVertices(), mIndices(), mNormal(), mUpdateVerticesFlag(true),
	HES_DRAW_LINE_WIDTH(1.0f)
{
	mHESVerticesList = new HES_Vertex_List();
	mHESEdgesList = new HES_Edge_List();
	mHESFacesList = new HES_Face_List();
}

HES_Mesh::~HES_Mesh()
{
	HES_Vertex_Iterator vertexI = mHESVerticesList->begin();
	for (; vertexI < mHESVerticesList->end(); vertexI++)
		delete (*vertexI);

	HES_Edge_Iterator edgeI = mHESEdgesList->begin();
	for (; edgeI < mHESEdgesList->end(); edgeI++)
		delete (*edgeI);

	HES_Face_Iterator faceI = mHESFacesList->begin();
	for (; faceI < mHESFacesList->end(); faceI++)
		delete (*faceI);

	delete mHESVerticesList;
	delete mHESEdgesList;
	delete mHESFacesList;
}

void HES_Mesh::readFromObj(const char * fin)
{
	std::ifstream iStream(fin);
	std::stringstream ss;
	string flag;

	string indiceString;
	std::stringstream stringToUint;

	float posX, posY, posZ;
	HES_Vertex_List verticesList;

	string line;
	while (!iStream.eof())
	{
		ss.clear();
		flag.clear();

		std::getline(iStream, line);
		ss.str(line);

		ss >> flag;
		indiceString = ss.str();

		if (flag == "v")
		{
			ss >> posX >> posY >> posZ;
			insertVertex(posX, posY, posZ);
		}
		else if (flag == "f")
		{
			indiceString = ss.str();
			while (ss >> indiceString)
			{
				verticesList.push_back((*mHESVerticesList)[atoi(indiceString.c_str()) - 1]);
			}
			insertFace(verticesList);
			verticesList.clear();
		}
	}

	iStream.close();
	mUpdateVerticesFlag = true;
}

void HES_Mesh::writeToFile(const char * fout)
{
	std::ofstream oStream(fout);

	HES_Vertex_Iterator vertexI = mHESVerticesList->begin();
	for (; vertexI < mHESVerticesList->end(); vertexI++)
	{
		oStream << std::setiosflags(std::ios::fixed) << std::setprecision(6);
		oStream << std::setw(15) << (*vertexI)->pos.x << "\t";
		oStream << std::setw(15) << (*vertexI)->pos.y << "\t";
		oStream << std::setw(15) << (*vertexI)->pos.z << std::endl;
	}
}

vec3 * HES_Mesh::getVerticesPos()
{
	//Comming soon
	return NULL;
}

unsigned int * HES_Mesh::getVertexIndices()
{
	//Comming soon
	return NULL;
}

HES_Vertex * HES_Mesh::insertVertex(const vec3 & pos)
{
	HES_Vertex *newVertex = new HES_Vertex(pos);
	newVertex->id = mHESVerticesList->size();

	mHESVerticesList->push_back(newVertex);
	updateMinMaxPos(newVertex->pos);
	mUpdateVerticesFlag = true;
	return newVertex;
}

HES_Vertex * HES_Mesh::insertVertex(const float posX, const float posY, const float posZ)
{
	HES_Vertex *newVertex = new HES_Vertex(posX, posY, posZ);
	newVertex->id = mHESVerticesList->size();

	mHESVerticesList->push_back(newVertex);
	updateMinMaxPos(newVertex->pos);
	mUpdateVerticesFlag = true;
	return newVertex;
}

HES_Edge * HES_Mesh::insertEdge(HES_Vertex * vertexS, HES_Vertex * vertexE)
{
	if (mEdgeMap[HES_Vertex_Pair(vertexS, vertexE)] != NULL)
		return mEdgeMap[HES_Vertex_Pair(vertexS, vertexE)];

	HES_Edge *newEdge = new HES_Edge();

	newEdge->vertex = vertexS;
	vertexS->edge = newEdge;

	mHESEdgesList->push_back(newEdge);
	mEdgeMap[HES_Vertex_Pair(vertexS, vertexE)] = newEdge;
	mUpdateVerticesFlag = true;
	return newEdge;
}

HES_Face * HES_Mesh::insertFace(HES_Vertex_List & verticesList)
{
	HES_Face *newFace = new HES_Face();
	newFace->valence = verticesList.size();

	HES_Vertex_Iterator vertexIS = verticesList.end() - 1;
	HES_Vertex_Iterator vertexIE = verticesList.begin();

	HES_Edge *edge1 = insertEdge(*vertexIS, *vertexIE);
	HES_Edge *edge2 = insertEdge(*vertexIE, *vertexIS);
	HES_Edge *lastEdge = edge1;
	HES_Edge *firstEdge = edge1;

	edge1->pair = edge2;
	edge2->pair = edge1;
	edge1->face = newFace;

	newFace->edge = edge1;

	for (vertexIS = vertexIE++; vertexIE < verticesList.end(); vertexIS++, vertexIE++)
	{
		edge1 = insertEdge(*vertexIS, *vertexIE);
		edge2 = insertEdge(*vertexIE, *vertexIS);

		edge1->pair = edge2;
		edge2->pair = edge1;
		edge1->face = newFace;

		lastEdge->next = edge1;
		edge1->prev = lastEdge;

		lastEdge = edge1;
	}

	lastEdge->next = firstEdge;
	firstEdge->prev = lastEdge;

	vec3 vector1 = newFace->edge->vertex->pos - newFace->edge->next->vertex->pos;
	vec3 vector2 = newFace->edge->vertex->pos - newFace->edge->prev->vertex->pos;
	vec3 normal = cross(vector1, vector2);
	newFace->normal = normal;

	mHESFacesList->push_back(newFace);
	mUpdateVerticesFlag = true;
	return newFace;
}

HES_Face * HES_Mesh::insertFace(std::vector<int> & verticesIdList)
{
	HES_Vertex_List newVerticesList;

	std::vector<int>::iterator i = verticesIdList.begin();
	for (; i < verticesIdList.end(); i++)
	{
		newVerticesList.push_back((*mHESVerticesList)[*i]);
	}

	return insertFace(newVerticesList);
}

bool HES_Mesh::isBorder(HES_Edge * edge)
{
	if (edge->face == NULL || edge->pair->face == NULL)
		return true;
	
	return false;
}

bool HES_Mesh::isBorder(HES_Vertex * vertex)
{
	bool isBorder = false;
	HES_Edge * edge = vertex->edge;
	do {
		if (edge == NULL)
		{
			isBorder = true;
			break;
		}
		edge = edge->pair->next;
	} while (edge != vertex->edge);

	return isBorder;
}

void HES_Mesh::setupMesh()
{
	checkFlags();

	if (mVertices.size() == 0)
		return;

	if (!glIsVertexArray(VAO))
		glGenVertexArrays(1, &VAO);
	if (!glIsBuffer(VBO))
		glGenBuffers(1, &VBO);
	if (!glIsBuffer(EBO))
		glGenBuffers(1, &EBO);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(HES_PRIMITIVE_RESTART_INDEX);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * mVertices.size(), &mVertices.front(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec3) * mVertices.size(), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * mVertices.size(), &mVertices.front());
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * mVertices.size(), sizeof(vec3) * mNormal.size(), &mNormal.front());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mIndices.size(), &mIndices.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)( sizeof(vec3) * mVertices.size() ));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void HES_Mesh::drawMeshLine()
{
	glLineWidth(HES_DRAW_LINE_WIDTH);
	glBindVertexArray(VAO);

	glDrawElements(GL_LINE_LOOP, mIndices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void HES_Mesh::drawMeshFace()
{
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLE_FAN, mIndices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void HES_Mesh::updateMinMaxPos(const vec3 & pos)
{
	if (pos.x > mMaxPos.x)
		mMaxPos.x = pos.x;
	if (pos.y > mMaxPos.y)
		mMaxPos.y = pos.y;
	if (pos.z > mMaxPos.z)
		mMaxPos.z = pos.z;

	if (pos.x < mMinPos.x)
		mMinPos.x = pos.x;
	if (pos.y < mMinPos.y)
		mMinPos.y = pos.y;
	if (pos.z < mMinPos.z)
		mMinPos.z = pos.z;
}

void HES_Mesh::updateVertices()
{
	mVertices.clear();
	mIndices.clear();

	updateVerticesNormal();
	updateVerticesPos();
	updateVerticesIndices();

	mUpdateVerticesFlag = false;
}

void HES_Mesh::updateVerticesNormal()
{
	vec3 normal;
	HES_Edge *edge;

	HES_Vertex_Iterator vertexI = mHESVerticesList->begin();
	for (; vertexI < mHESVerticesList->end(); vertexI++)
	{
		normal = vec3(0.0f, 0.0f, 0.0f);
		edge = (*vertexI)->edge;

		if (isBorder(*vertexI))
		{
			while (edge->pair->face != NULL)
				edge = edge->pair->next;

			while (edge->face != NULL)
			{
				normal += edge->face->normal;
				edge = edge->prev->pair;
			}
		}
		else
		{
			do {
				normal += edge->face->normal;
				edge = edge->prev->pair;
			} while (edge != (*vertexI)->edge);
		}

		(*vertexI)->normal = normalize(normal);
		mNormal.push_back((*vertexI)->normal);
	}
}

void HES_Mesh::updateVerticesPos()
{
	vec3 centralPos = (mMaxPos + mMinPos) / 2.0f;

	HES_Vertex_Iterator vertexI = mHESVerticesList->begin();
	for (; vertexI < mHESVerticesList->end(); vertexI++)
		mVertices.push_back((*vertexI)->pos - centralPos);
}

void HES_Mesh::updateVerticesIndices()
{
	HES_Edge *edge;

	HES_Face_Iterator faceI = mHESFacesList->begin();
	for (; faceI < mHESFacesList->end(); faceI++)
	{
		mIndices.push_back((*faceI)->edge->vertex->id);
		edge = (*faceI)->edge->next;

		while (edge != (*faceI)->edge)
		{
			mIndices.push_back(edge->vertex->id);
			edge = edge->next;
		}

		mIndices.push_back(HES_PRIMITIVE_RESTART_INDEX);
	}
}

void HES_Mesh::checkFlags()
{
	if (mUpdateVerticesFlag == true)
		updateVertices();
}