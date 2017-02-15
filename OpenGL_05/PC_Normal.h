#ifndef __PC_NORMAL_H__
#define __PC_NORMAL_H__

#include <vector>

#include "Third_party\include\ANN\ANN.h"

class PC_Normal
{
protected:
	std::vector<ANNpoint> mPoints;
	std::vector<float *> mpPointsf;
	ANNkd_tree * mPointKDTree;

public:
	PC_Normal();
	virtual ~PC_Normal();
	
	void setupKDTree();

	//	for double
	void addPoint(double * point3D);
	void removePoint(double * point3D);
	void getNeighbors(double * pointQuery3, int numOfNeighbors, int * neighborIndices);
	int getRNeighbors(double * pointQuery3, int numOfNeighbors, double r, int * neighborIndices);
	void getNormal3(double * pointQuery3, int numOfNeighbors, double distanceQuery, double & distanceMin, double * normal3);

	//	for float
	void addPointf(float * point3D);
	void removePointf(float * point3D);
	void getNeighborsf(float * pointQuery3f, int numOfNeighbors, int * neighborIndices);
	int getRNeighborsf(float * pointQuery3f, int numOfNeighbors, float r, int * neighborIndices);
	void getNormal3f(float * pointQuery3f, int numOfNeighbors, float distanceQueryf, float & distanceMinf, float * normal3f);

protected:
	void getNormalFromPointIndices(ANNidxArray indices, int numOfPoints, double * normal3);
};

#endif//__PC_NORMAL_H__
