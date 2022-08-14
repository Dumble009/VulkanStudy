#pragma once
#include <vulkan/vulkan.h>

class HelloTriangleApplication
{
public:
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();
};