#include "VulkanContext.h"

int VulkanContext::InitContext() {
  try {

    CreateInstance();
    // setupDebugMessenger();
    CreateSurface();
    GetPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();

  } catch (const std::runtime_error &e) {
    printf("ERROR: %s\n", e.what());
    return EXIT_FAILURE;
  }
  return 0;
}

void VulkanContext::CleanUp() {

  for (const auto &image : _vcxImages) {
    vkDestroyImageView(_vcxMainDevice.logicalDevice, image.imageView, nullptr);
  }
  vkDestroyDevice(_vcxMainDevice.logicalDevice, nullptr);
  vkDestroySurfaceKHR(_vcxInstance, _vcxSurface, nullptr);
  vkDestroyInstance(_vcxInstance, nullptr);
}

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
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to Create Instance");
  }
}

void VulkanContext::CreateSurface() {
  VkResult result =
      glfwCreateWindowSurface(_vcxInstance, _vcxWindow, nullptr, &_vcxSurface);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed creating surface");
  }
}

void VulkanContext::CreateLogicalDevice() {
  QueueFamilyIndices ind = GetQueueFamilies(_vcxMainDevice.physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> deviceQueueInfos;
  std::set<int> queuesIndex = {ind.graphicsFamily, ind.presentationFamily};

  std::set<int>::iterator it;
  for (it = queuesIndex.begin(); it != queuesIndex.end(); ++it) {
    // queues for logical Device
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = *it;
    queueInfo.queueCount = 1;
    float priority = 1.0f;
    queueInfo.pQueuePriorities = &priority;

    deviceQueueInfos.push_back(queueInfo);
  }

  VkDeviceCreateInfo deviceInfo = {};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.queueCreateInfoCount =
      static_cast<uint32_t>(deviceQueueInfos.size());
  deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
  deviceInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

  VkPhysicalDeviceFeatures deviceFeatures = {};

  deviceInfo.pEnabledFeatures =
      &deviceFeatures; // physical device features, device will use

  if (vkCreateDevice(_vcxMainDevice.physicalDevice, &deviceInfo, nullptr,
                     &_vcxMainDevice.logicalDevice) != VK_SUCCESS)
    throw std::runtime_error("failed to create Logical Device");

  vkGetDeviceQueue(_vcxMainDevice.logicalDevice, ind.graphicsFamily, 0,
                   &_vcxGraphicsQueue);
  vkGetDeviceQueue(_vcxMainDevice.logicalDevice, ind.presentationFamily, 0,
                   &_vcxPresentationQueue);
}

void VulkanContext::CreateSwapChain() {

  // CHoose best swapchainfeature
  SwapChainDetails swChainDetails =
      GetSwapChainDetails(_vcxMainDevice.physicalDevice);

  VkSurfaceFormatKHR format =
      ChooseBestFormatSurface(swChainDetails.imageFormat);
  VkPresentModeKHR mode =
      ChooseBestPresentMode(swChainDetails.presentationsMode);
  VkExtent2D extent = ChooseSwapExtent(swChainDetails.surfaceCapabilities);

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = _vcxSurface;
  createInfo.presentMode = mode;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  if (swChainDetails.surfaceCapabilities.maxImageCount > 0)
    createInfo.minImageCount =
        std::min(swChainDetails.surfaceCapabilities.minImageCount + 1,
                 swChainDetails.surfaceCapabilities
                     .maxImageCount); //+1 for tripple buffering
  else
    createInfo.minImageCount =
        swChainDetails.surfaceCapabilities.minImageCount + 1;
  // VK_IMAGE_USAGE_STORAGE_BIT
  createInfo.imageArrayLayers = 1; // number of layers per each array
  createInfo.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // normally in swapchain always color
  createInfo.preTransform = swChainDetails.surfaceCapabilities.currentTransform;
  createInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Handle blending images with more
                                         // windows
  createInfo.clipped = VK_TRUE;

  // getQueueFam
  QueueFamilyIndices indices = GetQueueFamilies(_vcxMainDevice.physicalDevice);

  // if presentation and graphics queues are different, swapchain must let
  // images be shared between families
  if (indices.graphicsFamily != indices.presentationFamily) {
    uint32_t queueFamilyIndex[] = {(uint32_t)indices.graphicsFamily,
                                   (uint32_t)indices.presentationFamily};
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndex;

  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  // if old one gets destoyed, this one repkaces it exactly the same
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(_vcxMainDevice.logicalDevice, &createInfo, nullptr,
                           &_vcxSwapchain) != VK_SUCCESS)
    throw std::runtime_error("Failed creating Swapchain");

  _vcxSwapChainExtent2D = extent;
  _vcxSwapChainFormat = format.format;

  uint32_t images_count;
  vkGetSwapchainImagesKHR(_vcxMainDevice.logicalDevice, _vcxSwapchain,
                          &images_count, nullptr);

  if (images_count == 0)
    throw std::runtime_error("No images in SwapChain!");

  std::vector<VkImage> swImages(images_count);
  vkGetSwapchainImagesKHR(_vcxMainDevice.logicalDevice, _vcxSwapchain,
                          &images_count, swImages.data());

  for (const auto &image : swImages) {
    SwapChainImage swImage = {};
    swImage.image = image;
    swImage.imageView =
        CreateImageView(image, _vcxSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    _vcxImages.push_back(swImage);
  }
}

void VulkanContext::GetPhysicalDevice() {

  uint32_t deviceCount = 0;
  if (vkEnumeratePhysicalDevices(_vcxInstance, &deviceCount, nullptr) !=
      VK_SUCCESS)
    throw std::runtime_error("Failed during enum Phyiscal Devices");

  if (deviceCount == 0)
    throw std::runtime_error("Devices not support Vulkan");

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(_vcxInstance, &deviceCount,
                             physicalDevices.data());

  for (const auto &device : physicalDevices) {
    if (CheckDeviceSuitable(device)) {
      _vcxMainDevice.physicalDevice = device;
      break;
    }
  }
  if (_vcxMainDevice.physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("No suitable physical Device");
  }
}

QueueFamilyIndices VulkanContext::GetQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices queueFamily;
  uint32_t queueFamilyCount = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queues(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queues.data());
  int i = 0;
  for (const auto &queue : queues) {
    if (queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queueFamily.graphicsFamily = i;
    }
    // check if queue family supports presentation
    VkBool32 presentationSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vcxSurface,
                                         &presentationSupport);
    if (presentationSupport && queue.queueCount > 0)
      queueFamily.presentationFamily = i;
    if (queueFamily.isValid())
      break;
    i++;
  }
  return queueFamily;
}

