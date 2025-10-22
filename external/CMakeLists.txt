include(FetchContent)

# List of external libraries
set(EXTERNAL_LIBS
    glfw
    imgui
)

# URLs for Git repos
set(glfw_URL "https://github.com/glfw/glfw.git")
set(imgui_URL "https://github.com/ocornut/imgui.git")

set(glfw_tag "master")
set(imgui_tag "docking")

foreach(lib ${EXTERNAL_LIBS})
    # dynamically get URL variable
    set(repo_url "${${lib}_URL}")

    FetchContent_Declare(
        ${lib}
        GIT_REPOSITORY ${repo_url}
        GIT_TAG ${${lib}_tag} # or a specific tag/commit
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${lib} # where to store repo
    )

    FetchContent_MakeAvailable(${lib})
    
    #imgui specific library setup
    if(${lib} STREQUAL "imgui")
        message(STATUS "${lib} does not have a CMakeLists.txt, setting up manually.")

        add_library(imgui
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/imgui.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/imgui_draw.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/imgui_tables.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/imgui_widgets.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/backends/imgui_impl_glfw.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/backends/imgui_impl_opengl3.cpp
        )

        target_include_directories(imgui PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}
            ${CMAKE_CURRENT_SOURCE_DIR}/${lib}/backends
        )

        target_link_libraries(imgui PUBLIC glfw)
    endif()
endforeach()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glad)
