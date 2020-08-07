# libpy python package template

A template repository for making [libpy](https://github.com/quantopian/libpy) python packages.

Places you need to make changes, generally replacing `your_package_name` or `your-package-name`:

- [ ] `setup.py`
- [ ] this `README.md`
- [ ] `MANIFEST.in` for cc/h files you need avaliable during build time
- [ ] `your_package_name` folder which contains your source files (h/cc/py)
- [ ] `.github/workflows/main.yml` which runs the main CI flow
- [ ] `.github/workflows/release.yml` which pushes out a release once a release is published

Don't forget to add some badges once CI is running and you've published to PyPI (https://badge.fury.io/for/py)


## Requirements

- OS: macOS>10.15, linux.
- Compiler: gcc>=9, clang >= 10 (C++17 code)
- Python: libpy>=0.2.3, numpy.

## Install

`pip install your-package-name`

Note: The installation of libpy (required by `your_package_name`) will use the python executable to figure out information about your environment. If you are not using a virtual environment or python does not point to the Python installation you want to use (checked with which python and python --version) you must point to your Python executable using the PYTHON environment variable, i.e. PYTHON=python3 make or PYTHON=python3 pip3 install libpy. Additionally, make sure that your CC and CXX environment variables point to the correct compilers.

## Usage

```

```
