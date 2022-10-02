#include "HelloTriangleApplication.hpp"

HWND handle_workerw;
HDC dc_workerw;
HDC dc_workerwCopy;
HDC dc_src;
HBITMAP hBitmap;
std::vector<RECT> monitorRects;

std::vector<char> HelloTriangleApplication::readFile(const std::string &filename)
{
    // ate->at the end ファイルの末尾から読み始める
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        printf("file name is : %s\n", filename.c_str());
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg(); // 末尾から読み始めているので、今の読み取り位置がファイルサイズと一致する
    std::vector<char> buffer(fileSize);

    file.seekg(0); // 読み取り位置を先頭に戻す
    file.read(buffer.data(), fileSize);

    file.close();

    return std::move(buffer);
}

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
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUnifomBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
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

void HelloTriangleApplication::createSurface()
{
    // Vulkanからウインドウにアクセスするために必要な構造体を作成するための情報を埋める
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);    // GLFWのwindowオブジェクトからHWNDを取り出している
    createInfo.hinstance = GetModuleHandle(nullptr); // 今のプロセスのHINSTANCEハンドルを取得する

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
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

    // リサイズを禁止したい場合は以下のフラグで出来ないように指定する
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // 3つめのパラメータはウインドウのタイトル、4つめは表示するモニタの指定、5つめはOpenGLで使用されるもの
    window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this); // HelloTriangleApplicationのインスタンスにアクセスできるようにthisを埋め込んでおく
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    auto progman = FindWindow("Progman", nullptr);
    PDWORD ptr;
    SendMessageTimeout(progman, 0x052C, 0, 0, 0x0, 1000, (PDWORD_PTR)&ptr);
    handle_workerw = 0;
    EnumWindows(enumProc, 0);
    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, 0);
    dc_workerw = GetDCEx(handle_workerw, 0, 0x403);
    RECT rect;
    GetWindowRect(handle_workerw, &rect);
    // 初期状態のデスクトップの様子を記録しておく
    auto x = rect.right - rect.left;
    auto y = rect.bottom - rect.top;
    dc_workerwCopy = CreateCompatibleDC(dc_workerw);
    hBitmap = CreateCompatibleBitmap(dc_workerw, x, y);
    SelectObject(dc_workerwCopy, hBitmap);
    BitBlt(dc_workerwCopy, 0, 0, x, y, dc_workerw, 0, 0, 0x00CC0020);
    dc_src = GetDC(glfwGetWin32Window(window));

    // モニターの情報を取得して、全てのモニタのデスクトップをオーバライドできるようにする
}

BOOL CALLBACK HelloTriangleApplication::enumProc(HWND hwnd, LPARAM lParam)
{
    auto shell = FindWindowEx(hwnd, 0, "SHELLDLL_DefView", nullptr);
    if (shell != nullptr)
    {
        handle_workerw = FindWindowEx(0, hwnd, "WorkerW", nullptr);
    }
    return true;
}

BOOL CALLBACK HelloTriangleApplication::monitorEnumProc(
    HMONITOR monitor,
    HDC hdc,
    LPRECT rect,
    LPARAM param)
{
    monitorRects.push_back(*rect);
    return true;
}

