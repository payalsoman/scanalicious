/*
 *  ImageProcessor.h
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 4/20/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "CalibrationUnit.h"

class ImageProcessor{

public:
	
	//the new instance
	static ImageProcessor * Instance();
	
	//used for accessing 1d array image
	char imageR(int xPic, int yPic);
	
	void setImageR(int xPic, int yPic, char val);
	
	//gets the red threshold
	void setRedThresh( char * data, int xRes, int yRes);
	
	//processes the line in an image and sends it to calibrator
	void processLine( char * data, int xRes, int yRes, double zWorld, double theta);
	
protected:
	
	//constructor -- must assign it a calibration unit
	ImageProcessor();
	
	//destructor
	~ImageProcessor();
	
private:
	
	static ImageProcessor* pinstance;
	
	//image components
	char * red;
	char * green;
	char * blue;
	char * alpha;
	
	double redThresh;
	
	//real space coords
	double zWorld;
	double angle;
		
	//the resolution of the mesh
	int res;
	
	//resolution of the incoming image
	int xRes;
	int yRes;
	
	//array of points on the line
	int linePts[500][2];
	int **samplePoints;
	
};