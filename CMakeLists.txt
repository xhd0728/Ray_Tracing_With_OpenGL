cmake_minimum_required(VERSION 3.17)
project(HEU_EASY_OPENGL)

find_package(OpenGL REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# 执行编译命令
add_executable(HEU_EASY_OPENGL RayTracing.cpp glad.c)
# # 添加头文件
set(OPT_H /usr/include)
target_include_directories(HEU_EASY_OPENGL PUBLIC ${OPT_H})
target_include_directories(HEU_EASY_OPENGL PUBLIC ${OPENGL_INCLUDE_DIR})
# # 添加目标链接
set(GLEW_LINK /usr/lib/x86_64-linux-gnu/libGLEW.so)
set(GLFW_LINK /usr/lib/x86_64-linux-gnu/libglfw.so)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
find_package(OpenGL REQUIRED)
add_definitions(${OPENGL_DEFINITIONS})
find_package(X11 REQUIRED)
set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
target_link_libraries(HEU_EASY_OPENGL ${GLEW_LINK} ${GLFW_LINK} )
target_link_libraries(HEU_EASY_OPENGL ${OPENGL_LIBRARIES})
