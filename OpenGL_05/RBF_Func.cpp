#include "RBF_Func.h"

RBF_Func::RBF_Func() :
	mBBoxList(NULL), mPoints()
{

}

RBF_Func::RBF_Func(const std::vector<vec3> & points)
{
	initPoints(points);
	initMesh();
}

RBF_Func::RBF_Func(const string & path)
{
	loadPoints(path.c_str());
	initMesh();
}

RBF_Func::~RBF_Func()
{

}

float RBF_Func::func(float x, float y, float z)
{

}

void RBF_Func::initPoints(const std::vector<vec3> & points)
{
	vec3 *newPoint;
	mPoints = new std::vector<vec3 *>();

	std::vector<vec3>::iterator pointI;
	for (; pointI < points.end(); pointI++)
	{
		newPoint = new vec3(*pointI);
		mPoints->push_back(newPoint);
	}
}

void RBF_Func::loadPoints(const string & path)
{
	std::ifstream fileIn(path);
	std::stringstream ss;
	string line;
	vec3 *newPoint;
	mPoints = new std::vector<vec3 *>();

	while (!fileIn.eof())
	{
		std::getline(fileIn, line);
		if (line.empty())
			continue;
		ss.str(line);
		ss.clear();

		newPoint = new vec3();
		ss >> newPoint->x >> newPoint->y >> newPoint->z;
		mPoints->push_back(newPoint);
	}

	fileIn.close();
}

void RBF_Func::initMesh()
{
	cutBBox();

}

void RBF_Func::cutBBox()
{

}

void RBF_Func::initFunc()
{

}