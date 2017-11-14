#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glui\glui.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "shader_install.h"
#include "Loading_func.h"
#include "Bind_Data.h"

using namespace std;
using glm::vec3;
using glm::mat4;
// ============================= //
// window size
int Win_w, Win_h;
float camera_fov = 45.0;

int  mainWindowID;
GLint programID;
GLint Skybox_programID;

GLuint TextureEarth;
GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint skyboxTexture;

extern GLuint earthVao;
extern int drawEarthSize;
// view matrix
glm::mat4 common_viewM;
glm::mat4 common_projection;

float earth_innRot_Degree = 0.0f;

// ============================= //
const float M_PI = 3.14159265;
float radius = 30.0f;
float initViewHorizontal = -90.0f;
float initViewVertical = -90.0f;

float cameraX = 0;
float cameraY = 0;
float cameraZ = radius;

float viewRotateDegree[3] = { 0.0f, 0.0f, 0.0f };

float a_brightness = 1.0f;
float d_brightness = 0.0f;
float s_brightness = 0.6f;

void Mouse_Wheel_Func(int button, int state, int x, int y)
{
	if ((button == 3) || (button == 4))
	{
		if (state == GLUT_UP) return;
		if (button == 3)
		{
			radius -= 1.0f;
			cameraX = radius* cos(glm::radians(initViewHorizontal + viewRotateDegree[1]))*sin(glm::radians(initViewVertical + viewRotateDegree[0]));
			cameraY = radius* cos(glm::radians(initViewVertical + viewRotateDegree[0]));
			cameraZ = radius* sin(glm::radians(initViewHorizontal + viewRotateDegree[1]))*sin(glm::radians(initViewVertical + viewRotateDegree[0]));
		}
		else
		{
			radius += 1.0f;
			cameraX = radius* cos(glm::radians(initViewHorizontal + viewRotateDegree[1]))*sin(glm::radians(initViewVertical + viewRotateDegree[0]));
			cameraY = radius* cos(glm::radians(initViewVertical + viewRotateDegree[0]));
			cameraZ = radius* sin(glm::radians(initViewHorizontal + viewRotateDegree[1]))*sin(glm::radians(initViewVertical + viewRotateDegree[0]));
		}
	}
}

void LoadAllTextures()
{
	TextureEarth = loadBMP2Texture("texture/earth.bmp");
}

GLuint loadBMP_Skybox(const char * imagepath, unsigned char * data) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);
	
}

//load skybox texture; for sea_skybox only xd
GLuint loadCubemap(vector,<const GLchar*> faces){
	int width=2048, height=2048;
	unsigned char* image;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i=0; i<faces.size(); i++){
		loadBMP_Skybox(faces[i], image);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, image);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP,0);	

	return textureID;
}

void sendDataToOpenGL()
{
	//Load objects and bind to VAO & VBO
	
	//Skybox cubemap
	GLfloat skyboxVertices[] = {
		 -100.0f,-100.0f,-100.0f, // triangle 1 : begin
    -100.0f,-100.0f, 100.0f,
    -100.0f, 100.0f, 100.0f, // triangle 1 : end
    100.0f, 100.0f,-100.0f, // triangle 2 : begin
    -100.0f,-100.0f,-100.0f,
    -100.0f, 100.0f,-100.0f, // triangle 2 : end
    100.0f,-100.0f, 100.0f,
    -100.0f,-100.0f,-100.0f,
    100.0f,-100.0f,-100.0f,
    100.0f, 100.0f,-100.0f,
    100.0f,-100.0f,-100.0f,
    -100.0f,-100.0f,-100.0f,
    -100.0f,-100.0f,-100.0f,
    -100.0f, 100.0f, 100.0f,
    -100.0f, 100.0f,-100.0f,
    100.0f,-100.0f, 100.0f,
    -100.0f,-100.0f, 100.0f,
    -100.0f,-100.0f,-100.0f,
    -100.0f, 100.0f, 100.0f,
    -100.0f,-100.0f, 100.0f,
    100.0f,-100.0f, 100.0f,
    100.0f, 100.0f, 100.0f,
    100.0f,-100.0f,-100.0f,
    100.0f, 100.0f,-100.0f,
    100.0f,-100.0f,-100.0f,
    100.0f, 100.0f, 100.0f,
    100.0f,-100.0f, 100.0f,
    100.0f, 100.0f, 100.0f,
    100.0f, 100.0f,-100.0f,
    -100.0f, 100.0f,-100.0f,
    100.0f, 100.0f, 100.0f,
    -100.0f, 100.0f,-100.0f,
    -100.0f, 100.0f, 100.0f,
    100.0f, 100.0f, 100.0f,
    -100.0f, 100.0f, 100.0f,
    100.0f,-100.0f, 100.0f
	}
	
	
	//Setup skybox 
	glGenVertexArrays(1, &skyboxVAO);
	glBenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnabaleVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
	
	vector<const GLchar*> sky_faces;
	sky_faces.push_back("texture/sea_skybox/right.bmp");
	sky_faces.push_back("texture/sea_skybox/left.bmp");
	sky_faces.push_back("texture/sea_skybox/bottom.bmp");
	sky_faces.push_back("texture/sea_skybox/top.bmp");
	sky_faces.push_back("texture/sea_skybox/back.bmp");
	sky_faces.push_back("texture/sea_skybox/front.bmp");
	skyboxTexture = loadBMP_Skybox(sky_faces);
	
	
	bindEarth("model_obj/planet.obj");
	// load all textures
	LoadAllTextures();
}

