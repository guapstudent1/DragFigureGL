#pragma once

struct SCENE_ELEMENT {
	/*GLuint type; // �� �����������  */
	GLfloat xpos;
	GLfloat ypos;
};

GLvoid BuildLists();

void DrawCircle(GLfloat r);
void FillCircle(GLfloat r);

void DrawPoint();

void InitFigurePos(SCENE_ELEMENT [], int);


