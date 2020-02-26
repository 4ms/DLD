def Settings( **kwargs ):
  return {
    'flags': [ '-x', 'c', 
    '-Wall', '-Wextra', '-Werror',
    '-DARM_MATH_CM4', 
    '-DUSE_STDPERIPH_DRIVER',
    '-D\'__FPU_PRESENT=1\'',
    '-DSTM32F427X',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/sys/',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/ssp/',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/newlib-nano/',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/machine/',
    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/lib/gcc/arm-none-eabi/8.2.1/include/',
    '-I', '/Users/dann/4ms/stm32/DLD/',
    '-I', '/Users/dann/4ms/stm32/DLD/stm32/periph/include/',
    '-I', '/Users/dann/4ms/stm32/DLD/stm32/device/include/',
    '-I', '/Users/dann/4ms/stm32/DLD/stm32/core/include/',
    ],
  }


#    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/c++/8.2.1/arm-none-eabi/arm/',
#    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/c++/8.2.1/arm-none-eabi/bits/',
#    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/c++/8.2.1/arm-none-eabi/ext/',
#    '-I', '/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/arm-none-eabi/include/c++/8.2.1/arm-none-eabi/thumb/',
# '-D\'__FPU_PRESENT=1\'',
# '-D\'F_CPU=216000000\'',
