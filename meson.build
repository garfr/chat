project('chat', 'c')

server_src = [
  'server/main.c',
]

client_src = [
  'client/main.c',
]

tls = dependency('libtls')
msgpack = dependency('msgpack')

server = executable(
  'server',
  server_src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  dependencies: [tls, msgpack],
)

client = executable(
  'client',
  client_src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  dependencies: [tls, msgpack],
)
