#!/bin/bash
set -ex

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DSFIZZ_JACK=OFF \
      -DSFIZZ_TESTS=ON \
      -DSFIZZ_SHARED=OFF \
      -DSFIZZ_STATIC_DEPENDENCIES=OFF \
      -DSFIZZ_LV2=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      ..
make -j2 sfizz_tests
tests/sfizz_tests
