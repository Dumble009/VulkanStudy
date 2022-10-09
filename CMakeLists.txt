cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(VulkanStudy VERSION 0.1.0)


include(CTest)
enable_testing()

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false RELATIVE "${CMAKE_SOURCE_DIR}" CONFIGURE_DEPENDS "sources/*.cpp")

add_executable(VulkanStudy ${SOURCES})

target_include_directories(VulkanStudy PUBLIC "${CMAKE_SOURCE_DIR}/sources" "C:/opengl/glfw-3.3.8.bin.WIN64/include" "C:/opengl/glm" "C:/VulkanSDK/1.3.216.0/Include" "C:/stb-master" "C:/tiny_obj_loader" "C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.2/include")
target_link_directories(VulkanStudy PUBLIC "C:/opengl/glfw-3.3.8.bin.WIN64/lib-vc2019" "C:/VulkanSDK/1.3.216.0/Lib" "${CMAKE_SOURCE_DIR}/libraries" "C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.2/lib/vs2019/x64/debug")
target_link_libraries(VulkanStudy glfw3 opengl32 vulkan-1 FBXTest libfbxsdk)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
