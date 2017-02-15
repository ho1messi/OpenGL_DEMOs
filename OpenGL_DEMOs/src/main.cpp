#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, ".\\third_party\\lib\\glfw3.lib")
#pragma comment(lib, ".\\third_party\\lib\\glew32s.lib")
#pragma comment(lib, ".\\third_party\\lib\\ANN.lib")

//	STD		include
#include <iostream>
#include <string>
#include <conio.h>

//	GLEW	include
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "..\third_party\include\GL\glew.h"

//	GLFW	include
#include "..\third_party\include\GLFW\glfw3.h"

//	GLM		include
#include "..\third_party\include\glm\glm.hpp"
#include "..\third_party\include\glm\gtc\matrix_transform.hpp"
#include "..\third_party\include\glm\gtc\type_ptr.hpp"

//	MY		include
#include "..\include\shader.h"
#include "..\include\HES_Mesh.h"
#include "..\include\HES_MeshSubdivition.h"
#include "..\include\MC_Mesh.h"
#include "..\include\PC_Mesh.h"
#include "..\include\PC_Normal.h"

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

bool menu();
bool childMenu1();
bool childMenu2();
void drawInit();
void draw();
void deleteMeshs();

void multiply(float & x, float & y, float & z, float p);
inline float f1(float x, float y, float z);
inline float f2(float x, float y, float z);
inline float f3(float x, float y, float z);
inline float f4(float x, float y, float z);
float(*f) (float, float, float);

const int NUM_OF_CUBES = 50;

int screenWidth = 1366;
int screenHeight = 768;
bool keyMap[1024];
bool drawLineFlag = true;
bool drawFaceFlag = true;
bool drawNormalFlag = false;
char menuOption = 0;
string fileName;

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
PC_Mesh<NUM_OF_CUBES> *PCmesh = NULL;


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	while (menu())
	{
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

		glfwDestroyWindow(window);
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
	//if (key == GLFW_KEY_N && action == GLFW_PRESS)
	//	drawNormalFlag = !drawNormalFlag;

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		HESmesh->writeToFile("resource\\vertices_out.txt");
		cout << "Writting vertices to file done" << endl;
	}
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

bool menu()
{
	bool flag = false;

	while (!flag)
	{
		cout << "===========================" << endl;
		cout << "=        Main Menu        =" << endl;
		cout << "===========================" << endl;
		cout << "请选择需要的功能：" << endl;
		cout << " 1. Marching Cubes 演示" << endl;
		cout << " 2. 曲面重建演示" << endl;
		cout << " ESC. 退出" << endl;
		cout << endl;

		menuOption = _getch();

		switch (menuOption)
		{
		case '1':
			flag = childMenu1();
			break;
		case '2':
			flag = childMenu2();
			break;
		default :
			return false;
		}
	}

	return true;
}

bool childMenu1()
{
	cout << "===========================" << endl;
	cout << "=     Marching  Cubes     =" << endl;
	cout << "===========================" << endl;
	cout << "请选择需要绘制的模型：" << endl;
	cout << " 1. 球形\tf = x^2 + y^2 + z^2 - 1" << endl;
	cout << " 2. 蘑菇形\tf = (x^2 + y^2 - 3)^3 + (z^3 - 2)^2" << endl;
	cout << " 3. 心形\tf = (x^2 + 2.25*y^2 + z^2 - 1)^3 - x^2*z^3 - 0.1125*y^2*z^3" << endl;
	cout << " ESC. 返回上级菜单" << endl;
	cout << endl;

	char childOption = _getch();
	switch (childOption)
	{
	case '1':
		f = &f1;
		break;
	case '2':
		f = &f4;
		break;
	case '3':
		f = &f3;
		break;
	default:
		return false;
	}

	return true;
}

bool childMenu2()
{
	cout << "===========================" << endl;
	cout << "=         曲面重建         =" << endl;
	cout << "===========================" << endl;
	cout << "请选择需要重建的模型：" << endl;
	cout << " 1. 球形" << endl;
	cout << " 2. 蘑菇形" << endl;
	cout << " 3. 心形" << endl;
	cout << " ESC. 返回上级菜单" << endl;
	cout << endl;

	char childOption = _getch();
	string s;

	switch (childOption)
	{
	case '1':
		s = "c";
		break;
	case '2':
		s = "m";
		break;
	case '3':
		s = "x";
		break;
	default:
		return false;
	}

	fileName = "resource\\vertices_" + s + ".txt";

	return true;
}

void drawInit()
{	
	switch (menuOption)
	{
	case '1':
		MCmesh = new MC_Mesh<NUM_OF_CUBES>(f, 1.0f);
		break;
	case '2':
		MCmesh = new PC_Mesh<NUM_OF_CUBES>(fileName, 1.0f);
		break;
	default:
		exit(1);
	}

	position = vec3(0.0f, 0.0f, 0.0f);
	scaleSize = vec3(5.0f, 5.0f, 5.0f);
	rotateX = rotateY = rotateZ = 0.0f;

	HESmesh = MCmesh->getMesh();
	//HESmesh = new HES_Mesh();
	//HESmesh->readFromObj("resource\\mannequin.obj");
	HESmeshSubdivition = new HES_MeshSubdivition(HESmesh);
	HESmesh->setupMesh();
	
	shader = new Shader("resource\\vShader.glsl", "resource\\fShader.glsl");
}

void draw()
{
	if (drawFaceFlag)
	{
		shader->use();

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
		shader->use();

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
	if (PCmesh)
		delete PCmesh;

	if (HESmeshSubdivition)
		delete HESmeshSubdivition;

	if (shader)
		delete shader;
}

float f1(float x, float y, float z)
{
	multiply(x, y, z, 1.5f);
	return x * x + y * y + z * z - 1.0f;
}

float f2(float x, float y, float z)
{
	multiply(x, y, z, 10.0f);
	return x * x + y * y - z * z - 1.0f;
}

float f3(float x, float y, float z)
{
	multiply(x, y, z, 1.3f);
	float temp = powf(x, 2.0f) + 2.25f * powf(z, 2.0f) + powf(y, 2.0f) - 1;
	return powf(temp, 3.0f) - powf(x, 2) * powf(y, 3.0f)
		- 0.1125f * powf(z, 2) * powf(y, 3.0f);
}

float f4(float x, float y, float z)
{
	multiply(x, y, z, 2.0f);
	return powf(x * x + z * z - 3.0f, 3.0f)
		+ powf(y * y * y - 2.0f, 2.0f);
}

void multiply(float & x, float & y, float & z, float p)
{
	x *= p;
	y *= p;
	z *= p;
}