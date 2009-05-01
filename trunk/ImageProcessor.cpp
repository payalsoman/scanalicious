/*
 *  ImageProcessor.cpp
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 4/20/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "ImageProcessor.h"

ImageProcessor* ImageProcessor::pinstance = 0;

ImageProcessor* ImageProcessor::Instance()
{
	if (pinstance == 0) //first call?
	{
		pinstance = new ImageProcessor; //create sole instance
	}
	
	return pinstance;	//address of sole instance
}

ImageProcessor::ImageProcessor()
{ 
	redThresh = 200;
	MeshContainer &meshBag = *MeshContainer::Instance();
	res = meshBag.getRes();
	
	//declare the amount of sampled points
	//depending on the resolution of the mesh
	samplePoints = new int*[res];
	for(int i = 0; i < res; i++)
		samplePoints[i] = new int[2];
}

ImageProcessor::~ImageProcessor()
{
	delete red;
	delete green;
	delete blue;
	delete alpha;
}


char ImageProcessor::imageR(int xPic, int yPic)
{
	return red[xPic + yPic*xRes];
}

void ImageProcessor::setImageR(int xPic, int yPic, char val)
{
	red[xPic + yPic*xRes] = val;
}

//gets the red threshold
void ImageProcessor::setRedThresh( char *data, int xRes, int yRes)
{	
	char temp = 0;
	
	//load image and set resolution
	red = data;
	
	ImageProcessor::xRes = xRes;
	ImageProcessor::yRes = yRes;
		
	for(int x=0; x< xRes; x++)
	{
		for(int y=0; y<yRes; y++)
		{
			if( imageR(x,y) > temp)
				temp = imageR(x,y);
		}
	}
	
	//set the new thresh -- TODO determine what works best here
	redThresh = temp + 10; 
	
}

//processes the line in an image and sends it to calibrator
void ImageProcessor::processLine( char * data, int xRes, int yRes, double zWorld, double theta)
{
	
	//load in the image
	red = data;
	ImageProcessor::xRes = xRes;
	ImageProcessor::yRes = yRes;
		
	//information about line 
	int lineLen = 0;
	int count;
	int temp;
	
	//step size for sampling
	double step;
		
	//this will be used to pass on the information
	CalibrationUnit &calibrator = *CalibrationUnit::Instance();
	
	
	//first scan through image and take out anything that is lower
	//than the specified threshold and make the line one pixel wide
	for(int y=0; y< yRes; y++)
	{
		count = 0;
		temp = 0;
		
		for(int x=0; x<xRes; x++)
		{
			if( imageR(x,y) >= redThresh )
			{
				count++;
				temp += x;
			}
				setImageR(x,y, 0);
		}//end for x
		
		//make the line one pixel wide
		
		/*might want to look into something that
		 takes into account if the points are close
		 to one another -- this would help eliminate noise*/
	
		if(count >= 3)
		{
			count = temp/count;
			setImageR( ((int)temp/count), y, 255);
			lineLen++;
		}
	
	}//end for y

	//step size when we sample
	step = (double)lineLen/res;
	
	//reuse count as a counter
	count = 0;
	
	//loop through the image again
	for(int y=0; y < yRes; y++)
	{		
		for(int x=0; x < xRes; x++)
		{
			if( imageR(x,y) > 0 )
			{
				linePts[count][0] = x;
				linePts[count][1] = y;
				count++;
			}
		}//end for x
	}//end for y
	
	//now finally sample the points
	for(int i=0; i < res;i++)
	{
		samplePoints[i][0] = linePts[((int)floor(i*step))][0];
		samplePoints[i][1] = linePts[((int)floor(i*step))][1];
	}
	
	//send the data over to the CalibrationUnit
	calibrator.setPicRes(xRes, yRes);
	calibrator.lineToMesh(samplePoints, zWorld, theta);
	
}

