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

struct BoundingBox
{
	std::vector<vec3 *> points;
	float xMin;
	float xMax;
};

typedef std::vector<BoundingBox *> PC_BBox_List;

class RBF_Func
{
private:
	PC_BBox_List *mBBoxList;
	std::vector<vec3 *> *mPoints;

	static const int BBOX_MAX_POINTS = 20;

public:
	RBF_Func();
	RBF_Func(const std::vector<vec3> & points);
	RBF_Func(const string & path);
	virtual ~RBF_Func();
	float func(float x, float y, float z);

private:
	void initPoints(const std::vector<vec3> & points);
	void loadPoints(const string & path);
	void initMesh();
	void cutBBox();
	void initFunc();
};



#endif//__RBF_FUNC_H__