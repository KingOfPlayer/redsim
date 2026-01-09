cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

project(redsim VERSION 0.0.0 LANGUAGES CXX)

add_subdirectory(external)

cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

find_package(glm REQUIRED)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
find_package(CGAL REQUIRED)

#configure_file(${CMAKE_SOURCE_DIR}/include/constants.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/constants.h)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(redsim
    ${SOURCES}
)

target_include_directories(redsim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_BINARY_DIR}/generated
#    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    #${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(redsim PRIVATE OpenGL::GL glfw glad imgui glm::glm X11 CGAL::CGAL)