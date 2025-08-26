# [TO_FIND]

## Original app

TODO

## Visuals

Version 1: Player (`Z`, `Q`, `S`, `D`), Map 2D, Raycasting 3D
![screenshot-1](/visuals/1.png)

Version 2: Handling `FPS`, Better player movements, Player collisions
![screenshot-2](/visuals/2.png)

Version 3: Refactor code, Map 2D dynamic size, Player speed and speed rotation
![screenshot-3](/visuals/3.png)

Version 4: Floors and ceilings, Slit compute and draw rays, Show/Hide mini map (`M`)
![screenshot-4](/visuals/4.png)

## Features

TODO

## Developer's zone

### Build & Launch

```zsh
$ git clone https://github.com/libsdl-org/SDL.git vendored/SDL
$ cmake -S . -B build   # setup
$ cmake --build build   # compile/build
$ ./build/bin/main      # execute
```

### Technical stack

- C++ 17
- SDL 3
- CMake 4.0
