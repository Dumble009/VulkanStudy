#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class HelloTriangleApplication
{
public:
    void run();

private:
    // -----変数の宣言-----
    const int DEFAULT_WINDOW_WIDTH = 800;  // ウインドウ幅の初期値
    const int DEFAULT_WINDOW_HEIGHT = 600; // ウインドウ高さの初期値
    GLFWwindow *window;                    // GLFWのウインドウハンドラ

    // -----関数の宣言-----
    void initVulkan(); // Vulkan関連の初期化を行う
    void initWindow(); // GLFW関連の初期化を行う
    void mainLoop();
    void cleanup();
};