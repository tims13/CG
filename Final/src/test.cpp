//#define GLEW_STATIC
//#pragma comment(lib,"glew.lib")
#include <GL\glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include "SnowParticle.h"
#include "FountainParticle.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, int key, int scancode, int action, int mods);
void autoUpdate();

unsigned int loadTexture(const char *path);

void renderScene(const Shader &shader);

// skybox
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// car's position
glm::vec3 car_position = glm::vec3(0.0f, 0.20f, 0.0f);
glm::vec3 car_forward = glm::vec3(0.0f, 0.0f, -1.0f);
float car_forward_speed = 0.0f;
float cat_forward_acceleration = -0.01f;
float car_rotation_ratio = 0.0f;
float car_rotation_acceleration = 0.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 2.3f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 camera_position = glm::vec3(car_position.x, car_position.y + 2.0f, car_position.z + 5.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;


// status
static enum STATUS
{
	NONE,           // 0
	MOVE
};
static STATUS status = STATUS::MOVE;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

														 // glfw window creation
														 // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, processInput);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("shaders/model_loading.vs", "shaders/model_loading.fs");
	Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
	Shader shadowShader("shaders/shadow_mapping.vs", "shaders/shadow_mapping.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float skyboxVertices[] = {
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//skybox faces
	vector<std::string> faces
	{
		"res/skybox/right.jpg",
		"res/skybox/left.jpg",
		"res/skybox/top.jpg",
		"res/skybox/bottom.jpg",
		"res/skybox/front.jpg",
		"res/skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// plane
	float planeVertices[] = {
		// positions            // normals         // texcoords
		25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	unsigned int Texture = loadTexture("res/textures/texture.jpg");

	// shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	shadowShader.use();
	shadowShader.setInt("diffuseTexture", 0);
	shadowShader.setInt("shadowMap", 1);

	// load models
	// -----------
	Model ourModel("res/mycar/mycar.obj");
	Model model_merge("res/sence_model/model_m1.obj");
	Model fence("res/obstacle/fence.obj");
	// Model myWay("res/test/myway.obj");

	// light position
	glm::vec3 lightPos(0.0f, 4.0f, -1.0f);

	// model view projection matries
	glm::mat4 model, view, projection;

	// Particles init
	SnowParticle::SnowParticle mSnow;
	FountainParticle::FountainParticle mFountain;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// don't forget to enable shader before setting uniforms
		ourShader.use();
		// view/projection transformations
		if (status == STATUS::NONE) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = glm::lookAt(camera_position, car_position, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// auto update
		autoUpdate();

		// render the car model
		glm::mat4 model;
		model = glm::translate(model, car_position);
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		// model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(car_rotation_ratio + 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		// render the model
		glm::mat4 model_col;
		ourShader.setMat4("model", model_col);
		model_merge.Draw(ourShader);

		// render the fence
		glm::mat4 model_fence;
		ourShader.setMat4("model", model_fence);
		fence.Draw(ourShader);

		// light
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// render the plane
		shadowShader.use();
		shadowShader.setMat4("projection", projection);
		shadowShader.setMat4("view", view);
		// set light uniforms
		if (status == STATUS::NONE) {
			shadowShader.setVec3("viewPos", camera.Position);
		}
		else {
			shadowShader.setVec3("viewPos", camera_position);
		}
		shadowShader.setVec3("lightPos", lightPos);
		shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		renderScene(shadowShader);

		// Snow
		glm::mat4 model_snow(1.0f);
		glm::mat4 projection_snow = glm::ortho(-15.0, 15.0, 0.0, 15.0, -3.0, 100.0);
		mSnow.Render(deltaTime, model_snow, view, projection_snow);

		// Fountain
		glm::mat4 model_fountain;
		model_fountain = glm::translate(model_fountain, glm::vec3(4.5, 0.0, 5.0));
		model_fountain = glm::scale(model_fountain, glm::vec3(0.05, 0.05, 0.05));
		mFountain.Render(deltaTime, model_fountain, view, projection);


		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		if (status == STATUS::NONE) {
			view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		}
		else {
			view = glm::mat4(glm::mat3(glm::lookAt(camera_position, car_position, glm::vec3(0.0f, 1.0f, 0.0f)))); // remove translation from the view matrix
		}
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


							  // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
							  // -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

void autoUpdate() {
	car_forward_speed += cat_forward_acceleration*deltaTime;
	car_forward_speed = car_forward_speed > 0.0f ? car_forward_speed : 0.0f;
	car_rotation_ratio += car_rotation_acceleration*deltaTime;
	if (car_rotation_ratio != 0.0f) {
		// std::cout << "Here rotate, car_rotation_ratio : " << car_rotation_ratio << " car_forward_speed : " << car_forward_speed << std::endl;
		/*std::cout << "Sin : " << sin(glm::radians(car_rotation_ratio)) << " Cos : " << cos(glm::radians(car_rotation_ratio)) << std::endl;*/
		car_forward.z = (-1)*sin(glm::radians(car_rotation_ratio + 90.0f));
		car_forward.x = cos(glm::radians(car_rotation_ratio + 90.0f));
	}
	car_position.z += car_forward.z*car_forward_speed*deltaTime;
	car_position.x += car_forward.x*car_forward_speed*deltaTime;
	camera_position.x = car_position.x - car_forward.x;
	camera_position.z = car_position.z - car_forward.z;
	camera_position.y = car_position.y + 0.4f;
}

void processInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, true);

	if (status == STATUS::MOVE) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_A)
				car_rotation_acceleration = 50.0f;
			if (key == GLFW_KEY_D)
				car_rotation_acceleration = -50.0f;
			if (key == GLFW_KEY_W)
				cat_forward_acceleration = 0.2f;
			if (key == GLFW_KEY_S)
				cat_forward_acceleration = -0.4f;
			if (key == GLFW_KEY_SPACE) {
				status = STATUS::NONE;
				std::cout << "Switch status from MOVE to NONE" << std::endl;
			}
		}

		if (action == GLFW_RELEASE) {
			if (key == GLFW_KEY_A || key == GLFW_KEY_D)
				car_rotation_acceleration = 0.0f;
			if (key == GLFW_KEY_W || key == GLFW_KEY_S)
				cat_forward_acceleration = -0.1f;
		}
	}
	else {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			status = STATUS::MOVE;
			std::cout << "Switch status from NONE to MOVE" << std::endl;
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// renders the scene
void renderScene(const Shader &shader)
{
	// floor
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(10.0, 10.0, 10.0));
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

