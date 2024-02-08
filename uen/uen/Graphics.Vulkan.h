#ifndef GRAPHICS_VULKAN_H_INCLUDED
#define GRAPHICS_VULKAN_H_INCLUDED
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <set>
#include <optional>
#include "Graphics.h"

using namespace uen::Graphics;

namespace uen{
    namespace Graphics{
        namespace Vulkan{
            class UENEX RenderWindow : public Universal::RenderWindow{
            public:
                RenderWindow();
                int Create(unsigned int width,unsigned int height,const char * title=NULL,unsigned int option = 0) override;
                int Create(unsigned int width,unsigned int height,string title,unsigned int option = 0) override;

            };

            class UENEX Application : public Universal::Application{
            public:
                struct QueueFamilyInd{
                    std::optional<uint32_t> gf;//graphics family
                    std::optional<uint32_t> pf;//present family
                    bool valid();
                };
                struct SwapChainSupportDetails {
                    VkSurfaceCapabilitiesKHR capabilities;
                    std::vector<VkSurfaceFormatKHR> formats;
                    std::vector<VkPresentModeKHR> presentModes;
                };

                Application();
                ~Application();
                int CreateApplication(const char * appName,uint32_t appVersion,uint32_t engineVersion,uint32_t apiVersion,const char * engineName = nullptr) override;
                void DestroyApplication() override;
                uint32_t getPhysicalDeviceCount() override;
                bool isInstanceValid();
                void BindWindow(RenderWindow*);
                VkInstance getInstance();

                static bool checkValidationLayerSupport();
                static bool isDeviceSuitable(VkPhysicalDevice devi,std::vector<const char *>,VkSurfaceKHR);
                static QueueFamilyInd findQueueFamily(VkPhysicalDevice fdev,VkSurfaceKHR su);
                static bool checkDeviceExtensions(VkPhysicalDevice fd,std::vector<const char *> &deviceExtensions);
                static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR);
                static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
                static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
                static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,GLFWwindow * win);

            private:
                friend class Utility;

                VkInstance instance;
                bool hasInstance;

                VkPhysicalDevice phyDevice;
                VkDevice device;
                VkQueue gqueue;
                VkQueue pqueue;
                VkSurfaceKHR surface;
                VkSwapchainKHR swapChain;
                std::vector<VkImageView> swapChainImageViews;
                std::vector<VkImage> swapChainImages;
                VkFormat swapChainImageFormat;
                VkExtent2D swapChainExtent;

                std::optional<RenderWindow*> win;

                const static std::vector<const char*> validationLayers;

                #ifdef NDEBUG
                    static const bool enableValidationLayers = false;
                #else
                    static const bool enableValidationLayers = true;
                #endif
            };

            DEF_UTILITY;
        }
    }
}

#endif // GRAPHICS_VULKAN_H_INCLUDED
