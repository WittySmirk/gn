cmake_minimum_required(VERSION 3.10)

project(gn CXX)

set(CMAKE_CXX_STANDARD 20)
set(FFMPEG_DIR "C:/Users/wyatt/OneDrive/Desktop/ffmpeg")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

set(SDLTTF_VENDORED ON)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

add_executable(gn ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE  
    ${FFMPEG_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${FFMPEG_DIR}/lib/avcodec.lib
    ${FFMPEG_DIR}/lib/avformat.lib
    ${FFMPEG_DIR}/lib/avutil.lib
    ${FFMPEG_DIR}/lib/avdevice.lib
    ${FFMPEG_DIR}/lib/swscale.lib
    ${FFMPEG_DIR}/lib/swresample.lib
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    opengl32
)