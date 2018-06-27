#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_s.h>
#include <Camera.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

#define pi 3.1415926  

int screenWidth = 800;//窗口宽度
int screenHeight = 600;//窗口高度

//摄像机参数
Camera camera(glm::vec3(0.0f, 3.0f, 8.0f));

//计时器
float deltaTime = 0.0f;//上一帧和当前帧的时间差
float lastFrame = 0.0f;//上一帧

//鼠标参数
bool firstMouse = true;
float lastX = screenWidth / 2.0;
float lastY = screenHeight / 2.0;

glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);//光源位置

//四边形缓冲对象
GLuint quadVAO = 0;
GLuint quadVBO;

//平面缓冲对象
GLuint planeVAO;
GLuint planeVBO;
GLuint woodTexture;

//立方体平面缓冲对象
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

//在每次窗口大小被调整的时候调用的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//若按下返回键(ESC)则关闭窗口
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	//按下W键
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	//按下S键
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	//按下A键
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	//按下D键
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

//在每次鼠标移动时调用回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	//如果是第一次鼠标输入则更新鼠标初始位置
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//计算当前帧和上一帧鼠标位置的偏移量
	float xoffset = xpos - lastX;//计算x方向上的偏移量
	float yoffset = lastY - ypos;//计算y方向上的偏移量，注意这里是相反的，因为y坐标从底部往顶部依次增大
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

//当使用鼠标滚轮时调用回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

//当点击鼠标左键时的回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				cout << "click!" << endl;
			default:
				return;
		}
	}
	return;
}

