add_library(mrfreeze SHARED
  ${src}/plugin/plugin.cc
  ${src}/plugin/plugin.h
)
# remove the lib prefix
set_target_properties(mrfreeze PROPERTIES PREFIX "")

target_link_libraries(mrfreeze
  freezer
  optimized rtff
  debug rtffd
  fftw3f
  pthread
)

target_include_directories(mrfreeze
  PRIVATE
    ${rtff_install_dir}/include
)

set_target_properties(mrfreeze
  PROPERTIES
    COMPILE_FLAGS -O3 -ffast-math -Wall -fPIC -DPIC
)

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
