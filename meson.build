project('chat', 'c')

server_src = [
  'server/main.c',
  'server/messages.c',
  'server/users.c',
  'server/io.c',
]

client_src = [
  'client/main.c',
]

common_src = [
  'common/helpers.c',
]

tls = dependency('libtls')

msgpack = dependency('msgpack')

common_inc = include_directories('common')
common_inner_inc = include_directories('common/common')

common = static_library(
  'common',
  common_src,
  c_args : ['-Wextra','-Wall', '-Werror', '-g'],
  dependencies: [msgpack],
  include_directories: common_inner_inc,
)

server = executable(
  'server',
  server_src,
  c_args : ['-Wextra', '-Wall', '-Werror', '-g'],
  dependencies: [tls, msgpack],
  link_with: common,
  include_directories: common_inc,
)

client = executable(
  'client',
  client_src,
  c_args : ['-Wextra', '-Wall', '-Werror', '-g'],
  dependencies: [tls, msgpack],
  link_with: common,
  include_directories: common_inc,
)
