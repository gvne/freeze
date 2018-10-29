# Freezer
A realtime audio filter based on
[Romi1502 freeze_audio](https://github.com/romi1502/freeze_audio).

## Build

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

The system is designed to run on an Linux. To build it under another platform,
we provide a docker container:
```bash
docker build -t freezer .
docker run -it --rm -v$(pwd):/code freezer bash
```
