#include "HES_Mesh.h"
#include "HES_MeshSubdivition.h"

HES_MeshSubdivition::HES_MeshSubdivition() :
	mHESMeshList()
{
	
}

HES_MeshSubdivition::HES_MeshSubdivition(HES_Mesh * HESmesh) :
	mHESMeshList({ HESmesh })
{
	mCurrentMeshI = mHESMeshList.begin();
}

HES_MeshSubdivition::~HES_MeshSubdivition()
{
	HES_Mesh_Iterator meshI = mHESMeshList.begin();
	for (; meshI < mHESMeshList.end(); meshI++)
	{
		if (*meshI)
			delete (*meshI);
	}
}

void HES_MeshSubdivition::setMesh(HES_Mesh * HESmesh)
{
	mHESMeshList.clear();
	mHESMeshList.push_back(HESmesh);
	mCurrentMeshI = mHESMeshList.begin();
}

void HES_MeshSubdivition::DooSabinSubdivition()
{
	HES_Mesh *newMesh = new HES_Mesh();
	HES_Vertex_List fFaceVertices;
	std::vector<vec3> newVerticesList;
	HES_Vertex *newVertex;
	HES_Edge *edge;

	std::map<HES_Edge *, HES_Vertex *> edgeSVertexMap;
	std::map<HES_Edge *, HES_Vertex *> edgeEVertexMap;

	mHESMeshList.erase(mCurrentMeshI + 1, mHESMeshList.end());

	HES_Face_Iterator faceI = (*mCurrentMeshI)->mHESFacesList->begin();
	for (; faceI < (*mCurrentMeshI)->mHESFacesList->end(); faceI++)
	{
		DooSabinVertices(*faceI, newVerticesList);

		fFaceVertices.clear();
		for (unsigned int i = 0; i < newVerticesList.size(); i++)
		{
			newVertex = newMesh->insertVertex(newVerticesList[i]);
			fFaceVertices.push_back(newVertex);
		}

		edge = (*faceI)->edge;
		for (int i = 0; i < (*faceI)->valence; i++)
		{
			edgeSVertexMap[edge] = fFaceVertices[i];
			edgeEVertexMap[edge] = fFaceVertices[(i + 1) %(*faceI)->valence];
			edge = edge->next;
		}

		//	Insert F-Face
		newMesh->insertFace(fFaceVertices);
	}

	//	Insert E-Face
	HES_Vertex_List eFaceVertices;
	HES_Edge_Iterator edgeI = (*mCurrentMeshI)->mHESEdgesList->begin();
	for (; edgeI < (*mCurrentMeshI)->mHESEdgesList->end(); edgeI += 2)
	{
		if ((*mCurrentMeshI)->isBorder(*edgeI))
			continue;

		eFaceVertices.clear();
		eFaceVertices.push_back(edgeEVertexMap[*edgeI]);
		eFaceVertices.push_back(edgeSVertexMap[*edgeI]);
		eFaceVertices.push_back(edgeEVertexMap[*(edgeI + 1)]);
		eFaceVertices.push_back(edgeSVertexMap[*(edgeI + 1)]);

		newMesh->insertFace(eFaceVertices);
	}
	
	//	Insert V-Face
	HES_Vertex_List vFaceVertices;
	HES_Vertex_Iterator vertexI = (*mCurrentMeshI)->mHESVerticesList->begin();
	for (; vertexI < (*mCurrentMeshI)->mHESVerticesList->end(); vertexI++)
	{
		vFaceVertices.clear();

		edge = (*vertexI)->edge;

		if ((*mCurrentMeshI)->isBorder(*vertexI))
		{
			while (edge->pair->face != NULL)
				edge = edge->pair->next;

			while (edge->face != NULL)
			{
				vFaceVertices.push_back(edgeSVertexMap[edge]);
				edge = edge->prev->pair;
			}
		}
		else
		{
			do {
				vFaceVertices.push_back(edgeSVertexMap[edge]);
				edge = edge->prev->pair;
			} while (edge != (*vertexI)->edge);
		}

		if (vFaceVertices.size() > 2)
			newMesh->insertFace(vFaceVertices);
	}
	
	mHESMeshList.push_back(newMesh);
	mCurrentMeshI = mHESMeshList.end() - 1;
}

inline void HES_MeshSubdivition::DooSabinVertices(HES_Face * face, std::vector<vec3> & newVerticesList)
{
	HES_Vertex_List verticesList;
	newVerticesList.clear();

	int *borderEdgeVertex = new int[face->valence];
	int *borderEdgeNum = new int[face->valence];
	for (int i = 0; i < face->valence; i++)
	{
		borderEdgeVertex[i] = -1;
		borderEdgeNum[i] = 0;
	}

	HES_Edge *edge = face->edge;
	for (int i = 0; i < face->valence; i++)
	{
		if ((*mCurrentMeshI)->isBorder(edge))
		{
			borderEdgeVertex[i] = (i + 1) % face->valence;
			borderEdgeVertex[(i + 1) % face->valence] = i;
			borderEdgeNum[i]++;
			borderEdgeNum[(i + 1) % face->valence]++;
		}

		verticesList.push_back(edge->vertex);
		edge = edge->next;
	}
	
	vec3 vertexPosition;
	for (int i = 0; i < face->valence; i++)
	{
		if (borderEdgeNum[i] > 1)
		{
			newVerticesList.push_back(verticesList[i]->pos);
		}
		else if (borderEdgeNum[i] == 1)
		{
			vertexPosition = verticesList[i]->pos * 0.75f +
							 verticesList[borderEdgeVertex[i]]->pos * 0.25f;
			newVerticesList.push_back(vertexPosition);
		}
		else
		{
			vertexPosition = verticesList[i]->pos * static_cast<float>(face->valence + 5);
			for (int j = 1; j < face->valence; j++)
			{
				vertexPosition += verticesList[(i + j) % face->valence]->pos *
					(3.0f + 2.0f * cosf((6.283f * j) / face->valence));
			}

			newVerticesList.push_back(vertexPosition / (4.0f * face->valence));
		}
	}

	delete borderEdgeVertex;
	delete borderEdgeNum;
}

HES_Mesh * HES_MeshSubdivition::getCurrentMesh()
{
	return *mCurrentMeshI;
}

HES_Mesh * HES_MeshSubdivition::lastMesh()
{
	if (mCurrentMeshI > mHESMeshList.begin())
		mCurrentMeshI--;

	return *mCurrentMeshI;
}

HES_Mesh * HES_MeshSubdivition::nextMesh()
{
	if (mCurrentMeshI < mHESMeshList.end() - 1)
		mCurrentMeshI++;

	return *mCurrentMeshI;
}