#ifndef __MESH_3D_H__
#define __MESH_3D_H__

#include <vector>
#include <map>

#include <glm\glm.hpp>

namespace MeshLib
{
	using glm::vec3;

	class HE_vert;
	class HE_face;
	class HE_edge;

	class HE_vert
	{
	public:
		vec3 pos;
		HE_edge *edge;
		vec3 normal;
		int id;
		int degree;
		bool tag;
		
	public:
		HE_vert(vec3 &v);
	};

	class HE_edge
	{
	public:
		HE_vert *vert;
		HE_edge *pair;
		HE_face *face;
		HE_edge *next;
		HE_edge *prev;
		int id;
		bool tag;

	public:
		HE_edge();
	};

	class HE_face
	{
	public:
		HE_edge *edge;
		int valence;
		vec3 normal;
		int id;
		bool tag;

	public:
		HE_face();
		vec3 GetCentroid();
	};

	class Mesh3D
	{
	public:
		typedef std::vector<HE_vert *> VERTEX_LIST;
		typedef std::vector<HE_face *> FACE_LIST;
		typedef std::vector<HE_edge *> EDGE_LIST;

		typedef VERTEX_LIST * PTR_VERTEX_LIST;
		typedef FACE_LIST * PTR_FACE_LIST;
		typedef EDGE_LIST * PTR_EDGE_LIST;

		typedef typename VERTEX_LIST::iterator VERTEX_ITER;
		typedef typename FACE_LIST::iterator FACE_ITER;
		typedef typename EDGE_LIST::iterator EDGE_ITER;

		typedef typename VERTEX_LIST::reverse_iterator VERTEX_RITER;
		typedef typename FACE_LIST::reverse_iterator FACE_RITER;
		typedef typename EDGE_LIST::reverse_iterator EDGE_ITER;
		typedef std::pair<HE_vert *, HE_vert *>PAIR_VERTEX;

	protected:
		PTR_VERTEX_LIST verticesList;
		PTR_EDGE_LIST edgesList;
		PTR_FACE_LIST facesList;

		bool m_closed;

		std::map<PAIR_VERTEX, HE_edge *> m_edgemap;

		int m_numComponents;
		int m_numBoundaries;
		int m_genus;

	public:
		float xmax, xmin, ymax, ymin, zmax, zmin;

		std::vector<std::vector<HE_vert *> > boundaryVertices;

	public:
		Mesh3D();
		~Mesh3D();

		inline PTR_VERTEX_LIST getVerticesList()
		{
			return verticesList;
		}

		inline PTR_EDGE_LIST getEdgesList()
		{
			return edgesList;
		}

		inline PTR_FACE_LIST facesList()
		{
			return facesList;
		}

		inline int getNumOfVerticesList()
		{
			return verticesList ? (int)verticesList->size() : 0;
		}

		inline int getNumOfFacesList()
		{
			return facesList ? (int)facesList->size() : 0;
		}

		inline int getNumOfEdgesList()
		{
			return edgesList ? (int)edgesList->size() : 0;
		}

		inline HE_vert * getVertex(int id)
		{
			return (id >= getNumOfVerticesList() || id < 0) ? NULL : (*verticesList)[id];
		}

		inline HE_edge * getEdge(int id)
		{
			return (id >= getNumOfEdgesList() || id < 0) ? NULL : (*edgesList)[id];
		}

		inline HE_face * getFace(int id)
		{
			return (id >= getNumOfFacesList() || id < 0) ? NULL : (*facesList)[id];
		}

		inline int getNumOfComponents()
		{
			return m_numComponents;
		}

		inline int getNumOfBoundaries()
		{
			return m_numBoundaries;
		}

		inline int getGenus()
		{
			return m_genus;
		}

		inline HE_vert * insert_vertex(vec3 & v);

		inline HE_face * insert_face(VERTEX_LIST & vec_hv);

		inline bool isValid()
		{
			if (getNumOfVerticesList() == 0 || getNumOfFacesList() == 0)
				return false;
			return true;
		}

		inline bool isClosed()
		{
			return m_closed;
		}

		inline bool isBorder(HE_vert * hv);
		inline bool isBorder(HE_edge * he);
		inline bool isBorder(HE_face * hf);

		bool load_off(const char * fin);
		void write_off(const char * fout);
		bool load_obj(const char * fin);
		void write_obj(const char * fout);

		inline void updateMesh();

		inline void updateNormal();

		inline void computeBoundingBox();

		Mesh3D * makeCopy();
		Mesh3D * reverseOrientation();

		inline void resetVerticesTag(bool tagStatus);
		inline void resetFacesTag(bool tagStatus);
		inline void resetEdgesTag(bool tagStatus);
		inline void resetAllTag(bool tagStatus);

		inline void reverseNormal();

		inline void clearData();

		void translate(const vec3 & trans);
		void scale(float scaleX, float scaleY, float scaleZ);

		void initEdgeTag();

		void glDraw(bool smooth = false);
		void glDrawLine();
		void glDrawVertex();

	private:
		HE_edge *insertEdge(HE_vert * vStart, HE_vert * vEnd);

		inline void clearVertices();
		inline void clearEdges();
		inline void clearFaces();

		inline void setNextEdgeForBorderVertices();

		inline void checkClosed();
		inline void checkMeshType();

		inline void computeFacesListNormal();
		inline void computePerFaceNormal();
		inline void computeVerticesListNormal();
		inline void computePerVerticesNormal();

		inline void computeNumComponents();
		inline void computeNumBoundaries();
		inline void computeGenus();

		inline void removeHangledVertices();
	};
}

#endif//__MESH_3D_H__