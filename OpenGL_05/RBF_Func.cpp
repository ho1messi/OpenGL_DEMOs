#include "RBF_Func.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include "Third_party\include\Eigen\Dense"

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
	float value = 0.0f;
	bool haveBBox = false;

	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
	{
		if (x >= (*boxI)->xMin && x < (*boxI)->xMax)
		{
			// each Bounding Box multi by it's weight
			value += bBoxFunc(*boxI, x, y, z) *bBoxFuncWeight(*boxI, x);
			haveBBox = true;
		}
	}

	if (haveBBox)
		return value;
	return 1.0f;
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
	pointNum = pointNormals->size();
	BoundingBox *box = getBBox(pointNormals);

	box->xMin = INFINITY_F;
	box->xMax = -INFINITY_F;

	// get the Min and Max x value
	RBF_Point_Iter pointI = box->points.begin();
	for (; pointI != box->points.end(); pointI++)
	{
		if (box->xMin > (*pointI)->point.x)
			box->xMin = (*pointI)->point.x;
		if (box->xMax < (*pointI)->point.x)
			box->xMax = (*pointI)->point.x;
	}
	box->xMax += BOUNDING_BOX_THICK;
	box->xMin -= BOUNDING_BOX_THICK;
	bBoxInfo(box);

	// cut the box to a list by x
	cutBBox(box);

	// sort by xMax
	mBBoxList.sort(bBoxCmp);

	// merge two near Bounding Box
	// still some trouble with this
	RBF_BBox_Iter boxI_i = mBBoxList.begin();
	RBF_BBox_Iter boxI_j = mBBoxList.begin();
	for (boxI_i++; boxI_i != mBBoxList.end(); boxI_i++, boxI_j++)
		mergeBBox(*boxI_j, *boxI_i);

	// update Bounding Box's Half Width and xMid
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
		bBoxInfo(*boxI);
}

void RBF_Func::initFunc()
{
	// solve the weights of each point
	RBF_BBox_Iter boxI = mBBoxList.begin();
	for (; boxI != mBBoxList.end(); boxI++)
		bBoxPointsWeight(*boxI);

	cout << "Function calculation done" << endl;
}

// get the normal and add some points to the Bounding Box
// still some trouble with this
// ================================================================================
BoundingBox * RBF_Func::getBBox(RBF_PointNormal_List * pointNormals)
{
	PointAndNormal *pointNormal;
	vec3 *point;
	BoundingBox *box = new BoundingBox;
	RBF_PointNormal_List pointNormalsTemp;
	int xMaxIndex, xMinIndex;
	int yMaxIndex, yMinIndex;
	int zMaxIndex, zMinIndex;
	vec3 Max(-INFINITY_F, -INFINITY_F, -INFINITY_F);
	vec3 Min(INFINITY_F, INFINITY_F, INFINITY_F);
	box->xMax = -INFINITY_F;
	box->xMin = INFINITY_F;

	int size = pointNormals->size();
	for (int i = 0; i < size; i++)
	{
		pointNormal = (*pointNormals)[i];
		point = &(pointNormal->point);

		if (Max.x < point->x)
		{
			Max.x = point->x;
			xMaxIndex = i;
		}
		if (Max.y < point->y)
		{
			Max.y = point->y;
			yMaxIndex = i;
		}
		if (Max.z < point->z)
		{
			Max.z = point->z;
			zMaxIndex = i;
		}

		if (Min.x > point->x)
		{
			Min.x = point->x;
			xMinIndex = i;
		}
		if (Min.y > point->y)
		{
			Min.y = point->y;
			yMinIndex = i;
		}
		if (Min.z > point->z)
		{
			Min.z = point->z;
			zMinIndex = i;
		}

		mNormal.addPointf( reinterpret_cast<float *>( &(pointNormal->point) ) );
		pointNormalsTemp.push_back(pointNormal);
	}

	//normalizeVetices(pointNormals, Max, Min);
	box->xMax = Max.x;
	box->xMin = Min.x;

	mNormal.setupKDTree();

	getPointNormal(pointNormals);

	spanningTreeTraversal(pointNormals, pointNormalsTemp, xMaxIndex, xMinIndex,
		yMaxIndex, yMinIndex, zMaxIndex, zMinIndex);

	//writeNormals(pointNormals);
	//setupNormals(pointNormals);

	addNewPoints(pointNormals, box);

	return box;
}

