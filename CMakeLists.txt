cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)

project(redsim VERSION 0.0.0 LANGUAGES CXX)

add_subdirectory(external)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glad)

cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

find_package(glm REQUIRED)

find_package(CGAL REQUIRED)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/GLFW)

#configure_file(${CMAKE_SOURCE_DIR}/include/constants.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/constants.h)

file(GLOB_RECURSE SOURCES "src/*.cpp")

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

# Optionally include the GLFW headers (if needed explicitly)
#target_include_directories(redsim PRIVATE external/glfw/include)