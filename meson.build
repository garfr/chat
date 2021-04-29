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

tls = dependency('libtls')
msgpack = dependency('msgpack')

server = executable(
  'server',
  server_src,
  c_args : ['-Wextra', '-Werror', '-g'],
  dependencies: [tls, msgpack],
)

client = executable(
  'client',
  client_src,
  c_args : ['-Wextra', '-Werror', '-g'],
  dependencies: [tls, msgpack],
)
