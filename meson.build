project(
	'bat',
	'c',
	license: 'MIT',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=true',
	],
)

add_global_arguments(['-Wno-unused-parameter'], language: 'c')

wayland_client = dependency('wayland-client')
wayland_protos = dependency('wayland-protocols', version: '>=1.14')
udev = dependency('libudev')

subdir('protocol')

include_files = include_directories('include')
source_files = files(
	'src/battery.c',
	'src/buffer.c',
	'src/config.c',
	'src/event-loop.c',
	'src/main.c',
	'src/output.c',
	'src/render.c',
	'src/string-list.c',
	'src/udev.c',
	'src/wayland.c'
)
executable(
	'bat',
	source_files,
	include_directories: include_files,
	dependencies: [
		client_protos,
		udev,
		wayland_client
	]
)
