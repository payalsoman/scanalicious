/*
 *  MeshContainer.h
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/16/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "Mesh.h"
#include <vector>

class MeshContainer{

public:
	
	static MeshContainer * Instance();
		
	int getRes();
	
	//names and addes a mesh to the bag
	void addMesh();
	
	//returns the size of the bag
	int size();
	
	//gets a mesh at the specific index
	Mesh& getMesh(int n);
	
	//concatinates the names of the meshes for output
	char* concatNames();
	
	//gets the current mesh number
	int getCurrentMesh();
	
	//sets the current mesh number
	void setCurrentMesh(int n);
	
	//writes the meshes int the bag to files
	void writeMeshFiles();
	
	int nMembers();
	
	void point2Mesh(int n, Mesh &mesh);
	
protected:
	
	//constructors 
	MeshContainer();
	
private:
	
	static MeshContainer* pinstance;
	
	//holds the meshes
	Mesh *meshBag;
	int index;
	int length;
	char *meshData;
	int currentMesh;
	int res;
	
};