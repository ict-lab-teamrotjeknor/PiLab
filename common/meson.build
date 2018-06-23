lib_pilab_common = static_library(
  'pilab-common',
  files(
    'pilab-list.c',
    'pilab-string.c',
    'pilab-stringbuilder.c',
    'pilab-json-parser.c',
    'pilab-hashtable.c',
    'pilab-slave-device.c',
    'pilab-gpio-device.c',
    'pilab-i2c-device.c',
    'pilab-host-device.c',
    'pilab-slave-device.c',
    'pilab-api-client.c',
    'pilab-api-calls.c',
    'pilab-popup.c',
    'pilab-log.c',
    'pilab-readline.c',
    'pilab-config.c',
    'pilab-time.c',
    'pilab-lcd.c',
  ),
  dependencies: [
     gtk3, curl, jsonc, wpi, wpi_dev,
  ],
  include_directories: pilab_inc
)
