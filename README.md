## PanzerJson
[![Build Status (master)](https://travis-ci.org/Panzerschrek/PanzerJson.svg?branch=master)](https://travis-ci.org/Panzerschrek/PanzerJson)

"PanzerJson" is a C++ library for JSON reading.

It allows parsing of JSON`s and easy manipalate result JSON structure in C++ code.

Main feature of this library is compile-time JSON structure building via special Python-script.
This script parses JSON file and prodices C++ file with structured JSON file content.
Because all constructors of used structures marked as "noexcept", full structure builded at compile-time and does nothing at runtime (during globals initialization).

### Disclaimer
Library is still on early stage of develompent. It can not compile on some compilers, works incorrect.  
Pull-requests are welcome.

### Requirements
Modern compiler with C++14 support. But, maybe it can work on C++11.
Library tested on GCC 4.9.4, but in clang/msvc maybe it does not compiles.  
Python3 needed for generation of C++ files with JSON content.

### How to build
If you wish to use this library in your code, you need include files in "src" dir in your project and add "include" dir to compiler include path.  
If you need generate C++ files from JSON, you need integrate Python-script invocation in you build.

### Authors
Copyright © 2017 Artöm "Panzerscrek" Kunz.

