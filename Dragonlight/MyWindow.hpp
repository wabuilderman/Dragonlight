#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int WIDTH = 800;
const int HEIGHT = 600;

class MyWindow {
public:
	GLFWwindow * window;
	
	MyWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, false);
		//glfwWindowHint(GLFW_RESIZABLE, false);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

		glfwSetKeyCallback(window, key_callback);
	}

	~MyWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	GLFWwindow * getWindow() {
		return window;
	}
private :
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
};