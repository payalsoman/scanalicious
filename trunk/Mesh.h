/*
 *  Mesh.h
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/12/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include <vector>
#include <cmath> 
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

typedef vector<float> coord;
typedef vector<int> ind;

class Mesh{
	
private:
	
	vector< coord >vertices;
	vector< ind > triangles;
	
	vector< int > pointBuffer;
	vector< int > edgePts;
	
	//ind of pointBuffer
	int nBuf;
	
	//name of the mesh
	char *name;
	
	//resolution of mesh
	int res;
	
	void addVert(float x, float y, float z);
	void addTri(int x, int y, int z);
	
public:
	
	Mesh();
	Mesh(int res);
	~Mesh();
	
	//returns the resolution of the mesh
	int getRes();
	
	void setRes(int resolution);
	
	//returns the number of triangles
	int getNumTris(); 
	
	//returns the number of vertices
	int getNumVerts();
	
	//tells whether or not it needs to be drawn
	bool draw; 
	
	//gets a specific vert of a triangle
	float getTriVert(int triInd, int vertInd, int xyz);
	
	//gets the indices of a triangle
	int getTriInd(int tri, int i);
	
	//get a specific vertex
	float getVert(int n, int xyz);
	
	//names the mesh by number
	void giveName(int num);
	
	//gets the name of the mesh
	char* getName();
	
	//loads a mesh from file
	void loadMesh( char* filename);
	
	//adds vertex to the point buffer 
	void addToBuf(double x, double y, double z);
	
	//compiles the mesh
	void compileMesh();
		
	void printMeshData();
};