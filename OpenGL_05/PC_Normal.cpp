#include "PC_Normal.h"
#include <Eigen\Dense>

#include <cmath>
#include <iostream>
#include <ctime>

PC_Normal::PC_Normal() :
	mPoints(), mpPointsf(), mPointKDTree(NULL)
{

}

PC_Normal::~PC_Normal()
{

}

void PC_Normal::addPoint(double * point3D)
{
	mPoints.push_back(point3D);
}

void PC_Normal::addPointf(float * point3D)
{
	ANNcoord *coord = new ANNcoord[3];
	coord[0] = static_cast<double>(point3D[0]);
	coord[1] = static_cast<double>(point3D[1]);
	coord[2] = static_cast<double>(point3D[2]);

	mPoints.push_back(coord);

	//	An index to the ptr of pointf
	//	Easy way to remove the pointf
	mpPointsf.push_back(point3D);
}

void PC_Normal::removePoint(double * point3D)
{
	int size = mPoints.size();
	for (int i = 0; i < size; i++)
	{
		if (mPoints[i] == point3D)
		{
			mPoints.erase(mPoints.begin() + i);
			break;
		}
	}
}

void PC_Normal::removePointf(float * point3D)
{
	int size = mpPointsf.size();
	for (int i = 0; i < size; i++)
	{
		if (mpPointsf[i] == point3D)
		{
			mpPointsf.erase(mpPointsf.begin() + i);
			mPoints.erase(mPoints.begin() + i);
			break;
		}
	}
}

void PC_Normal::setupKDTree()
{
	mPointKDTree = new ANNkd_tree(
		&mPoints.front(),	//	points 3D data
		mPoints.size(),		//	num of points
		3					//	dimension = 3
	);
}

void PC_Normal::getNeighbors(double * queryPoint3, int numOfNeighbors,
	int * neighborIndices)
{
	numOfNeighbors++;

	ANNidxArray indices = new ANNidx[numOfNeighbors];
	ANNdistArray distance = new ANNdist[numOfNeighbors];

	mPointKDTree->annkSearch(
		queryPoint3,
		numOfNeighbors,
		indices,
		distance
	);

	for (int i = 1; i < numOfNeighbors; i++)
		neighborIndices[i - 1] = indices[i];

	delete[] indices;
	delete[] distance;
}

void PC_Normal::getNeighborsf(float * queryPoint3f, int numOfNeighbors,
	int * neighborIndices)
{
	double *queryPoint3 = new double[3];

	queryPoint3[0] = static_cast<double>(queryPoint3f[0]);
	queryPoint3[1] = static_cast<double>(queryPoint3f[1]);
	queryPoint3[2] = static_cast<double>(queryPoint3f[2]);

	getNeighbors(queryPoint3, numOfNeighbors, neighborIndices);

	delete[] queryPoint3;
}

void PC_Normal::getNormal3(double * queryPoint3, int numOfNeighbors, 
	double & distanceMin, double * normal3)
{
	//	indices[0] always equal to the query point
	numOfNeighbors++;

	ANNidxArray neighborIndices = new ANNidx[numOfNeighbors];
	ANNdistArray distance = new ANNdist[numOfNeighbors];

	mPointKDTree->annkSearch(
		queryPoint3,
		numOfNeighbors,
		neighborIndices,
		distance
	);

	//	indices[0] always equal to the query point
	distanceMin = sqrt(distance[1]);

	getNormalFromPointIndices(neighborIndices, numOfNeighbors, normal3);

	delete[] distance;
	delete[] neighborIndices;
}

void PC_Normal::getNormal3f(float * queryPoint3f, int numOfNeighbors, 
	float & distanceMinf, float * normal3f)
{
	double *queryPoint3 = new double[3];
	double *normal3 = new double[3];
	double distanceMin;

	queryPoint3[0] = static_cast<double>(queryPoint3f[0]);
	queryPoint3[1] = static_cast<double>(queryPoint3f[1]);
	queryPoint3[2] = static_cast<double>(queryPoint3f[2]);

	getNormal3(queryPoint3, numOfNeighbors, distanceMin, normal3);

	distanceMinf = distanceMin;
	normal3f[0] = static_cast<float>(normal3[0]);
	normal3f[1] = static_cast<float>(normal3[1]);
	normal3f[2] = static_cast<float>(normal3[2]);

	delete[] queryPoint3;
	delete[] normal3;
}

void PC_Normal::getNormalFromPointIndices(ANNidxArray indices, int numOfPoints, double * normal3)
{
	double A1, B1, B2, C1, C2, C3, D1, D2, D3;
	A1 = B1 = B2 = C1 = C2 = C3 = D1 = D2 = D3 = 0.0;

	double *pPoint;

	//	indices[0] always equal to the query point
	for (int i = 1; i < numOfPoints; i++)
	{
		pPoint = mPoints[indices[i]];

		A1 += pPoint[0] * pPoint[0];		//	¡Æ x^2
		B1 += pPoint[0] * pPoint[1];		//	¡Æ x * y
		B2 += pPoint[1] * pPoint[1];		//	¡Æ y^2
		C1 += pPoint[0] * pPoint[2];		//	¡Æ x * z
		C2 += pPoint[1] * pPoint[2];		//	¡Æ y * z
		C3 += pPoint[2] * pPoint[2];		//	¡Æ z^2
		D1 += pPoint[0];					//	¡Æ x
		D2 += pPoint[1];					//	¡Æ y
		D3 += pPoint[2];					//	¡Æ z
	}

	Eigen::Matrix3d M;
	M <<	A1, B1, C1,
			B1, B2, C2,
			C1, C2, C3;
	Eigen::Vector3d d(D1, D2, D3);
	Eigen::Vector3d w = M.colPivHouseholderQr().solve(d);
	//std::cout << w << std::endl;

	// w(0)*x + w(1)*y + w(2)*z + 1 = 0
	normal3[0] = w(0);
	normal3[1] = w(1);
	normal3[2] = w(2);
}