float zLightPos = 0.0f;
float yLightPos = 20.0f;
void set_lighting()
{
	glUseProgram(programID);

	// ambient
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	glm::vec3 ambientLight(a_brightness, a_brightness, a_brightness);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);
	// diffusion
	GLint kd = glGetUniformLocation(programID, "coefficient_d");
	glm::vec3 vec_kd(d_brightness, d_brightness, d_brightness);
	glUniform3fv(kd, 1, &vec_kd[0]);
	// specular
	GLint ks = glGetUniformLocation(programID, "coefficient_s");
	glm::vec3 vec_ks(s_brightness, s_brightness, s_brightness);
	glUniform3fv(ks, 1, &vec_ks[0]);
	// eye position
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(cameraX, cameraY, cameraZ);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);
	// light position
	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	glm::vec3 lightPosition(0.0, yLightPos, zLightPos);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);

	// light color
	GLint lightColorUniformLocation = glGetUniformLocation(programID, "lightColor");
	glm::vec4 lightColor(1.0, 1.0, 1.0, 1.0);
	glUniform4fv(lightColorUniformLocation, 1, &lightColor[0]);
}

void drawEarth(void)
{
	//earth
	GLfloat scale_fact = 3.0f;

	glUseProgram(programID);

	glBindVertexArray(earthVao);
	glm::mat4 scale_M = glm::scale(glm::mat4(1.0f), glm::vec3(scale_fact));
	glm::mat4 rot_M = glm::rotate(glm::mat4(1.0f), glm::radians(earth_innRot_Degree), glm::vec3(0, 1, 0));
	glm::mat4 trans_M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 Model = trans_M * rot_M * scale_M;

	GLint M_ID = glGetUniformLocation(programID, "MM");
	glUniformMatrix4fv(M_ID, 1, GL_FALSE, &Model[0][0]);
	GLint V_ID = glGetUniformLocation(programID, "VM");
	glUniformMatrix4fv(V_ID, 1, GL_FALSE, &common_viewM[0][0]);
	GLint P_ID = glGetUniformLocation(programID, "PM");
	glUniformMatrix4fv(P_ID, 1, GL_FALSE, &common_projection[0][0]);

	// texture
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureEarth);
	glUniform1i(TextureID, 0);
	glActiveTexture(GL_TEXTURE1);

	glDrawArrays(GL_TRIANGLES, 0, drawEarthSize);
}

void paintGL(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// ================================ //
	// view matrix
	common_viewM = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	// projection matrix
	common_projection = glm::perspective(camera_fov, 1.0f, 0.1f, 200.0f);
	
	//=== draw ===//
	
	//draw Skybox
	glDepthMask(GL_FALSE);
	glUseProgram(Skybox_programID);
	
	GLuint skb_ModelUniformLocation = glGetUniformLocation(Skybox_programID, "M");
	glm::mat4 Skb_ModelMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(Skb_ModelUniformLocation, 1, GL_FALSE, &Skb_ModelMatrix[0][0]);
	//tbc
	
	// set lighting parameters
	set_lighting();
	// draw earth
	drawEarth();

	glutSwapBuffers();
	glutPostRedisplay();
}

void Shaders_Installer()
{
	programID = installShaders("shader/Common.vs", "shader/Common.frag");

	if (!checkProgramStatus(programID))
		return;
}

void initializedGL(void)
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// install all shaders
	Shaders_Installer();
	// load required obj and textures
	sendDataToOpenGL();
}

void timerFunction(int id)
{
	earth_innRot_Degree += 0.3;
	
	glutPostRedisplay();
	glutTimerFunc(700.0f / 60.0f, timerFunction, 1);
}

int main(int argc, char *argv[])
{
	/*Initialization of GLUT library*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	int height = GetSystemMetrics(SM_CYSCREEN)*0.8;
	glutInitWindowSize(height, height);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	mainWindowID = glutCreateWindow("i-Navigation");

	// initialize openGL
	initializedGL();
	// draw
	glutDisplayFunc(paintGL);

	glutMouseFunc(Mouse_Wheel_Func);

	glutTimerFunc(700.0f / 60.0f, timerFunction, 1);

	/*Enter the GLUT event processing loop which never returns.*/
	glutMainLoop();

	return 0;
}