project('chat', 'c')

src = [
  'src/main.c',
]

#inc = include_directories('include')

libtls = dependency('libtls')

chat = executable(
  'chat',
  src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  dependencies: [libtls],
  #include_directories : [inc]
)
