project('ToxBlinkenwall', 'c', license: 'GPL')

compiler = meson.get_compiler('c')

sub_toxcore = subproject('toxcore')
toxcore = sub_toxcore.get_variable('toxcore')
toxav = sub_toxcore.get_variable('toxav')


#toxcore = dependency('libtoxcore')
#toxav = dependency('libtoxav')
sodium = dependency('libsodium')
#ao = dependency('ao')
alsa = dependency('alsa')

# TODO: A switch between X11 and rpi platform
gl = dependency('brcmglesv2')
#egl = dependency('brcmegl')
#gl = dependency('glesv2')
#egl = dependency('egl')
#x11 = dependency('x11')

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
	dependencies: [toxcore, toxav, sodium, alsa, gl, v4lconvert, v4l2],
	c_args: '-DRPI_NO_X'
)

executable('print_cam_formats', 'print_cam_formats.c',
	dependencies: v4l2)
