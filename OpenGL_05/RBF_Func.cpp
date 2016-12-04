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
RBF_Func::RBF_Func(std::vector<vec3> & points)
{
	RBF_PointNormal_List *pointNormals = initPoints(points);
	initBBox(pointNormals);
	initFunc();
}

// init from file
// format vector3
RBF_Func::RBF_Func(const string & path)
{
	RBF_PointNormal_List *pointNormals = loadPoints(path.c_str());
	initBBox(pointNormals);
	initFunc();
}

RBF_Func::~RBF_Func()
{

}

// the final sectioned function
float RBF_Func::func(float x, float y, float z)
{
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
	{
		if (x >= (*boxI)->xMin && x < (*boxI)->xMax)
			return bBoxFuncWeight(*boxI, x) * bBoxFunc(*boxI, x, y, z);
	}

	return 1.0f;
	//throw NoSuitBoxError();
}

// load points from vector<vec3>
RBF_PointNormal_List * RBF_Func::initPoints(std::vector<vec3> & points)
{
	PointAndNormal *newPoint;
	RBF_PointNormal_List *pointNormals = new RBF_PointNormal_List;

	std::vector<vec3>::iterator pointI = points.begin();
	for (; pointI < points.end(); pointI++)
	{
		newPoint = new PointAndNormal;
		newPoint->point = vec3(*pointI);
		pointNormals->push_back(newPoint);
	}

	return pointNormals;
}

// load points from file
// format vector3
RBF_PointNormal_List * RBF_Func::loadPoints(const string & path)
{
	std::ifstream fileIn(path);
	std::stringstream ss;
	string line;
	RBF_PointNormal_List *pointNormals = new RBF_PointNormal_List;
	PointAndNormal *newPoint = new PointAndNormal;

	while (!fileIn.eof())
	{
		std::getline(fileIn, line);
		if (line.empty())
			continue;
		ss.str(line);
		ss.clear();

		//newPoint->w is Wn in RBF func
		newPoint = new PointAndNormal;
		ss >> newPoint->point.x >> newPoint->point.y >> newPoint->point.z;
		pointNormals->push_back(newPoint);
	}

	fileIn.close();

	return pointNormals;
}

// make num of points in one bounding box less than BBOX_MAX_POINTS
void RBF_Func::initBBox(RBF_PointNormal_List * pointNormals)
{
	BoundingBox *box = getBBox(pointNormals);

	box->xMin = INFINITY_F;
	box->xMax = -INFINITY_F;

	std::list<vec4 *>::iterator pointI = box->points.begin();
	for (; pointI != box->points.end(); pointI++)
	{
		if (box->xMin > (*pointI)->x)
			box->xMin = (*pointI)->x;
		if (box->xMax < (*pointI)->x)
			box->xMax = (*pointI)->x;
	}
	box->xMax += BOUNDING_BOX_THICK;
	box->xMin -= BOUNDING_BOX_THICK;
	bBoxInfo(box);

	cutBBox(box);

	mBBoxList.sort(bBoxCmp);

	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
		bBoxInfo(*boxI);
}

void RBF_Func::initFunc()
{
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
	{
		bBoxPointsWeight(*boxI);
	}
}

BoundingBox * RBF_Func::getBBox(RBF_PointNormal_List * pointNormals)
{
	BoundingBox *box = new BoundingBox;
	float pointDistanceMin = INFINITY_F;
	float **pointDistanceList = new float *[pointNormals->size()];
	PointAndNormal **pointNormalIndexs = new PointAndNormal *[pointNormals->size()];

	RBF_PointNormal_Iter pointNormalI_i = pointNormals->begin();
	RBF_PointNormal_Iter pointNormalI_j;
	for (int i = 0; pointNormalI_i != pointNormals->end(); pointNormalI_i++, i++)
	{
		pointNormalIndexs[i] = *pointNormalI_i;
		pointDistanceList[i] = new float[pointNormals->size()];

		pointNormalI_j = pointNormalI_i;
		pointNormalI_j++;
		for (int j = i + 1; pointNormalI_j != pointNormals->end(); pointNormalI_j++, j++)
		{
			pointDistanceList[i][j] = glm::distance((*pointNormalI_i)->point, (*pointNormalI_j)->point);

			if (pointDistanceMin > pointDistanceList[i][j])
				pointDistanceMin = pointDistanceList[i][j];
		}
	}

	getPointNormal(pointNormalIndexs, pointDistanceList, pointNormals->size(), pointDistanceMin);
	addNewPoints(pointNormals, box, pointDistanceMin / 2);

	return box;
}

