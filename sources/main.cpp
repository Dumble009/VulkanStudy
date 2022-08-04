#include <iostream>
#include "add.h"
#include "sub/sub.h"

int main()
{
    std::cout << "Hello World! " << add(1, 0) << ":" << sub(2, 2) << std::endl;

    return 0;
}