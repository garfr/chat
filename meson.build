project('chat', 'c')

server_src = [
  'server/main.c',
  'server/users.c',
  'server/io.c',
]

client_src = [
  'client/main.c',
]

common_src = [
  'common/helpers.c',
  'common/message.c',
]

tls = dependency('libtls')

jansson = dependency('jansson')

common_inc = include_directories('common')
common_inner_inc = include_directories('common/common')

common = static_library(
  'common',
  common_src,
  c_args : ['-Wextra','-Wall', '-Werror', '-g'],
  dependencies: [jansson],
  include_directories: common_inner_inc,
)

server = executable(
  'server',
  server_src,
  c_args : ['-Wextra', '-Wall', '-Werror', '-g'],
  dependencies: [tls, jansson],
  link_with: common,
  include_directories: common_inc,
)

client = executable(
  'client',
  client_src,
  c_args : ['-Wextra', '-Wall', '-Werror', '-g'],
  dependencies: [tls, jansson],
  link_with: common,
  include_directories: common_inc,
)
