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

## Execute

To test the execution, we use the `lv2file` project
```bash
lv2file -l  # list available plugins
lv2file -m -i untitled.wav -o output.wav http://romain-hennequin.fr/plugins/mod-devel/Freeze
```

## Build for Mod

```bash
docker run --rm -it -v$(pwd):/code moddevices/mod-plugin-builder bash
./build mrfreeze
```
