# Freezer
A realtime audio filter and its lv2 plugin based on
[Romi1502 freeze_audio](https://github.com/romi1502/freeze_audio).

## Build

The system is designed to run on an Linux. To build it under another platform,
we provide a docker container:
```bash
docker build -t freezer .
docker run -it --rm -v$(pwd):/code freezer bash
```

Then, to build:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/lib/lv2/ ..
make
```

Or using the `Makefile`
```bash
make
make install
```

## Execute

To test the execution, we use the `lv2file` project
```bash
lv2file -l  # list available plugins
lv2file -m -i untitled.wav -o output.wav http://romain-hennequin.fr/plugins/mod-devel/Freeze
```

## Build for Mod

To cross compile

```bash
docker run --rm dockcross/linux-armv7 > dockcross
chmod +x dockcross
./dockcross cmake -H. -Bbuild -GNinja -Dfreezer_build_for_mod=ON -DCMAKE_INSTALL_PREFIX=build/install
./dockcross ninja -Cbuild
./dockcross ninja -Cbuild install
```
