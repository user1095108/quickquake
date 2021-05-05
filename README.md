![screenshot.png](screenshot.png?raw=true)
# quickquake
There is very little, that speaks in favor of porting Quake to Qt Quick 2, but I needed a complex example to test multithreaded OpenGL rendering with. The port, such as it is, is not playable, but the demos do render. If you wish to improve the port, please send your patches and I'll add you to the list of maintainers.
# build instructions
    git submodule init
    git submodule update
    qmake
    make
