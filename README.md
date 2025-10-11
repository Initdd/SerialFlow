# SerialFlow

**SerialFlow** is a lightweight and user-friendly serial port monitor built with **C++** and **Qt 6**.

![Version](https://img.shields.io/badge/version-1.0-blue)
![Qt](https://img.shields.io/badge/Qt-6.0+-green)
![License](https://img.shields.io/badge/license-MIT-orange)

---

## Features

- **Automatic port detection** (COM/ttyUSB)
- **Configurable connection settings**  
  Baud rate, data bits, stop bits, and parity
- **Send & receive data** in ASCII or HEX
- **Timestamps and colour-coded TX/RX output**
- **Logging** to `.log` or `.txt` with timestamps
- **Persistent settings** between sessions
- **Customisable keyboard shortcuts**
- **Simple, clean Qt interface**

---

## Build Requirements

- **Qt 6.0+** (`Qt6::Core`, `Qt6::Widgets`, `Qt6::SerialPort`)  
- **CMake 3.16+**  
- **C++17 compiler** (GCC 7+, Clang 5+, MSVC 2017+)

---

## Build Instructions

### Linux
```bash
sudo apt install qt6-base-dev qt6-serialport-dev cmake build-essential
git clone https://github.com/yourname/SerialFlow.git
cd SerialFlow && mkdir build && cd build
cmake ..
make
./SerialFlow
```

### macOS
```bash
brew install qt@6 cmake
git clone https://github.com/yourname/SerialFlow.git
cd SerialFlow && mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) ..
make
./SerialFlow
```

### Windows
```cmd
cd SerialFlow
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64 ..
cmake --build . --config Release
Release\SerialFlow.exe
```

---

## Usage

1. Launch **SerialFlow**
2. Select a **serial port** and **baud rate**
3. Click **Connect** (or press `Ctrl+K`)
4. Send/receive data in real time
5. Enable logging via **File → Start Logging**

---

## Keyboard Shortcuts

| Action | Shortcut |
|--------|-----------|
| Connect / Disconnect | `Ctrl+K` |
| Send Data | `Ctrl+Return` |
| Clear Output | `Ctrl+L` |
| Refresh Ports | `F5` |
| Open Settings | `Ctrl+,` |
| Quit | `Ctrl+Q` |

---

## Troubleshooting

- **Permission denied (Linux):**
  ```bash
  sudo usermod -a -G dialout $USER
  ```
  Log out and back in.

- **Qt not found:**  
  Ensure `CMAKE_PREFIX_PATH` points to your Qt installation.

---

## License

Released under the **MIT License**.  
© 2025 SerialFlow Development Team
