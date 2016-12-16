#ifndef __RBF_FUNC_H__
#define __RBF_FUNC_H__

#include "HES_Mesh.h"

#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <glm\glm.hpp>

using std::string;
using glm::vec3;
using glm::vec4;

struct RBF_Point
{
	vec3 point;
	float w;
	float d;
};

struct BoundingBox
{
	std::list<RBF_Point *> points;
	float xMin;
	float xMax;
	float xMid;
	float xHalfWidth;
	float a;
	float b;
	float c;
};

struct PointAndNormal
{
	vec3 point;

	// easy to multyply by vec4
	vec3 normal;
};

typedef std::list<RBF_Point *> RBF_Point_List;
typedef std::list<BoundingBox *> RBF_BBox_List;
typedef std::list<PointAndNormal *> RBF_PointNormal_List;
typedef RBF_BBox_List::iterator RBF_BBox_Iter;
typedef RBF_PointNormal_List::iterator RBF_PointNormal_Iter;
typedef RBF_Point_List::iterator RBF_Point_Iter;

// build the infinity float num from bitmap
const unsigned int P_INFINITY_F_BITMAP = 0x7F7FFFFF;
const float INFINITY_F = *((float *)(&P_INFINITY_F_BITMAP));

const float BOUNDING_BOX_THICK = 0.0f;
const float BBOX_MIN_HALF_WIDTH = 0.0001f;

class RBF_Func
{
private:
	RBF_BBox_List mBBoxList;

	static const int BBOX_MAX_POINTS = 500;

public:
	RBF_Func();
	RBF_Func(std::vector<vec3> & points);
	RBF_Func(const string & path);
	virtual ~RBF_Func();
	float func(float x, float y, float z);

private:
	RBF_PointNormal_List * initPoints(std::vector<vec3> & points);
	RBF_PointNormal_List * loadPoints(const string & path);
	void initBBox(RBF_PointNormal_List * pointNormals);
	void initFunc();

	BoundingBox * getBBox(RBF_PointNormal_List * pointNormals);
	void cutBBox(BoundingBox * box);
	void getPointNormal(PointAndNormal ** pointNormalIndexs, float ** pointDistanceList, int numOfPoints, float disMin);
	void addNewPoints(RBF_PointNormal_List * pointNormals, BoundingBox * box, float pointDistance);

	inline void divBBoxByX(BoundingBox * box1, BoundingBox * box2);
	inline void mergeBBox(BoundingBox * box1, BoundingBox * box2);

	inline void bBoxPointsWeight(BoundingBox * box);
	inline void bBoxInfo(BoundingBox * box);

	inline float bBoxFunc(BoundingBox & box, float x, float y, float z);
	inline float bBoxFunc(BoundingBox * box, float x, float y, float z);
	inline float bBoxFuncWeight(BoundingBox * box, float x);

	inline static float vec3DisModuleCube(const vec3 & point1, const vec3 & point2);
};

bool bBoxCmp(const BoundingBox * box1, const BoundingBox * box2);

#endif//__RBF_FUNC_H__