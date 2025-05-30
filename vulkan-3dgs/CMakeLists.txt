file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "headers/*.h")

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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders"
    COMMENT "Copying shader files..."
)