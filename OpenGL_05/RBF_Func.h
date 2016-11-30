#ifndef __RBF_FUNC_H__
#define __RBF_FUNC_H__

#include "HES_Mesh.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <glm\glm.hpp>
#include <Eigen\Dense>

using std::string;
using glm::vec3;
using glm::vec4;

class NoSuitBoxError
{

};

struct BoundingBox
{
	std::vector<vec4 *> points;
	float xMin;
	float xMax;
	float a;
	float b;
	float c;
};

typedef std::vector<BoundingBox *> RBF_BBox_List;
typedef RBF_BBox_List::iterator RBF_BBox_Iter;

class RBF_Func
{
private:
	RBF_BBox_List mBBoxList;
	std::vector<vec3 *> *mPoints;

	static const int BBOX_MAX_POINTS = 20;

public:
	RBF_Func();
	RBF_Func(const std::vector<vec3> & points);
	RBF_Func(const string & path);
	virtual ~RBF_Func();
	inline float func(float x, float y, float z);

private:
	void initPoints(const std::vector<vec3> & points);
	void loadPoints(const string & path);
	void initMesh();
	void cutBBox();
	void initFunc();

	inline float bBoxFunc(const BoundingBox & box, float x, float y, float z);
	inline float bBoxFunc(const BoundingBox * box, float x, float y, float z);
};



#endif//__RBF_FUNC_H__