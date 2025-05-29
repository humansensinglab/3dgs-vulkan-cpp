cmake_minimum_required(VERSION 3.10)
project(vulkan-3dgs)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
elseif(APPLE)
    # Help macOS find MoltenVK at runtime
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path;/usr/local/lib")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(DEBUG)
else()
    add_compile_definitions(NDEBUG)
endif()

add_subdirectory(vulkan-3dgs)