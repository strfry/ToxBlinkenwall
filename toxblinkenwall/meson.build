project('ToxBlinkenwall', 'c', license: 'GPL')

compiler = meson.get_compiler('c')

toxcore = dependency('libtoxcore')
toxav = dependency('libtoxav')
sodium = dependency('libsodium')
#ao = dependency('ao')
alsa = dependency('alsa')
gl = dependency('glesv2')
egl = dependency('egl')

# TODO: A switch between X11 and rpi platform
x11 = dependency('x11')

v4lconvert = compiler.find_library('v4lconvert')
v4l2 = compiler.find_library('v4l2')

sources = [
#	'a.c',
	'rb.c',
	'ringbuf.c',
	'toxblinkenwall.c',
	'openGL/esShader.c',
	'openGL/esUtil.c'

]

executable('ToxBlinkenWall', sources,
	dependencies: [toxcore, toxav, sodium, alsa, gl, egl, x11, v4lconvert, v4l2],
)

executable('print_cam_formats', 'print_cam_formats.c',
	dependencies: v4l2)
