#ifndef GENERAL_H_INCLUDED
#define GENERAL_H_INCLUDED
#include <GLFW/glfw3.h>
#include <iostream>

namespace uen{
    struct General{
        static int InitEnvironment();
        static void DestroyEnvironment();
        static bool inited;
    };
}

#endif // GENERAL_H_INCLUDED