// cut the Bounding Box to a list by x
void RBF_Func::cutBBox(BoundingBox * box)
{
	// until small enough
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

	// cut two new Bounding Box
	cutBBox(box);
	cutBBox(newBBox);
}

void RBF_Func::normalizeVetices(RBF_PointNormal_List * pointNormals, vec3 & max, vec3 & min)
{
	vec3 diff = (max - min) / NORMALIZE_MAX / 2.0f;

	cout << max.x << "\t" << max.y << "\t" << max.z << endl;

	vec3 *point;
	RBF_PointNormal_Iter pointNormalI = pointNormals->begin();
	for (; pointNormalI != pointNormals->end(); pointNormalI++)
	{
		point = &( (*pointNormalI)->point );
		//cout << "O:\t" << point->x << "\t" << point->y << "\t" << point->z << endl;
		(*point) = ( (*point) - min ) / diff - 1.0f;
		//cout << "N:\t" << point->x << "\t" << point->y << "\t" << point->z << endl;
	}

	cout << "Normalization done" << endl;
}

// get the normal of each point
// still some trouble with this
// ===============================================================================================
void RBF_Func::getPointNormal(RBF_PointNormal_List * pointNormals)
{
	vec3 normal;
	PointAndNormal *pointNormal;

	RBF_PointNormal_Iter pointNormalI = pointNormals->begin();
	for (; pointNormalI != pointNormals->end(); pointNormalI++)
	{
		pointNormal = *pointNormalI;

		/* for debug
		normal = pointNormal->point;
		pointNormal->distanceMin = 0.02;
		*/

		mNormal.getNormal3f(
			reinterpret_cast<float *>(&(pointNormal->point)),
			100,
			0.1f,
			pointNormal->distanceMin, 
			reinterpret_cast<float *>(&normal)
		);

		if (pointNormal->distanceMin < 0.001f)
			pointNormal->distanceMin = 0.001f;

		//normal = glm::normalize(normal);
		//cout << normal.x * normal.x + normal.y * normal.y + normal.z * normal.z << endl;
		pointNormal->normal = glm::normalize(normal);

	}
}

