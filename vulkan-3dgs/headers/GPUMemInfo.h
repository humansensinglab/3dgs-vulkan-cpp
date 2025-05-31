#pragma once
#include "vulkan/vulkan.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class GPUMemoryInfo {
public:
  struct MemoryStats {
    uint64_t totalMemory = 0;
    uint64_t usedMemory = 0;
    uint64_t availableMemory = 0;
    bool hasDetailedInfo = false;
  };

  static MemoryStats GetCompleteMemoryStats(VkPhysicalDevice physicalDevice,
                                            VkDevice device) {
    MemoryStats stats;

    // First try with memory budget extension
    if (HasMemoryBudgetExtension(physicalDevice)) {
      VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps = {};
      budgetProps.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;

      VkPhysicalDeviceMemoryProperties2 memProps2 = {};
      memProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
      memProps2.pNext = &budgetProps;

      vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memProps2);

      // Find device local heap
      for (uint32_t i = 0; i < memProps2.memoryProperties.memoryHeapCount;
           i++) {
        if (memProps2.memoryProperties.memoryHeaps[i].flags &
            VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          stats.totalMemory = budgetProps.heapBudget[i];
          stats.usedMemory = budgetProps.heapUsage[i];
          stats.availableMemory = stats.totalMemory - stats.usedMemory;
          stats.hasDetailedInfo = true;
          break;
        }
      }
    } else {

      VkPhysicalDeviceMemoryProperties memProps;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

      for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          stats.totalMemory += memProps.memoryHeaps[i].size;
        }
      }
      stats.hasDetailedInfo = false;
    }

    return stats;
  }

  static std::string FormatBytes(uint64_t bytes) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 4) {
      size /= 1024.0;
      unitIndex++;
    }

    if (unitIndex == 0) {
      return std::to_string(static_cast<int>(size)) + " " + units[unitIndex];
    } else {

      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%.1f %s", size, units[unitIndex]);
      return std::string(buffer);
    }
  }

private:
  static bool HasMemoryBudgetExtension(VkPhysicalDevice physicalDevice) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                         &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                         &extensionCount, extensions.data());

    for (const auto &ext : extensions) {
      if (strcmp(ext.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
        return true;
      }
    }
    return false;
  }
};