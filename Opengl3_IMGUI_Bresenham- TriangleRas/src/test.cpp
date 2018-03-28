#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "my_shader.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

vector<int> Bresenham(int x0, int y0, int x1, int y1);
vector<int> DrawTriangleBresenham(int x1, int y1, int x2, int y2, int x3, int y3);

void getAllPoints(int x0, int y0, int x, int y, vector<int> &points);
vector<int> DrawCircleBresenham(int x0, int y0, int r);

vector<int> lineEquation(int x1, int y1, int x2, int y2);
vector<int> rasterizeTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

float *creatVertices(vector<int> v);

int main()
{
	//glfw初始化  
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfw创建窗口  
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

	bool if_draw_circle = false;
	bool if_ras = false;

	// get the vector for triangle
	vector<int> triangle_lines= DrawTriangleBresenham(0, 180, 300, -200, -300, -200);
	vector<int> triangle_ras = rasterizeTriangle(0, 180, 300, -200, -300, -200);

	//Render loop
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		{
			ImGui::Begin("HW2");
			ImGui::SetWindowFontScale(1.4);
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Text("Choose if rasterize:(DEFAULT: Not Rasterize)");
			ImGui::Checkbox("Rasterize", &if_ras);
			ImGui::Text("Choose if draw circle:(DEFAULT: Draw Triangle)");
			ImGui::Checkbox("Draw circle", &if_draw_circle);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		static float f = 0.0f;
		if (if_draw_circle) {
			ImGui::Begin("Circle Change Window", &if_draw_circle);
			ImGui::Text("Change the R of the circle(min= 0 max= 300)");
			ImGui::SliderFloat("R", &f, 0.0f, 300.0f);
			if (ImGui::Button("Close Me")) {
				if_draw_circle = false;
			}
			ImGui::End();
		}
		vector<int> test;

		if (if_draw_circle) {
			test = DrawCircleBresenham(0, 0, (int)f);
		}
		else {
			if (if_ras) {
				test = rasterizeTriangle(0, 180, 300, -200, -300, -200);
			}
			else {
				test = DrawTriangleBresenham(0, 180, 300, -200, -300, -200);
			}
		}
		int point_num = test.size() / 2;
		int total = point_num * 6;
		float *vertices = creatVertices(test);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		myShader.use();
		// VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, total* sizeof(float), vertices, GL_STREAM_DRAW);

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
}

vector<int> Bresenham(int x0, int y0, int x1, int y1) {
	vector<int> points;
	points.push_back(x0);
	points.push_back(y0);
	int dx = x1 - x0;
	int dy = y1 - y0;
	int direct_x = dx> 0 ? 1 : -1;
	int direct_y = dy> 0 ? 1 : -1;
	if (dx< 0) dx = -dx;
	if (dy< 0) dy = -dy;
	if (abs(dx)> abs(dy)) {
		int p = 2 * dy - dx;
		int x = x0;
		int y = y0;
		int two_dy = 2 * dy;
		int two_dy_sub_two_dx = 2 * dy - 2 * dx;
		while (x != x1) {
			points.push_back(x);
			points.push_back(y);
			if (p> 0) {
				y += direct_y;
				p = p + two_dy_sub_two_dx;
			}
			else {
				p = p + two_dy;
			}
			x += direct_x;
		}
	}
	else {
		int p = 2 * dx - dy;
		int x = x0;
		int y = y0;
		int two_dx = 2 * dx;
		int two_dx_sub_two_dy = 2 * dx - 2 * dy;
		while (y != y1) {
			points.push_back(x);
			points.push_back(y);
			if (p> 0) {
				x += direct_x;
				p = p + two_dx_sub_two_dy;
			}
			else {
				p = p + two_dx;
			}
			y += direct_y;
		}
	}
	points.push_back(x1);
	points.push_back(y1);
	return points;
}

vector<int> DrawTriangleBresenham(int x1, int y1, int x2, int y2, int x3, int y3) {
	vector<int> points;
	vector< vector<int> > edges;
	edges.push_back(Bresenham(x1, y1, x2, y2));
	edges.push_back(Bresenham(x1, y1, x3, y3));
	edges.push_back(Bresenham(x3, y3, x2, y2));
	for (int i = 0; i < edges.size(); i++) {
		for (int j = 0; j < edges[i].size(); j++) {
			points.push_back(edges[i][j]);
		}
	}
	return points;
}

void getAllPoints(int x0, int y0, int x, int y, vector<int> &points) {
	points.push_back(x0 + x); points.push_back(y0 + y);
	points.push_back(x0 + x); points.push_back(y0 - y);
	points.push_back(x0 - x); points.push_back(y0 + y);
	points.push_back(x0 - x); points.push_back(y0 - y);
	points.push_back(x0 + y); points.push_back(y0 + x);
	points.push_back(x0 + y); points.push_back(y0 - x);
	points.push_back(x0 - y); points.push_back(y0 + x);
	points.push_back(x0 - y); points.push_back(y0 - x);
}

vector<int> DrawCircleBresenham(int x0, int y0, int r) {
	vector<int> points;
	int x, y, d;
	y = r;
	x = 0;
	d = 3 - 2 * r;
	getAllPoints(x0, y0, x, y, points);
	while (x< y) {
		if (d< 0) {
			d = d + 4 * x + 6;
		}
		else {
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
		getAllPoints(x0, y0, x, y, points);
	}
	return points;
}

float *creatVertices(vector<int> v) {
	int point_num = v.size() / 2;
	int total = point_num * 6;
	float *vertices = new float[total];
	for (int i = 0; i< point_num; i++) {
		// position
		vertices[i * 6 + 0] = (float)v[i * 2 + 0] / (float)640;
		vertices[i * 6 + 1] = (float)v[i * 2 + 1] / (float)360;
		vertices[i * 6 + 2] = 0.0f;
		// colors
		vertices[i * 6 + 3] = 0.0f;
		vertices[i * 6 + 4] = 1.0f;
		vertices[i * 6 + 5] = 0.0f;
	}
	return vertices;
}

vector<int> lineEquation(int x1, int y1, int x2, int y2) {
	// line equation: Ax+ By+ C= 0
	vector<int> res;
	int A = y2 - y1;
	int B = x1 - x2;
	int C = x2* y1 - x1* y2;
	res.push_back(A);
	res.push_back(B);
	res.push_back(C);
	return res;
}

vector<int> rasterizeTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
	int max_x = max(x1, max(x2, x3));
	int min_x = min(x1, min(x2, x3));
	int max_y = max(y1, max(y2, y3));
	int min_y = min(y1, min(y2, y3));
	int center_x = (max_x - min_x) / 2;
	int center_y = (max_y - min_y) / 2;
	vector< vector<int> > lines;
	lines.push_back(lineEquation(x1, y1, x2, y2));
	lines.push_back(lineEquation(x1, y1, x3, y3));
	lines.push_back(lineEquation(x3, y3, x2, y2));
	// centerlize the equations
	for (int i = 0; i< 3; i++) {
		int x_temp, y_temp;
		if (i == 0) {
			x_temp = x3; y_temp = y3;
		}
		else if (i == 1) {
			x_temp = x2; y_temp = y2;
		}
		else {
			x_temp = x1; y_temp = y1;
		}
		// A*x+ B*y+ C
		if (lines[i][0] * x_temp + lines[i][1] * y_temp + lines[i][2]< 0) {
			for (int j = 0; j< lines[i].size(); j++) {
				lines[i][j] *= -1;
			}
		}
	}
	// set pixel
	vector<int> pixels;
	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {
			bool isInside = true;
			for (int i = 0; i< 3; i++) {
				if (lines[i][0] * x + lines[i][1] * y + lines[i][2]< 0) {
					isInside = false;
					break;
				}
			}
			if (isInside) {
				pixels.push_back(x);
				pixels.push_back(y);
			}
		}
	}
	return pixels;
}