void RBF_Func::spanningTreeTraversal(RBF_PointNormal_List * pointNormals, 
	RBF_PointNormal_List & pointNormalsTemp, int xMaxIndex, int xMinIndex,
	int yMaxIndex, int yMinIndex, int zMaxIndex, int zMinIndex)
{
	unsigned int size = pointNormals->size();
	const int k = 5;
	const float r = 0.1f;
	const int queueSize = size;

	//	queue of pointNormal
	PointAndNormal **indexQueue = new PointAndNormal * [queueSize];
	int neighbors[k];
	int count = size - 6;
	int p1 = 0;
	int p2 = 6;

	int indexToFix;
	int numOfNeighbors;
	int find;
	PointAndNormal *pointNormal;
	float *pPoint;

	//	the point has the largest X value
	//	it's normal's X value must be positive
	if ( pointNormalsTemp[xMaxIndex]->normal.x < 0.0f )
		pointNormalsTemp[xMaxIndex]->normal *= -1.0f;
	if ( pointNormalsTemp[yMaxIndex]->normal.y < 0.0f )
		pointNormalsTemp[yMaxIndex]->normal *= -1.0f;
	if ( pointNormalsTemp[zMaxIndex]->normal.z < 0.0f )
		pointNormalsTemp[zMaxIndex]->normal *= -1.0f;

	if (pointNormalsTemp[xMinIndex]->normal.x > 0.0f)
		pointNormalsTemp[xMinIndex]->normal *= -1.0f;
	if (pointNormalsTemp[yMinIndex]->normal.y > 0.0f)
		pointNormalsTemp[yMinIndex]->normal *= -1.0f;
	if (pointNormalsTemp[zMinIndex]->normal.z > 0.0f)
		pointNormalsTemp[zMinIndex]->normal *= -1.0f;
	/*
	// fix one point first
	indexQueue[0] = pointNormalsTemp[xMaxIndex];
	indexQueue[1] = pointNormalsTemp[yMaxIndex];
	indexQueue[2] = pointNormalsTemp[zMaxIndex];
	indexQueue[3] = pointNormalsTemp[xMinIndex];
	indexQueue[4] = pointNormalsTemp[yMinIndex];
	indexQueue[5] = pointNormalsTemp[zMinIndex];
	*/
	int indices[6] = { xMaxIndex, yMaxIndex, zMaxIndex, xMinIndex, yMinIndex, zMinIndex };
	std::sort(indices, indices + 6, std::greater<int>());
	for (int i = 0; i < 6; i++)
	{
		// fix one point first
		indexQueue[i] = pointNormalsTemp[indices[i]];

		//	delete the point which has the max X
		pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[indices[i]]->point));
		mNormal.removePointf(pPoint);
		pointNormalsTemp.erase(pointNormalsTemp.begin() + indices[i]);
	}
	/*
	pPoint = reinterpret_cast<float *>( &( pointNormalsTemp[xMaxIndex]->point ) );
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + xMaxIndex);

	pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[yMaxIndex]->point));
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + yMaxIndex);

	pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[zMaxIndex]->point));
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + zMaxIndex);

	pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[xMinIndex]->point));
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + xMinIndex);

	pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[yMinIndex]->point));
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + yMinIndex);

	pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[zMinIndex]->point));
	mNormal.removePointf(pPoint);
	pointNormalsTemp.erase(pointNormalsTemp.begin() + zMinIndex);
	*/
	//	breadth-first traversal
	//	base on k-nearest point
	while (count > 0 && p1 < p2)
	{
		//	delete the point in index p1
		pointNormal = indexQueue[p1];
		pPoint = reinterpret_cast<float *>( &(pointNormal->point ) );
		p1++;

		//	build the KD tree again, because the data changed
		mNormal.setupKDTree();

		//	get k nearest point in r
		numOfNeighbors = std::min(k, count);
		find = mNormal.getRNeighborsf(
			pPoint,
			numOfNeighbors,
			r,
			neighbors
		);
		find = std::min(find, numOfNeighbors);

		//	add them to the index queue
		for (int i = 0; i < find; i++, p2++)
		{
			indexToFix = neighbors[i];

			fixNormalDirect(pointNormal, pointNormalsTemp[indexToFix]);

			indexQueue[p2] = pointNormalsTemp[indexToFix];
		}

		std::sort(neighbors, neighbors + find, std::greater<int>());

		for (int i = 0; i < find; i++)
		{
			indexToFix = neighbors[i];

			pPoint = reinterpret_cast<float *>(&(pointNormalsTemp[indexToFix]->point));
			mNormal.removePointf(pPoint);
			pointNormalsTemp.erase(pointNormalsTemp.begin() + indexToFix);
		}

		count -= find;
	}

	delete indexQueue;

	cout << "Normal calculation done" << endl;
}

