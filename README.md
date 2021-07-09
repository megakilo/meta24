# meta24
The C++ meta-programming version of [calc24](https://github.com/megakilo/calc24).

Notes
- Build project and dependency (Boost mp11) using Bazel: `bazel build meta24`
- Use C++17 standard
- Challenge to build for more than 4 numbers

Tested under Linux with
- clang++ 10.0/12.0
- g++ 9.3/11.1 (avoid g++, very slow compilation on deep templates)
