#pragma once
// ----------STLのinclude----------
#include <stdexcept> // 例外を投げるために必要
#include <vector>    // レイヤーのリストを取り扱うために必要
#include <cstring>   // レイヤー文字列の比較に必要
#include <iostream>  // デバッグメッセージを表示するのに使用
#include <optional>  // QueueFamilyIndicesの値が未定義であるかどうかをチェック出来るようにするために必要
// ----------GLFW(Vulkan込み)のinclude-----------
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// 各コマンドに対応するキューのIDをまとめて保持する構造体
struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;

    // graphicsFamilyに何らかの値が代入されているかをチェックする
    bool isComplete(){
        return graphicsFamily.has_value();
    }
};

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

    GLFWwindow *window;                               // GLFWのウインドウハンドラ
    VkInstance instance;                              // Vulkanアプリケーションのインスタンス
    VkDebugUtilsMessengerEXT debugMessenger;          // validation layerへのコールバック関数の登録を行ってくれるオブジェクト
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理GPUの情報が格納されるオブジェクト。instanceが破棄されると自動的に破棄される。

    // -----関数の宣言-----
    void initVulkan();                                 // Vulkan関連の初期化を行う
    bool checkValidationLayerSupport();                // 指定したvalidation layerがサポートされているかを確かめる
    std::vector<const char *> getRequiredExtensions(); // GLFWからウインドウマネージャのextensionsをもらってくる
    void setupDebugMessenger();                        // debugMessengerを作成し、validation layerへのコールバック関数の登録を行う

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo); // debugMessengerを作成するために必要なオブジェクトを作成する

    VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger); // 引数に渡された情報を元にdebugMessengerを作成する関数

    void destroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator); // 引数に渡されたdebugMessengerを削除する関数

    void createInstance();                          // Vulkanアプリケーションのインスタンスを作成する
    void initWindow();                              // GLFW関連の初期化を行う
    void pickPhysicalDevice();                      // 物理GPUの設定を行う
    bool isDeviceSuitable(VkPhysicalDevice device); // 物理GPU deviceが要求する機能を満たすかどうかをチェックする
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device); // 物理GPU deviceが持っているキューファミリーの中から要求する機能に対応するものを探す
    void mainLoop();
    void cleanup();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageSeverityFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData); // validation layerが実行された際にメッセージを出力するコールバック関数
};