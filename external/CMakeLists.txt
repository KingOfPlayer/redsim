include(FetchContent)

# List of external libraries
set(EXTERNAL_LIBS
    glfw
    imgui
#    glm
)

# URLs for Git repos
set(glfw_URL "https://github.com/glfw/glfw.git")
set(glm_URL "https://github.com/g-truc/glm.git")
set(imgui_URL "https://github.com/ocornut/imgui.git")

set(glfw_tag "162896e5b9a40dc382c5c438cd12c90a5ff86ddd")
set(glm_tag "a583c59e1616a628b18195869767ea4d6faca5f4")
set(imgui_tag "772cbfd0146350d05702b923354aa677c9c5a4ef")

foreach(lib ${EXTERNAL_LIBS})
    # dynamically get URL variable
    set(repo_url "${${lib}_URL}")

    FetchContent_Declare(
        ${lib}
        GIT_REPOSITORY ${repo_url}
        GIT_TAG ${${lib}_tag} # or a specific tag/commit
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${lib} # where to store repo
        GIT_PROGRESS TRUE
    )

    message(CHECK_START "${lib} Fetching")
    FetchContent_MakeAvailable(${lib})
    
    # if(${lib} STREQUAL "glfw")
    #     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glad)
    # endif()

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

    if(${lib} STREQUAL "glm")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)

        target_link_libraries(redsim PRIVATE glm::glm)
    endif()

    message(CHECK_PASS "Fetched")
endforeach()

# Glad setup
message("glad does not have a CMakeLists.txt, setting up manually.")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glad)
