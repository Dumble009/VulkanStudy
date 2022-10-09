// ----------STLのinclude----------
#include <iostream>  // 入出力
#include <stdexcept> // 例外処理のexceptionクラスを使用するのに必要
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESSマクロに使用

// STBの実装部をコンパイルするために必要な宣言。
// HelloTriangleApplication.hppの方で宣言してしまうと、実装部が2つ出来てしまうのでコンパイルが上手くいかない。
#define STB_IMAGE_IMPLEMENTATION

// tinyobjloaderの実装部をコンパイルするために必要な宣言。
// STBと同様の理由で、HelloTriangleApplication.hppの方で宣言してしまうと、実装部が2つ出来てしまうのでコンパイルが上手く行かない。
#define TINYOBJLOADER_IMPLEMENTATION

// ----------自作クラスのinclude----------
#include <HelloTriangleApplication.hpp>
#include "fbxtest.h"

int main(void)
{
    /*HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "error finished" << std::endl;
        std::cerr << e.what() << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "success" << std::endl;
    return EXIT_SUCCESS;*/

    helloWorld();

    return EXIT_SUCCESS;
}