void RBF_Func::addNewPoints(RBF_PointNormal_List * pointNormals, BoundingBox * box)
{
	PointAndNormal *pointNormal;
	vec3 *point, *normal;
	vec3 point_temp;

	RBF_PointNormal_Iter pointNormalI = pointNormals->begin();
	for (; pointNormalI != pointNormals->end(); pointNormalI++)
	{
		pointNormal = *pointNormalI;
		point = &(pointNormal->point);
		normal = &(pointNormal->normal);

		// it's a problem
		pointNormal->distanceMin = 0.01f;

		box->points.push_back(new RBF_Point({ *point, 0.0f, 0.0f }));

		box->points.push_back(new RBF_Point({ (*point) + (*normal) * pointNormal->distanceMin, 0.0f, 1.0f }));

		box->points.push_back(new RBF_Point({ (*point) - (*normal) * pointNormal->distanceMin, 0.0f, -1.0f }));

		/*//	for debug
		point_temp = (*point) + (*normal) * pointNormal->distanceMin;
		//if ( fabs(point_temp.x) > fabs(point->x) 
		//	&& fabs(point_temp.y) > fabs(point->y)
		//	&& fabs(point_temp.z) > fabs(point->z) )
		if (pointNormal->distanceMin < 0.001f)
			cout << point->x << "\t"
				<< point->y << "\t"
				<< point->z << endl
				<< normal->x << "\t"
				<< normal->y << "\t"
				<< normal->z << endl 
				<< pointNormal->distanceMin << endl << endl;
		*/
		
		/*// for debug
		point_temp = *point;
		cout << point_temp.x << "\t"
			<< point_temp.y << "\t"
			<< point_temp.z << "\t"
			<< endl;
		
		point_temp = (*point) + (*normal) * pointNormal->distanceMin;
		cout << point_temp.x << "\t"
			<< point_temp.y << "\t"
			<< point_temp.z << "\t"
			<< endl;

		point_temp = (*point) - (*normal) * pointNormal->distanceMin;
		cout << point_temp.x << "\t"
			<< point_temp.y << "\t"
			<< point_temp.z << "\t"
			<< endl << endl;
		*/
	}

	cout << "Adding new points done" << endl;
}

void RBF_Func::divBBoxByX(BoundingBox * box1, BoundingBox * box2)
{
	float xMid = (box1->xMax + box1->xMin) / 2.0f;
	box2->xMax = box1->xMax;
	box2->xMin = xMid - BOUNDING_BOX_THICK;
	box1->xMax = xMid + BOUNDING_BOX_THICK;

	RBF_Point_Iter pointI = box1->points.begin();
	for (; pointI != box1->points.end(); )
	{
		if ((*pointI)->point.x < xMid)
		{
			pointI++;
			continue;
		}

		box2->points.push_back(*pointI);
		pointI = box1->points.erase(pointI);
	}
}

void RBF_Func::fixNormalDirect(PointAndNormal * pointNormal1, PointAndNormal * pointNormal2)
{
	if (glm::dot(pointNormal1->normal, pointNormal2->normal) < -0.0f)
		pointNormal2->normal *= -1.0f;
}

void RBF_Func::mergeBBox(BoundingBox * box1, BoundingBox * box2)
{
	RBF_Point *newPoint;
	box1->xMax = box2->xMax;

	RBF_Point_Iter pointI = box2->points.begin();
	for (; pointI != box2->points.end(); pointI++)
	{
		newPoint = new RBF_Point(**pointI);
		box1->points.push_back(newPoint);
	}
}

