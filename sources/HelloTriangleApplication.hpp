#pragma once
// ----------STLのinclude----------
#include <stdexcept> // 例外を投げるために必要
#include <vector>    // レイヤーのリストを取り扱うために必要
#include <cstring>   // レイヤー文字列の比較に必要
#include <iostream>  // デバッグメッセージを表示するのに使用
// ----------GLFW(Vulkan込み)のinclude-----------
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
    // 使用するvalidation layerの種類を指定
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    GLFWwindow *window;  // GLFWのウインドウハンドラ
    VkInstance instance; // Vulkanアプリケーションのインスタンス

    // -----関数の宣言-----
    void initVulkan();                                 // Vulkan関連の初期化を行う
    bool checkValidationLayerSupport();                // 指定したvalidation layerがサポートされているかを確かめる
    std::vector<const char *> getRequiredExtensions(); // GLFWからウインドウマネージャのextensionsをもらってくる
    void createInstance();                             // Vulkanアプリケーションのインスタンスを作成する
    void initWindow();                                 // GLFW関連の初期化を行う
    void mainLoop();
    void cleanup();
};