# qtTerm <img src="resources/images/qtterm.png" width="27"/>
## Build
There are mutliple options to build qtTerm:


### CMake
Execute the following commands in the base directory
```bash
mkdir build
cd build
cmake ..
make
```
If your *GR* installation is not in `/usr/local/gr` you can specify it with an environment variable before the `cmake ..` call.


### Qt6
```bash
qmake-qt6
make
```
A custom *GR* installation path can be set with the environment variable `GRDIR`


### Qt5
```bash
qmake-qt5
make
```
A custom *GR* installation path can be set with the environment variable `GRDIR`



### Sender examples
The `sender` folder contains examples which have to be build as well before use.
```bash
cd sender
make
```

## Starting qtTerm
After successfully building qtTerm, it can be launched by double clicking the executable.
