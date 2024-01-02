
### cifX tcp-server

A demo server application which allows remote access (e.g. with Communication Studio).


1. create a build folder and enter it
```
mkdir demo_build; cd demo_build
```
2. Prepare the build environment via cmake call and pass the path to the examples lists (CMakelists.txt within examples folder) file.
Run the preparation with your required options e.g.:
```
cmake ../ -DDEBUG=ON
```
3. Build the example and run the demo application. Note that you may need root rights. This depends on your system setup. For more information see [System and hardware setup]()
```
make
./cifx_tcpserver
```

To list all available options run:
```
./cifx_tcpserver -h
```
