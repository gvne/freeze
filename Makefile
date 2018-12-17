all: cmake
	cd build && make

cmake:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release -Dfreezer_enable_tests=OFF -DCMAKE_INSTALL_PREFIX=/usr/lib/lv2/ ..

install: cmake
	cd build && make install
