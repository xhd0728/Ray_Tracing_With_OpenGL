# Introduction

This project is the award-winning work of Chinese Collegiate Computing Competition (4C 2023). We have implemented ray tracing using the OpenGL library and NVIDIA driver.

# Start

Before running the project, you need to install necessary librarys.

```shell
sudo apt update
sudo apt install -y gcc g++ gdb libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev freeglut3-dev libglfw3-dev
```

Besides, you need to use `cmake -version` to view your CMake version, make sure the `cmake_version >= 3.21.3`.

## Build

```shell
cmake -B build
```

## Execute

```shell
cd build
./HEU_EASY_OPENGL
```

# Contact

If you have any questions about this project, please contact:

```
hdxin2002@gmail.com
```

# Citation

If this project is helpful to you, please cite it in your paper:

```bibtex
@software{Ray_Tracing_With_OpenGL,
  author = {Haidong Xin and Xianyu Zhang},
  title = {{Ray_Tracing_With_OpenGL}},
  url = {https://github.com/xhd0728/Ray_Tracing_With_OpenGL},
  version = {1.0},
  year = {2024}
}
```