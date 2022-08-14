#include "HelloTriangleApplication.hpp"

void HelloTriangleApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
}

void HelloTriangleApplication::createInstance()
{
    // Vulkanにこのアプリについての情報を伝えるための構造体。{}で初期化する事で、指定していないパラメータをnullptrにしている
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Vulkanアプリケーションのインスタンスを作成するために必要な情報を保持する構造体
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Vulkanにウインドウシステムを伝えるために必要な情報をglfwから取得してくる処理
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // glfwから取得してきたウインドウシステムの情報をセットする
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // Vulkanの挙動をより厳格に監視するためのvalidation layerの数を指定する
    createInfo.enabledLayerCount = 0;

    // 今まで設定してきた情報を元にinstanceを作成。失敗したら例外を投げる
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void HelloTriangleApplication::initWindow()
{
    glfwInit();

    // GLFWはOpenGLでの使用を想定されてる作られているので、OpenGLのコンテキストを作成しないように指示を出す必要がある
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // リサイズできるウインドウは取り扱いが厄介なので、今はリサイズは出来ないように指定する
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // 3つめのパラメータはウインドウのタイトル、4つめは表示するモニタの指定、5つめはOpenGLで使用されるもの
    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::mainLoop()
{
    // ウインドウが閉じられるまでwhileループを回す
    while (!glfwWindowShouldClose(window))
    {
        // 入力などのイベントを受け取るのに必要らしい
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanup()
{
    vkDestroyInstance(instance, nullptr);

    // ウインドウ関連のリソースを削除する
    glfwDestroyWindow(window);
    // GLFW自身が確保しているリソースを解放する
    glfwTerminate();
}