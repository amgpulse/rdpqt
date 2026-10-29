# RdpQt - FreeRDP GUI Client (Qt6 & C++17)

A lightweight graphical user interface for managing FreeRDP (`xfreerdp`) remote desktop connections on Linux.

## Requirements

- **C++17** compiler (`g++` or `clang++`)
- **CMake** (>= 3.16)
- **Qt 6** (`qt6-base-dev` or `qt6-declarative-dev`)
- **FreeRDP** (`freerdp2-x11` or `freerdp3-x11`)

### Installing dependencies (Ubuntu / Debian):
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev freerdp2-x11
```

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Running the Application

```bash
./rdpqt
```

## Features
- Configure Server Host/IP, Port, Username, Domain, and Password (with show/hide toggle).
- Screen resolution presets (Full HD, 2K, 4K, Dynamic, Fullscreen).
- Clipboard redirection toggle (`+clipboard`).
- Ignore SSL certificate warnings (`/cert:ignore`).
- Shared local directory / drive redirection (`/drive:<name>,<path>`).
- Asynchronous process management with `QProcess`.
- Real-time output and error logs view.
- Automatic settings persistence via `QSettings`.
