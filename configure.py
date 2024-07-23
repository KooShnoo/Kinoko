#!/usr/bin/env python3

from glob import glob
import io
import os
import sys
from tools.generate_tests import generate_tests
from vendor.ninja_syntax import Writer

generate_tests()

out_buf = io.StringIO()
n = Writer(out_buf)

file_extension = '.so'
if sys.platform.startswith('win32'):
    file_extension = '.dll'

n.variable('ninja_required_version', '1.3')
n.newline()

n.variable('builddir', 'build')
n.variable('outdir', 'out')
n.newline()

n.variable('compiler', 'clang++')
n.newline()

common_ccflags = [
    '-fPIC', # dylib
    '-fvisibility=hidden',
    # '-fsanitize=address',
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
    '-isystem', '/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/Headers/',
    '-std=c++23',
    # '-Wall',
    # '-Werror',
    # '-Wextra',
    '-Wno-delete-non-virtual-dtor',
    # '-Wno-packed-bitfield-compat',
    '-Wsuggest-override',
]

target_cflags = [
    '-O3',
]

debug_cflags = [
    '-O0',
    '-ggdb',
]

common_ldflags = [
    '-dynamiclib', # dylib
    '-fvisibility=hidden',
    # '-fsanitize=address',
    # '-shared-libsan',
    # '-shared-libasan',
    # /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/Python3
    # /usr/local/Cellar/python@3.12/3.12.3/Frameworks/Python.framework/Versions/3.12/lib/libpython3.12.dylib
    # '-Wl,-rpath,/Library/Developer/CommandLineTools/Library/Frameworks/',
    # '-L/usr/local/Cellar/python@3.11/3.11.6_1/Frameworks/Python.framework/Versions/3.11/lib/',
    # '-L/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.9/lib/',
    # '-lpython3.11'
    '-undefined dynamic_lookup'
]

n.rule(
    'cc',
    command='$compiler -MD -MT $out -MF $out.d $ccflags -c $in -o $out',
    depfile='$out.d',
    deps='gcc',
    description='CC $out',
)
n.newline()

n.rule(
    'ld',
    command='$compiler $ldflags $in -o $out',
    description='LD $out',
)

code_in_files = [file for file in glob('**/*.cc', recursive=True)]

target_code_out_files = []
debug_code_out_files = []
skip_files = ["source/host/main.cc", "source/host/System.cc", "source/abstract/File.cc"] + [file for file in glob('source/test/*.cc', recursive=True)]

for in_file in code_in_files:
    if in_file in skip_files:
        continue
    _, ext = os.path.splitext(in_file)

    target_out_file = os.path.join('$builddir', in_file + '.o')
    target_code_out_files.append(target_out_file)

    debug_out_file = os.path.join('$builddir', in_file + 'D.o')
    debug_code_out_files.append(debug_out_file)

    n.build(
        target_out_file,
        ext[1:],
        in_file,
        variables={
            'ccflags': ' '.join([*common_ccflags, *target_cflags])
        }
    )
    n.newline()

    n.build(
        debug_out_file,
        ext[1:],
        in_file,
        variables={
            'ccflags': ' '.join([*common_ccflags, *debug_cflags])
        }
    )
    n.newline()


n.build(
    os.path.join('$outdir', f'kinoko{file_extension}'),
    'ld',
    target_code_out_files,
    variables={
        'ldflags': ' '.join([
            *common_ldflags,
        ])
    },
)

n.build(
    os.path.join('$outdir', f'kinokoD{file_extension}'),
    'ld',
    debug_code_out_files,
    variables={
        'ldflags': ' '.join([
            *common_ldflags,
        ])
    },
)

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
