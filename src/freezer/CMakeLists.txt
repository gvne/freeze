if (${freezer_enable_tests})
  add_executable(freezer_test
      ${src}/freezer/test.cc
      ${src}/freezer/freezer.cc
      ${src}/freezer/freezer.h
      ${src}/freezer/operation.h
      ${src}/freezer/operation.cc
  )
  add_dependencies(freezer_test
    external_rtff
    external_googletest
    external_wave
    external_eigen
  )
  target_include_directories(freezer_test
    PUBLIC
      ${gtest_install_dir}/include
      ${wave_install_dir}/include
      ${rtff_install_dir}/include
      ${eigen_install_dir}
  )
  target_link_libraries(freezer_test
    optimized gtest
    optimized gtest_main
    optimized rtff
    debug gtestd
    debug gtest_maind
    debug rtffd
    fftw3f
    debug waved
    optimized wave
  )
  target_compile_definitions(freezer_test
    PUBLIC
      -DTEST_RESOURCES_PATH="${test_resource_path}"
  )
endif()
