pilab_sources = files(
  'main.c',
)

pilab_deps = [
	jsonc,
	curl,
  gtk3,
]

executable(
  'pilab',
  pilab_sources,
  include_directories: [pilab_inc],
	dependencies: [pilab_deps, pthread, x11, x11t], 
  link_with: [lib_pilab_common],
  install: true,
)
