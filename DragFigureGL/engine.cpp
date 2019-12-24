#include <windows.h>		// Header File For Windows
#include <tchar.h>
#include <stdio.h>			// Header File For Standard Input/Output
#include <math.h>			// Header File For The Math Library
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

#include "engine.h"

extern GLfloat xyzoom;
extern GLuint memlist;

void InitFigurePos(SCENE_ELEMENT sc[], int ncount)
{
	float r = 0.9f * 4;
	float xstart = -r;
	float ystart = -r;

	for (int i = 0; i < ncount; i++) {
		float theta = 2.0f * 3.1415926f * i / ncount;
		
		sc[i].xpos = xstart + r * cosf(theta);
		sc[i].ypos = ystart + r * sinf(theta);
	}
	
}

GLvoid BuildLists()     //создаем список отображени€
{
	memlist = glGenLists(1);      //создаем два списка

	glNewList(memlist, GL_COMPILE);      // Ќовый откомпилированный список отображени€ box
	
	FillCircle(0.5f);
	DrawCircle(0.9f);
	DrawCircle(1.4f);

	glEndList();	
}
void DrawCircle(GLfloat r) 
{
	int num_segments = 100;
	glColor3f(0.1f, 0.1f, 0.4f);
	glBegin(GL_LINE_LOOP);	
	for (int i = 0; i < num_segments; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);//get the current angle 
		float x = r * cosf(theta);//calculate the x component 
		float y = r * sinf(theta);//calculate the y component 
		glVertex2f(x, y);//output vertex 
	}
	glEnd();
}

void FillCircle(GLfloat r)
{
	int num_segments = 100;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glBegin(GL_POLYGON);
	
	for (int i = 0; i < num_segments; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);//get the current angle 
		float x = r * cosf(theta);//calculate the x component 
		float y = r * sinf(theta);//calculate the y component 
		glVertex2f(x, y);//output vertex 
	}
	glEnd();
}


void DrawPoint()
{
	float r = 0.04f / xyzoom;
	int num_segments = 50;

	glBegin(GL_POLYGON);
	for (int i = 0; i < num_segments; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);//get the current angle 
		float x = r * cosf(theta);//calculate the x component 
		float y = r * sinf(theta);//calculate the y component 
		glVertex2f(x, y);//output vertex 
	}
	glEnd();
}
