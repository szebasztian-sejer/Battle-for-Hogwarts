cmake_minimum_required(VERSION 3.16)

project(hogwarts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>")
    add_compile_options(/arch:AVX2)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
    add_compile_options(-mavx2 -mfma)
endif()

# --------------------
# Third-party libraries
# --------------------

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

add_subdirectory(thirdparty/raylib-5.0)
add_subdirectory(thirdparty/enet-1.3.18)

# --------------------
# Source groups
# --------------------

file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shared/*.cpp"
)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp"
)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp"
)

# --------------------
# Shared code
# --------------------

add_library(hogwarts_shared STATIC
    ${SHARED_SOURCES}
)

target_include_directories(hogwarts_shared PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shared"
)

target_compile_definitions(hogwarts_shared PUBLIC
    $<$<BOOL:${PRODUCTION_BUILD}>:PRODUCTION_BUILD=1>
    $<$<NOT:$<BOOL:${PRODUCTION_BUILD}>>:PRODUCTION_BUILD=0>
)

if(PRODUCTION_BUILD)
    target_compile_definitions(hogwarts_shared PUBLIC
        RESOURCES_PATH="./resources/"
    )
else()
    target_compile_definitions(hogwarts_shared PUBLIC
        RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    )
endif()

# --------------------
# Client executable
# --------------------

add_executable(hogwarts_client
    ${CLIENT_SOURCES}
)

target_include_directories(hogwarts_client PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include"
)

target_link_libraries(hogwarts_client PRIVATE
    hogwarts_shared
    raylib_static
    enet
)

# --------------------
# Server executable
# --------------------

add_executable(hogwarts_server
    ${SERVER_SOURCES}
)

target_include_directories(hogwarts_server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include"
)

target_link_libraries(hogwarts_server PRIVATE
    hogwarts_shared
    enet
)

# --------------------
# Windows-specific setup
# --------------------

if(WIN32)

    #need these flags in order to not conflict with raylib...
    target_compile_definitions(hogwarts_client PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        NOGDI
        NOUSER
        _CRT_SECURE_NO_WARNINGS
    )

    target_compile_definitions(hogwarts_server PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )

    target_link_libraries(hogwarts_client PRIVATE
        ws2_32
        winmm
        opengl32
        gdi32
    )

    target_link_libraries(hogwarts_server PRIVATE
        ws2_32
        winmm
    )
endif()

# --------------------
# Production options
# --------------------

if(PRODUCTION_BUILD)
    if(MSVC)
        set_target_properties(hogwarts_client PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
    elseif(MINGW)
        target_link_options(hogwarts_client PRIVATE -mwindows)
    elseif(APPLE OR UNIX)
        target_link_options(hogwarts_client PRIVATE -s)
        target_link_options(hogwarts_server PRIVATE -s)
    endif()
endif()