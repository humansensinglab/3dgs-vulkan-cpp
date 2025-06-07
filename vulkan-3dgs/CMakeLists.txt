option(BUILD_PYTHON_BINDING "Build Python binding" OFF)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "headers/*.h")

list(FILTER SOURCES EXCLUDE REGEX ".*bindings\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*entrypoint_python\\.cpp$")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()


source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" 
             PREFIX "Source Files" 
             FILES ${SOURCES})
             
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/headers" 
             PREFIX "Header Files" 
             FILES ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/third-party/GLFW/include
    ${CMAKE_SOURCE_DIR}/third-party/GLM
    ${Vulkan_INCLUDE_DIRS}
)

if(WIN32)
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/third-party/GLFW/lib-vc2022
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${Vulkan_LIBRARIES}
        glfw3.lib
        imgui
    )
else()
    find_package(glfw3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${Vulkan_LIBRARIES}
        glfw
        imgui
    )
endif()


if(BUILD_PYTHON_BINDING)
    file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")
    list(FILTER CORE_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
    list(FILTER CORE_SOURCES EXCLUDE REGEX ".*bindings\\.cpp$")
    
    add_library(vulkan_3dgs_core STATIC ${CORE_SOURCES} ${HEADERS})
    
    target_include_directories(vulkan_3dgs_core PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
        ${CMAKE_SOURCE_DIR}/third-party/GLFW/include
        ${CMAKE_SOURCE_DIR}/third-party/GLM
        ${Vulkan_INCLUDE_DIRS}
    )
    
    if(WIN32)
        target_compile_definitions(vulkan_3dgs_core PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
        target_link_directories(vulkan_3dgs_core PUBLIC
            ${CMAKE_SOURCE_DIR}/third-party/GLFW/lib-vc2022
        )
        target_link_libraries(vulkan_3dgs_core PUBLIC
            ${Vulkan_LIBRARIES}
            glfw3.lib
            imgui
        )
    elseif(APPLE)
        target_compile_definitions(vulkan_3dgs_core PRIVATE VK_USE_PLATFORM_MACOS_MVK)
        target_link_libraries(vulkan_3dgs_core PUBLIC
            ${Vulkan_LIBRARIES}
            glfw
            imgui
        )
    elseif(UNIX)
        target_compile_definitions(vulkan_3dgs_core PRIVATE VK_USE_PLATFORM_XLIB_KHR)
        target_link_libraries(vulkan_3dgs_core PUBLIC
            ${Vulkan_LIBRARIES}
            glfw
            imgui
        )
    endif()
    
    # Python module
    pybind11_add_module(vulkan_3dgs_py src/bindings.cpp)
    target_link_libraries(vulkan_3dgs_py PUBLIC vulkan_3dgs_core)
    
    target_include_directories(vulkan_3dgs_py PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
    )
endif()


if(WIN32)
    set(SHADER_COMPILE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/compile.bat")
elseif(APPLE) 
    set(SHADER_COMPILE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/compile_shaders.sh")
endif()

# Create custom target to compile shaders
add_custom_target(compile_shaders ALL
    COMMAND ${SHADER_COMPILE_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts
    COMMENT "Compiling shaders for ${CMAKE_SYSTEM_NAME}..."
    VERBATIM
)


add_dependencies(vulkan-3dgs compile_shaders)

# Copy compiled shaders to output directory  
add_custom_command(TARGET vulkan-3dgs POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders       
    $<TARGET_FILE_DIR:vulkan-3dgs>/Shaders
    COMMENT "Copying shaders to output directory"
)