void RBF_Func::cutBBox(BoundingBox * box)
{
	if (box->points.size() < BBOX_MAX_POINTS
		|| box->xHalfWidth < BBOX_MIN_HALF_WIDTH)
	{
		if (box->points.size() > 0)
		{
			mBBoxList.push_back(box);
		}
		return;
	}

	BoundingBox *newBBox = new BoundingBox;
	divBBoxByX(box, newBBox);
	bBoxInfo(newBBox);

	cutBBox(box);
	cutBBox(newBBox);
}

void RBF_Func::getPointNormal(PointAndNormal ** pointNormalIndexs, float ** pointDistanceList, int numOfPoints, float disMin)
{
	PointAndNormal * pointInPlane[3];
	vec3 vector1, vector2;
	vec3 normal;
	float pointDistance = 100.0f * disMin;
	int k;

	for (int i = 0; i < numOfPoints; i++)
	{
		k = 0;
		for (int j = 0; k < 3 && j < numOfPoints; j++)
		{
			if ( (i < j && pointDistanceList[i][j] < pointDistance)
			  || (i > j && pointDistanceList[j][i] < pointDistance) )
				pointInPlane[k++] = pointNormalIndexs[j];
		}

		if (k < 3)
			continue;

		vector1 = pointInPlane[0]->point - pointInPlane[1]->point;
		vector2 = pointInPlane[1]->point - pointInPlane[2]->point;
		normal = glm::cross(vector1, vector2);
		pointNormalIndexs[i]->Normal = vec4(glm::normalize(normal), 0.0f);
	}
}

void RBF_Func::addNewPoints(RBF_PointNormal_List * pointNormals, BoundingBox * box, float pointDistance)
{
	vec4 *point, *newPoint;

	RBF_PointNormal_Iter pointNormalI = pointNormals->begin();
	for (; pointNormalI != pointNormals->end(); pointNormalI++)
	{
		point = new vec4((*pointNormalI)->point, 0.0f);
		box->points.push_back(point);

		newPoint = new vec4(*point + (*pointNormalI)->Normal * pointDistance);
		box->points.push_back(newPoint);

		newPoint = new vec4(*point - (*pointNormalI)->Normal * pointDistance);
		box->points.push_back(newPoint);
	}
}

void RBF_Func::divBBoxByX(BoundingBox * box1, BoundingBox * box2)
{
	float xMid = (box1->xMax + box1->xMin) / 2.0f;
	box2->xMax = box1->xMax;
	box2->xMin = xMid - BOUNDING_BOX_THICK;
	box1->xMax = xMid + BOUNDING_BOX_THICK;

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

	//Eigen::VectorXf w = matrix.colPivHouseholderQr().solve(d);
	Eigen::VectorXf w = matrix.fullPivLu().solve(d);

	pointI_i = box->points.begin();
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
		(*pointI_i)->w = w(i);

	box->a = w(numOfPoints);
	box->b = w(numOfPoints + 1);
	box->c = w(numOfPoints + 2);

	/* for debug
	pointI_i = box->points.begin();
	for (; pointI_i != box->points.end(); pointI_i++)
		cout << func((*pointI_i)->x, (*pointI_i)->y, (*pointI_i)->z) << endl;
	*/
}

void RBF_Func::bBoxInfo(BoundingBox * box)
{
	box->xMid = (box->xMax + box->xMin) / 2.0f;
	box->xHalfWidth = (box->xMax - box->xMin) / 2.0f;
}

float RBF_Func::bBoxFunc(BoundingBox & box, float x, float y, float z)
{
	return bBoxFunc(&box, x, y, z);
}

float RBF_Func::bBoxFunc(BoundingBox * box, float x, float y, float z)
{
	float value = box->a * x + box->b * y + box->c * z + 1.0f;
	vec4 point(x, y, z, 0.0f);
	float phi;

	std::list<vec4 *>::iterator pointI = box->points.begin();
	for (; pointI != box->points.end(); pointI++)
	{
		phi = (*pointI)->w * vec3DisModuleCube(&point, *pointI);
		value += phi;
	}

	return -value;
}

float RBF_Func::bBoxFuncWeight(BoundingBox * box, float x)
{
	return 1 - fabs(x - box->xMid) / box->xHalfWidth;
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