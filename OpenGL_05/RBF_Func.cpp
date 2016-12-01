#include "RBF_Func.h"

#include <cmath>
#include <Eigen\Dense>

// for debug
#include <iostream>
using std::cout;
using std::endl;

RBF_Func::RBF_Func() :
	mBBoxList(NULL)
{

}

// init from vector<vec3>
RBF_Func::RBF_Func(const std::vector<vec3> & points)
{
	initPoints(points);
	initBBox();
	initFunc();
}

// init from file
// format vector3
RBF_Func::RBF_Func(const string & path)
{
	loadPoints(path.c_str());
	initBBox();
	initFunc();
}

RBF_Func::~RBF_Func()
{

}

// the final sectioned function
float RBF_Func::func(float x, float y, float z)
{
	// find the right bounding box
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
	{
		if (x > (*boxI)->xMin && x < (*boxI)->xMax)
			return bBoxFunc(*boxI, x, y, z);
	}

	throw NoSuitBoxError();
}

// load points from vector<vec3>
void RBF_Func::initPoints(const std::vector<vec3> & points)
{
	vec4 *newPoint;
	mBBoxList.push_back(new BoundingBox);
	mBBoxList.front()->xMax = INFINITY_F;
	mBBoxList.front()->xMin = -INFINITY_F;

	std::vector<vec3>::iterator pointI;
	for (; pointI < points.end(); pointI++)
	{
		//newPoint->w is Wn in RBF func
		newPoint = new vec4(*pointI, 1.0f);
		mBBoxList.front()->points.push_back(newPoint);
	}
}

// load points from file
// format vector3
void RBF_Func::loadPoints(const string & path)
{
	std::ifstream fileIn(path);
	std::stringstream ss;
	string line;
	vec4 *newPoint;
	mBBoxList.push_back(new BoundingBox);

	while (!fileIn.eof())
	{
		std::getline(fileIn, line);
		if (line.empty())
			continue;
		ss.str(line);
		ss.clear();

		//newPoint->w is Wn in RBF func
		newPoint = new vec4();
		ss >> newPoint->x >> newPoint->y >> newPoint->z;
		mBBoxList.front()->points.push_back(newPoint);
	}

	fileIn.close();
}

// make num of points in one bounding box less than BBOX_MAX_POINTS
void RBF_Func::initBBox()
{
	float xMin = INFINITY_F;
	float xMax = -INFINITY_F;

	std::list<vec4 *>::iterator pointI = mBBoxList.front()->points.begin();
	for (; pointI != mBBoxList.front()->points.end(); pointI++)
	{
		if (xMin > (*pointI)->x)
			xMin = (*pointI)->x;
		if (xMax < (*pointI)->x)
			xMax = (*pointI)->x;
	}
	mBBoxList.front()->xMax = xMax;
	mBBoxList.front()->xMin = xMin;

	cutBBox(*mBBoxList.begin());

	mBBoxList.sort(bBoxCmp);

	mBBoxList.front()->xMin = -INFINITY_F;
	mBBoxList.back()->xMax = INFINITY;
}

void RBF_Func::initFunc()
{
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
	{
		bBoxPointsWeight(*boxI);
	}
}

void RBF_Func::cutBBox(BoundingBox * box)
{
	if (box->points.size() < BBOX_MAX_POINTS)
		return;

	BoundingBox *newBBox = new BoundingBox;
	divBBoxByX(box, newBBox);

	cutBBox(box);
	
	if (newBBox->points.size() > 0)
	{
		mBBoxList.push_back(newBBox);
		cutBBox(newBBox);
	}
}

void RBF_Func::divBBoxByX(BoundingBox * box1, BoundingBox * box2)
{
	float xMid = (box1->xMax + box1->xMin) / 2.0f;
	box2->xMax = box1->xMax;
	box2->xMin = box1->xMax = xMid;

	std::list<vec4 *>::iterator pointI = box1->points.begin();
	for (; pointI != box1->points.end(); )
	{
		if ((*pointI)->x < xMid)
		{
			pointI++;
			continue;
		}

		box2->points.push_back(*pointI);
		pointI = box1->points.erase(pointI);
	}
}

void RBF_Func::bBoxPointsWeight(BoundingBox * box)
{
	int numOfPoints = box->points.size();
	Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(numOfPoints + 4, numOfPoints + 3);
	Eigen::VectorXf d = Eigen::VectorXf::Zero(numOfPoints + 4);

	std::list<vec4 *>::iterator pointI_i = box->points.begin();
	std::list<vec4 *>::iterator pointI_j;
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
	{
		pointI_j = box->points.begin();
		for (int j = 0; j < numOfPoints; j++, pointI_j++)
		{
			matrix(i, j) = vec3DisModuleCube(*pointI_i, *pointI_j);
		}

		matrix(i, numOfPoints) = (*pointI_i)->x;
		matrix(i, numOfPoints + 1) = (*pointI_i)->y;
		matrix(i, numOfPoints + 2) = (*pointI_i)->z;
	}

	pointI_i = box->points.begin();
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
	{
		matrix(numOfPoints, i) = (*pointI_i)->x;
		matrix(numOfPoints + 1, i) = (*pointI_i)->y;
		matrix(numOfPoints + 2, i) = (*pointI_i)->z;
		matrix(numOfPoints + 3, i) = 1.0f;

		d(i) = -1.0f;
	}

	Eigen::VectorXf w = matrix.householderQr().solve(d);

	cout << w << endl;

	pointI_i = box->points.begin();
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
		(*pointI_i)->w = w(i);

	box->a = w(numOfPoints);
	box->b = w(numOfPoints + 1);
	box->c = w(numOfPoints + 2);
}

float RBF_Func::bBoxFunc(BoundingBox & box, float x, float y, float z)
{
	return bBoxFunc(&box, x, y, z);
}

float RBF_Func::bBoxFunc(BoundingBox * box, float x, float y, float z)
{
	float value = box->a * x + box->b * y + box->c * z;
	vec4 point(x, y, z, 0.0f);
	float phi;

	std::list<vec4 *>::iterator pointI = box->points.begin();
	for (; pointI != box->points.end(); pointI++)
	{
		phi = (*pointI)->w * vec3DisModuleCube(&point, *pointI);
		value += phi;
	}

	return value;
}

float RBF_Func::vec3DisModuleCube(const vec4 * point1, const vec4 * point2)
{
	vec4 pd = *point1 - *point2;

	float module = std::sqrtf(pd.x * pd.x + pd.y * pd.y + pd.z * pd.z);
	return module * module * module;
}

bool bBoxCmp(const BoundingBox * box1, const BoundingBox * box2)
{
	return box1->xMax > box2->xMax;
}