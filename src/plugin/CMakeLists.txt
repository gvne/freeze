add_library(mrfreeze SHARED
  ${src}/plugin/plugin.cc
  ${src}/plugin/plugin.h
)
# remove the lib prefix
set_target_properties(mrfreeze PROPERTIES PREFIX "")

target_link_libraries(mrfreeze
  # freezer
  # optimized ${rtff_install_dir}/lib/librtff.a
  # debug ${rtff_install_dir}/lib/librtffd.a
  # fftw3f
  pthread
)

target_include_directories(mrfreeze
  PRIVATE
    ${rtff_install_dir}/include
    ${lv2_include_dir}
)

if (${freezer_build_for_mod})
  set_target_properties(mrfreeze
    PROPERTIES
      COMPILE_FLAGS -O3 -ffast-math -Wall -fPIC -DPIC -mtune=generic -msse -msse2 -mfpmath=sse
  )
else()
  add_dependencies(mrfreeze external_lv2)
endif()

# install stage: create the lv2 plugin structure
install(TARGETS mrfreeze
  LIBRARY DESTINATION mrfreeze.lv2
)

# also install the content of the ttl folder
set(ttl_folder ${src}/plugin/ttl)
file(GLOB_RECURSE ttl_files RELATIVE ${ttl_folder} *.ttl *.html *.png *.css)
set(ttl_files_paths "")
foreach(test ${ttl_files})
  set(ttl_files_paths ${ttl_files_paths} ${ttl_folder}/${test})
endforeach()
install(FILES
  ${ttl_files_paths}
  DESTINATION mrfreeze.lv2
)

# and the wisdom file
set(wisdom_path "${CMAKE_BINARY_DIR}/rtff_2048.fftw")
message(STATUS "Downloading precomputed wisdom file to ${wisdom_path}")
file(DOWNLOAD
  https://s3.amazonaws.com/gvne-public/code-requirements/mrfreeze/rtff_2048.fftw
  ${wisdom_path}
)
install(FILES
  ${wisdom_path}
  DESTINATION mrfreeze.lv2
)
