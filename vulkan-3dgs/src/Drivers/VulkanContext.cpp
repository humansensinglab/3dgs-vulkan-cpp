#include "VulkanContext.h"

int VulkanContext::InitContext() {
  try {

    CreateInstance();
    // setupDebugMessenger();
    // CreateSurface();
    // GetPhysicalDevice();
    // CreateLogicalDevice();

  } catch (const std::runtime_error &e) {
    printf("ERROR: %s\n", e.what());
    return EXIT_FAILURE;
  }
  return 0;
}

void VulkanContext::CleanUp() { vkDestroyInstance(_vcxInstance, nullptr); }

VulkanContext::~VulkanContext() {}

void VulkanContext::CreateInstance() {
  /* if (enableValidationLayers && !checkValidationLayerSupport()) {
     throw std::runtime_error(
         "Application requires validation layer but not available");
   }*/

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan 3DGS API";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t extensionCount;
  std::vector<const char *> instanceExtensions;
  const char **glfwExtensions;

  // instance extensions
  glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

  for (size_t k = 0; k < extensionCount; k++) {
    instanceExtensions.push_back(glfwExtensions[k]);
  }
  if (enableValidationLayers) {
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  if (!CheckInstanceExtensionSupport(&instanceExtensions))
    throw std::runtime_error("Vk instance does not support required Extension");

  createInfo.enabledExtensionCount =
      extensionCount + static_cast<int>(enableValidationLayers);
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  /*if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    ;
    createInfo.ppEnabledLayerNames = validationLayers.data();
    printf("%s", validationLayers[0]);
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

  } else {*/
  {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
  }

  // create Instance
  VkResult res = vkCreateInstance(&createInfo, nullptr, &_vcxInstance);
  if (res != EXIT_SUCCESS) {
    throw std::runtime_error("Failed to Create Instance");
  }
}

bool VulkanContext::CheckInstanceExtensionSupport(
    std::vector<const char *> *checkExtensions) {

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  for (const auto &checkExtension : *checkExtensions) {
    bool hasExtension = false;
    for (const auto &extension : extensions) {
      if (strcmp(checkExtension, extension.extensionName) == 0) {
        hasExtension = true;
        break;
      }
    }
    if (!hasExtension) {
      return false;
    }
  }
  return true;
}
