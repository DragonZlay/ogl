// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace std;
using namespace glm;
// Include AntTweakBar
//#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float OriginalColor[4];
	float Normal[3];
	void SetPosition(float* coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float* color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
		OriginalColor[0] = Color[0];
		OriginalColor[1] = Color[1];
		OriginalColor[2] = Color[2];
		OriginalColor[3] = Color[3];
	}
	void SetNormal(float* coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void loadObject(char*, glm::vec4, Vertex*&, GLushort*&, int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 9;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];

// TL
size_t VertexBufferSize[NumObjects];
size_t IndexBufferSize[NumObjects];
size_t NumIdcs[NumObjects];
size_t NumVerts[NumObjects];

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;

// Declare global objects
// TL
const size_t CoordVertsCount = 6;
Vertex CoordVerts[CoordVertsCount];
Vertex* baseVerts;
GLushort* baseIdcs;
Vertex* topVerts;
GLushort* topIdcs;
Vertex* arm1Verts;
GLushort* arm1Idcs;
Vertex* jointVerts;
GLushort* jointIdcs;
Vertex* arm2Verts;
GLushort* arm2Idcs;
Vertex* penVerts;
GLushort* penIdcs;
Vertex* buttonVerts;
GLushort* buttonIdcs;
std::string currentselection = "";
vector<Vertex*> movesWithBase = { baseVerts, topVerts, arm1Verts, jointVerts, arm2Verts, penVerts, buttonVerts };
vector<Vertex*> rotatesWithTop = { topVerts, arm1Verts, jointVerts, arm2Verts, penVerts, buttonVerts };
vector<Vertex*> rotatesWithArm1 = { arm1Verts, jointVerts, arm2Verts, penVerts, buttonVerts };
vector<Vertex*> rotatesWithArm2 = { arm2Verts, penVerts, buttonVerts };
vector<Vertex*> rotatesWithPen = { penVerts, buttonVerts };
bool shiftheld = false;

const float radius = 10.0f;
//float zpos = 10.0f;
//float ypos = 10.0f;
//float xpos = 10.0f;
///float parallelaxis = 0.0f;
//float orthogonalaxis = 0.0f
// ;

const float floatpi = 3.14159274101257324219;
float camerayaw = 90.0f;
float camerapitch = 30.0f;
bool cameraselected = false;
vec3 cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
glm::vec3 lightPosition(1.0f, 1.0f, 1.0f);
glm::mat4 myMatrix;
glm::mat4 scaleMatrix;



glm::mat4 trans = glm::mat4(1.0f);


int initWindow(void) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// FOR MAC

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Hull,Tyler(32028280)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	//TwInit(TW_OPENGL_CORE, NULL);
	//TwWindowSize(window_width, window_height);
	//TwBar* GUI = TwNewBar("Picking");
	//TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	//TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void) {
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);

	// Projection matrix : 45  Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(radius*cameradirection,	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// TL
	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	VertexBufferSize[0] = sizeof(CoordVerts);
	NumVerts[0] = CoordVertsCount;

	createVAOs(CoordVerts, NULL, 0);

}

void createVAOs(Vertex Vertices[], GLushort Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);	// TL

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

