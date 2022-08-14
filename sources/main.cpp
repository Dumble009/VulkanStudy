// ----------STLのinclude----------
#include <iostream>  // 入出力
#include <stdexcept> // 例外処理のexceptionクラスを使用するのに必要
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESSマクロに使用
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