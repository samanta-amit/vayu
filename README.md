# ML Inference Server and Client

Implementation of the inference server, clients (and the scheduler) using the boost asio framework.

## Compiling ##

This code was only tested under a 64-bit Linux distribution.

### Dependencies ###

The main external dependencies are boost, boost-asio (1.60.1) and msgpack-c. 

msgpack-c is used to serialize/deserialize objects. Header only version is used so
there is no need to build/install msgpack.

Then build the project from the source directory with

```
mkdir build
cd build
cmake ..
make
```
This will generate two executables in the build path namely ```tcp_server``` and ```work_load_generator```. Run these using

```
./tcp_server &
./work_load_generator
```

