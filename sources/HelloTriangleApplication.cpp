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
    setupDebugMessenger();
    pickPhysicalDevice();
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
    auto extensions = getRequiredExtensions();

    // glfwから取得してきたウインドウシステムの情報をセットする
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; // if文の片方でしか使われないが、if文から抜ける前に削除されると困るのでここで宣言しておく
    // validation layerの指定
    if (enableValidationLayers)
    {
        std::cout << "Validation Layer is enabled." << std::endl;
        // validationLayersで指定したvalidation layerが有効かどうかを確認
        if (!checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // 全てのvalidation layerが有効であればcreateInfoに情報を設定
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        std::cout << "Validation Layer is not enabled." << std::endl;

        createInfo.enabledLayerCount = 0;
    }

    // 今まで設定してきた情報を元にinstanceを作成。失敗したら例外を投げる
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

std::vector<const char *> HelloTriangleApplication::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // glfwExtensionCountにextensionの数がセットされる

    // glfwExtensions(char*の配列の先頭ポインタ)からglfwExtensions+glfwExtensionCount(char*の配列の末尾ポインタ)を指定する事で、その範囲の配列をvectorに変換している
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        // validation layerが有効な場合はextensionsの一つとしてvalidation layerを仕込む
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
    // この関数ではvalidationLayersで指定したレイヤーを今の環境で使用可能かを確認する
    // 方針としては、Vulkanから使用可能なレイヤーのリストを取得して、その中に指定したレイヤーが含まれているかを確認する

    // まずはVulkanに使用可能なレイヤーのリストの要素数を問い合わせる
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // layerCountにリストのサイズが入る

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // availableLayersの中に使用可能なレイヤーのリストを格納する

    // validationLayersの要素を一つずつ見ていき、availableLayersの中に存在しないレイヤー名があればfalseを投げ、全ての要素がavailableLayersの中にあればtrueを返す
    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // コールバック関数を呼び出すメッセージの重要度をビットマスク形式で指定する
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; // コールバック関数を呼び出すメッセージの種類をビットマスク形式で指定する
    createInfo.pfnUserCallback = debugCallback;                               // コールバック関数
    createInfo.pUserData = nullptr;                                           // コールバック関数に渡すことが出来る任意のオブジェクト。今回は使用しない
}

void HelloTriangleApplication::setupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    // debugMessengerを作成するために必要な情報を集約するオブジェクト
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("faiuled to set up debug messenger!");
    }
}

VkResult HelloTriangleApplication::createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessegner)
{
    // vkCreateDebugUtilsMessengerEXTという名前の関数のポインタを探してくる
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        // 見つかった関数を実行する
        return func(instance, pCreateInfo, pAllocator, pDebugMessegner);
    }
    else
    {
        // 関数が見つからなければエラー
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApplication::destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator)
{
    // vkDestroyDebugUtilsMessengerEXTという名前の関数ポインタを探してくる
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
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

void HelloTriangleApplication::pickPhysicalDevice()
{
    // まずはdeviceCountのポインタを渡して認識しているGPUの数を受け取る
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan suppoprt!");
    }

    // 先ほど渡されたdeviceCountと、データを格納するvectorのポインタを渡すことで、devicesに物理GPUのデータを格納する
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // devices内の各GPUを見ていき、要求仕様を満たすものを1つ探す
    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    // physicalDeviceが初期値VK_NULL_HANDLEのまま→仕様を満たすGPUを見つけられなかったと言う事なので、エラー
    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    // GPUの名前やサポートするVulkanのバージョンなどの基本情報をdevicePropertiesに、
    // GPUが対応している機能などの情報をdeviceFeaturesに格納する
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // ジオメトリシェーダーに対応しているかどうか
    // チュートリアルではこれに加えて単体GPUであるかどうか(iGPU等ではないか)のチェックもあったが、ノートPCで作業をする時に厄介になるので今回は省略する
    return deviceFeatures.geometryShader;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;

    return indices;
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
    if (enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    // ウインドウ関連のリソースを削除する
    glfwDestroyWindow(window);
    // GLFW自身が確保しているリソースを解放する
    glfwTerminate();
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageSeverityFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "validation layer: " << (pCallbackData->pMessage) << std::endl;

    return VK_FALSE;
}