void HelloTriangleApplication::framebufferResizeCallback(
    GLFWwindow *window,
    int width,
    int height)
{
    // windowに埋め込んだthisを取り出す
    auto app = reinterpret_cast<HelloTriangleApplication *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
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
            msaaSamples = getMaxUsableSampleCount(); // MSAA用のサンプル点の数を決定する
            break;
        }
    }

    // physicalDeviceが初期値VK_NULL_HANDLEのまま→仕様を満たすGPUを見つけられなかったと言う事なので、エラー
    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void HelloTriangleApplication::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        // 物理デバイスが持つ各キューファミリーに対していくつのキューを要求するか指定する
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority; // キューの実行優先順位。0~1の範囲で指定する

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};  // キューに要求する機能。今は空にしておく
    deviceFeatures.samplerAnisotropy = VK_TRUE; // 異方性フィルタリングが出来る事

    // ここから論理デバイスの作成情報を埋めていく
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // どんなキューをいくつ持つのか
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // どんな拡張機能に対応するのか
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        // validation layerの設定。今後のアップデートでこのプロパティは廃止されるらしい
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // 論理デバイスで抽象化する物理デバイスと、先ほど作成した情報を渡して論理デバイスを作成する
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // 出来上がった論理デバイスから、各種命令を扱えるキューのハンドラを取得する。
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    // deviceにアプリの条件を満たすキューファミリーがすべて存在しているかどうか
    auto indices = findQueueFamilies(device);

    // deviceが必要な拡張機能全てに対応しているかどうか
    bool extensionSupported = checkDeviceExtensionSupport(device);

    // deviceがスワップチェインに対応しているかどうか
    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    // deviceがサポートしている拡張機能の名称のリストを取得
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // deviceが、要求している拡張機能をすべて満たしているかチェック
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    // まずはキューファミリーが全部で何個あるのか調べ、キューファミリーのリストを取得する
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // queueFamiliesの中から各コマンドを実行可能なキューを探してその番号を格納する
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {

        // グラフィックスコマンドを実行可能なキューかどうか
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // ウインドウへの表示コマンドを実行可能なキューかどうか
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        // 全部のパラメータに何らかの値が入っていればそれ以上探索する必要は無い
        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    // 物理デバイスが対応しているスワップチェインについての情報を順に取得していき、detailsのメンバに格納する
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void HelloTriangleApplication::createSwapChain()
{
    // 物理GPUが対応しているスワップチェインについての情報を取得
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    // 対応している画像フォーマット、画像の渡し方、画像サイズの中から最適な物を選んで取得
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // スワップチェインに含まれる画像の数を決定する。デフォルトは最小枚数+1とする。
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // 最大枚数を超えていないかのチェック。maxImageCount = 0は最大数が無限であることを意味している
    if (swapChainSupport.capabilities.maxImageCount != 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // スワップチェインを作成するための情報を埋めていく
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;                             // レンダリング結果のレイヤー数。VRとかじゃない限りは1でOK
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // スワップチェインの使い道。この値はレンダリング結果の表示に使用することを示している

    // 複数のキューファミリーでスワップチェインを共有する際の設定
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        // graphicsFamilyとpresentFamilyが異なるキューファミリーの時は複数のキューファミリーが共有できるようにする
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        // 共有するキューファミリーの数とポインタを渡しておく
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        // graphicsFamilyとpresentFamilyが同じキューファミリーな場合は一つのキューファミリーが排他的に取り扱えるようにする。
        // こちらの方がパフォーマンス的に良い。
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    // 画像をレンダリングする際に回転や反転などの操作を加えられる。currentTransformは何もしない
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    // レンダリング結果のアルファチャンネルをどう取り扱うか。COMPOSITE_ALPHA_OPAQUEは透明度を無視する
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;             // 他のウインドウが重なったりしたときにその部分のピクセルの色を気にしない(？)
    createInfo.oldSwapchain = VK_NULL_HANDLE; // ウインドウのリサイズ等によって使用していたスワップチェインが使えなくなって作り直す時に、この部分に古いスワップチェインを渡す

    // スワップチェインオブジェクトの作成。失敗したら例外を投げる
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // スワップチェインに渡す画像のリストを作る
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        // widthかheightが0になる→ウインドウが最小化されている
        // ウインドウが最小化されている場合はループし続けることで一時停止させる
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device); // レンダリング中のフレームバッファを操作したりしないようにアイドル状態になるまで待機する

    cleanupSwapChain(); // スワップチェインを作り直す前に既存のリソースを全て削除する。

    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

VkImageView HelloTriangleApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void HelloTriangleApplication::createRenderPass()
{
    // 色を取り扱うサブパスに渡されるテクスチャの情報を定義する。このテクスチャはMSAA用の物で最終的に画面に表示されるテクスチャではない
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                   // フレームバッファに書き込む前に既存の内容をクリアする
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                 // フレームバッファに書き込まれた値を保持し、後でウインドウに表示したりする際に読みだされるようにする
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;        // ステンシルの値はクリアされてもされなくてもどっちでもいい
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;      // ステンシルの値は保持されてもされなくてもどっちでもいい
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;              // 最初にフレームバッファに書き込まれているデータの構造は気にしない
    colorAttachment.samples = msaaSamples;                                  // MSAAのサンプル点の数を指定
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // フレームバッファに書き込んだ後のデータはスワップチェインに表示できる形式にする

    // サブパスに渡すテクスチャのメタデータ
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;                                    // どのテクスチャについてか。レンダーパス全体における0番のテクスチャのメタ情報
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 色を取り扱うのに最適となるようにサブパスを並び変えるように指示する

    // 深度バッファに使用するテクスチャの情報を定義する
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // 描画が完了したら深度バッファは使用しないので、レンダリング後はどういう形式になっても気にしない
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.samples = msaaSamples;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // MSAA用のテクスチャの解像度を変更して画面に出力できるようにしたテクスチャの情報を定義する
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // このサブパスがレンダリング用のものであることを示す
    subpass.colorAttachmentCount = 1;                            // このサブパスが何枚のカラーテクスチャを持つか
    subpass.pColorAttachments = &colorAttachmentRef;             // サブパスに渡されてくるMSAA用のカラーテクスチャのメタデータの配列(ポインタ)
    subpass.pDepthStencilAttachment = &depthAttachmentRef;       // サブパスに渡されてくる深度テクスチャのメタデータの配列。深度テクスチャは最大1枚しか使用しないので、colorAttachmentCountに相当するメンバは無い。
    subpass.pResolveAttachments = &colorAttachmentResolveRef;    // サブパスに渡されてくる画面に出力するテクスチャのメタデータの配列

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // レンダーパス全体で扱うテクスチャの数
    renderPassInfo.pAttachments = attachments.data();                           // レンダーパスで扱うテクスチャ情報の配列
    renderPassInfo.subpassCount = 1;                                            // レンダーパスに含まれるサブパスの数
    renderPassInfo.pSubpasses = &subpass;                                       // レンダーパスに含まれるサブパスの配列

    // サブパス間の依存関係を定義する
    VkSubpassDependency dependency{};
    // どのサブパスからどのサブパスの間の依存関係なのか
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 暗黙的に作られる全てのサブパスが開始される前段階のサブパスをsrcとして指定
    dependency.dstSubpass = 0;                   // 0番目のサブパス(上で作ったサブパス)をdstとして指定
    // どのステージのどの操作がどのステージのどの操作を待つのかを指定する
    // ここでは色を書き込むアタッチメントと深度を書き込むアタッチメントの読み込み処理が完了してから書き込むように指定している
    // 待たれる対象
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0; // ここで操作の種類を指定するが、何も指定しないと全ての操作になる(?)
    // 待つ対象
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
    // MVP行列をシェーダ内で使えるようにバインディングする
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // シェーダー内で何番目のバインディングにあたるか
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // どのシェーダが使用するデスクリプタなのか
    uboLayoutBinding.pImmutableSamplers = nullptr;            // 画像をデスクリプタとして渡す際に使用するパラメータ

    // テクスチャをシェーダ内で使えるようにバインディングする
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // テクスチャはフラグメントシェーダが使用する

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data(); // デスクリプタのバインディングの配列の先頭ポインタ。

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void HelloTriangleApplication::createGraphicsPipeline()
{
    auto vertexShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // vertex shaderの作成
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // シェーダー開始時に実行される関数名

    // fragment shaderの作成
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // パイプライン作成後に動的に変更可能なプロパティを定義する
    // ここではフレームバッファの描画に使う範囲やウインドウのサイズを変更できるようにしておく
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // 頂点データの取り扱い方を定義する
    // 今回はシェーダーにハードコーディングしているので、頂点データは存在しない事にしている
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // 頂点データがどのような図形を表しているのかを定義する。
    // ここでは、3つの頂点ごとにそれらを結んだ三角形を表すように指定している
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // ある三角形で使われた頂点を次の三角形で再利用する場合に、どの頂点を再利用するかを手で設定できるかどうか。

    // ビューポートの範囲の指定
    // ウインドウのどれだけの範囲をレンダリングのために使用するか
    // ここでは端から端まで全部に描画するよう指定している
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // シザーの範囲の指定
    // レンダリング結果のどの範囲を使用するか。この範囲の外のレンダリング結果は破棄される
    // ここでは端から端まで全てのレンダリング結果を描画するように指定している
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // ビューポートとシザーをいくつ使用するかの定義
    // GPUによっては複数のビューポートを扱えたりするらしい
    // 今回は一つしか使わない。
    // またビューポートとシザーを静的な値でフィックスしたい場合はここでビューポートとシザーの値を渡す
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // ラスタライザの設定
    // ラスタライザは三角形などの図形をピクセル単位に分割し、フラグメントシェーダーを適用する
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;                 // ニアプレーンとファープレーンの範囲の外にあるポリゴンを内部に収めるか
    rasterizer.rasterizerDiscardEnable = VK_FALSE;          // これがtrueだとラスタライザはポリゴンを無視する
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;          // ポリゴンをどう描画するか。ここでは三角形として中身を埋めるように指定している
    rasterizer.lineWidth = 1.0f;                            // 線を描画する際の太さ
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;            // カリングの設定。ここではバックカリングすることを指定している
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // 何をもって正面を向いているとするかの指定。頂点が時計回りに並んでいたら正面としている。ただし、プロジェクション行列のY軸を反転しているため、正しく反時計回りを正面とするためには時計回りを正面と設定する必要がある
    rasterizer.depthBiasEnable = VK_FALSE;                  // 深度値にバイアスを加えるかの設定。ここでは無効としている
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // AAのマルチサンプリングの設定
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // カラーブレンディングの設定
    // AttachmentStateの方はフレームバッファごとの独立した設定
    // 今回はカラーブレンディングは無効化している
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // グローバルなカラーブレンディングの設定
    // また、ビットワイズな操作を適用することも出来る。
    // ここではさっき作ったAttachmentをフレームバッファと結びつけ、ビットワイズな操作は無効化している
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;           // 深度テストを行うように設定
    depthStencil.depthWriteEnable = VK_TRUE;          // レンダリング時に深度値を書き込むように設定
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // 深度値が低い方(カメラに近い方)がテストに通るように設定。
    depthStencil.depthBoundsTestEnable = VK_FALSE;    // 特殊な深度テストをするかどうか。今回はしない。
    depthStencil.minDepthBounds = 0.0f;               // 上記の深度テストで使用する値。今回は無関係。
    depthStencil.maxDepthBounds = 1.0f;               // 上記の深度テストで使用する値。今回は無関係。
    depthStencil.stencilTestEnable = VK_FALSE;        // ステンシルバッファを使用するかどうか。今回はしない。
    depthStencil.front = {};                          // ステンシルバッファに関する設定。今回は無関係。
    depthStencil.back = {};                           // ステンシルバッファに関する設定。今回は無関係。

    // シェーダーにグローバルな変数を渡し、動的に挙動を変更する
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // シェーダに渡したい変数についての情報
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;         // グラフィックパイプラインに含まれるシェーダーの段階の数
    pipelineInfo.pStages = shaderStages; // バーテックスシェーダーとフラグメントシェーダーを使用することを指示する
    // パイプライン中の固定ステージの設定
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // 使用するサブパスのインデックス
    // どのパイプラインから派生するか。今回はどのパイプラインからも派生しない
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // 派生するパイプラインオブジェクト
    pipelineInfo.basePipelineIndex = -1;              // 派生するパイプラインのインデックス

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // グラフィックスパイプラインが出来たらシェーダーモジュールはもう不要なので削除する
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data()); // uint32_tの配列として渡すする必要があるのでreinterpretする

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        printf("shader size %d\n", (int)code.size());
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void HelloTriangleApplication::createFramebuffers()
{
    // スワップチェインの画像1枚につきフレームバッファは一つ必要
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapChainImageViews[i]}; // 深度バッファは全てのフレームバッファで使い回す

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void HelloTriangleApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;      // コマンドを個別に上書きできるようにする
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // コマンドプールは一つのキューにつき一つ作られる。ここではグラフィックコマンドを扱うキューに対応するプールを作っている

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

VkCommandBuffer HelloTriangleApplication::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void HelloTriangleApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    createImage(swapChainExtent.width,
                swapChainExtent.height,
                1,
                msaaSamples,
                colorFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                colorImage,
                colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void HelloTriangleApplication::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();
    createImage(swapChainExtent.width,
                swapChainExtent.height,
                1,
                msaaSamples,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImage,
                depthImageMemory);

    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(depthImage,
                          depthFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          1);
}

VkFormat HelloTriangleApplication::findDepthFormat()
{
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat HelloTriangleApplication::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                                       VkImageTiling tiling,
                                                       VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        // 物理デバイスが、要求するタイリングの仕方で必要とする機能を提供できるかどうかを調べる
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

bool HelloTriangleApplication::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void HelloTriangleApplication::createTextureImage()
{
    // テクスチャ画像を読み込む
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4; // RGBAが1バイトずつ並ぶ

    // ミップマップをいくつ作成するかの計算。長辺を2で何回割れるかに元の画像の分で1を足す事で求められる。
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    // まずはCPUから見える領域にテクスチャを転送して、その後GPUのみが見える領域にコピーする
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    // 最終的な伝送先となるImageを作成する
    createImage(texWidth,
                texHeight,
                mipLevels,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // ミップマップを作成する際に書き込む必要があるので、TRAANSFER_SRCも指定する必要がある
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage,
                textureImageMemory);

    // textureImageのレイアウトを転送するのに最適な形に変換する
    transitionImageLayout(textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          mipLevels);

    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // 転送用のステージングバッファはもう不要なので消してしまう。
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void HelloTriangleApplication::createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void HelloTriangleApplication::createImage(uint32_t width,
                                           uint32_t height,
                                           uint32_t mipLevels,
                                           VkSampleCountFlagBits numSamples,
                                           VkFormat format,
                                           VkImageTiling tiling,
                                           VkImageUsageFlags usage,
                                           VkMemoryPropertyFlags properties,
                                           VkImage &image,
                                           VkDeviceMemory &imageMemory)
{
    // 画像をVkImageの形でロードする
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 3次元テクスチャにおける3次元方向のサイズ。今回は平面のテクスチャなので厚みは1
    imageInfo.mipLevels = mipLevels;
    imageInfo.samples = numSamples;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;                           // STBで読み込んだ画像はRGBAになっているので、それと合わせた形式にしておく必要がある
    imageInfo.tiling = tiling;                           // テクセルの配置を最適な形で再配置するか。再配置するとピクセル単位でのアクセスする事は出来なくなるが、今回はその必要が無いので再配置をしてもらう
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // このImageにテクセル情報が書き込まれる段階で書き込まれたテクセル情報の並びをそのまま維持し続けるかどうか。今回はその必要が無いので、自由にしてもらう
    imageInfo.usage = usage;                             // このImageは転送される側であり、シェーダから色をサンプルできるようにしてほしい
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // グラフィックスコマンドを扱うキューのみからアクセスできれば良い
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;           // マルチサンプリングに関する設定。アタッチメントに使用するImageのみに関連する設定項目。ここではマルチサンプリングはしないように設定する
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void HelloTriangleApplication::generateMipmaps(VkImage image,
                                               VkFormat imageFormat,
                                               int32_t texWidth,
                                               int32_t texHeight,
                                               uint32_t mipLevels)
{
    // Blitするためには、ハードウェアでBlit対象の画像フォーマットのリニアフィルタを扱える必要がある。
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("texture image format does not support linear blitting");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        // i - 1番目のミップマップが埋まるのを待ってから(Blitが終わるのを待ってから)、そのレベルのミップマップをVK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMALに変換する
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        // i - 1番目のミップマップをi番目のミップマップにコピーする
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};                // どこからコピーするか
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1}; // [0]の点からどの程度の範囲をコピーするのか
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};                                                               // どこにコピーするか
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}; // [0]の点からどの程度の範囲にコピーするのか。ミップマップのサイズは半分ずつに減らしていく
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // コピー元になるミップレベルは上の変換でSRC_OPTIMALになっているはず
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // コピー先になるミップレベルはDST_OPTIMAL(textureImageの元の形式)にする
                       1, &blit,
                       VK_FILTER_LINEAR);

        // i - 1番目のミップマップはもうBlitで読み込まれることは無いので、SRC_OPTIMALからシェーダで読み込むためにREAD_ONLY_OPTIMALに変換する
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    // 最後に作成したミップレベルをSRC_OPTIMALからREAD_ONLY_OPTIMALに変換する
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::transitionImageLayout(VkImage image,
                                                     VkFormat format,
                                                     VkImageLayout oldLayout,
                                                     VkImageLayout newLayout,
                                                     uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // image memory barrierを使用してレイアウトの変更ができるらしい
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    // キューファミリーのオーナーシップを転送する際にはこれらのインデックスにキューファミリーのインデックスを入れる
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    // imageのどの範囲のレイアウトを変更するかをsubresourceRangeで指定する。ここでは画像全域を指定している。
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // どのレイアウトからどのレイアウトに変化するか次第でどのステージを待ってどのステージに進むのかを決定する
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        // 作られたばかりの未定義なレイアウトのImageへBuffer上に読み込んだデータを転送するために転送に適した形式に変換する場合
        barrier.srcAccessMask = 0;                            // バリアが完了するのを待つ操作。ここでは特に何も待たない
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // 全ての処理を待った後に行われる処理。転送の書き込み処理

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;   // パイプラインの開始→待つステージは特にない
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // 転送処理ステージ
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // Bufferからデータを転送されたImageをシェーダがサンプリングするのに適した形式に変換する場合
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // バリアが完了するのを待つ操作。転送の書き込み処理
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;    // 全ての処理を待った後に行われる処理。シェーダからの読み込み処理

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;               // パイプラインの開始→待つステージは特にない
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // フラグメントバッファにおける深度テストのステージ
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,      // バリアの前までにパイプラインのどのステージが終わっていないといけないか
        destinationStage, // バリアが解除された後に実行された後に実行されるステージ。バリアはどのステージのために処理を待つのか
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR; // フラグメントの数がテクセルの数より多い(拡大されている)時にどうやってサンプリングするか
    samplerInfo.minFilter = VK_FILTER_LINEAR; // フラグメントの数がテクセルの数より少ない(縮小されている)時にどうやってサンプリングするか
    // テクスチャの範囲外のテクセルをサンプリングする時にどうするか。ここでは同じテクスチャがループして続いているものとして扱っている
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    // 異方性フィルタリングの設定
    samplerInfo.anisotropyEnable = VK_TRUE;
    // 異方性フィルタリングのためにいくつのテクセルからサンプリングすればいいかをデバイスのプロパティから決める
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    // addressModeをBorderにした時に使用されるプロパティ。今回は何でもいい
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    // UV座標系として0~画像幅の座標系を使用するか、0~1の座標系を使用するか。ここでは0~1を使用するよう指定する
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    // シャドウマップ等を作成するのに使う処理。ここでは何も使用しない
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // ミップマップ関連の処理
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels); // カメラから遠ざかっていった時にミップマップを何段階に分けるか

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void HelloTriangleApplication::loadModel()
{
    tinyobj::attrib_t attrib;             // 頂点の座標、法線、UV情報を全て持っているコンテナ
    std::vector<tinyobj::shape_t> shapes; // 一つのファイルに含まれるモデルの配列。モデルを構成する各面の頂点数は任意だが、tinyobjが自動的に全て三角形に変換してくれる
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
    {
        throw std::runtime_error(err);
    }

    // 全ての配列を一つの頂点配列としてまとめて扱う
    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1], // Objファイルは画像下をVの0と扱っているが、Vulkanでは画像上をVの0としているため、上下を反転してやる必要がある。
            };

            vertex.color = {1.0f, 1.0f, 1.0};

            // 頂点vertexと一致する頂点がuniqueVerticesに含まれているかチェックする
            if (uniqueVertices.count(vertex) == 0)
            {
                // 含まれていなかった場合は、vertexをキーとして新たなインデックスを加える
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex); // 今まで存在していなかった頂点のみを頂点バッファに加える
            }

            indices.push_back(uniqueVertices[vertex]); // uniqueVerticesに含まれているvertexと一致する頂点のインデックスを格納する。
        }
    }
}

