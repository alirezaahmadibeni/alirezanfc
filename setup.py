#!/bin/env python3

import os
from distutils.core import setup, Extension

define_macros = []
libraries = ['pthread', 'nfc']
extra_link_args = []
extra_compile_args = []
sources = ['alirezanfc.cpp']

alirezanfc = Extension('alirezanfc',
                       sources=sources,
                       libraries=libraries,
                       define_macros=define_macros,
                       extra_compile_args=extra_compile_args,
                       extra_link_args=extra_link_args,
                       )

setup(name='alirezanfc',
      version='1.0',
      description='Python nfc interface',
      author='Alireza Ahmadi Beni',
      author_email='a.ahmadi.sku@gmail.com',
      url='',
      ext_modules=[alirezanfc])
