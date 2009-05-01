/*
 *  CalibrationUnit.h
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/21/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "MeshContainer.h"

class CalibrationUnit{
	
public:
	
	static CalibrationUnit * Instance();
	
	//returns the resolution of the current mesh
	int getMeshRes();
	
	//takes points in the image and converts them into real space coords
	void pointToMesh(int xPic, int yPic, double points[3]);
	
	//takes the image data line and puts it into the mesh
	void lineToMesh(int **points, double zWorld, double theta);
	
	//set up the resolution of the image being used
	void setPicRes(int xRes, int yRes);
	
protected:
	
	//constructor
	CalibrationUnit();
	
private:
	
	static CalibrationUnit* pinstance;
	
	//camera to laser distance
	double camToLas;
	double lasToCenter;
	double camLasAng;
	
	//points for real space used in lineToMesh
	double realPts[3];
	
	char* filename;
	
	//resolution of the mesh
	int meshRes;
	
	//position of the actuator in real space
	int zWorld;
	
	int xRes, yRes;
	
	//camera parameters
	double xFOV, yFOV;
	double f;
	
	//projection parameters
	double xPD;
	double yPD;
	
};