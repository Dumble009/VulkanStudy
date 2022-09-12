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
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
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

    VkPhysicalDeviceFeatures deviceFeatures{}; // キューに要求する機能。今は空にしておく

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

    return indices.isComplete() & extensionSupported & swapChainAdequate;
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

    createSwapChain();
    // イメージバッファとフレームバッファのサイズ等はスワップチェインの設定に依存しているので作り直す
    createImageViews();
    createFramebuffers();
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        // 各チャンネルの扱い方を指定する。ここではデフォルトの扱い方を指定している
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // 画像の扱い方とミップマップ、レイヤーの設定を行う
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void HelloTriangleApplication::createRenderPass()
{
    // 色を取り扱うサブパスに渡されるテクスチャの情報を定義する
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;              // フレームバッファに書き込む前に既存の内容をクリアする
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;            // フレームバッファに書き込まれた値を保持し、後でウインドウに表示したりする際に読みだされるようにする
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // ステンシルの値はクリアされてもされなくてもどっちでもいい
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // ステンシルの値は保持されてもされなくてもどっちでもいい
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;         // 最初にフレームバッファに書き込まれているデータの構造は気にしない
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;     // フレームバッファに書き込んだ後のデータはスワップチェインに表示できる形式にする

    // サブパスに渡すテクスチャのメタデータ
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;                                    // どのテクスチャについてか。レンダーパス全体における0番のテクスチャのメタ情報
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 色を取り扱うのに最適となるようにサブパスを並び変えるように指示する

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // このサブパスがレンダリング用のものであることを示す
    subpass.colorAttachmentCount = 1;                            // このサブパスが何枚のテクスチャを持つか
    subpass.pColorAttachments = &colorAttachmentRef;             // サブパスに渡されてくる各テクスチャのメタデータの配列(ポインタ)

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;             // レンダーパス全体で扱うテクスチャの数
    renderPassInfo.pAttachments = &colorAttachment; // レンダーパスで扱うテクスチャ情報の配列
    renderPassInfo.subpassCount = 1;                // レンダーパスに含まれるサブパスの数
    renderPassInfo.pSubpasses = &subpass;           // レンダーパスに含まれるサブパスの配列

    // サブパス間の依存関係を定義する
    VkSubpassDependency dependency{};
    // どのサブパスからどのサブパスの間の依存関係なのか
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 暗黙的に作られる全てのサブパスが開始される前段階のサブパスをsrcとして指定
    dependency.dstSubpass = 0;                   // 0番目のサブパス(上で作ったサブパス)をdstとして指定
    // どのステージのどの操作がどのステージのどの操作を待つのかを指定する
    // 待たれる対象
    // アタッチメントへの出力のステージで出力画像への色の出力が完了する
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0; // ここで操作の種類を指定するが、何も指定しないと全ての操作になる(?)
    // 待つ対象
    // アタッチメントへの出力のステージで、画像への色の出力を行う
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
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
    rasterizer.depthClampEnable = VK_FALSE;         // ニアプレーンとファープレーンの範囲の外にあるポリゴンを内部に収めるか
    rasterizer.rasterizerDiscardEnable = VK_FALSE;  // これがtrueだとラスタライザはポリゴンを無視する
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // ポリゴンをどう描画するか。ここでは三角形として中身を埋めるように指定している
    rasterizer.lineWidth = 1.0f;                    // 線を描画する際の太さ
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;    // カリングの設定。ここではバックカリングすることを指定している
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // 何をもって正面を向いているとするかの指定。頂点が時計回りに並んでいたら正面としている
    rasterizer.depthBiasEnable = VK_FALSE;          // 深度値にバイアスを加えるかの設定。ここでは無効としている
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // AAのマルチサンプリングの設定
    // ここではAAは使用しないように設定している
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

    // シェーダーにグローバルな変数を渡し、動的に挙動を変更したい時に使用する。
    // 今回は使用しない。
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
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
    pipelineInfo.pDepthStencilState = nullptr;
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
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
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
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

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

    // 描画コマンド
    // 第二引数以降の意味は
    // vertexCount : 今回は三角形を表示するので3
    // instanceCount : 今回はインスタンス度レンダリングを行わないので1を指定する
    // firstVertex : 何番目の頂点からレンダリングを始めるか
    // firstInstance : 何番目のインスタンスからレンダリングを始めるか
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // レンダーパスを操作するのを終了する
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
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