void HelloTriangleApplication::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // CPUからGPUへデータを転送するための一次バッファであるステージングバッファを作成する
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);
    void *data;
    // VRAMをRAMにマッピングする
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    // マップした領域に頂点情報を流し込む
    memcpy(data, vertices.data(), (size_t)bufferSize);
    // マッピングを解除する
    vkUnmapMemory(device, stagingBufferMemory);

    // 実際にGPUがレンダリング用に使用する頂点バッファを作成する
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // ステージングバッファから転送する先として使用でき、かつ頂点バッファとして使用できるよう指定しておく
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory);

    // GPUとCPUが両方アクセスできるメモリ領域よりもGPUのみが専有的にアクセスできるメモリ領域の方がGPUからのアクセス効率がいい
    // 今回は実行途中で頂点情報がアップデートされることは無いので、わざわざ一次バッファを用意してGPUのみがアクセス可能な頂点バッファにデータをコピーした方が
    // 読み込みが速いので効率が良くなる
    // もしも頂点情報が実行中に変化するのであれば、毎回バッファ間のコピーを行うと無駄なので、CPUとGPUがアクセスできる領域をそのまま頂点バッファにした方がいい
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize); // 一次バッファから頂点バッファに頂点データを移す

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    // インデックスバッファはGPUのみがアクセスできる領域に作成するので、CPUとGPUが両方アクセス可能な一次バッファを作成する
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    void *data;
    // VRAM上の一次バッファをRAMにマッピングし、そこにインデックス情報を流し込む。使い終わった後はアンマップする。
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // インデックスバッファを作成する。
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 indexBuffer,
                 indexBufferMemory);

    // 一次バッファからインデックスバッファに内容をコピーする
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::copyBufferToImage(VkBuffer buffer,
                                                 VkImage image,
                                                 uint32_t width,
                                                 uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    // バッファのどの範囲をコピー対象とするか
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    // 画像のどの範囲にコピーするか
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 現在の画像のレイアウトを指定する
        1,
        &region);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;                             // バッファ全体のサイズ
    bufferInfo.usage = usage;                           // バッファをどう使用するか。ここでは頂点データを保存するために使用することを示している
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // バッファをキューの間で共有できるかどうか。グラフィックキューでしか使用しないので排他的に使用するよう指示している

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // vertexBufferに割り付けるべきメモリのサイズや種類を取得する
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // 頂点バッファに実際に割り付けるメモリのサイズや機能を指定する
    // メモリの機能にはCPU側から見ることが出来て、ホスト側でそのメモリをマッピングした即座に内容が同期される事を指定している
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // メモリの確保
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    // vertexBufferに確保したメモリを割り付ける
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void HelloTriangleApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createUnifomBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // フレーム数分のバッファを用意する
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffers[i],
                     uniformBuffersMemory[i]);
    }

    // MVP行列の情報は毎フレーム変化するため、ここではMapMemory等を使用して書き込むことはしない
}

