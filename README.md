# gn
## Minimalist clipping software without the bloat
Easily record and trim clips and save to computer or upload to Google Drive

## Dependencies
- FFMPEG (library and binary)
- SDL3
- Win32

## Building
```bash
# Get ffmpeg binaries with lib. Edit CMakeLists to the correct path of ffmpeg.
# Get SDL3 via command:
git clone https://github.com/libsdl-org/SDL.git vendored/SDL
# GET SDL3_ttf via command:
git clone --recursive https://github.com/libsdl-org/SDL_ttf.git vendored/SDL_ttf
# Run 
cmake -S . -B build
cmake --build build
```

## Potential improvemnts
- Encoding/trimming using ffmpeg libs and not binary
- Putting ffmpeg in vendored folder like SDL3

## Disclaimer
Right now this application is only available on Windows. It is possible to make multiplatform, but right now it relies heavily on Win32 apis.
