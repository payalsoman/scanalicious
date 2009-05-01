/*
 *  CalibrationUnit.cpp
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/21/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "CalibrationUnit.h"

CalibrationUnit* CalibrationUnit::pinstance = 0;

CalibrationUnit* CalibrationUnit::Instance()
{
	if (pinstance == 0) //first call?
	{
	pinstance = new CalibrationUnit; //create sole instance
	}
	
	return pinstance;	//address of sole instance
}

CalibrationUnit::CalibrationUnit()
{
	filename = "calibration_data.txt";
		
	//set-up camera parameters
	xFOV = 43.7;
	yFOV = 23.8;
	f = 3.6;
	
	//TODO - change these to the actual values
	lasToCenter = 10;
	camToLas = 5;
	camLasAng = 45 * 360/(2*3.14);
}

int CalibrationUnit::getMeshRes()
{
	//singleton so this is ok
	MeshContainer &meshBag = *MeshContainer::Instance();
	
	int i = meshBag.getCurrentMesh();
	
	meshRes = meshBag.getMesh(i).getRes();
	
	return meshRes;
}

void CalibrationUnit::pointToMesh(int xPic, int yPic, double points[3])
{
	double x, y, z;
	double thetaX, phiZ;
	
	z = zWorld;

	//find the angles
	thetaX = atan( (xRes - .5*xRes) / xPD);
	phiZ = atan( (yRes - .5*yRes) / yPD);
	
	thetaX = camLasAng + thetaX;
	
	x = camToLas*tan(thetaX);
	
	y = x*tan(phiZ);
	
	x = lasToCenter - x;
	
	points[0] = x;
	points[1] = y;
	points[2] = z;
	
}

void CalibrationUnit::lineToMesh( int** points, double zWorld, double theta)
{
	//get the mesh resolution
	MeshContainer &meshBag = *MeshContainer::Instance();
	meshRes = meshBag.getRes();
	int n = meshBag.getCurrentMesh();
	
	CalibrationUnit::zWorld = zWorld;
	
	for(int i=0; i < meshRes ; i++)
	{
		pointToMesh(points[i][0], points[i][1], realPts);
		meshBag.getMesh(n).addToBuf(realPts[0], realPts[1], realPts[2]);
	}
	
	meshBag.getMesh(n).compileMesh();
}

void CalibrationUnit::setPicRes(int xRes, int yRes)
{	
	CalibrationUnit::xRes = xRes;
	CalibrationUnit::yRes = yRes;
	
	//projection parameters
	
	xPD = (.5*xRes) / tan(.5*xFOV * (2*3.14)/360);
	yPD = (.5*yRes) / tan(.5*yFOV * (2*3.14)/360);
}