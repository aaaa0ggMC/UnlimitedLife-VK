#include "Graphics.Vulkan.h"
#include <string.h>
#include <limits>
#include <algorithm>
#include <set>

using namespace uen::Graphics::Vulkan;

const std::vector<const char*> Application::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

RenderWindow::RenderWindow(){

}

int RenderWindow::Create(unsigned int w,unsigned int h,const char * title,unsigned int option){
    if(hasWindow)return UENE_ALREADY_EXIST;
    if(!title)return UENE_ARG_NULL;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if(option & UEN_OPT_NO_RESIZEABLE)glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(w,h,title,nullptr,nullptr);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);///TODO check resize
    hasWindow = true;
    return UENE_SUC;
}

int RenderWindow::Create(unsigned int width,unsigned int height,string title,unsigned int opt){
    return Create(width,height,title.c_str(),opt);
}

uint32_t Utility::getInstanceExtensionsCount(){
    uint32_t ret = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,&ret,nullptr);
    return ret;
}

uint32_t Application::getPhysicalDeviceCount(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    return deviceCount;
}

int Application::CreateApplication(const char * appName,uint32_t appVersion,uint32_t engineVersion,uint32_t apiVersion,const char * engineName){
    if(!win.has_value())return UENE_WINDOW_NOT_BINDED;
    if(!win.value()->hasWindow)return UENE_WINDOW_NOT_CREATED;
    if(hasInstance)return UENE_ALREADY_EXIST;
    if(!appName)return UENE_ARG_NULL;
    VkApplicationInfo info;
    memset(&info,0,sizeof(info));
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.apiVersion = apiVersion;
    info.engineVersion = engineVersion;
    info.pEngineName = engineName?engineName:"No Engine";
    info.applicationVersion = appVersion;
    info.pApplicationName = appName;

    VkInstanceCreateInfo createInfo;
    memset(&createInfo,0,sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &info;

    ///Enable extensions
    ///TODO: handle the validation messages with an interface
    auto extensions = Utility::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if(enableValidationLayers && checkValidationLayerSupport()){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }else createInfo.enabledLayerCount = 0;

    ///Create Instance
    VkResult result = vkCreateInstance(&createInfo,nullptr,&instance);

    if(result)return result;

    ///Check Surface
    if(glfwCreateWindowSurface(instance, win.value()->window, nullptr, &surface) != VK_SUCCESS){
        vkDestroyInstance(instance,nullptr);
        return UENE_NO_SURFACE;
    }

    ///Create Device
    uint32_t deviceCount = Application::getPhysicalDeviceCount();
    if(deviceCount == 0){
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);
        return UENE_NO_DEVICE_SUPPORT;
    }

    ///Get Physical Devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    std::vector<const char *> dvex = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    for (const auto& devicess : devices) {
        bool result = isDeviceSuitable(devicess,dvex,surface);
        if (result) {
            phyDevice = devicess;
            break;
        }
    }


    if(phyDevice == VK_NULL_HANDLE){
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);
        return UENE_NO_PHYSICAL_DEVICE_SUPPORT;
    }

    ///Create Logical Device
    QueueFamilyInd ind = findQueueFamily(phyDevice,surface);
    std::set<uint32_t> uniqueQueueFamilies = {ind.gf.value(), ind.pf.value()};
    float queuePriority = 1.0f;

        ///Queue
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

        ///Features
    VkPhysicalDeviceFeatures deviceFeatures{};

    {
            ///Device Create Info
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

            ///Extensions & Layers
        createInfo.enabledExtensionCount = static_cast<uint32_t>(dvex.size());
        createInfo.ppEnabledExtensionNames = dvex.data();


        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if(vkCreateDevice(phyDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
            vkDestroySurfaceKHR(instance,surface,nullptr);
            vkDestroyInstance(instance,nullptr);
            return UENE_NO_DEVICE_SUPPORT;
        }
    }
    vkGetDeviceQueue(device, ind.gf.value(),0, &gqueue);
    vkGetDeviceQueue(device, ind.pf.value(),0, &pqueue);

    ///Create Swap Chain
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phyDevice,surface);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities,win.value()->window);

    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {ind.gf.value(), ind.pf.value()};

        if (ind.gf != ind.pf) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS){
            vkDestroyDevice(device,nullptr);
            vkDestroySurfaceKHR(instance,surface,nullptr);
            vkDestroyInstance(instance,nullptr);
            return UENE_NO_PROPER_SWAPCHAIN;
        }
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainExtent = extent;
    swapChainImageFormat = surfaceFormat.format;

    ///Create Image Views
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS){
            for(unsigned int j = 0;j < i;++j){
                vkDestroyImageView(device,swapChainImageViews[j],nullptr);
            }
            vkDestroySwapchainKHR(device,swapChain,nullptr);
            vkDestroyDevice(device,nullptr);
            vkDestroySurfaceKHR(instance,surface,nullptr);
            vkDestroyInstance(instance,nullptr);
            return UENE_CANT_CREATE_IMAGE_VIEWS;
        }
    }



    hasInstance = true;

    return VK_SUCCESS;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,GLFWwindow * window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}


