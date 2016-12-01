//	GLEW	include
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL\glew.h>

//	GLFW	include
#include <GLFW\glfw3.h>

//	GLM		include
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//	MY		include
#include "shader.h"
#include "HES_Mesh.h"
#include "HES_MeshSubdivition.h"
#include "MC_Mesh.h"
#include "PC_Mesh.h"

//	STD		include
#include <iostream>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::translate;
using glm::rotate;
using glm::scale;

using std::cout;
using std::endl;

void key_callback(GLFWwindow *window, int key, int scanCode, int action, int mode);
void mouse_callback(GLFWwindow *window, int key, int action, int mode);
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);
void cursorPos_callback(GLFWwindow *window, double xOffset, double yOffset);

void drawInit();
void draw();
void deleteMeshs();

const int NUM_OF_CUBES = 20;

int screenWidth = 1366;
int screenHeight = 768;
bool keyMap[1024];
bool drawLineFlag = true;
bool drawFaceFlag = true;

vec3 position(0.0f, 0.0f, 0.0f);
vec3 scaleSize(3.0f, 3.0f, 3.0f);
float rotateX = 0, rotateY = 0, rotateZ = 0;
float rotateSpeed = 0.5f;
float scaleBigger = 1.05f;
float scaleSmaller = 1.0f / scaleBigger;
float moveSpeed = 0.02f;

float vertices[] = {
	 1.0f,  3.0f,  0.0f,
	 3.0f,  3.0f,  0.0f,
	 3.0f,  1.0f,  0.0f,
	 1.0f,  1.0f,  0.0f
};

float vertices1[] = {
	1.4375f,	1.9375f,	0.0f,
	2.3125f,	2.3125f,	0.0f,
	1.9375f,	1.4375f,	0.0f,
	1.3125f,	1.3125f,	0.0
};

unsigned int indices[] = {
	0, 1, 2, 3, 0
};

GLuint VAO[2];
GLuint VBO[2];
GLuint EBO[2];
Shader *shader;
HES_Mesh *HESmesh;
HES_MeshSubdivition *HESmeshSubdivition;
MC_Mesh_Base<NUM_OF_CUBES> *MCmesh = NULL;

float f(float x, float y, float z)
{
	multiply(x, y, z, 2.0f);
	return powf(x * x + y * y - 3.0f, 3.0f)
		+ powf(z * z * z - 2.0f, 2.0f);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL DEMO", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursorPos_callback);

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	drawInit();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw();

		glfwSwapBuffers(window);
	}

	deleteMeshs();

	return 0;
}

void key_callback(GLFWwindow *window, int key, int scanCode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		HESmeshSubdivition->DooSabinSubdivition();
		HESmesh = HESmeshSubdivition->getCurrentMesh();
		HESmesh->setupMesh();
	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		HESmesh = HESmeshSubdivition->lastMesh();
		HESmesh->setupMesh();
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		HESmesh = HESmeshSubdivition->nextMesh();
		HESmesh->setupMesh();
	}

	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		drawLineFlag = !drawLineFlag;
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		drawFaceFlag = !drawFaceFlag;

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		HESmesh->writeToFile("Resource\\vertices.txt");
}

void mouse_callback(GLFWwindow *window, int key, int action, int mode)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
			keyMap[GLFW_MOUSE_BUTTON_LEFT] = true;
		else if (action == GLFW_RELEASE)
			keyMap[GLFW_MOUSE_BUTTON_LEFT] = false;
	}

	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
			keyMap[GLFW_MOUSE_BUTTON_RIGHT] = true;
		else if (action == GLFW_RELEASE)
			keyMap[GLFW_MOUSE_BUTTON_RIGHT] = false;
	}
}

