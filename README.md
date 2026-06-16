# ELARA
**E**mbedded **L**ibrary **A**sset **R**esolver & **A**rchiver

This is the PC-side tool for the ELFARIA music player. It walks through a music folder, reads ID3/FLAC tags, extracts album art, resizes it to 64x64 JPEGs, deduplicates duplicates, and packs everything into two files:
- `library.idx` – binary index of tracks
- `art.bin` – concatenated thumbnails

The firmware just reads these two files off an SD card – no scanning, no tag parsing, no runtime overhead.

---

## Dependencies

- C++17 compiler
- CMake
- TagLib (for metadata)
- stb_image / stb_image_resize / stb_image_write (vendored in `lib/stb`)

On Ubuntu/Debian/WSL:
```bash
sudo apt install build-essential cmake pkg-config libtag1-dev
```

On Windows (MSYS2 UCRT64):
```bash
pacman -S mingw-w64-ucrt-x86_64-taglib
```

---

## Build

Clone the repo and update submodules:
```bash
git clone <repo-url>
cd ELARA
git submodule update --init --recursive
```

Build with CMake:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Or just run the build script (Linux/WSL):
```bash
./linux-build.sh
```

Output:
- `build/builder/elara_builder` (or `.exe` on Windows)
- `build/validator/elara_validator` (or `.exe` on Windows)

---

## Run

Scan a music folder and generate the index:
```bash
./build/builder/elara_builder /path/to/music
```

It creates `library.idx` and `art.bin` in the current directory.

Validate the output:
```bash
./build/validator/elara_validator library.idx art.bin
```

If it says `PASS`, you're good to copy the files to your SD card.

---

## Submodules

This repo uses `lib/shared` (ELMIR) and `lib/stb`.  
Pull latest changes:
```bash
git submodule update --remote
```

---

## Attributions

- **stb** – Public domain single-file libraries for image loading/resizing/writing.  
  Source: https://github.com/nothings/stb