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

    GLFWwindow *window;                      // GLFWのウインドウハンドラ
    VkInstance instance;                     // Vulkanアプリケーションのインスタンス
    VkDebugUtilsMessengerEXT debugMessenger; // validation layerへのコールバック関数の登録を行ってくれるオブジェクト

    // -----関数の宣言-----
    void initVulkan();                                 // Vulkan関連の初期化を行う
    bool checkValidationLayerSupport();                // 指定したvalidation layerがサポートされているかを確かめる
    std::vector<const char *> getRequiredExtensions(); // GLFWからウインドウマネージャのextensionsをもらってくる
    void setupDebugMessenger();                        // debugMessengerを作成し、validation layerへのコールバック関数の登録を行う

    VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger); // 引数に渡された情報を元にdebugMessengerを作成する関数

    void destroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator); // 引数に渡されたdebugMessengerを削除する関数

    void createInstance(); // Vulkanアプリケーションのインスタンスを作成する
    void initWindow();     // GLFW関連の初期化を行う
    void mainLoop();
    void cleanup();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageSeverityFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData); // validation layerが実行された際にメッセージを出力するコールバック関数
};