#ifndef __MESH_H__
#define __MESH_H__

//	STD include
#include <vector>
#include <string>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;

//	GLEW include
#include <GL\glew.h>

//	GLM include
#include <glm\glm.hpp>

//	ASSIMP include
#include <assimp\postprocess.h>

//	MY include
#include "shader.h"

enum TextureType
{
	TEXTURE_DIFFUSE,
	TEXTURE_SPECULAR
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

typedef unsigned int Indice;

struct Texture
{
	GLuint id;
	TextureType type;
	aiString path;
};

class Mesh
{
private:
	vector<Vertex> vertices;
	vector<Indice> indices;
	vector<Texture> textures;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

public:
	Mesh(const vector<Vertex> & vertices, const vector<GLuint> & indices, const vector<Texture> & textures);
	void Draw(const Shader & shader);
	vector<Vertex> getVertices();
	vector<Indice> getIndices();
	vector<Texture> getTextures();

private:
	void setupMesh();
};

Mesh::Mesh(const vector<Vertex> & vertices, const vector<GLuint> & indices, const vector<Texture> & textures) :
	vertices(vertices), indices(indices), textures(textures)
{
	this->setupMesh();
}

void Mesh::Draw(const Shader & shader)
{
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;

	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		stringstream ss;
		string number;
		TextureType type = this->textures[i].type;

		if (type == TEXTURE_DIFFUSE)
		{
			ss << "material.diffuse";
			ss << diffuseNr++;
		}
		else if (type == TEXTURE_SPECULAR)
		{
			ss << "material.specular";
			ss << specularNr++;
		}

		glUniform1f(glGetUniformLocation(shader.program, ss.str().c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	glUniform1f(glGetUniformLocation(shader.program, "material.shininess"), 32);

	glBindVertexArray(this->VAO);

	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	for (int i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

vector<Vertex> Mesh::getVertices()
{
	return this->vertices;
}

vector<Indice> Mesh::getIndices()
{
	return this->indices;
}

vector<Texture> Mesh::getTextures()
{
	return this->textures;
}
void Mesh::setupMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), 
				 &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
				 &this->indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, normal) );

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, texCoord) );

	glBindVertexArray(0);
}

#endif//__MESH_H__