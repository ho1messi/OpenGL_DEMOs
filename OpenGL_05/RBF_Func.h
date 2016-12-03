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

class NoSuitBoxError {};

struct BoundingBox
{
	std::list<vec4 *> points;
	float xMin;
	float xMax;
	float xMid;
	float xHalfWidth;
	float a;
	float b;
	float c;
};

typedef std::list<BoundingBox *> RBF_BBox_List;
typedef RBF_BBox_List::iterator RBF_BBox_Iter;

// build the infinity float num from bitmap
const unsigned int P_INFINITY_F_BITMAP = 0x7F7FFFFF;
const float INFINITY_F = *((float *)(&P_INFINITY_F_BITMAP));

const float BBOX_MIN_HALF_WIDTH = 0.001f;

class RBF_Func
{
private:
	RBF_BBox_List mBBoxList;

	static const int BBOX_MAX_POINTS = 200;

public:
	RBF_Func();
	RBF_Func(const std::vector<vec3> & points);
	RBF_Func(const string & path);
	virtual ~RBF_Func();
	float func(float x, float y, float z);

private:
	BoundingBox * initPoints(const std::vector<vec3> & points);
	BoundingBox * loadPoints(const string & path);
	void initBBox(BoundingBox * box);
	void initFunc();

	void cutBBox(BoundingBox * box);
	void bBoxInfo(BoundingBox * box);
	void movBBoxPoints(BoundingBox * box1, BoundingBox * box2);
	inline void divBBoxByX(BoundingBox * box1, BoundingBox * box2);
	
	inline void bBoxPointsWeight(BoundingBox * box);

	inline float bBoxFunc(BoundingBox & box, float x, float y, float z);
	inline float bBoxFunc(BoundingBox * box, float x, float y, float z);
	inline float bBoxFuncWeight(BoundingBox * box, float x);

	inline static float vec3DisModuleCube(const vec4 * point1, const vec4 * point2);
};

bool bBoxCmp(const BoundingBox * box1, const BoundingBox * box2);

#endif//__RBF_FUNC_H__