// Ensure your .obj files are in the correct format and properly loaded by looking at the following function
void loadObject(char* file, glm::vec4 color, Vertex*& out_Vertices, GLushort*& out_Indices, int ObjectId) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIdcs[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects(void) {
	//-- COORDINATE AXES --//
	CoordVerts[0] = { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[1] = { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[2] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[3] = { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[4] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[5] = { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };

	//-- GRID --//

	const size_t GridVertsCount = 44;
	Vertex GridVerts[GridVertsCount];


	// ATTN: Create your grid vertices here!
	for (int i = 0; i < 22; i += 2) { //Horizontal Lines
		GridVerts[i] = { { float(i) / 2.0f - 5.0f, 0.0, -5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 1.0, 0.0} };
		GridVerts[i + 1] = { { float(i) / 2.0f - 5.0f, 0.0, 5.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0 } };
	}

	for (int i = 22; i < 44; i += 2) { //Vertical Lines
		GridVerts[i] = { { -5.0, 0.0, float(i) / 2.0f - 16.0f, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 1.0, 0.0} };
		GridVerts[i + 1] = { { 5.0, 0.0, float(i) / 2.0f - 16.0f, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0 } };
	}

	
	VertexBufferSize[1] = sizeof(GridVerts);
	NumVerts[1] = GridVertsCount;

	createVAOs(GridVerts, NULL, 1);

	loadObject("../objects/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), baseVerts, baseIdcs, 2);
	createVAOs(baseVerts, baseIdcs, 2);
	
	

	loadObject("../objects/top.obj", glm::vec4(0.0, 1.0, 0.0, 1.0), topVerts, topIdcs, 3);
	createVAOs(topVerts, topIdcs, 3);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[3], topVerts);	// update buffer data
	glBindVertexArray(0);

	loadObject("../objects/arm1.obj", glm::vec4(0.0, 0.0, 1.0, 1.0), arm1Verts, arm1Idcs, 4);
	createVAOs(arm1Verts, arm1Idcs, 4);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[4], arm1Verts);	// update buffer data
	glBindVertexArray(0);

	loadObject("../objects/joint.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), jointVerts, jointIdcs, 5);
	createVAOs(jointVerts, jointIdcs, 5);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[5], jointVerts);	// update buffer data
	glBindVertexArray(0);


	loadObject("../objects/arm2.obj", glm::vec4(0.0, 0.75, 1.0, 1.0), arm2Verts, arm2Idcs, 6);
	createVAOs(arm2Verts, arm2Idcs, 6);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[6], arm2Verts);	// update buffer data
	glBindVertexArray(0);

	loadObject("../objects/pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), penVerts, penIdcs, 7);
	createVAOs(penVerts, penIdcs, 7);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[7], penVerts);	// update buffer data
	glBindVertexArray(0);

	loadObject("../objects/button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), buttonVerts, buttonIdcs, 8);
	createVAOs(buttonVerts, buttonIdcs, 8);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[8], buttonVerts);	// update buffer data
	glBindVertexArray(0);

	


	//-- .OBJs --//

	// ATTN: Load your models here through .obj files -- example of how to do so is as shown
	// Vertex* Verts;
	// GLushort* Idcs;
	// loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
	// createVAOs(Verts, Idcs, ObjectID);

}

void pickObject(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 255) { // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void renderScene(void) {
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		//std::cout << NumIdcs[2] << " " << NumIdcs[1] << std::endl;
		//for (int i = 0; i< NumIdcs[2]; i++) {
		//	baseVerts[i].ModifyPosition(4.0f, 0.0f, 0.0f);
		//}
		//glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[2], baseVerts);	// update buffer data

		//Manipulating Specific objects: aka remake vao everytime or what?, half of objects not showing up
		for (int i = 0; i < NumObjects; i++) {
			if (i < 2) {
				glBindVertexArray(VertexArrayId[i]);
				glDrawArrays(GL_LINES, 0, NumVerts[i]);
			} else if (i >= 2){
				glBindVertexArray(VertexArrayId[i]);
				glDrawElements(GL_TRIANGLES, NumIdcs[i], GL_UNSIGNED_SHORT,(void*)0);
			}
			glBindVertexArray(0);

		}
	}
	glUseProgram(0);
	// Draw GUI
	//TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void cleanup(void) {
	// Cleanup VBO and shader

	for (int i = 0; i < NumObjects; i++) {
		if (&VertexBufferId[i] != nullptr) {
			glDeleteBuffers(1, &VertexBufferId[i]);
			glDeleteBuffers(1, &IndexBufferId[i]);
			glDeleteVertexArrays(1, &VertexArrayId[i]);
		}
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

// Alternative way of triggering functions on keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_A:
			glUseProgram(programID);
			trans = glm::scale(trans, glm::vec3(5.0f, 2.0f, 2.0f));
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(trans));
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
			glVertexAttribPointer(
				0,      // shader layout location
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);
			glUseProgram(0);	
			std::cout << "Moving" << endl;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shiftheld = !shiftheld;
			break;
		case GLFW_KEY_C:
			//cameraselected = !cameraselected;
			currentselection = "camera";
			break;
		case GLFW_KEY_P:
			currentselection = "pen";
			break;
		case GLFW_KEY_B:
			currentselection = "base";
			break;
		case GLFW_KEY_T:
			currentselection = "top";
			break;
		case GLFW_KEY_1:
			currentselection = "arm1";
			break;
		case GLFW_KEY_2:
			currentselection = "arm2";
			break;
		case GLFW_KEY_UP:

			if (currentselection == "camera") {
				camerapitch += 5.0f;
				if (camerapitch >= 89.0)
				{
					camerapitch = 89.0f;
				}
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		case GLFW_KEY_DOWN:
			if (currentselection == "camera") {
				camerapitch -= 5.0f;
				if (camerapitch <= -89.0)
				{
					camerapitch = -89.0f;
				}
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		case GLFW_KEY_LEFT:
			if (currentselection == "camera") {
				camerayaw += 5.0f;
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
				break;
		case GLFW_KEY_RIGHT:
			if (currentselection == "camera") {
				camerayaw -= 5.0f;
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_REPEAT) {
		switch (key)
		{
		case GLFW_KEY_UP:
			if (currentselection == "camera") {
				camerapitch += 5.0f;
				if (camerapitch >= 89.0)
				{
					camerapitch = 89.0f;
				}
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		case GLFW_KEY_DOWN:
			if (currentselection == "camera") {
				camerapitch -= 5.0f;
				if (camerapitch <= -89.0)
				{
					camerapitch = -89.0f;
				}
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		case GLFW_KEY_LEFT:
			if (currentselection == "camera") {
				camerayaw += 5.0f;
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		case GLFW_KEY_RIGHT:
			if (currentselection == "camera") {
				camerayaw -= 5.0f;
				cameradirection = glm::vec3(cos(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)), sin(glm::radians(camerapitch)), sin(glm::radians(camerayaw)) * cos(glm::radians(camerapitch)));
				gViewMatrix = glm::lookAt(radius * cameradirection, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			}
			break;
		}
	}
}
// Alternative way of triggering functions on mouse click events
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

int main(void) {
	// TL
	// ATTN: Refer to https://learnopengl.com/Getting-started/Transformations, https://learnopengl.com/Getting-started/Coordinate-Systems,
	// and https://learnopengl.com/Getting-started/Camera to familiarize yourself with implementing the camera movement

	// ATTN (Project 3 only): Refer to https://learnopengl.com/Getting-started/Textures to familiarize yourself with mapping a texture
	// to a given mesh
	
	// Initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Initialize OpenGL pipeline
	initOpenGL();
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// DRAWING POINTS
		renderScene();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}