#include "General.h"
#include <stdlib.h>

using namespace uen;

bool General::inited = false;

int General::InitEnvironment(){
    if(inited)return 0;
    inited = true;
    atexit(General::DestroyEnvironment);
    return glfwInit();
}

void General::DestroyEnvironment(){
    glfwTerminate();
}