void cursorPos_callback(GLFWwindow *window, double xOffset, double yOffset)
{
	static double lastX = xOffset;
	static double lastY = yOffset;

	int mouseFlag = static_cast<int>(keyMap[GLFW_MOUSE_BUTTON_LEFT]) * 2 +
					static_cast<int>(keyMap[GLFW_MOUSE_BUTTON_RIGHT]);
	switch (mouseFlag)
	{
	case 1:	//	Right button
		position.x += (xOffset - lastX) * moveSpeed;
		position.y -= (yOffset - lastY) * moveSpeed;
		break;

	case 2:	//	Left button
		rotateY += (xOffset - lastX) * rotateSpeed;
		rotateX += (yOffset - lastY) * rotateSpeed;
		break;

	case 3:	//	Left and Right button
		//rotateY += yOffset - lastY;
		break;
	}

	lastX = xOffset;
	lastY = yOffset;
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset)
{
	if (yOffset > 0)
	{
		scaleSize.x *= scaleBigger;
		scaleSize.y *= scaleBigger;
		scaleSize.z *= scaleBigger;
	}
	else if (yOffset < 0)
	{
		scaleSize.x *= scaleSmaller;
		scaleSize.y *= scaleSmaller;
		scaleSize.z *= scaleSmaller;
	}
}

void drawInit()
{	
	//MCmesh = new MC_Mesh<NUM_OF_CUBES>(&f);
	MCmesh = new PC_Mesh<NUM_OF_CUBES>("Resource\\vertices1.txt");
	HESmesh = MCmesh->getMesh();
	HESmeshSubdivition = new HES_MeshSubdivition(HESmesh);
	//HESmesh->readFromObj("Resource\\mannequin.obj");
	HESmesh->setupMesh();

	
	shader = new Shader("Resource\\vShader.glsl", "Resource\\fShader.glsl");
}

void draw()
{
	shader->use();

	if (drawFaceFlag)
	{
		mat4 model;
		model = translate(model, position);
		model = scale(model, scaleSize);
		model = rotate(model, rotateX, vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, rotateY, vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, rotateZ, vec3(0.0f, 0.0f, 1.0f));

		mat4 view = glm::lookAt(vec3(0.0f, 0.0f, 20.0f),
			vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));

		mat4 projection = glm::perspective(45.0f, (float)screenWidth / (float)screenHeight, 0.1f, 1000.f);

		glUniformMatrix4fv(glGetUniformLocation(shader->program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader->program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader->program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform4f(glGetUniformLocation(shader->program, "material.color"), 1.0f, 1.0f, 0.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader->program, "material.shiness"), 32.0f);

		glUniform4f(glGetUniformLocation(shader->program, "light.position"), -3.0f, 3.0f, -10.0f, 1.0f);
		glUniform4f(glGetUniformLocation(shader->program, "light.ambient"),  0.2f, 0.2f, 0.2f, 1.0f);
		glUniform4f(glGetUniformLocation(shader->program, "light.diffuse"),  0.7f, 0.7f, 0.7f, 1.0f);

		glCullFace(GL_BACK);
		HESmesh->drawMeshFace();

		glUniform4f(glGetUniformLocation(shader->program, "material.color"), 0.0f, 0.5f, 0.0f, 1.0f);
		glCullFace(GL_FRONT);
		HESmesh->drawMeshFace();
	}

	if (drawLineFlag)
	{
		mat4 modelL1;
		modelL1 = translate(modelL1, position);
		modelL1 = scale(modelL1, scaleSize * 1.002f);
		modelL1 = rotate(modelL1, rotateX, vec3(1.0f, 0.0f, 0.0f));
		modelL1 = rotate(modelL1, rotateY, vec3(0.0f, 1.0f, 0.0f));
		modelL1 = rotate(modelL1, rotateZ, vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(glGetUniformLocation(shader->program, "model"), 1, GL_FALSE, glm::value_ptr(modelL1));

		glUniform4f(glGetUniformLocation(shader->program, "material.color"), 0.0f, 0.0f, 0.0f, 1.0f);

		HESmesh->drawMeshLine();


		mat4 modelL2;
		modelL2 = translate(modelL2, position);
		modelL2 = scale(modelL2, scaleSize * 0.998f);
		modelL2 = rotate(modelL2, rotateX, vec3(1.0f, 0.0f, 0.0f));
		modelL2 = rotate(modelL2, rotateY, vec3(0.0f, 1.0f, 0.0f));
		modelL2 = rotate(modelL2, rotateZ, vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(glGetUniformLocation(shader->program, "model"), 1, GL_FALSE, glm::value_ptr(modelL2));

		HESmesh->drawMeshLine();
	}

	glBindVertexArray(0);
}

void deleteMeshs()
{
	if (MCmesh)
		delete MCmesh;
	if (HESmeshSubdivition)
		delete HESmeshSubdivition;

	if (shader)
		delete shader;
}