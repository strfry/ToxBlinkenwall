cmake_minimum_required(VERSION 2.8)

project(ToxBlinkenwall)

set(MUST_BUILD_TOXAV ON CACHE BOOL "Force ToxAV")
#set(MUST_BUILD_TOXAV "" ON CACHE)

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
	c-toxcore/toxcore
	c-toxcore/toxav
	)

# Do proper discovery
set(EXTRA_LIBRARIES
	asound
	v4lconvert
	GLESv2
	EGL
	X11
	)

target_link_modules(toxblinkenwall
	m pthread
	${EXTRA_LIBRARIES}
	toxcore)

