#ifndef GRAPHICS_H_INCLUDED
#define GRAPHICS_H_INCLUDED
#include "UenKernel.h"
#include "General.h"
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define UEN_OPT_NO_RESIZEABLE 0x00000001

#define UENE_SUC 0
#define UENE_ARG_NULL 1
#define UENE_ALREADY_EXIST 2
#define UENE_NO_PHYSICAL_DEVICE_SUPPORT 3
#define UENE_NO_DEVICE_SUPPORT 4
#define UENE_FN_NOT_OVERRIDED 5
#define UENE_WINDOW_NOT_BINDED 6
#define UENE_WINDOW_NOT_CREATED 7
#define UENE_NO_SURFACE 8
#define UENE_NO_PROPER_SWAPCHAIN 9
#define UENE_CANT_CREATE_IMAGE_VIEWS 10

using namespace std;

namespace uen::Graphics::Universal{

    class UENEX RenderWindow{
    public:
        RenderWindow();
        ~RenderWindow();
        //#pragma push_macro("CreateWindow")
        //#undef CreateWindow
        virtual int Create(unsigned int width,unsigned int height,const char * title=NULL,unsigned int option = 0){return UENE_FN_NOT_OVERRIDED;}
        virtual int Create(unsigned int width,unsigned int height,string title,unsigned int option=0){return UENE_FN_NOT_OVERRIDED;}
        //#pragma pop_macro("CreateWindow")
        static void PollEvents();
        bool ShouldClose();
        void Destroy();

        GLFWwindow *window;
        bool hasWindow;
    };

    class UENEX Application{
    public:
        ///Vulkan Interfaces
        virtual int CreateApplication(const char * appName,uint32_t appVersion,uint32_t engineVersion,uint32_t apiVersion,const char * engineName = nullptr){return UENE_FN_NOT_OVERRIDED;}
        virtual void DestroyApplication(){}
        virtual uint32_t getPhysicalDeviceCount(){return UENE_FN_NOT_OVERRIDED;}
        virtual void BindWindow(RenderWindow*){}
    };

}

#define DEF_UTILITY \
struct UENEX Utility{\
    static uint32_t getInstanceExtensionsCount();\
    static std::vector<const char*> getRequiredExtensions();\
};


#endif // GRAPHICS_H_INCLUDED
