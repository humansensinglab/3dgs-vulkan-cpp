cmake_minimum_required(VERSION 3.10)
project(vulkan-3dgs)

set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
elseif(APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path;/usr/local/lib")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
else()
    add_compile_definitions(NDEBUG)
endif()

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third-party/imgui)
if(BUILD_PYTHON_BINDING)
    add_subdirectory(third-party/pybind11)
endif()

if(NOT EXISTS ${IMGUI_DIR})
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. Did you add the submodule?")
endif()

file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/third-party/GLFW/include
)


if(WIN32)
    target_compile_definitions(imgui PUBLIC VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
elseif(APPLE)
    target_compile_definitions(imgui PUBLIC VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(imgui PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vulkan-3dgs)
add_subdirectory(vulkan-3dgs)