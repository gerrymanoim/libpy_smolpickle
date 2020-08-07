
import ast
import glob
import os
import sys

from libpy.build import LibpyExtension
from setuptools import find_packages, setup

if ast.literal_eval(os.environ.get("LIBPY_your_package_name_DEBUG_BUILD", "0")):
    optlevel = 0
    debug_symbols = True
    max_errors = 5
else:
    optlevel = 3
    debug_symbols = False
    max_errors = None

# See https://github.com/quantopian/libpy/blob/2ceb709381935f98ca9bd55e98d98a203bfec17c/libpy/build.py#L45
def extension(*args, **kwargs):
    extra_compile_args = []
    # extra_compile_args = ["-DLIBPY_AUTOCLASS_UNSAFE_API"]
    # see https://github.com/quantopian/libpy/blob/2ceb709381935f98ca9bd55e98d98a203bfec17c/include/libpy/autoclass.h#L63
    if sys.platform == "darwin":
        extra_compile_args.append("-mmacosx-version-min=10.15")

    return LibpyExtension(
        *args,
        optlevel=optlevel,
        debug_symbols=debug_symbols,
        werror=True,
        max_errors=max_errors,
        include_dirs=(
            ["."] + kwargs.pop("include_dirs", [])
        ),
        extra_compile_args=extra_compile_args,
        depends=glob.glob("**/*.h", recursive=True),
        **kwargs
    )


install_requires = [
    "setuptools",
    "libpy",
]

setup(
    name="your-package-name",
    version="0.1.0",
    description="Python bindings for your-package-name, using libpy",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="",
    author="",
    author_email="",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Natural Language :: English",
        "Topic :: Software Development",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: C++",
        "Operating System :: POSIX",
        "Intended Audience :: Developers",
    ],
    # we need the headers to be available to the C compiler as regular files;
    # we cannot be imported from a ziparchive.
    zip_safe=False,
    install_requires=install_requires,
    extras_require={
        "test": ["pytest"],
        "benchmark": ["pytest-benchmark"],
    },
    ext_modules=[
        extension(
            "your_package_name.module",
            ["your_package_name/module.cc"],
        ),
    ],
)