//画四边形函数
void RenderQuad() {
	if (quadVAO == 0) {
		//平面四个顶点
		GLfloat quadVertices[] = {
			//顶点                    //纹理坐标
			-1.0f, 1.0f, 0.0f,       0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,      0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,        1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,       1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

//渲染立方体
void RenderCube() {
	if (cubeVAO == 0) {
		GLfloat vertices[] = {
			//顶点坐标              //法向量             //纹理坐标
			// Back face
            -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Bottom-left
            0.5f, 0.5f, -0.5f,     0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // bottom-right         
            0.5f, 0.5f, -0.5f,     0.0f, 0.0f, -1.0f,   1.0f, 1.0f,  // top-right
            -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,  // bottom-left
            -0.5f, 0.5f, -0.5f,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f,// top-left
            // Front face
            -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f,    1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f,  // top-right
            0.5f, 0.5f, 0.5f,      0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // top-right
            -0.5f, 0.5f, 0.5f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, 0.5f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,  // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f,     -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top-right
            -0.5f, 0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,  // bottom-left
            -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, 0.5f,    -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f,     -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top-right
            // Right face
            0.5f, 0.5f, 0.5f,       1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top-left
            0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // bottom-right
            0.5f, 0.5f, -0.5f,      1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top-right         
            0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,   0.0f, 1.0f,  // bottom-right
            0.5f, 0.5f, 0.5f,       1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  // top-left
            0.5f, -0.5f, 0.5f,      1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // bottom-left     
            // Bottom face
            -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,  0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,     0.0f, -1.0f, 0.0f,  1.0f, 1.0f, // top-left
            0.5f, -0.5f, 0.5f,      0.0f, -1.0f, 0.0f,  1.0f, 0.0f,// bottom-left
            0.5f, -0.5f, 0.5f,      0.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, 0.5f,     0.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom-right
            -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,  0.0f, 1.0f, // top-right
            // Top face
            -0.5f, 0.5f, -0.5f,     0.0f, 1.0f, 0.0f,  0.0f, 1.0f,// top-left
            0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, -0.5f,      0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // top-right     
            0.5f, 0.5f, 0.5f,       0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom-right
            -0.5f, 0.5f, -0.5f,     0.0f, 1.0f, 0.0f,  0.0f, 1.0f,// top-left
            -0.5f, 0.5f, 0.5f,      0.0f, 1.0f, 0.0f,  0.0f, 0.0f // bottom-left
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	//渲染立方体
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

//渲染场景函数
void RenderScene(Shader &shader) {
	//**************渲染平面**************
	glm::mat4 model;//变换矩阵
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//**************渲染三个立方体**************
	//第一个立方体
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
	shader.setMat4("model", model);
	RenderCube();
	//第二个立方体
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
	shader.setMat4("model", model);
	RenderCube();
	//第三个立方体
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0f));
	model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	shader.setMat4("model", model);
	RenderCube();
}

//加载纹理函数
GLuint loadTexture(string path)
{
	GLuint texture;//纹理
	glGenTextures(1, &texture);//第一个参数表示输入生成纹理的数量
	glBindTexture(GL_TEXTURE_2D, texture);//绑定纹理
	//为当前绑定的纹理对象设置环绕、过滤方式
	//设置环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//设置过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//加载纹理
	int width;//图像宽度
	int	height;//图像高度
	int	nrChannels;//颜色通道的个数
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	//生成纹理
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cout << "Failed to load texture,please check the path!" << endl;
	}
	stbi_image_free(data);//释放内存
	return texture;
}

int main() {
	glfwInit();//初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置GLFW主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置GLFW次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);(MAC系统解除注释)

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "shadowingMapping", NULL, NULL);//声明窗口对象
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);//GLFW注册回调函数，当点击鼠标mouse_button_callback函数就会被调用
	glfwSetCursorPosCallback(window, mouse_callback);//GLFW注册回调函数，当鼠标一移动mouse_callback函数就会被调用
	glfwSetScrollCallback(window, scroll_callback);//GLFW注册回调函数，当滑动鼠标滚轮scroll_callback函数就会被调用

	//隐藏鼠标光标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//调用任何OpenGL的函数之前需要初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//启用深度测试
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, screenWidth, screenHeight);


	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	bool my_tool_active = true;//设置窗口是否有效

	ImVec4 change_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);//imGUI调色板生成的颜色

	Shader simpleDepthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");//光空间转换着色器
	Shader debugDepthQuad("debug_quad_depth.vs", "debug_quad_depth.fs");//调试着色器
	Shader shader("shadow_mapping.vs", "shadow_mapping.fs");//像素着色器

	//设置纹理样例
	shader.use();
	shader.setInt("diffuseTexture", 0);
	shader.setInt("shadowMap", 1);

	//加载平面纹理
	string path = "土星.jpg";
	woodTexture = loadTexture(path);

	//平面顶点声明以及平面VBO和VAO的绑定
	GLfloat planeVertices[] = {
		// 顶点坐标                法向量                    纹理坐标
		25.0f, -0.5f, 25.0f,      0.0f, 1.0f, 0.0f,         25.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,    0.0f, 1.0f, 0.0f,         0.0f, 25.0f,
		-25.0f, -0.5f, 25.0f,     0.0f, 1.0f, 0.0f,         0.0f, 0.0f,

		25.0f, -0.5f, 25.0f,      0.0f, 1.0f, 0.0f,         25.0f, 0.0f,
		25.0f, -0.5f, -25.0f,     0.0f, 1.0f, 0.0f,         25.0f, 25.0f,
		-25.0f, -0.5f, -25.0f,    0.0f, 1.0f, 0.0f,         0.0f, 25.0f
	};
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);
	
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO;//阴影贴图的帧缓冲对象
	glGenFramebuffers(1, &depthMapFBO);
	//创建深度贴图
	GLuint depthMap;
	glGenTextures(1, &depthMap);//第一个参数表示输入生成纹理的数量
	glBindTexture(GL_TEXTURE_2D, depthMap);//绑定纹理
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//生成深度纹理
	//为当前绑定的纹理对象设置环绕、过滤方式
	//设置环绕方式
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// set texture wrapping to GL_REPEAT (default wrapping method)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0,1.0,1.0,1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//设置过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//把生成的深度纹理作为帧缓冲的深度缓冲
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//不用任何颜色数据进行渲染
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//如果GLFW被要求退出则结束循环
	while (!glfwWindowShouldClose(window))
	{
		//更新定时器
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//输入
		processInput(window);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		//-----------------渲染指令-----------------
		static float f = 0.0f;

		//定义菜单栏
		ImGui::Begin("shadowingMapping", &my_tool_active, ImGuiWindowFlags_MenuBar);//开始GUI窗口渲染
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					my_tool_active = false;
					glfwSetWindowShouldClose(window, true);//关闭窗口
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// Display contents in a scrolling region
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Created By Richbabe.");
		ImGui::BeginChild("Scrolling");
		/*
		for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
		*/
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::EndChild();
		ImGui::End();//结束GUI窗口渲染

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//设置清空屏幕所用的颜色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空屏幕的颜色缓冲和深度缓冲

		if (1) {
			//从光的位置的视野下使用不同的投影和视图矩阵来渲染场景
			glm::mat4 lightProjection, lightView;//光源的投影矩阵和视图矩阵
			glm::mat4 lightSpaceMatrix;//光空间的变换矩阵，将每个世界空间坐标变换到光源处所见到的空间
			//将光源设置为正交投影
			GLfloat near_plane = 1.0f, far_plane = 7.5f;
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);//设置正交投影矩阵
			//lightProjection = glm::perspective(60.0f, (float)screenWidth / (float)screenHeight, near_plane, far_plane);//设置透视投影矩阵
			lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));//设置视图矩阵，为从光源的位置看向场景中央
			lightSpaceMatrix = lightProjection * lightView;//计算光空间变换矩阵
			//从光的点渲染场景
			simpleDepthShader.use();
			simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			//1.首先渲染深度贴图
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);//改变视口的参数以适应阴影贴图的尺寸
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);
			RenderScene(simpleDepthShader);//渲染场景
			glCullFace(GL_BACK);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//2.像往常一样渲染场景，但这次使用深度贴图
			glViewport(0, 0, screenWidth, screenHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.use();
			glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);//投影矩阵
			glm::mat4 view = camera.GetViewMatrix();//视图矩阵
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			//将光源参数传进像素片段着色器
			shader.setVec3("lightPos", lightPos);
			shader.setVec3("viewPos", camera.Position);
			shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
			glActiveTexture(GL_TEXTURE0);//激活物体自身纹理
			glBindTexture(GL_TEXTURE_2D, woodTexture);//绑定物体自身纹理
			glActiveTexture(GL_TEXTURE1);//激活阴影纹理
			glBindTexture(GL_TEXTURE_2D, depthMap);//绑定阴影纹理
			RenderScene(shader);//渲染场景

			//Debug
			debugDepthQuad.use();
			debugDepthQuad.setFloat("near_plane", near_plane);
			debugDepthQuad.setFloat("far_plane", far_plane);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			//RenderQuad();
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		//检查并调用事件、交换缓冲
		glfwSwapBuffers(window);//交换颜色缓冲
		glfwPollEvents();//检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
	}
	//释放资源;
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();//正确释放/删除之前的分配的所有资源

	return 0;
}