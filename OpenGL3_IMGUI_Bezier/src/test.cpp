#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "my_shader.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;

struct Point
{
	float x;
	float y;
	Point(float x_, float y_) {
		x = x_;
		y = y_;
	}
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

// Bezier functions
int getBernsteinPre(int ii, int n);
vector<int> getBernsteinCoeff(int n);
Point getBezierPoint(float t, vector<Point> ctrlPoints, vector<int> bern, int n);
vector<Point> Bezier(vector<Point> ctrlPoints, int pres);


// convert a vector to vertices
float *creatVertices(vector<Point> v);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float mouseX = SCR_WIDTH / 2.0f;
float mouseY = SCR_HEIGHT / 2.0f;

vector<Point> ctrlPoints;

// IMGUI
bool if_change_ctrl = false;
bool if_change_color = false;
int which= 0;
ImVec4 lineColor= ImVec4(0.0f, 1.0f, 0.0f, 1.00f);

int main()
{
	//glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfw
	GLFWwindow* window = glfwCreateWindow(1280, 720, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Set up IMGUI binding
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	//Callbacks
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//Viewport
	glViewport(0, 0, 1280, 720);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//User the my_shader.h
	Shader myShader("my_shader.vs", "my_shader.fs");

	unsigned int VAO;
	unsigned int VBO;
	//unsigned int EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	//Render loop
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		{
			ImGui::Begin("HW8");
			ImGui::SetWindowFontScale(1.4);
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Text("Control Point Num: %d", ctrlPoints.size());
			ImGui::Checkbox("Change the control point", &if_change_ctrl);
			ImGui::Checkbox("Change the color", &if_change_color);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		static float f = 0.0f;
		if (ctrlPoints.size() < 4) {
			if_change_ctrl = false;
		}
		if (if_change_ctrl) {
			ImGui::Begin("Control Point Change Window", &if_change_ctrl);
			ImGui::Text("Choose the point you wanna change:");

			ImGui::RadioButton("Point 1", &which, 1);
			ImGui::RadioButton("Point 2", &which, 2);
			ImGui::RadioButton("Point 3", &which, 3);
			ImGui::RadioButton("Point 4", &which, 4);

			ImGui::End();
		}

		if (if_change_color) {
			ImGui::Begin("Color Change Window", &if_change_ctrl);

			ImGui::ColorEdit3("", (float*)&lineColor);

			ImGui::End();
		}

		vector<Point> test;
		
		if (ctrlPoints.size() == 4) {
			test = Bezier(ctrlPoints, 2000);
			//cout << test.size() << endl;
		}
		for (int i = 0; i < ctrlPoints.size(); i++) {
			test.push_back(ctrlPoints[i]);
		}
		int point_num = test.size();
		int total = point_num * 6;

		float *vertices = creatVertices(test);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		myShader.use();
		// VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, total * sizeof(float), vertices, GL_STREAM_DRAW);

		//Link vertex contributes, tell opengl how to explain the vertex datas
		//Location
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		//Unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);
		delete vertices;

		glDrawArrays(GL_POINTS, 0, point_num);
		//glDrawArrays(GL_LINE_STRIP, 0, point_num);

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		if_change_ctrl= true;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		if_change_ctrl = false;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		if_change_color = true;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if_change_color = false;
	}

	if (if_change_ctrl == true) {
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			which = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			which = 2;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			which = 3;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			which = 4;
		}

	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = xpos;
	mouseY = ypos;
	//cout << mouseX << " , " << mouseY << endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		if (ctrlPoints.size() < 4) {
			Point p(mouseX, mouseY);
			ctrlPoints.push_back(p);
			//cout << ctrlPoints.size() << " "<< mouseX<< ","<< mouseY<< endl;
		}
		else {
			if (if_change_ctrl == true) {
				if (which != 0) {
					ctrlPoints[which - 1].x = mouseX;
					ctrlPoints[which - 1].y = mouseY;
				}
			}
		}
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (ctrlPoints.size() < 4&& ctrlPoints.size() >= 1) {
			ctrlPoints.pop_back();
		}
		break;
	default:
		return;
	}
	return;
}


float *creatVertices(vector<Point> v) {
	int point_num = v.size();
	int total = point_num * 6;
	float *vertices = new float[total];
	for (int i = 0; i< point_num; i++) {
		// position
		vertices[i * 6 + 0] = -1.0+ (float)v[i].x / (float)640;
		vertices[i * 6 + 1] = 1.0- (float)v[i].y / (float)360;
		vertices[i * 6 + 2] = 0.0f;
		// colors
		vertices[i * 6 + 3] = lineColor.x;
		vertices[i * 6 + 4] = lineColor.y;
		vertices[i * 6 + 5] = lineColor.z;
	}
	return vertices;
}

int getBernsteinPre(int ii, int n) {
	int fenzi = 1;
	int fenmu = 1;
	for (int i = n; i >= 1; i--) {
		fenzi *= i;
	}
	for (int i = ii; i >= 1; i--) {
		fenmu *= i;
	}
	for (int i = (n - ii); i >= 1; i--) {
		fenmu *= i;
	}
	int res = fenzi / fenmu;
	return res;
}

vector<int> getBernsteinCoeff(int n) {
	vector<int> res;
	for (int i = 0; i <= n; i++) {
		res.push_back(getBernsteinPre(i, n));
	}
	return res;
}

Point getBezierPoint(float t, vector<Point> ctrlPoints, vector<int> bern, int n) {
	Point p(0, 0);
	float bezierRes;
	for (int i = 0; i <= n; i++) {
		bezierRes = bern[i] * pow(t, i)* pow(1 - t, n - i);
		p.x += ctrlPoints[i].x* bezierRes;
		p.y += ctrlPoints[i].y* bezierRes;
	}
	return p;
}

vector<Point> Bezier(vector<Point> ctrlPoints, int pres) {
	int n = 3;
	vector<int> bern;
	bern = getBernsteinCoeff(n);

	//	for (int i= 0; i< bern.size(); i++) {
	//		cout<< bern[i]<< endl;
	//	}
	vector<Point> res;
	float t;
	for (int i = 0; i <= pres; i++) {
		t = (float)(i) / (float)(pres);
		Point p = getBezierPoint(t, ctrlPoints, bern, n);
		res.push_back(p);
	}
	return res;
}