project('ToxBlinkenwall', 'c', license: 'GPL')

compiler = meson.get_compiler('c')

raspberry_pi = dependency('brcmegl', required: false).found()

toxcore_sub = subproject('toxcore')

toxcore = toxcore_sub.get_variable('toxcore')
toxav = toxcore_sub.get_variable('toxav')

alsa = dependency('alsa')
v4lconvert = compiler.find_library('v4lconvert')
v4l2 = compiler.find_library('v4l2')

deps = [toxcore, toxav, alsa, v4lconvert, v4l2]
c_args = []

if (raspberry_pi)
	deps += dependency('brcmglesv2')
	deps += dependency('brcmegl')
	c_args += '-DRPI_NO_X'
else
	deps += dependency('glesv2')
	deps += dependency('egl')
	deps += dependency('x11')
endif

sources = [
	'rb.c',
	'ringbuf.c',
	'toxblinkenwall.c',
	'openGL/esShader.c',
	'openGL/esUtil.c'
]

executable('toxblinkenwall', sources,
	dependencies: deps,
	c_args: c_args
)

executable('print_cam_formats', 'print_cam_formats.c',
	dependencies: v4l2)