void RBF_Func::bBoxPointsWeight(BoundingBox * box)
{
	static int idOfBox = 0;
	int numOfPoints = box->points.size();
	cout << "Bounding box " << idOfBox++ << " has " << numOfPoints << " points" << endl;

	Eigen::MatrixXf matrix = Eigen::MatrixXf::Zero(numOfPoints + 3, numOfPoints + 3);
	Eigen::VectorXf d = Eigen::VectorXf::Zero(numOfPoints + 3);

	RBF_Point_Iter pointI_i = box->points.begin();
	RBF_Point_Iter pointI_j;
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
	{
		pointI_j = box->points.begin();
		for (int j = 0; j < numOfPoints; j++, pointI_j++)
		{
			matrix(i, j) = vec3DisModuleCube((*pointI_i)->point, (*pointI_j)->point);
		}

		matrix(i, numOfPoints) = (*pointI_i)->point.x;
		matrix(i, numOfPoints + 1) = (*pointI_i)->point.y;
		matrix(i, numOfPoints + 2) = (*pointI_i)->point.z;
		//matrix(i, numOfPoints + 3) = 1.0f;
	}

	pointI_i = box->points.begin();
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
	{
		matrix(numOfPoints, i) = (*pointI_i)->point.x;
		matrix(numOfPoints + 1, i) = (*pointI_i)->point.y;
		matrix(numOfPoints + 2, i) = (*pointI_i)->point.z;
		//matrix(numOfPoints + 3, i) = 1.0f;

		d(i) = (*pointI_i)->d -1.0f;
	}

	Eigen::VectorXf w = matrix.colPivHouseholderQr().solve(d);

	pointI_i = box->points.begin();
	for (int i = 0; i < numOfPoints; i++, pointI_i++)
		(*pointI_i)->w = w(i);

	box->a = w(numOfPoints);
	box->b = w(numOfPoints + 1);
	box->c = w(numOfPoints + 2);
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
	vec3 point(x, y, z);
	float phi;

	RBF_Point_Iter pointI = box->points.begin();
	for (; pointI != box->points.end(); pointI++)
	{
		phi = (*pointI)->w * vec3DisModuleCube(point, (*pointI)->point);
		value += phi;
	}

	return value;
}

float RBF_Func::bBoxFuncWeight(BoundingBox * box, float x)
{
	return 1.0f - fabs(x - box->xMid) / box->xHalfWidth;
}

float RBF_Func::vec3DisModuleCube(const vec3 & point1, const vec3 & point2)
{
	vec3 pd = point1 - point2;

	float module = std::sqrtf(pd.x * pd.x + pd.y * pd.y + pd.z * pd.z);
	return module * module * module;
}

void RBF_Func::writeNormals(RBF_PointNormal_List *pointNormals)
{
	std::ofstream fileOut("Resource\\normals_out.txt");

	vec3 *point, *normal;
	PointAndNormal *pointNormal;
	for (size_t i = 0; i < pointNum; i++)
	{
		pointNormal = (*pointNormals)[i];
		point = &(pointNormal->point);
		normal = &(pointNormal->normal);
		
		fileOut << "v\t" << point->x << "\t" << point->y << "\t" << point->z << endl;
		fileOut << "n\t" << normal->x << "\t" << normal->y << "\t" << normal->z << endl;
	}

	fileOut.close();
}

void RBF_Func::setupNormals(RBF_PointNormal_List * pointNormals)
{
	if (!glIsVertexArray(VAO))
		glGenVertexArrays(1, &VAO);
	if (!glIsBuffer(VBO))
		glGenBuffers(1, &VBO);

	size_t vec3Size = sizeof(vec3);
	size_t offset = 0;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, 2 * vec3Size * pointNum, 0, GL_STATIC_DRAW);

	vec3 newVertex;
	PointAndNormal *pointNormal;
	for (size_t i = 0; i < pointNum; i++)
	{
		pointNormal = (*pointNormals)[i];
		newVertex = pointNormal->point + pointNormal->normal;

		glBufferSubData(GL_VERTEX_ARRAY, offset, vec3Size, &(pointNormal->point) );
		offset += vec3Size;

		glBufferSubData(GL_VERTEX_ARRAY, offset, vec3Size, &newVertex);
		offset += vec3Size;
	}
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vec3Size, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}


//	something wrong
void RBF_Func::drawNormals()
{
	glBindVertexArray(VAO);

	glDrawArrays(GL_LINES, 0, pointNum * 2);

	glBindVertexArray(0);
}

bool bBoxCmp(const BoundingBox * box1, const BoundingBox * box2)
{
	return box1->xMin < box2->xMin;
}