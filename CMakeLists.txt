cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(VulkanStudy VERSION 0.1.0)


include(CTest)
enable_testing()

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false RELATIVE "${CMAKE_SOURCE_DIR}" CONFIGURE_DEPENDS "sources/*.cpp")

add_executable(VulkanStudy ${SOURCES})

target_include_directories(VulkanStudy PUBLIC "${CMAKE_SOURCE_DIR}/headers" "C:/opengl/glfw-3.3.8.bin.WIN64/include" "C:/opengl/glm")
target_link_directories(VulkanStudy PUBLIC "C:/opengl/glfw-3.3.8.bin.WIN64/lib-mingw-w64/")
target_link_libraries(VulkanStudy glfw3 opengl32)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