void HelloTriangleApplication::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    // MVP行列のための設定
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    // テクスチャのための設定
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void HelloTriangleApplication::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // MVP行列を保存してあるバッファ群の各フレームに対応するバッファと、同じフレームに対応するデスクリプタの関連付けを設定する
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // 各フレームに対して、テクスチャのビューとサンプラーを渡す
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // i番目のdescriptorSetsの0番目の要素をシェーダの0番目のバインディングに設定する
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        // デスクリプタの種類を設定する。複数のデスクリプタを与えている場合一気に種類を設定する事が出来るので個数を設定する。今回は1
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        // デスクリプタがどのような形で保持されているか。今回はバッファとして保持されているのでpBufferInfoにbufferInfoを渡す
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        // デスクリプタの情報を更新する。後ろ2つのパラメータは既存のデスクリプタをコピーする際に使用する
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void HelloTriangleApplication::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (int32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    // コマンドバッファはコマンドプールが破棄されたときに自動的に破棄されるのでcleanupで何かする必要は無い
}

void HelloTriangleApplication::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 最初のフレームをレンダリングする時にフェンスがシグナルされていないと無限に待機してしまうので、最初にシグナルが立った状態にしておく

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // コマンドバッファの扱い方を指定できる。ここでは何も指定しない
    beginInfo.pInheritanceInfo = nullptr; // 二次的なコマンドバッファの場合一次のバッファから状態等を継承できる。ここでは関係なので何も継承はしない

    // コマンドバッファへのコマンドの書き込みを開始する
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // このコマンドでどのレンダーパスをどのように扱うかを設定する
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // どのレンダーパスのどのフレームバッファに書き込むか
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    // どこからどの程度のサイズでレンダリングを行うか。ここでは端から端まで指定している
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    // 背景色(何もポリゴンが存在しないところ)を何色に塗るか
    std::array<VkClearValue, 2> clearValues{};
    // カラーバッファと深度バッファを初期化する際に塗りつぶす値を設定する
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0}; // 深度の範囲は0~1。最も遠い点(遠い方のクリッピングプレーンまでの距離)が1なので、深度バッファは最も遠い点までの距離でクリアしている
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // コマンドバッファに今まで設定したレンダーパス関連の情報を流し込むコマンド
    // 最後のフラグは、このコマンドバッファが二次的なものでないことを示している
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // コマンドバッファをグラフィックスパイプラインと結びつけるコマンド
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // ビューポートとシザーの設定を動的に変えられるようにしたので、その値を設定するコマンド
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // 頂点バッファのバインディング
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0}; // 何バイト目から頂点情報を読むか
    // 0番目から1つのバインド情報でvertexBuffersをバインドする
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // currentFrame番目のデスクリプタセットをシェーダのデスクリプタに割り当てる
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, // デスクリプタはGPGPUにも使用できるので、グラフィックスかコンピュートのどちらに使用するかを指定する必要がある
                            pipelineLayout,
                            0,                             // 何番目のデスクリプタから使い始めるか
                            1,                             // 何個のデスクリプタを使うか
                            &descriptorSets[currentFrame], // デスクリプタの配列。ここでは1つしからデスクリプタが無いので、それのポインタを渡している
                            0,                             // ラスト2つのパラメータはダイナミックデスクリプタのために使用する。今は無視してOK
                            nullptr);

    // インデックスバッファを使用しない描画コマンド
    // 第二引数以降の意味は
    // vertexCount : 頂点データの要素数を流し込む
    // instanceCount : 今回はインスタンスドレンダリングを行わないので1を指定する
    // firstVertex : 何番目の頂点からレンダリングを始めるか
    // firstInstance : 何番目のインスタンスからレンダリングを始めるか
    // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    // インデックスバッファを使用する描画コマンド
    // 第三引数まではvmCmdDrawと同じ
    // 第四引数以降の意味は
    // 4 : インデックスバッファ内のオフセット。今回は先頭から使用するので0。1にすると2番目のインデックスから読み込まれる
    // 5 : インデックスバッファの値に対するオフセット。今回はインデックスバッファの値をそのまま使用するので0。1等にするとその値が加わったインデックスの頂点情報を参照する
    // 6 : インスタンスのオフセット。今回はインスタンスドレンダリングを行わないので0
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    // レンダーパスを操作するのを終了する
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // VRAMが対応しているメモリの種類を取得する
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // スワップチェインが対応している画像フォーマットの中から、BGRAが8ビットずつのフォーマットでsRGB色空間の値を扱うものを探して返す
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // 上記の設定に対応していなければとりあえず先頭の要素を返しておく
    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    // MAILBOXが使えるならそれを使う。 MAILBOXは、キューが満杯の時に新しいレンダリング結果が来た時に新しいもので上書きする方式。
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // FIFO_KHRは絶対に対応しているので、MAILBOXがだめならとりあえずこれを使う。
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    // currentExtent.widthがuint32_tの最大値の時、自分で最適な解像度を決定する必要がある。
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height); // ピクセル単位でのウインドウのサイズを取得

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };

        // ウインドウサイズがVulkanが対応する最小サイズから最大サイズの間に収まるように変更する。
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.minImageExtent.height);

        return actualExtent;
    }
}

