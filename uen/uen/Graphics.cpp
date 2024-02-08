#include "Graphics.h"

using namespace uen::Graphics::Universal;


void RenderWindow::PollEvents(){
    glfwPollEvents();
}

bool RenderWindow::ShouldClose(){
    return glfwWindowShouldClose(window);
}

RenderWindow::RenderWindow(){
    hasWindow = false;
    uen::General::InitEnvironment();
}

RenderWindow::~RenderWindow(){
    Destroy();
}

void RenderWindow::Destroy(){
    if(hasWindow){
        hasWindow = false;
        glfwDestroyWindow(window);
    }
}
