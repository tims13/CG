#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "my_shader.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <iostream>
#include <vector>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

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

	vector<ImVec4> colors;
	ImVec4 color_whole = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
	colors.push_back(ImVec4(0.0f, 0.0f, 1.0f, 1.00f));
	colors.push_back(ImVec4(1.0f, 0.0f, 0.0f, 1.00f));
	colors.push_back(ImVec4(0.0f, 1.0f, 0.0f, 1.00f));

	//Viewport
	glViewport(0, 0, 1280, 720);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//User the my_shader.h
	Shader myShader("my_shader.vs", "my_shader.fs");

	float vertices[] = {
		// TRIANGLE
		// 位置              // 颜色
		-0.25f, -0.25f, 0.0f,  0.0f, 0.0f, 1.0f,   // 右下or 2顶部
		-0.75f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f,   // 左下
		-0.5f,  0.25f, 0.0f,  0.0f, 1.0f, 0.0f,    // 顶部
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // 2左下
		0.0f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // 3右下

		// LINES
		0.25f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左上
		0.25f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左下
		0.75f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 右下
		0.75f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,    // 右上
		// POINTS
		0.5f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, 0.4f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, 0.3f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, 0.2f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, 0.1f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, -0.1f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, -0.2f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, -0.3f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, -0.4f, 0.0f,  0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f
	};

	unsigned int indices[] = { // 注意索引从0开始! 
		0, 1, 2, // 第一个三角形
		0, 4, 3  // 第二个三角形
	};

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	bool if_change_whole = false;

	//Render loop
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		{
			ImGui::Begin("COLOR CHANGER");
			ImGui::SetWindowFontScale(1.4);
			//ImGui::SetWindowSize(ImVec2(510, 170));
			//ImGui::SetWindowPos(ImVec2(0, 0));
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Text("Change the colors!");
			// Display some text (you can use a format string too)
			ImGui::ColorEdit3("RIGHT", (float*)&colors[0]); // Edit 3 floats representing a color
			ImGui::ColorEdit3("LEFT", (float*)&colors[1]); // Edit 3 floats representing a color
			ImGui::ColorEdit3("TOP", (float*)&colors[2]); // Edit 3 floats representing a color

			ImGui::Checkbox("IF CHANGE THE WHOLE COLOR", &if_change_whole);
			ImGui::ColorEdit3("WHOLE COLOR", (float*)&color_whole, 1);
														  //Change the color
			if (if_change_whole) {
				for (int i = 0; i < 3; i++) {
					vertices[i * 6 + 3] = color_whole.x;
					vertices[i * 6 + 4] = color_whole.y;
					vertices[i * 6 + 5] = color_whole.z;
				}
			}
			else {
				for (int i = 0; i < 3; i++) {
					vertices[i * 6 + 3] = colors[i].x;
					vertices[i * 6 + 4] = colors[i].y;
					vertices[i * 6 + 5] = colors[i].z;
				}
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		myShader.use();
		// VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_LINES, 5, 4);
		glDrawArrays(GL_POINTS, 9, 11);

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