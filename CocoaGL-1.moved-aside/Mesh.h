/*
 *  Mesh.h
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/12/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include <vector>
//#include <cstdio> 
//#include <cstdlib> 
#include <cmath> 

using namespace std;

class Mesh{

private:
	vector<float[3]> vertices;
	vector<int[3]> triangles;
	
	int numVerts;
	int numTris;
	
public:
	
	Mesh();
	~Mesh();
	
	//returns the number of triangles
	int getNumTris(); 
	
	//adds vertices to the edge of the mesh and
	//into the mesh structure
	void addVertices(vector<float[3]> vertList);
	
	//putputs a specific vert of a triangle
	float getTriVert(int triInd, int vertInd, int xyz);
};