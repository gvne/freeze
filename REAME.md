# Freezer
A realtime audio filter based on
[Romi1502 freeze_audio](https://github.com/romi1502/freeze_audio).

## Build

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/lib/lv2/ ..
make
```

The system is designed to run on an Linux. To build it under another platform,
we provide a docker container:
```bash
docker build -t freezer .
docker run -it --rm -v$(pwd):/code freezer bash
```

## Execute

To test the execution, we use the `lv2file` project
```bash
sudo apt-get install lv2file
lv2file -l  # list available plugins
lv2file -m -i untitled.wav -o output.wav http://romain-hennequin.fr/plugins/mod-devel/Freeze
```
