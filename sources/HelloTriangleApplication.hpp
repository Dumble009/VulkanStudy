#pragma once
// ----------STLのinclude----------
#include <stdexcept> // 例外を投げるために必要
#include <vector>    // レイヤーのリストを取り扱うために必要
#include <cstring>   // レイヤー文字列の比較に必要
#include <iostream>  // デバッグメッセージを表示するのに使用
#include <optional>  // QueueFamilyIndicesの値が未定義であるかどうかをチェック出来るようにするために必要
#include <set>       // 今回のアプリケーションで使用するキューのIDの集合を取り扱うために必要
#include <cstdint>   // uint32_tを使用するために必要
#include <limits>    // numeric_limitsを使用するために必要
#include <algorithm> // clampを使用するために必要
// ----------GLFW(Vulkan込み)のinclude-----------
#define VK_USE_PLATFORM_WIN32_KHR // win32のAPIを使用してウインドウにアクセスするために必要
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // win32のAPIを使用してウインドウにアクセスするために必要

// 各コマンドに対応するキューのIDをまとめて保持する構造体
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; // レンダリングコマンドを実行可能なキューファミリーのID
    std::optional<uint32_t> presentFamily;  // レンダリング結果をウインドウサーフェースに表示するコマンドが実行可能なキューファミリーのID

    // 各パラメータに何らかの値が代入されているかをチェックする
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// 物理GPUが対応しているスワップチェインの情報をまとめた構造体
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;      // スワップチェインが取り扱うイメージの最小・最大数、解像度の最小・最大値
    std::vector<VkSurfaceFormatKHR> formats;    // ウインドウサーフェースが取り扱うピクセルのフォーマット、色空間の種類
    std::vector<VkPresentModeKHR> presentModes; // ウインドウサーフェースが対応している表示モード
};

class HelloTriangleApplication
{
public:
    void run();

private:
    // -----変数の宣言-----
    const int DEFAULT_WINDOW_WIDTH = 800;  // ウインドウ幅の初期値
    const int DEFAULT_WINDOW_HEIGHT = 600; // ウインドウ高さの初期値

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};   // 使用するvalidation layerの種類を指定
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; // 物理GPUが対応していてほしい拡張機能の名称のリスト
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    VkQueue graphicsQueue; // グラフィック命令を受け付けるキューのハンドラ。論理デバイスが削除されたら自動的に消えるので明示的にcleanupする必要は無い
    VkQueue presentQueue;  // ウインドウへの表示命令を受け付けるキューのハンドラ。

    GLFWwindow *window;                               // GLFWのウインドウハンドラ
    VkInstance instance;                              // Vulkanアプリケーションのインスタンス
    VkDebugUtilsMessengerEXT debugMessenger;          // validation layerへのコールバック関数の登録を行ってくれるオブジェクト
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // 物理GPUの情報が格納されるオブジェクト。instanceが破棄されると自動的に破棄される。
    VkDevice device;                                  // GPUの論理デバイスの情報が格納されるオブジェクト。
    VkSurfaceKHR surface;                             // ウインドウサーフェースオブジェクト。ウインドウにレンダリング結果を表示するために必要
    VkSwapchainKHR swapChain;                         // スワップチェインオブジェクト
    std::vector<VkImage> swapChainImages;             // スワップチェインに表示する画像のリスト
    VkFormat swapChainImageFormat;                    // スワップチェインに表示する画像の形式
    VkExtent2D swapChainExtent;                       // スワップチェインに表示する画像のサイズ

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

    void createInstance();                                                  // Vulkanアプリケーションのインスタンスを作成する
    void createSurface();                                                   // Vulkanのレンダリング結果をウインドウに表示するために必要なウインドウサーフェースを作成する
    void initWindow();                                                      // GLFW関連の初期化を行う
    void pickPhysicalDevice();                                              // 物理GPUの設定を行う
    void createLogicalDevice();                                             // 物理デバイスから論理デバイスを作成する
    bool isDeviceSuitable(VkPhysicalDevice device);                         // 物理GPU deviceが要求する機能を満たすかどうかをチェックする
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);              // 物理GPU deviceが必要な拡張機能に対応しているかどうかをチェックする
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);          // 物理GPU deviceが持っているキューファミリーの中から要求する機能に対応するものを探す
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); // 物理GPU deviceが対応しているスワップチェインの情報を取得する

    void createSwapChain(); // Vulkanのレンダリング結果をウインドウに表示するためのスワップチェインを作成

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats); // スワップチェインが対応している画像フォーマットの中から最適なものを選んで返す
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);  // スワップチェインへの画像の渡し方の中で最適な物を選んで返す
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);                           // スワップチェインへ渡す画像の解像度を決定して返す

    void mainLoop();
    void cleanup();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageSeverityFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData); // validation layerが実行された際にメッセージを出力するコールバック関数
};