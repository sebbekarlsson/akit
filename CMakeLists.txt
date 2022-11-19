cmake_minimum_required(VERSION 3.20)

project(akit)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB sourcefiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_executable(akit_e ${sourcefiles})
add_library(akit SHARED ${sourcefiles})
add_library(akit_static STATIC ${sourcefiles})


target_compile_options(akit_e PUBLIC -g -Wall)

include(FetchContent)

if (NOT TARGET vec3_static)
  FetchContent_Declare(
    vec3_static
    URL /home/ianertson/workspace/vec3
    )
  FetchContent_MakeAvailable(vec3_static)
endif()

if (NOT TARGET date_static)
  FetchContent_Declare(
    date_static
    URL /home/ianertson/workspace/date
    )
  FetchContent_MakeAvailable(date_static)
endif()

if (NOT TARGET hashmap_static)
  FetchContent_Declare(
    hashmap_static
    URL /home/ianertson/workspace/hashmap
  )
  FetchContent_MakeAvailable(hashmap_static)
endif()

  if (NOT TARGET hashy_static)

    FetchContent_Declare(
      hashy_static
      URL /home/ianertson/workspace/hashy
      # GIT_REPOSITORY https://github.com/sebbekarlsson/hashmap.git
      # GIT_TAG master
      )

    FetchContent_MakeAvailable(hashy_static)
  endif()

if (NOT TARGET waves_static)
  FetchContent_Declare(
  waves_static
  URL /home/ianertson/workspace/waves
  )
FetchContent_MakeAvailable(waves_static)
endif()

target_compile_options(waves_static PUBLIC -fPIC)

function (akit_init TARGET_NAME)
  target_link_libraries(
    ${TARGET_NAME}
    vec3_static
    hashmap_static
    hashy_static
    waves_static
    date_static
    cglm
    asound
    pthread
  )
endfunction()


akit_init(akit)
akit_init(akit_static)
akit_init(akit_e)

target_include_directories(
  akit
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)



target_include_directories(
  akit_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)
