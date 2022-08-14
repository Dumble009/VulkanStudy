// ----------STLのinclude----------
#include <iostream>
#include <stdexcept>
#include <cstdlib>
// ----------自作クラスのinclude----------
#include <HelloTriangleApplication.hpp>

int main(void)
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}