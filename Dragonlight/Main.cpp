#include <stdexcept>
#include <iostream>
#include <functional>

#include <chrono>
#include <thread>

#include "MyWindow.hpp"
#include "MyVulkan.hpp"

class MyApp {
private:
	MyWindow * window = 0;
	MyVulkan * vulkan = 0;

public:
	void init() {
		window = new MyWindow();
		vulkan = new MyVulkan(window->getWindow());
	}

	void run() {
		while (!glfwWindowShouldClose(window->getWindow())) {
			glfwPollEvents();
			vulkan->drawFrame();
		}
	}

	void shutdown() {
		delete vulkan;
		delete window;
	}
};

void handledCall(const char * msg, std::function<void()> func) {
	try {
		func();
	}
	catch (const std::exception& e) {
		std::cerr << msg << e.what() << std::endl;
		exit(1);
	}
}

int main() {
	MyApp app;
	
	handledCall("Initialization Error: ", std::bind(&MyApp::init, &app));
	handledCall("Runtime Error: ", std::bind(&MyApp::run, &app));
	handledCall("Shutdown Error: ", std::bind(&MyApp::shutdown, &app));

	return 0;
}