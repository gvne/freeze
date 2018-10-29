cmake_minimum_required(VERSION 3.1)

include(ExternalProject)
project(freezer CXX)

set(CMAKE_CXX_STANDARD 14)

option(freezer_enable_tests "Build Unit tests" ON)

# Audionamix - rtff
set(rtff_install_dir ${CMAKE_BINARY_DIR}/rtff-install)
ExternalProject_Add(external_rtff
  GIT_REPOSITORY "https://github.com/audionamix/rtff"
  GIT_TAG "a381dcb5254b94196faec42cebcf1541c86388e4"
  CMAKE_ARGS
    "-Drtff_use_mkl=OFF"  # force the use of fftw
    "-DCMAKE_INSTALL_PREFIX=${rtff_install_dir}"
)
link_directories(${rtff_install_dir}/lib)

# Eigen
set(eigen_install_dir ${CMAKE_BINARY_DIR}/eigen-install)
ExternalProject_Add(external_eigen
  HG_REPOSITORY "https://bitbucket.org/eigen/eigen/"
  HG_TAG "3.3.4"
  SOURCE_DIR ${eigen_install_dir}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
)
set(eigen_include_dir ${eigen_install_dir})
add_definitions("-DEIGEN_RUNTIME_NO_MALLOC")

# tests
if (${freezer_enable_tests})
  enable_testing()

  # gtest
  set(gtest_install_dir ${CMAKE_BINARY_DIR}/googletest-install)
  ExternalProject_Add(external_googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "718fd88d8f145c63b8cc134cf8fed92743cc112f"
    CMAKE_ARGS
      "-Dgtest_force_shared_crt=ON"
      "-DCMAKE_INSTALL_PREFIX=${gtest_install_dir}"
  )
  link_directories("${gtest_install_dir}/lib")

  # download test resource from https://archive.org/details/test_wav
  set(test_resource_path "${CMAKE_BINARY_DIR}/test_resource")
  file(MAKE_DIRECTORY "${test_resource_path}")
  file(DOWNLOAD
    "https://s3.amazonaws.com/gvne-public/test_sample.wav"
    "${test_resource_path}/test_sample.wav"
  )

  # Audionamix - wave
  set(wave_install_dir ${CMAKE_BINARY_DIR}/wave-install)
  ExternalProject_Add(external_wave
    GIT_REPOSITORY "https://github.com/audionamix/wave"
    GIT_TAG "e73572dfdc15fba8ac8e544a7f2410adb2588d10"
    # don't update so that we don't rebuild every time
    UPDATE_COMMAND ""
    CMAKE_ARGS
      "-DCMAKE_INSTALL_PREFIX=${wave_install_dir}"
      "-Dwave_enable_tests=OFF"
  )
  link_directories("${wave_install_dir}/lib")
endif()

set(src "${CMAKE_SOURCE_DIR}/src")
include_directories(${src})

add_subdirectory("./src")
