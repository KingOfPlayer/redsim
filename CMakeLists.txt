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

target_precompile_headers(redsim PRIVATE
    <string>
    <vector>
    <thread>
    <memory>
    <iostream>
    <algorithm>
    <CGAL/Exact_predicates_inexact_constructions_kernel.h>
    <CGAL/Polygon_2.h>
    <CGAL/Polygon_set_2.h>
    <CGAL/create_straight_skeleton_2.h>
    <CGAL/create_offset_polygons_2.h>
    <CGAL/Polygon_mesh_processing/extrude.h>
    <CGAL/Polygon_mesh_processing/corefinement.h>
    <CGAL/Boolean_set_operations_2.h>
    <CGAL/Surface_mesh.h>
    <CGAL/Triangulation_face_base_with_info_2.h>
    <CGAL/Polygon_mesh_processing/triangulate_faces.h>
    <CGAL/Constrained_Delaunay_triangulation_2.h>
    <CGAL/mark_domain_in_triangulation.h>
    <CGAL/Polygon_mesh_processing/remesh.h>
    <CGAL/Polygon_mesh_processing/detect_features.h>
    <CGAL/Polyhedron_3.h>
    <CGAL/Nef_polyhedron_3.h>
    <CGAL/boost/graph/convert_nef_polyhedron_to_polygon_mesh.h>
    <CGAL/Polyhedral_mesh_domain_with_features_3.h>
    <CGAL/Mesh_triangulation_3.h>
    <CGAL/Mesh_complex_3_in_triangulation_3.h>
    <CGAL/Mesh_criteria_3.h>
    <CGAL/make_mesh_3.h>
    <CGAL/tetrahedral_remeshing.h>
)

target_include_directories(redsim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_BINARY_DIR}/generated
#    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    #${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(redsim PRIVATE OpenGL::GL glfw glad imgui ImViewGuizmo glm::glm X11 CGAL::CGAL)