SwapChainDetails VulkanContext::GetSwapChainDetails(VkPhysicalDevice device) {
  // PER device/surface
  SwapChainDetails swapChainDetails;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, _vcxSurface, &swapChainDetails.surfaceCapabilities);
  uint32_t formatCount;
  uint32_t presentationCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vcxSurface, &formatCount,
                                       nullptr);

  if (formatCount != 0) {
    swapChainDetails.imageFormat.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vcxSurface, &formatCount,
                                         swapChainDetails.imageFormat.data());
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vcxSurface,
                                            &presentationCount, nullptr);

  if (presentationCount != 0) {
    swapChainDetails.presentationsMode.resize(presentationCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, _vcxSurface, &presentationCount,
        swapChainDetails.presentationsMode.data());
  }

  return swapChainDetails;
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

bool VulkanContext::CheckDeviceSuitable(VkPhysicalDevice device) {

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  bool isDiscreteGPU =
      deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  // queue families
  QueueFamilyIndices indices = GetQueueFamilies(device);

  //// swapchain extension
  bool deviceExtensionSupport = CheckDeviceExtensionSupport(device);

  //// see if device swapchain/surface has format/presentation modes.
  bool swapChainValid = false;
  if (deviceExtensionSupport) {
    SwapChainDetails swapChainDetails = GetSwapChainDetails(device);
    swapChainValid = swapChainDetails.imageFormat.size() > 0 &&
                     swapChainDetails.presentationsMode.size() > 0;
  }

  return indices.isValid() && deviceExtensionSupport && swapChainValid;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  if (extensionCount == 0)
    return false;

  std::vector<VkExtensionProperties> extensionProperties(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       extensionProperties.data());

  for (const auto &neededExtensions : deviceExtensions) {
    bool hasExtension = false;
    for (const auto &extension : extensionProperties) {
      if (strcmp(neededExtensions, extension.extensionName) == 0) {
        hasExtension = true;
        break;
      }
    }
    if (!hasExtension)
      return false;
  }

  return true;
}

VkSurfaceFormatKHR VulkanContext::ChooseBestFormatSurface(
    const std::vector<VkSurfaceFormatKHR> &formats) {
  {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
      return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    for (const auto &format : formats) {
      if ((format.format == VK_FORMAT_R8G8B8A8_UNORM ||
           format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return format;
    }
    return formats[0];
  }
}
VkPresentModeKHR VulkanContext::ChooseBestPresentMode(
    const std::vector<VkPresentModeKHR> &presentModes) {
  for (const auto &presentMode : presentModes) {
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return presentMode;
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  // if extent is in numeric limits, extent may vary. If not, its already the
  // size of the window.
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.currentExtent;
  else {
    int width, height;
    glfwGetFramebufferSize(_vcxWindow, &width, &height);

    VkExtent2D newExtent = {};
    newExtent.width = static_cast<uint32_t>(width);
    newExtent.height = static_cast<uint32_t>(height);

    // surface also defines max and min, so make sure no clamping value. so it
    // complies with surface sizes.
    newExtent.width =
        std::max(std::min(capabilities.maxImageExtent.width, newExtent.width),
                 capabilities.minImageExtent.width);
    // surface also defines max and min, so make sure no clamping value. so it
    // complies with surface sizes.
    newExtent.height =
        std::max(std::min(capabilities.maxImageExtent.height, newExtent.height),
                 capabilities.minImageExtent.height);

    return newExtent;
  }
}

VkImageView VulkanContext::CreateImageView(VkImage image, VkFormat format,
                                           VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.format = format;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1; // number of mip map
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount =
      1; // array levels to view. textures or 3d images

  VkImageView imageView;
  if (vkCreateImageView(_vcxMainDevice.logicalDevice, &viewInfo, nullptr,
                        &imageView) != VK_SUCCESS) {
    throw std::runtime_error("Failed Image View Creation");
  }

  return imageView;
}