bool Application::checkDeviceExtensions(VkPhysicalDevice fd,std::vector<const char *> &deviceExtensions){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(fd, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(fd, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

bool Application::isDeviceSuitable(VkPhysicalDevice devi,std::vector<const char *> ext,VkSurfaceKHR surf){
    //VkPhysicalDeviceProperties deviceProperties;
    //VkPhysicalDeviceFeatures deviceFeatures;
    //vkGetPhysicalDeviceProperties(devi, &deviceProperties);
    //vkGetPhysicalDeviceFeatures(devi, &deviceFeatures);

    bool swapChainAdequate = false;
    if (checkDeviceExtensions(devi,ext)) {
        Application::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(devi,surf);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto at = findQueueFamily(devi,surf);

    return //deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           /*deviceFeatures.geometryShader &&*/ swapChainAdequate && at.gf.has_value() && at.pf.has_value();
}

Application::Application(){
    hasInstance = false;
    phyDevice = VK_NULL_HANDLE;
}

Application::~Application(){if(hasInstance)DestroyApplication();}

void Application::DestroyApplication(){
    if(!hasInstance)return;
    for(unsigned int j = 0;j < swapChainImageViews.size();++j){
        vkDestroyImageView(device,swapChainImageViews[j],nullptr);
    }
    vkDestroySwapchainKHR(device,swapChain,nullptr);
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
    hasInstance = false;
}

bool Application::checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

std::vector<const char*> Utility::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (Application::enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

Application::SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device,VkSurfaceKHR surface) {
    Application::SwapChainSupportDetails details;
    uint32_t formatCount;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool Application::QueueFamilyInd::valid(){
    return this->gf.has_value() && pf.has_value();
}

Application::QueueFamilyInd Application::findQueueFamily(VkPhysicalDevice fd,VkSurfaceKHR surface){
    Application::QueueFamilyInd indices;
    uint32_t queueFamilyCount = 0;
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(fd, &queueFamilyCount, nullptr);
    vkGetPhysicalDeviceQueueFamilyProperties(fd, &queueFamilyCount, queueFamilies.data());

    unsigned int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.gf = i;
            break;
        }
        i++;
    }
    if(!indices.gf.has_value())indices.gf = 0;

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(fd, i, surface, &presentSupport);
    if (presentSupport) {
        indices.pf = i;
    }else indices.pf = 0;

    return indices;
}


bool Application::isInstanceValid(){return hasInstance;}
VkInstance Application::getInstance(){return hasInstance?instance:NULL;}

void Application::BindWindow(RenderWindow * w){
    if(w){
        win = w;
    }
}
