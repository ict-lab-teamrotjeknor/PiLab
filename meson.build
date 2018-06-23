project(
  'pilab',
  'c',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=true',
  ],
)

add_project_arguments('-Wno-unused-parameter', language: 'c')
add_project_arguments('-Wno-unused-function', language: 'c')
add_project_arguments('-Wno-unused-result', language: 'c')

cc = meson.get_compiler('c')

datadir = get_option('datadir')
sysconfdir = get_option('sysconfdir')
prefix = get_option('prefix')

jsonc     = dependency('json-c', version: '>=0.13')
curl      = dependency('libcurl')
gtk3      = dependency('gtk+-3.0')
git       = find_program('git', required: false)
wpi       = cc.find_library('wiringPi', dirs: ['/usr/local/lib'])
wpi_dev   = cc.find_library('wiringPiDev', dirs: ['/usr/local/lib'])
pthread   = cc.find_library('pthread')
x11       = cc.find_library('X11')
x11t      = cc.find_library('Xtst')

add_project_arguments('-DSYSCONFDIR="@0@"'.format(sysconfdir), language : 'c')

version = get_option('pilab_version')

if version != ''
   version = '"@0@"'.format(version)
else
  if not git.found()
     error('git is required to make the version string')
   endif

   git_commit_hash = run_command([git.path(), 'describe', '--always', '--tags']).stdout().strip()
   git_branch = run_command([git.path(), 'rev-parse' , '--abbrev-ref', 'HEAD']).stdout().strip()
   version = '"@0@ (" __DATE__", branch \'@1@\')"'.format(git_commit_hash, git_branch)
endif

add_project_arguments('-DPILAB_VERSION=@0@'.format(version), language: 'c')

pilab_inc = include_directories('include')

subdir('common')
subdir('pilab')

config = configuration_data()
config.set('sysconfdir', join_paths(prefix, sysconfdir))
config.set('datadir', join_paths(prefix, datadir))
config.set('prefix', prefix)

configure_file(
  configuration: config,
	input: 'config.in',
	output: '@BASENAME@',
  install_dir: sysconfdir + '/pilab'
)

glade_data = configuration_data()

configure_file(
  configuration: glade_data,
	input: 'popup.glade',
	output: 'popup.glade',
  install_dir: sysconfdir + '/pilab'
)

sensor_data = configuration_data()
configure_file(
  configuration: sensor_data,
	input: 'sensors.in',
	output: '@BASENAME@',
  install_dir: sysconfdir + '/pilab'
)

install_data(
  'pilab.desktop',
  install_dir: datadir + '/applications'
)
