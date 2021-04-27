project('chat', 'c')

src = [
  'src/main.c',
]

#inc = include_directories('include')

openssl = dependency('openssl')

chat = executable(
  'chat',
  src,
  c_args : ['-Wextra', '-Werror', '-g', '-std=c99', '-pedantic'],
  dependencies: [openssl],
  #include_directories : [inc]
)
