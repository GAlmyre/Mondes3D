project(mds3d_td4)

cmake_minimum_required(VERSION 3.2.0)

add_subdirectory(ext/glfw)
add_subdirectory(ext/glbinding)

include_directories(ext/glfw/include)
include_directories(ext/glbinding/include)
include_directories(ext/eigen3)
include_directories(ext/)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SRC_FILES
    ext/ObjFormat/ObjFormat.cpp
    ext/ObjFormat/ObjUtil.cpp
    src/main.cpp
    src/viewer.cpp
    src/viewer.h
    src/shader.cpp
    src/shader.h
    src/opengl.h
    src/camera.h
    src/camera.cpp
    src/mesh.h
    src/mesh.cpp
    src/trackball.cpp
    src/trackball.h
)

add_definitions(-DDATA_DIR="${PROJECT_SOURCE_DIR}/data")

add_executable(mds3d_td4 ${SRC_FILES})

target_link_libraries(mds3d_td4 glfw ${GLFW_LIBRARIES} glbinding)
