cmake_minimum_required(VERSION 2.8)

project(ToxBlinkenwall)

set(MUST_BUILD_TOXAV ON CACHE BOOL "Force ToxAV")
#set(MUST_BUILD_TOXAV "" ON CACHE)

## Git submodule auto checkout
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
	message(STATUS "Submodule update")
	execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE GIT_SUBMOD_RESULT)
	if(NOT GIT_SUBMOD_RESULT EQUAL "0")
	    message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
	endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/c-toxcore/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(c-toxcore)


add_executable(toxblinkenwall
	toxblinkenwall/rb.c
	toxblinkenwall/toxblinkenwall.c
	toxblinkenwall/openGL/esShader.c # OpenGL option specific
	toxblinkenwall/openGL/esUtil.c
	)

# should be target option?
target_compile_definitions(toxblinkenwall PRIVATE
	-DHAVE_OUTPUT_OPENGL)

target_include_directories(toxblinkenwall PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/c-toxcore/toxcore
	${CMAKE_CURRENT_SOURCE_DIR}/c-toxcore/toxav
	)

# Do proper discovery
set(EXTRA_LIBRARIES
	asound
	v4lconvert
	GLESv2
	EGL
	X11
	)

target_link_libraries(toxblinkenwall LINK_PUBLIC
	m pthread
	${toxcore_LINK_MODULES}
	toxcore_static
	${EXTRA_LIBRARIES}
	)
