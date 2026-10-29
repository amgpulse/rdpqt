<div align="center">

# 🖥️ rdpqt

**A modern, lightweight, and native Qt6 graphical frontend for FreeRDP (`xfreerdp`) on Linux.**

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Qt](https://img.shields.io/badge/Qt-6.x-green.svg?style=for-the-badge&logo=qt)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-red.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg?style=for-the-badge&logo=linux)](https://kernel.org/)
[![FreeRDP](https://img.shields.io/badge/Backend-FreeRDP-brightgreen.svg?style=for-the-badge)](https://www.freerdp.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

<br />

<p align="center">
  <b>rdpqt</b> eliminates the need to remember complex command-line arguments for <code>xfreerdp</code>.<br/>
  Manage multiple display resolutions, folder sharing, clipboard synchronization, and credentials through a clean, intuitive desktop interface.
</p>

</div>

---

## ✨ Features

- 🚀 **High Performance & Native**: Built with modern **C++17** and **Qt6 Widgets** for instant startup and negligible memory footprint.
- 🔄 **Asynchronous Process Execution**: Powered by `QProcess`, keeping the UI responsive while streaming real-time stdout/stderr output.
- 🔒 **Secure Credential Handling**: Toggle password masking with one click; passwords are automatically sanitized and masked in console logs.
- 📐 **Flexible Resolution Management**:
  - Presets for **Full HD (1080p)**, **2K**, **4K**, **720p**, and **1366x768**.
  - **Dynamic Resolution** (`/dynamic-resolution`) and **Fullscreen Mode** (`/f`).
  - Custom arbitrary resolutions supported via editable combo-box.
- 📋 **Seamless Clipboard Sharing**: Toggle bidirectional clipboard synchronization (`+clipboard` / `-clipboard`).
- 📁 **Local Folder Redirection**: Share local directories as virtual drives (`/drive:<name>,<path>`) using an integrated folder picker.
- 🛡️ **Self-Signed Certificate Bypass**: Quick toggle for `/cert:ignore` when working with internal servers and self-signed certificates.
- 💾 **Persistent Session Settings**: Automatically restores your last configured host, port, username, domain, and display settings across restarts via `QSettings`.
- 📊 **Real-time Log Terminal**: Built-in dark monospace console viewer displaying connection events and live session state.

---

## 🛠️ FreeRDP Flag Mapping

| Feature | rdpqt UI Control | Generated FreeRDP Flag |
| :--- | :--- | :--- |
| **Server Address** | Host + Port SpinBox | `/v:<host>:<port>` |
| **User Account** | Username input | `/u:<username>` |
| **Password** | Secure Password input | `/p:<password>` *(masked in logs)* |
| **Domain** | Optional Domain input | `/d:<domain>` |
| **Display Mode** | Resolution Dropdown | `/size:<WxH>`, `/f`, or `/dynamic-resolution` |
| **Clipboard** | Checkbox | `+clipboard` or `-clipboard` |
| **TLS/SSL Cert** | Checkbox | `/cert:ignore` |
| **Shared Drive** | Checkbox + Path Browser | `/drive:<FolderName>,<Path>` |

---

## 📦 Prerequisites & Dependencies

Before building **rdpqt**, ensure your system has a C++17 compiler, CMake, Qt6 development libraries, and FreeRDP installed.

### 🐧 Debian / Ubuntu / Linux Mint
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev freerdp2-x11
# Or for FreeRDP 3:
# sudo apt install freerdp3-x11
```

### 🎩 Fedora / RHEL
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel freerdp
```

### 🏹 Arch Linux / Manjaro
```bash
sudo pacman -S base-devel cmake qt6-base freerdp
```

---

## 🚀 Building from Source

```bash
# 1. Clone repository
git clone https://github.com/<your-username>/rdpqt.git
cd rdpqt

# 2. Generate build tree
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. Compile executable
make -j$(nproc)
```

---

## 🎮 Running the Application

From the build directory:
```bash
./rdpqt
```

### Optional: System-wide Installation
```bash
sudo install -m 755 rdpqt /usr/local/bin/rdpqt
```

---

## 📂 Project Architecture

```text
rdpqt/
├── CMakeLists.txt         # Modern CMake build configuration (Qt6 automoc/autorcc/autouic)
├── README.md              # Project documentation
├── .gitignore             # Standard git exclusions
└── src/
    ├── main.cpp           # App entrypoint, metadata, and Fusion style application
    ├── MainWindow.h       # UI presentation layer and control logic header
    ├── MainWindow.cpp     # Grid/box layouts, input validators, QSettings storage
    ├── RdpManager.h       # Backend interface for xfreerdp lifecycle
    └── RdpManager.cpp     # QProcess argument construction, signals, and log stream
```

---

## 🤝 Contributing

Contributions, bug reports, and feature requests are welcome!

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add some amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

---

## 📜 License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details.

---

<div align="center">
  <sub>Built with ❤️ using C++17 and Qt6.</sub>
</div>
