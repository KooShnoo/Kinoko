#!/usr/bin/env python3

import argparse
from glob import glob
import io
import os
import sys
from tools.generate_tests import generate_tests
from vendor.ninja_syntax import Writer

generate_tests()

out_buf = io.StringIO()
n = Writer(out_buf)

file_extension = ''
if sys.platform.startswith('win32'):
    file_extension = '.exe'

n.variable('ninja_required_version', '1.3')
n.newline()

n.variable('builddir', 'build')
n.variable('outdir', 'out')
n.newline()

n.variable('wasm_compiler', '~/.wasm/wasi-sdk-22.0/bin/clang -I~/.wasm/wasi-sdk-22.0/share/wasi-sysroot')
n.variable('compiler', 'g++')
n.newline()

wasm_ccflags = [
    # '-nostartfiles',
    # '-Wl,--profiling-funcs',
    # '-lc++',
    # '-lc',
    # '-Wl,-emit-obj',
    # '-Wl,--relocatable',
    # '-Wl,--no-entry',
    # '-Wl,--strip-all',
    '-fno-asynchronous-unwind-tables',
    '-fno-exceptions',
    '-fno-rtti',
    '-fshort-wchar',
    '-fstack-protector-strong',
    '-isystem', '.',
    '-isystem', 'include',
    '-isystem', 'source',
    '-isystem', 'vendor',
    '-isystem', 'build',
    '-std=c++23',
    '-Wno-unknown-attributes'
    # '-Wall',
    # '-Werror',
    # '-Wextra',
    # '-Wno-delete-non-virtual-dtor',
    # '-Wsuggest-override',
    # '--target=wasm32',
]

common_ccflags = [
    '-DREVOLUTION',
    '-fno-asynchronous-unwind-tables',
    '-fno-exceptions',
    '-fno-rtti',
    '-fshort-wchar',
    '-fstack-protector-strong',
    '-isystem', '.',
    '-isystem', 'include',
    '-isystem', 'source',
    '-isystem', 'vendor',
    '-isystem', 'build',
    '-std=c++23',
    '-Wall',
    '-Werror',
    '-Wextra',
    '-Wno-delete-non-virtual-dtor',
    '-Wno-packed-bitfield-compat',
    '-Wsuggest-override',
]

target_cflags = [
    '-O3',
    '-flto'
]

debug_cflags = [
    '-O0',
    '-g'
]

wasm_ldflags = [
    "-lc++",
    "-lstdc++",
    "-lc",
    # "-Oz",
    # "-nostartfiles",
    "-fno-exceptions",
    # "-Wl,-emit-obj",
    "-Wl,--allow-undefined",
    "-Wl,-e,main",
    # "-Wl,--strip-all",
    # "-Wl,--relocatable",
    # "-Wl,--no-entry"
]

n.variable('common_ccflags', common_ccflags)
n.variable('wasm_ccflags', wasm_ccflags)
n.newline()

n.rule(
    'cc',
    command='$compiler -MD -MT $out -MF $out.d $ccflags -c $in -o $out',
    depfile='$out.d',
    deps='gcc',
    description='CC $out',
)
n.newline()

n.rule(
    'wasmcc',
    command='$wasm_compiler -MD -MT $out -MF $out.d $ccflags -c $in -o $out',
    depfile='$out.d',
    deps='gcc',
    description='CC $out',
)
n.newline()

n.rule(
    'wasmld',
    command='$wasm_compiler $ldflags $in -o $out',
    description='LD $out',
)
n.newline()

n.rule(
    'ld',
    command='$compiler $ldflags $in -o $out',
    description='LD $out',
)
n.newline()

code_in_files = [file for file in glob('**/*.cc', recursive=True)]
no_wasm_files = ["source/host/main.cc", "source/abstract/File.cc"]
no_native_files = ["Pilz/pilz.cc"]

target_code_out_files = []
debug_code_out_files = []
target_wasm_out_files = []
debug_wasm_out_files = []

for in_file in code_in_files:
    in_filename, ext = os.path.splitext(in_file)
    wasm = not in_file in no_wasm_files
    native = not in_file in no_native_files
    
    # if native:
    #     target_out_file = os.path.join('$builddir', in_file + '.o')
    #     target_code_out_files.append(target_out_file)
    if wasm:
        target_out_wasm_file = os.path.join('$builddir', in_file + '.wasm')
        target_wasm_out_files.append(target_out_wasm_file)
        
        debug_out_wasm_file = os.path.join('$builddir', in_file + 'D.wasm')
        debug_wasm_out_files.append(debug_out_wasm_file)
        
    # if native:
    #     debug_out_file = os.path.join('$builddir', in_file + 'D.o')
    #     debug_code_out_files.append(debug_out_file)

    # if native:
    #     n.build(
    #         target_out_file,
    #         ext[1:],
    #         in_file,
    #         variables={
    #             'ccflags': ' '.join(['$common_ccflags', *target_cflags])
    #         }
    #     )
    #     n.newline()
        
        # n.build(
        #     debug_out_file,
        #     ext[1:],
        #     in_file,
        #     variables={
        #         'ccflags': ' '.join(['$common_ccflags', *debug_cflags])
        #     }
        # )
        # n.newline()

    if wasm:
        n.build(
            target_out_wasm_file,
            'wasmcc',
            in_file,
            variables={
                'ccflags': ' '.join(['$wasm_ccflags', *target_cflags])
            }
        )
        n.newline()
        
        n.build(
            debug_out_wasm_file,
            'wasmcc',
            in_file,
            variables={
                'ccflags': ' '.join(['$wasm_ccflags', *debug_cflags])
            }
        )
        n.newline()

if wasm:
    
    n.build(
        os.path.join('$outdir', f'kinoko.wasm'),
        'wasmld',
        target_wasm_out_files,
        variables={
            'ldflags': ' '.join([
                *wasm_ldflags,
                '-Wl,--strip-all'
            ])
        },
    )
    
    n.build(
        os.path.join('$outdir', f'kinokoD.wasm'),
        'wasmld',
        debug_wasm_out_files,
        variables={
            'ldflags': ' '.join([
                *wasm_ldflags
            ])
        },
    )

# n.build(
#     os.path.join('$outdir', f'kinoko{file_extension}'),
#     'ld',
#     target_code_out_files,
#     variables={
#         'ldflags': ' '
#     },
# )

# n.build(
#     os.path.join('$outdir', f'kinokoD{file_extension}'),
#     'ld',
#     debug_code_out_files,
#     variables={
#         'ldflags': ' '
#     },
# )


n.variable('configure', 'configure.py')
n.newline()

n.rule(
    'configure',
    command=f'{sys.executable} $configure',
    generator=True,
)
n.build(
    'build.ninja',
    'configure',
    implicit=[
        '$configure',
        os.path.join('vendor', 'ninja_syntax.py'),
    ],
)

with open('build.ninja', 'w') as out_file:
    out_file.write(out_buf.getvalue())
n.close()
