#pragma once

#include "Includes\GL\glew.h"
#include "Includes\GLFW\glfw3.h"
#include "Vector3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>

//namespaces
using namespace std;

//files
const char* terrainHFile = "Textures/test07.rsc";
const char* terrainNFile = "Textures/test07n.rsc";

struct MouseData
{
	float xPos;
	float yPos;

	MouseData(float xPos, float yPos) : xPos(xPos), yPos(yPos) {}
};
struct CameraData
{
	float xAng;
	float yAng;

	CameraData(float xAng, float yAng) : xAng(xAng), yAng(yAng) {}
};
struct Texture
{
	int Width;
	int Heigth;
	unsigned char *Data;

	Texture(int Width, int Heigth) : Width(Width), Heigth(Heigth) {}
};
struct PlayerData
{
	Vector3 Pos;
	Vector3 viewDir;
	Vector3 viewRight;
	Vector3 viewUp;

	PlayerData(Vector3 Pos, Vector3 viewDir, Vector3 viewRight, Vector3 viewUp) : Pos(Pos), viewDir(viewDir), viewRight(viewRight), viewUp(viewUp) {}
};

//function definitions
void InitGL(GLFWwindow *window);
void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void GetFPS(GLFWwindow *window);
string readShader(char *filePath);
int ReadAllBytes(char const* FileName, vector<char>& File);
int WriteAllBytes(char const* FileName, vector<char>& File);
int LoadTexFromFile(char const* FileName, Texture& Tex);
void SetTexture(const char* FileName, const char *TexName, GLint WarpMode, GLint InterplMode, int TextureChannel, GLenum ColorFormat, GLint BitFormat);
void SetPlayerViewDir(CameraData camData, PlayerData& playerData);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MovePlayer(PlayerData& playerData);

//small voids
float DegToRad(float deg)
{
	return deg * 0.0174532925199;
}