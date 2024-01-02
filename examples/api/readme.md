
### cifX API example

The demo shows the basic functions of the cifX API and how to use it.

1. create a build folder and enter it
```
mkdir build; cd build
```
2. Prepare the build environment via cmake call and pass the path to the examples lists (CMakelists.txt within examples folder) file.
Run the preparation with your required options e.g.:
```
cmake ../ -DDEBUG=ON
```
3. Build the example and run a demo application. Note that you may need root rights. This depends on your system setup. For more information see [System and hardware setup]()
```
make
./cifx_api
```
