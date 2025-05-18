cmake_minimum_required(VERSION 3.10...4.0)

project(NHF VERSION 0.1)

set(CMAKE_CXX_STANDARD 14) # 14^ is required for gtest
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

set(CMAKE_CXX_FLAGS "-O3")

set(OPENGL ON CACHE BOOL "Compile with OpenGL" FORCE)
# set(GTEST OFF CACHE BOOL "Compile test code" FORCE)

# if (NOT MSVC)
#     add_compile_options(-pedantic -Wall -Werror -D_MEMTRACE)
# endif()

# ---

include(FetchContent)
set(DOWNLOAD_EXTRACT_TIMESTAMP true)

# if(GTEST)
#     # GoogleTest
#     FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

#     set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#     FetchContent_MakeAvailable(googletest)
# endif() # GTEST

if(OPENGL)
    # GLFW and glad generator deps
    FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG master)
    FetchContent_Declare(glfw3 GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)
    FetchContent_Declare(glm GIT_REPOSITORY	https://github.com/g-truc/glm.git GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    
    FetchContent_MakeAvailable(glfw3)
    FetchContent_MakeAvailable(glad)
    FetchContent_MakeAvailable(glm)
endif() # OPENGL

set(SOURCES
    src/main.cpp
    src/gfx.cpp 
    src/map.cpp
    src/lib.cpp
    src/geo.cpp
    src/io.cpp 
    src/weights.cpp
    src/network.cpp
)

set(EXTERNAL 
    external/memtrace.cpp
    external/memtrace.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(OPENGL) # link with glfw and glad
    target_link_libraries(${PROJECT_NAME}
        PUBLIC glfw
        PUBLIC glad
    )

    # windows-only
    # target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
endif() # OPENGL

# ---

target_compile_definitions(${PROJECT_NAME} PRIVATE MEMTRACE)

if (MSVC)
    # ignore comparison of int and size_t
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4267)
endif()

# ---

# if(GTEST)
#     enable_testing()
#     set(TEST_NAME "NHF_test")

#     add_executable(
#         ${TEST_NAME}
#         tests/test.cpp
#     )

#     target_link_libraries(
#         ${TEST_NAME}
#         GTest::gtest_main
#     )
    
#     include(GoogleTest)
#     gtest_discover_tests(${TEST_NAME})    
# endif() # GTEST
