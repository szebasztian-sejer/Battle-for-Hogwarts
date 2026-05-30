cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

#! ! ! ! ! ! !
#set this to ON instead of OFF to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#delete the out folder after changing if visual studio doesn recognize the change
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

if(MSVC) 
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
	add_compile_options(-mavx2 -mfma) # make sure SIMD optimizations take place, but on linux and other platforms
endif()


project(hogwarts)

if(ANDROID)
	find_package(raylib REQUIRED)
else()
	# Ensure raylib is built as static libraries
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as static libraries" FORCE)
#add raylib, and here you would add other libraries if needed!
	add_subdirectory(thirdparty/raylib-5.0)
endif()

add_subdirectory(thirdparty/enet-1.3.18)

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


add_executable("${CMAKE_PROJECT_NAME}")

add_executable(server_test src/main/server.cpp)
add_executable(client_test src/main/client.cpp)

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)



if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions(client_test PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		#YOU CAN REMOVE THE CONSOLE WITH THIS LINE!
		set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	endif()

elseif(MINGW) # Windows / MinGW GCC/Clang
	if(PRODUCTION_BUILD)
		# Remove console on Windows under MinGW
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
			-mwindows
		)
	endif()

elseif(APPLE OR UNIX) # macOS / Linux
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -s)
	endif()
endif()

target_include_directories("${CMAKE_PROJECT_NAME}"  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")
target_include_directories("${CMAKE_PROJECT_NAME}"  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/main")
target_include_directories("${CMAKE_PROJECT_NAME}"  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/game")
target_include_directories("${CMAKE_PROJECT_NAME}"  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include")


target_include_directories(server_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")
target_include_directories(server_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/main")
target_include_directories(server_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/game")
target_include_directories(server_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include")

target_include_directories(client_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")
target_include_directories(client_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/main")
target_include_directories(client_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/game")
target_include_directories(client_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enet-1.3.18/include")


target_link_libraries(server_test PRIVATE enet)
target_link_libraries(client_test PRIVATE enet)

if(ANDROID)
	#target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE raylib)
else()
# Link raylib library statically, Here wou would add other libraries!
#target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE raylib_static enet)
endif()
if(WIN32)
	#target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE ws2_32 winmm)
	target_link_libraries(server_test PRIVATE ws2_32 winmm)
    target_link_libraries(client_test PRIVATE ws2_32 winmm)
endif()



