#Minimal Win32 App with no cruntime

Credit goes to:
[Handmade hero](https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c++_runtime_on_windows)

For debugging added external code for printf with minimal modification
* https://github.com/ARMmbed/mbed*os 

Caveats:
* No try/catch handler
* Many libraries won't work as they need cruntime
* Stack fixed to 1MB, won't allocate more if needed (I don't need)
* No array initialization (int a[100] = {};)

Pros:
* No hidden stuff

Usage:
In project folder do:
```
cd src
..\build.bat debug
or
..\build.bat release
```


Implementations:
* Window creation interface
* Input interface

Wanna do:
* Multithread interface

Maaaaaaaybe:
* Linux

