#pragma once
// ----------STLのinclude----------
#include <stdexcept> // 例外を投げるために必要
#include <vector>
#include <array>
#include <cstring>
#include <iostream>  // デバッグメッセージを表示するのに使用
#include <optional>  // QueueFamilyIndicesの値が未定義であるかどうかをチェック出来るようにするために必要
#include <set>       // 今回のアプリケーションで使用するキューのIDの集合を取り扱うために必要
#include <cstdint>   // uint32_tを使用するために必要
#include <limits>    // numeric_limitsを使用するために必要
#include <algorithm> // clampを使用するために必要
#include <fstream>   // シェーダーコードを読み込むために必要
#include <chrono>    // 時間に関する処理を扱うために必要

// ----------GLFW(Vulkan込み)のinclude-----------
#define VK_USE_PLATFORM_WIN32_KHR // win32のAPIを使用してウインドウにアクセスするために必要
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // win32のAPIを使用してウインドウにアクセスするために必要

// ----------GLMのinclude----------
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ----------Win32APIのinclude----------------
#include "windows.h"

// ----------STB(画像ライブラリ)のinclude---------------
#include "stb_image.h"

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

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        // CPU上の頂点情報をGPUに渡す際に、情報一つ当たりのデータサイズを決定する
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;                             // 今から作ろうとしているバインディングのインデックス
        bindingDescription.stride = sizeof(Vertex);                 // 一つの頂点データのサイズ
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 各データが頂点ごとかインスタンス毎か。インスタンスレンダリングとかでは別の値にするらしい

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        // CPU上の頂点情報をGPUに渡し際の渡し方を決定する
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        // 頂点座標のバインディングの設定
        attributeDescriptions[0].binding = 0;                         // どのインデックスのバインディングと紐づくか
        attributeDescriptions[0].location = 0;                        // vertexシェーダの何番目のinputと紐づくか
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // データのフォーマット、32ビットのfloatデータが3つ
        attributeDescriptions[0].offset = offsetof(Vertex, pos);      // 構造体の先頭アドレスから頂点座標が入っているアドレスのオフセット

        // 頂点色のバインディングの設定
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // テクスチャのUVマッピングのバインディングの設定
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class HelloTriangleApplication
{
public:
    void run();

private:
    // -----変数の宣言-----
    const int DEFAULT_WINDOW_WIDTH = 800;  // ウインドウ幅の初期値
    const int DEFAULT_WINDOW_HEIGHT = 600; // ウインドウ高さの初期値
    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};   // 使用するvalidation layerの種類を指定
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; // 物理GPUが対応していてほしい拡張機能の名称のリスト

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0};
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
    std::vector<VkImageView> swapChainImageViews;     // スワップチェインに表示する各画像にアクセスするために必要なオブジェクト
    std::vector<VkFramebuffer> swapChainFramebuffers; // スワップチェインに含まれる各画像のフレームバッファのリスト
    VkFormat swapChainImageFormat;                    // スワップチェインに表示する画像の形式
    VkExtent2D swapChainExtent;                       // スワップチェインに表示する画像のサイズ
    VkRenderPass renderPass;                          // パイプラインの中で取り扱われるテクスチャ群をまとめたレンダーパスのオブジェクト
    VkDescriptorSetLayout descriptorSetLayout;        // シェーダ渡すデスクリプタの情報をまとめるオブジェクト
    VkDescriptorPool descriptorPool;                  // デスクリプタセットを払いだすためのプール
    std::vector<VkDescriptorSet> descriptorSets;      // プールから払いだされるデスクリプタセット。スワップチェーンの各フレーム毎に一つ作られる
    VkPipelineLayout pipelineLayout;                  // シェーダーにグローバルな変数を渡して動的に挙動を変更するために使用する。
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;                   // レンダリングなどのVulkanへのコマンドをキューに流し込むオブジェクト
    std::vector<VkCommandBuffer> commandBuffers; // コマンドプールの記憶実体(?)
    VkBuffer vertexBuffer;                       // 頂点データを格納するバッファ
    VkDeviceMemory vertexBufferMemory;           // 頂点データを格納するバッファのメモリ実体
    VkBuffer indexBuffer;                        // 各ポリゴンがどの頂点を使用するかをまとめたデータのためのバッファ
    VkDeviceMemory indexBufferMemory;            // インデックスバッファのメモリ実体

    std::vector<VkBuffer> uniformBuffers;             // MVP行列を書き込むためのバッファ。フレーム数分用意するので配列にしている
    std::vector<VkDeviceMemory> uniformBuffersMemory; // uniformBuffersが使用するメモリ実体

    std::vector<VkSemaphore> imageAvailableSemaphores; // スワップチェインから書き込み先の画像を取得してくるのを待つためのセマフォ
    std::vector<VkSemaphore> renderFinishedSemaphores; // スワップチェインへの書き込みが完了するのを待つためのセマフォ
    std::vector<VkFence> inFlightFences;               // あるフレームへのレンダリングが終わるのを待つためのフェンス

    VkImage textureImage;              // モデルに貼り付けるテクスチャ画像
    VkDeviceMemory textureImageMemory; // テクスチャ画像が格納されるメモリ実体
    VkImageView textureImageView;      // テクスチャのビュー
    VkSampler textureSampler;          // テクスチャのサンプラー

    bool framebufferResized = false; // ウインドウサイズの変更等があったときにそれを知らせるために立てられるフラグ

    uint32_t currentFrame = 0; // 今使用しているフレームバッファのインデックス

    int monitorLeftOffset = 0; // 全モニタの左上の座標を(0, 0)にするためのオフセット
    int monitorTopOffset = 0;  // 全モニタの左上の座標を(0, 0)にするためのオフセット

    // -----関数の宣言-----
    static std::vector<char> readFile(const std::string &filename); // filenameのパスの指すファイルを読み込んでバイトコードのvectorとして返す

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

    void createSwapChain();                                      // Vulkanのレンダリング結果をウインドウに表示するためのスワップチェインを作成
    void recreateSwapChain();                                    // ウインドウサイズが変わったりしたときにスワップチェインを再作成する
    void createImageViews();                                     // スワップチェイン内の各画像にアクセスするためのビューを作成する
    VkImageView createImageView(VkImage image, VkFormat format); // 画像のビューを作成する処理をまとめたヘルパー関数
    void createRenderPass();                                     // フレームバッファーに含まれるバッファの種類や数などを定める
    void createDescriptorSetLayout();                            // シェーダに頂点情報以外の情報を伝えるためのデスクリプタを作成する
    void createGraphicsPipeline();                               // グラフィックパイプラインを作成する
    void createFramebuffers();                                   // フレームバッファを作成する
    void createCommandPool();                                    // コマンドプールを作成する
    VkCommandBuffer beginSingleTimeCommands();                   // 単発実行するためのコマンドバッファを作成する。
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);   // 単発実行するためのコマンドバッファの中身を実行に移す
    void createTextureImage();                                   // モデルに貼り付けるテクスチャ画像を読み込む
    void createImage(uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage &image,
                     VkDeviceMemory &imageMemory); // VkImageを作成する処理をまとめたユーティリティ関数
    void transitionImageLayout(VkImage image,
                               VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout); // VkImageのレイアウトを変更する
    void createTextureImageView();                       // モデルに貼り付けるテクスチャのビューを作成する。
    void createTextureSampler();                         // テクスチャのサンプラー(テクセルのサンプル方法を定義するオブジェクト)を作成する
    void createVertexBuffer();                           // 頂点データを保存しておくためのバッファを作成し、CPUからGPUにデータを転送する
    void createIndexBuffer();                            // インデックスバッファを作成し、CPUからGPUにデータを転送する
    void copyBufferToImage(VkBuffer buffer,
                           VkImage image,
                           uint32_t width,
                           uint32_t height); // buffer上のテクセルデータをimageに転送する
    void createUnifomBuffers();              // シェーダに渡すMVP行列を書き込むためのバッファを作成する
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);                                          // VkBufferとVkDeviceMemoryの作成処理をまとめたヘルパ関数
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size); // 頂点データを一次バッファからGPUが管理する頂点バッファにコピーする
    void createDescriptorPool();                                                // デスクリプタセットを発行するためのプールを作成する
    void createDescriptorSets();                                                // プールからデスクリプタセットを作成する
    void createCommandBuffers();                                                // コマンドバッファを作成する
    void createSyncObjects();                                                   // セマフォやフェンスなど同期するためのオブジェクトを作成する

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); // コマンドバッファにコマンドを記録する

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties); // VRAMが対応しているメモリの種類と用途が必要とするメモリの機能を比較して最適なメモリの種類を選んで返す

    VkShaderModule createShaderModule(const std::vector<char> &code);                                    // shaderのバイトコードからシェーダーモジュールを作成する
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats); // スワップチェインが対応している画像フォーマットの中から最適なものを選んで返す
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);  // スワップチェインへの画像の渡し方の中で最適な物を選んで返す
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);                           // スワップチェインへ渡す画像の解像度を決定して返す

    void calcMonitorOffset(); // 全モニタが構成する矩形領域の左上の座標を計算する

    void mainLoop();
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage); // MVP行列をアップデートする。引数はスワップチェーン上の現在使用している画像の番号

    void cleanup();
    void cleanupSwapChain();

    // GLFWがコールバックする際にどのインスタンスの関数を呼ぶべきかを
    // 知ることが出来ないのでstatic関数として定義しておく
    static void framebufferResizeCallback(
        GLFWwindow *window,
        int width,
        int height); // ウインドウサイズの変更等が起こった時に呼ばれるコールバック

    static BOOL CALLBACK enumProc(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK monitorEnumProc(
        HMONITOR monitor,
        HDC hdc,
        LPRECT rect,
        LPARAM param);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageSeverityFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData); // validation layerが実行された際にメッセージを出力するコールバック関数
};