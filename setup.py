from setuptools import setup, Extension
import pybind11
import os
import platform

# Define source files with relative paths
sources = [
    os.path.join('python', 'graphevo', 'core.cpp'),
    os.path.join('cpp', 'src', 'DynamicBitSet.cpp'),
    os.path.join('cpp', 'src', 'FitnessEvaluator.cpp'),
    os.path.join('cpp', 'src', 'GeneticAlgorithm.cpp'),
    os.path.join('cpp', 'src', 'GeneticOperators.cpp'),
    os.path.join('cpp', 'src', 'GraphGenerator.cpp'),
    os.path.join('cpp', 'src', 'Grow.cpp'),
    os.path.join('cpp', 'src', 'Helper.cpp')
]

# Define include directories with relative paths
include_dirs = [
    pybind11.get_include(),
    os.path.join('cpp', 'include'),
    os.path.join('cpp', 'third_party', 'eigen'),
    os.path.join('cpp', 'third_party', 'spectra', 'include')
]
# Add system Eigen path if available
eigen_paths = [
    '/opt/homebrew/include/eigen3',
    '/usr/local/include/eigen3'
]
for path in eigen_paths:
    if os.path.exists(path):
        include_dirs.append(path)
        break

# Platform-specific compiler flags
if platform.system() == "Windows":
    extra_compile_args = ['/std:c++17', '/O2', '/Wall']
    extra_link_args = []
else:
    extra_compile_args = [
        '-std=c++17',
        '-O3',
        '-Wall',
        '-Wextra',
        '-Wno-sign-compare',
        '-fPIC'
    ]
    extra_link_args = ['-fPIC']

# Define the extension module
ext_modules = [
    Extension(
        'graphevo.core',
        sources=sources,
        include_dirs=include_dirs,
        language='c++',
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        define_macros=[
            ('NDEBUG', None),
            ('PYBIND11_STRICT_ASSERTS_CLASS_HOLDER_VS_TYPE_CASTER_MIX', None)
        ]
    )
]

# Read README.md
with open('README.md', encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='graphevo',
    version='0.1.0',
    author='Junsung Hwang',
    author_email='hwang30916@gmail.com',
    description='Genetic Algorithm for Graph Optimization',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/Js-Hwang1/GraphEvo',
    packages=['graphevo'],
    package_dir={'': 'python'},
    ext_modules=ext_modules,
    python_requires='>=3.7',
    package_data={
        'graphevo': ['py.typed'],
    },
    zip_safe=False,
    include_package_data=True,
)