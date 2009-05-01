/*
 *  MeshContainer.cpp
 *  Cocoa OpenGL
 *
 *  Created by Ryan Dunn on 3/16/09.
 *  Copyright 2009 Cornell University. All rights reserved.
 *
 */

#include "MeshContainer.h"


MeshContainer* MeshContainer::pinstance = 0;

MeshContainer* MeshContainer::Instance()
{
	if (pinstance == 0) //first call?
	{
		pinstance = new MeshContainer; //create sole instance
	}
	
	return pinstance;	//address of sole instance
}

MeshContainer::MeshContainer()
{
	index = 0;
	length = 5;
	res = 10;
	meshBag = new Mesh[length];
}

//adds a mesh to the bag of meshes
void MeshContainer::addMesh()
{
	
	if(index < length)
	{
		
	    printf("creating mesh\n");
		//names the mesh according to the number of the mesh
		meshBag[index].giveName( index );
		meshBag[index].setRes(res);
		
		printf("named mesh\n");
		
		index++;
		printf("index updated\n");
	}
	return;
}

int MeshContainer::getRes()
{
	return res;
}

//returns the size of the bag
int MeshContainer::size()
{
	return length;
}

//returns the mesh at the requested index
Mesh& MeshContainer::getMesh(int n)
{
	if(n > length-1) 
		n = length-1;
	
	return meshBag[n];
}

//other mesh return method
void MeshContainer::point2Mesh(int n, Mesh &mesh)
{
	if(n > length-1) 
		n = length-1;
	
	mesh = meshBag[n];
	return;
}

//prints out mesh statistics
char* MeshContainer::concatNames()
{ 
	
	delete meshData;
	
	char *meshData;
	
	if(index == 0)
	{
		meshData = new char[10];
		meshData = "No Loaded Meshes";
	}
	
	else if(index == 1)
	{
		meshData = new char[50];
		sprintf(meshData, "%s  %dv  %dt", meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris());
	}
	
	else if(index == 2)
	{	
		meshData = new char[50];
		sprintf(meshData, "%s  %dv  %dt\n%s  %dv  %dt", 
				meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris(),
				meshBag[1].getName(), meshBag[1].getNumVerts(), meshBag[1].getNumTris());
	}	
	else if(index == 3)
	{	
		meshData = new char[50];
		sprintf(meshData, "%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt", 
				meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris(),
				meshBag[1].getName(), meshBag[1].getNumVerts(), meshBag[1].getNumTris(),
				meshBag[2].getName(), meshBag[2].getNumVerts(), meshBag[2].getNumTris());
	}	
	else if(index == 4)
	{	
		meshData = new char[100];
		sprintf(meshData, "%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt",
				meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris(),
				meshBag[1].getName(), meshBag[1].getNumVerts(), meshBag[1].getNumTris(),
				meshBag[2].getName(), meshBag[2].getNumVerts(), meshBag[2].getNumTris(),
				meshBag[3].getName(), meshBag[3].getNumVerts(), meshBag[3].getNumTris());
	}	
	
	else if(index == 5)
	{	
		meshData = new char[100];
		sprintf(meshData, "%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt",
				meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris(),
				meshBag[1].getName(), meshBag[1].getNumVerts(), meshBag[1].getNumTris(),
				meshBag[2].getName(), meshBag[2].getNumVerts(), meshBag[2].getNumTris(),
				meshBag[3].getName(), meshBag[3].getNumVerts(), meshBag[3].getNumTris(),
				meshBag[4].getName(), meshBag[4].getNumVerts(), meshBag[4].getNumTris());
	}	
	
	else 
	{
		meshData = new char[100];
		sprintf(meshData, "%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt\n%s  %dv  %dt ALL FULL",
				meshBag[0].getName(), meshBag[0].getNumVerts(), meshBag[0].getNumTris(),
				meshBag[1].getName(), meshBag[1].getNumVerts(), meshBag[1].getNumTris(),
				meshBag[2].getName(), meshBag[2].getNumVerts(), meshBag[2].getNumTris(),
				meshBag[3].getName(), meshBag[3].getNumVerts(), meshBag[3].getNumTris(),
				meshBag[4].getName(), meshBag[4].getNumVerts(), meshBag[4].getNumTris());
	}
	
	return meshData;
	
}

int MeshContainer::getCurrentMesh()
{
	return currentMesh;
}

void MeshContainer::setCurrentMesh(int n)
{
	if(n < length && n >= 0)
		currentMesh = n;
}

void MeshContainer::writeMeshFiles()
{
	FILE *fp;
	char *filename;
	int j, i;
	
	for(i=0; i <= index; i++)
	{
		
		filename = new char[20];
		sprintf(filename, "%s.%s", meshBag[i].getName(), "obj");
		
		fp = fopen( filename, "w+"); 
		
		//just to make sure file is opened
		if( NULL == fp ) { 
			printf("cannot open file\n"); 
			return; 
		} 
		
		//prints out the vertices
		for(j=0; j < meshBag[i].getNumVerts(); j++)
		{
			fprintf(fp, "%16.8f %16.8f %16.8f\n", meshBag[i].getVert(j, 0), meshBag[i].getVert(j, 1), meshBag[i].getVert(j, 2));		
		}// end for j
		
		//blank spaces
		fprintf(fp, "\n\n");
		
		//prints out the triangles
		for(j=0; j < meshBag[i].getNumTris(); j++)
		{
			fprintf(fp, "%16d %16d %16d\n", meshBag[i].getTriInd(j, 0), meshBag[i].getTriInd(j, 1), meshBag[i].getTriInd(j, 2)); 		
		}
		
		fclose(fp);
		
		delete filename;
		delete fp;
		
	}// end for i
	
	
	
	return;
}

int MeshContainer::nMembers()
{
	return index;
}

