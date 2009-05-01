/*
 *  Mesh.cpp
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/12/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "Mesh.h"

//constuctor 
Mesh::Mesh() 
{
	draw = true;
	res = 10;
	pointBuffer.resize(res);
	nBuf = 0;
}

//alternate constructor
Mesh::Mesh(int res)
{
	draw = true;
	Mesh::res = res;
	pointBuffer.resize(res);
	nBuf = 0;
}

//destructor
Mesh::~Mesh()
{
	//TODO: figure out what to do here
	vertices.clear();
	triangles.clear();
	delete(name);
	//vertices.~vector();
	//triangles.~vector();
}

int Mesh::getRes()
{
	return res;
}

void Mesh::setRes(int resolution)
{
	res = resolution;
}

//returns the number of triangles in the mesh
int Mesh::getNumTris() 
{ 
	return triangles.size();
}

int Mesh::getNumVerts()
{
	return vertices.size();
}

float Mesh::getVert(int n, int xyz)
{
	return vertices[n][xyz];
}

//gets a specific vert of the mesh
float Mesh::getTriVert(int triInd, int vertInd, int xyz)
{	
	return vertices[ triangles[triInd][vertInd] ][xyz];
}

int Mesh::getTriInd(int tri, int i)
{
	return triangles[tri][i];
}

//give the mesh a name
void Mesh::giveName(int num)
{
	name = new char[9];
	sprintf(name, "Mesh_%d", num );
}

char* Mesh::getName()
{
	return name;
}

void Mesh::loadMesh( char* filename)
{
	
	//Mesh::addVert(1.0f, 2.3f, 4.5f);
	//Mesh::addTri(1,2,3);
	
	ifstream inFile;
	bool isVert = true;
	char in;
	int i;
	float x, y, z;
	int a, b, c;
	
	
	int count = 3;
	
    inFile.open(filename);
	printf("filename:  %s", filename);
    
	if (!inFile) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
    
	
	while(inFile >> in)
	{
		if( in == 'v')
			isVert = true;
		else isVert = false;
		
		if(isVert)
		{
			//vector<int> temp;
			i = vertices.size();
			//vertices.push_back( coord(3) );
			inFile >> x;
			inFile >> y;
			inFile >> z;
			Mesh::addVert(x, y, z);
			//printf("added vert at %d\n", i);
		}
		else
		{
			i = triangles.size();
			inFile >> a;
			inFile >> b;
			inFile >> c;
			Mesh::addTri(a-1, b-1, c-1);
			//printf("found a triangle\n");
			
		}
	}
	
	
    
    inFile.close();
	
	return;
}

void Mesh::addVert(float x, float y, float z)
{
	int i = Mesh::vertices.size();
	Mesh::vertices.push_back(coord(3));
	Mesh::vertices[i][0] = x;
	Mesh::vertices[i][1] = y;
	Mesh::vertices[i][2] = z;
	
	return;
}

void Mesh::addTri(int x, int y, int z)
{
	int i = Mesh::triangles.size();
	Mesh::triangles.push_back(ind(3));
	Mesh::triangles[i][0] = x;
	Mesh::triangles[i][1] = y;
	Mesh::triangles[i][2] = z;
	
	return;
	
	return;
}

void Mesh::addToBuf(double x, double y, double z)
{
	int nVert = vertices.size();
	
	if(nBuf < res)
	{
		printf("%i\n", nBuf);
		addVert(x,y,z);
		pointBuffer[nBuf] = nVert;
		nBuf++;		
	}
	
}

// take the point buffer points and add them to
// the overall mesh structure
void Mesh::compileMesh()
{
	int edgePtr=0, bufPtr = 0;
	int nV;
	
	//if no edge points exist then add the buffer to the edge points
	if(edgePts.size() == 0)
	{
		for(int i=0; i < res; i++)
		{	
			edgePts.push_back(pointBuffer[i]);
		}
	}//end if
	
	else
	{
		
		while( bufPtr != res-1 )
		{		/*
		 eP->*		*<-bP
		 
		 *		*
		 */
			addTri(edgePts[edgePtr++], edgePts[edgePtr], pointBuffer[bufPtr]);
			
			/*
			 *-------*<-bP
			 |   /
			 eP->*		*
			 */
			addTri(pointBuffer[bufPtr++],edgePts[edgePtr],pointBuffer[bufPtr]); 
			/*
			 *-------*
			 |   /   |
			 eP->*_______*<-bP
			 */
		}//end while
		
		//make the edge points the old buffer points
		for(int i=0; i< res; i++)
			edgePts[i] = pointBuffer[i];
		
	}//end else
	
	
	//reset the pointBuffer
	nBuf = 0;
}

void Mesh::printMeshData()
{
	int i;
	
	printf("----------Vertices----------\n\n");
	for(i=0; i<vertices.size(); i++)
		printf("%i --- x: %f y: %f z: %f\n", i,vertices[i][0], vertices[i][1], vertices[i][2]);
	
	printf("\n\n----------Triangles----------\n\n");
	for(i=0; i<triangles.size();i++)
		printf("%i --- i: %i ii: %i iii: %i\n", i, triangles[i][0],triangles[i][1],triangles[i][2]); 
	
}
