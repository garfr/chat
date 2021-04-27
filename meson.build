project('chat', 'c')

src = [
  'src/main.c',
]

#inc = include_directories('include')

tls = dependency('libtls')
msgpack = dependency('msgpack')

chat = executable(
  'chat',
  src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  dependencies: [tls, msgpack],
  #include_directories : [inc]
)
