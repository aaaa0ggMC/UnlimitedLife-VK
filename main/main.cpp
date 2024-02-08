#include <uen/Graphics.Vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

using namespace uen::Graphics::Vulkan;
using namespace std;

int main(){
    RenderWindow window;
    Application app;

    window.Create(800, 600, "Vulkan window",UEN_OPT_NO_RESIZEABLE);
    app.BindWindow(&window);
    cout << app.CreateApplication("aaaa0ggmc",VK_MAKE_VERSION(1,0,0),VK_MAKE_VERSION(1,0,0),VK_API_VERSION_1_0);

    while(!window.ShouldClose()){
        window.PollEvents();
    }

    window.Destroy();
    app.DestroyApplication();
    return 0;
}