VkSampleCountFlagBits HelloTriangleApplication::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    // カラーバッファと深度バッファの両方でサポートしているサンプリング数を計算する
    VkSampleCountFlags counts =
        physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    // 64点から2点まで全パターンを点が多い物から順に調べていき、カラーと深度の両方でサポートされているものが見つかったらそれを返す
    // つまり、ここでは使用可能な形式の中で最も点の数が多いものを優先的に探すようにしている。
    std::array<VkSampleCountFlagBits, 6> flags = {
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT};

    for (auto f : flags)
    {
        if (counts & f)
        {
            return f;
        }
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void HelloTriangleApplication::calcMonitorOffset()
{
    // 全モニタの左上の座標を計算する。
    // メインモニタの左上が(0, 0)であることから、
    // offsetが0より大きくなることは無いので、まずは0で初期化する
    monitorLeftOffset = 0, monitorTopOffset = 0;
    for (auto rect : monitorRects)
    {
        if (rect.left < monitorLeftOffset)
        {
            monitorLeftOffset = rect.left;
        }
        if (rect.top < monitorTopOffset)
        {
            monitorTopOffset = rect.top;
        }
    }
}

void HelloTriangleApplication::mainLoop()
{
    // ウインドウが閉じられるまでwhileループを回す
    while (!glfwWindowShouldClose(window))
    {
        // 入力などのイベントを受け取るのに必要らしい
        glfwPollEvents();
        drawFrame();

        calcMonitorOffset();

        for (auto rect : monitorRects)
        {
            BitBlt(dc_workerw, rect.left - monitorLeftOffset, rect.top - monitorTopOffset, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, dc_src, 0, 0, 0x00CC0020);
        }
    }

    // 裏でレンダリング等のプロセスが走っている時にcleanupが呼ばれると厄介なので、
    // 全ての処理が完了するまで待つ
    vkDeviceWaitIdle(device);
}

void HelloTriangleApplication::drawFrame()
{
    // フェンスを利用して前のフレームのレンダリングが完了するのを待つ
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // スワップチェインから画像を取得してくる。画像そのものが返ってくるわけではなく、次に利用可能なswapChainImagesの要素のインデックスが返ってくる
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame], // 処理が終了したらこのセマフォを発火させる
        VK_NULL_HANDLE,                         // ここでフェンスを渡すこともできる。(今回は使わない)
        &imageIndex);

    // OUTOF_DATA_KHR : ウインドウサイズが変わったりして既に作ったスワップチェインが使い物にならない
    // SUBOPTIMAL_KHR : ダイナミックレンジ等のプロパティが変化した。
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);

    vkResetFences(device, 1, &inFlightFences[currentFrame]); // フェンスの状態を次の待機のためにリセットする

    // コマンドバッファにレンダリングのためのコマンドを記録していくために、まずは既存のコマンドをリセットする
    // 第二引数としてフラグを渡すことが出来るが、ここを0にしておくことで、全てデフォルトの動作をさせている
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    // コマンドバッファを実行するための情報を設定する
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // このコマンドバッファのどのステージでどのセマフォを待つのか
    // ここでは、出力画像に色を書き込むのをimageAvailableSemaphoreがシグナルされるまで待つ
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    // 実行するコマンドバッファの数とポインタ
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    // 実行が完了したときにどのセマフォをシグナルするか
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // 第四引数でコマンドバッファが完了したときに立てるフェンスを指定する
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 画像をウインドウに表示するために設定
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 表示するために待つセマフォ
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // どのスワップチェインのどの画像を出力するか
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    // 第一引数は回転する元となる行列
    // 第二引数は回転する角度
    // 第三引数は回転軸
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // 第一引数はカメラの位置
    // 第二引数はカメラが見る位置
    // 第三引数はカメラから見て上方向のベクトル
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // 第一引数は縦方向の視野角
    // 第二引数は画面のアスペクト比
    // 第三引数は手前のクリッピングプレーンまでの距離
    // 第四引数は奥のクリッピングプレーンまでの距離
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    // GLMはOpenGL用に作られており、Vulkanとはクリップ座標系におけるY座標が反転しているので、-1をかけて上下を反転させてVulkanの座標系に揃える
    ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

void HelloTriangleApplication::cleanup()
{
    calcMonitorOffset();

    for (auto rect : monitorRects)
    {
        if (!BitBlt(
                dc_workerw,
                rect.left - monitorLeftOffset, rect.top - monitorTopOffset,
                DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
                dc_workerwCopy, 0, 0, SRCCOPY))
        {
            char buffer[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, 0);
            printf("BitBlt failed %s\n", buffer);
        }
    }
    cleanupSwapChain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    // instanceよりも先にinstanceに依存する機能のクリーンアップを行う
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
    {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    // ウインドウ関連のリソースを削除する
    glfwDestroyWindow(window);
    // GLFW自身が確保しているリソースを解放する
    glfwTerminate();
}

void HelloTriangleApplication::cleanupSwapChain()
{
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
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