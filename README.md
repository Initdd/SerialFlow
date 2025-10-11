# SerialFlow - Serial Port Monitor

A powerful and user-friendly serial port monitoring application built with C++ and Qt 6.

![Version](https://img.shields.io/badge/version-1.0-blue)
![Qt](https://img.shields.io/badge/Qt-6.0+-green)
![License](https://img.shields.io/badge/license-MIT-orange)

## Features

### 🔌 Connection Management
- **Auto-detect Available Serial Ports** - Automatically discovers all COM/ttyUSB devices using QSerialPortInfo
- **Flexible Connection Settings**:
  - Port selection with refresh capability
  - Configurable baud rates (9600 to 921600)
  - Customizable data bits (5, 6, 7, 8)
  - Stop bits configuration (1, 1.5, 2)
  - Parity options (None, Even, Odd, Space, Mark)

### 📡 Data Communication
- **Send Data** - Text input box for typing and sending messages
- **Receive Data** - Real-time display of incoming serial data
- **Timestamp Support** - Optional timestamp display for each message
- **Display Modes**:
  - ASCII text display
  - HEX format display

### 📊 Display Features
- **Auto-scroll** - Automatically scrolls to the latest received data
- **Clear Output** - One-click button to clear the text area
- **Formatted Output** - Color-coded TX/RX messages with timestamps
- **Connection Status Icon** - Visual indicator showing connection state

### 💾 Logging
- **Save to File** - Log all received data to a text file
- **Automatic Timestamps** - Logs include timestamps for data tracking
- **Flexible File Format** - Save as .log or .txt files

### ⚙️ Settings & Customization
- **Settings Dialog** with organized tabs:
  - Display options (HEX mode, auto-scroll, timestamps)
  - Connection parameters (data bits, stop bits, parity)
  - Keyboard shortcuts customization
- **Customizable Shortcuts**:
  - Connect/Disconnect: `Ctrl+K`
  - Send Data: `Ctrl+Return`
  - Clear Output: `Ctrl+L`
  - Refresh Ports: `F5`
- **Persistent Settings** - All preferences are saved between sessions

### 🎨 User Interface
- Clean and intuitive design
- **Light/Dark mode** toggle (Ctrl+D)
- Compact main page with essential controls
- Advanced settings hidden in dedicated dialog
- Menu bar with File, Tools, and Help menus
- Status bar with connection indicator
- No redundant toolbars - streamlined interface

## Requirements

### Build Dependencies
- **Qt 6.0 or higher**
  - Qt6::Core
  - Qt6::Widgets
  - Qt6::SerialPort
- **CMake 3.16 or higher**
- **C++17 compatible compiler**
  - GCC 7+ / Clang 5+ / MSVC 2017+

### Runtime Dependencies
- Qt 6 runtime libraries

## Building from Source

### Linux

1. **Install Qt 6 and dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev qt6-serialport-dev cmake build-essential

   # Fedora
   sudo dnf install qt6-qtbase-devel qt6-qtserialport-devel cmake gcc-c++

   # Arch Linux
   sudo pacman -S qt6-base qt6-serialport cmake gcc
   ```

2. **Clone and build**:
   ```bash
   cd SerialFlow
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Run the application**:
   ```bash
   ./SerialFlow
   ```

### macOS

1. **Install Qt 6 using Homebrew**:
   ```bash
   brew install qt@6 cmake
   ```

2. **Build the project**:
   ```bash
   cd SerialFlow
   mkdir build
   cd build
   cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
   make
   ```

3. **Run the application**:
   ```bash
   ./SerialFlow
   ```

### Windows

1. **Install Qt 6**:
   - Download and install Qt 6 from [qt.io](https://www.qt.io/download)
   - Make sure to include Qt SerialPort module

2. **Build with CMake**:
   ```cmd
   cd SerialFlow
   mkdir build
   cd build
   cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64 ..
   cmake --build . --config Release
   ```

3. **Run the application**:
   ```cmd
   Release\SerialFlow.exe
   ```

## Usage

### Quick Start

1. **Launch SerialFlow**
2. **Select a Port** from the dropdown menu (click ↻ to refresh)
3. **Choose Baud Rate** (default: 115200)
4. **Click Connect** (or press `Ctrl+K`)
5. **Send/Receive Data** using the input box

### Receiving Data
- Incoming data appears in the main text area
- Each message shows timestamp (if enabled)
- RX messages are displayed in black
- TX messages are displayed in blue

### Sending Data
- Type your message in the input box
- Press `Enter` or click **Send** (or `Ctrl+Return`)
- Messages automatically include newline character

### Logging
- Go to **File > Start Logging**
- Choose a file location
- All received data will be saved with timestamps

### Customizing Settings
- Click the **⚙ Settings** button (or `Ctrl+,`)
- Adjust display preferences in the **Display** tab
- Configure serial parameters in the **Connection** tab
- Customize keyboard shortcuts in the **Shortcuts** tab
- Click **OK** to save changes

## Keyboard Shortcuts

| Action | Default Shortcut | Customizable |
|--------|-----------------|--------------|
| Connect/Disconnect | `Ctrl+K` | ✓ |
| Send Data | `Ctrl+Return` | ✓ |
| Clear Output | `Ctrl+L` | ✓ |
| Refresh Ports | `F5` | ✓ |
| Toggle Dark Mode | `Ctrl+D` | ✗ |
| Open Settings | `Ctrl+,` | ✗ |
| Quit | `Ctrl+Q` | ✗ |

## Project Structure

```
SerialFlow/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
├── src/
│   ├── main.cpp               # Application entry point
│   ├── mainwindow.h           # Main window header
│   ├── mainwindow.cpp         # Main window implementation
│   ├── serialportmanager.h    # Serial port manager header
│   ├── serialportmanager.cpp  # Serial port manager implementation
│   ├── settingsdialog.h       # Settings dialog header
│   └── settingsdialog.cpp     # Settings dialog implementation
└── resources/
    └── resources.qrc          # Qt resource file
```

## Architecture

### SerialPortManager
- Handles all serial port operations
- Uses Qt's `QSerialPort` and `QSerialPortInfo`
- Emits signals for data reception and errors
- Thread-safe communication

### MainWindow
- Main application interface
- Manages UI components and user interactions
- Handles display formatting (ASCII/HEX)
- Implements logging functionality
- Loads/saves application settings

### SettingsDialog
- Tabbed interface for settings
- Display preferences
- Serial port configuration
- Keyboard shortcut customization

## Troubleshooting

### Port Access Issues (Linux)
If you can't access serial ports, add your user to the `dialout` group:
```bash
sudo usermod -a -G dialout $USER
```
Log out and log back in for changes to take effect.

### Port Not Found
- Check that your device is properly connected
- Click the refresh button (↻) to update the port list
- On Linux, ensure the device appears in `/dev/` (e.g., `/dev/ttyUSB0`)

### Build Errors
- Ensure Qt 6 is properly installed with SerialPort module
- Check that CMake can find Qt (`CMAKE_PREFIX_PATH`)
- Verify C++17 compiler support

## License

This project is licensed under the MIT License - see below:

```
MIT License

Copyright (c) 2025 SerialFlow

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Author

SerialFlow Development Team

## Acknowledgments

- Built with [Qt Framework](https://www.qt.io/)
- Uses Qt SerialPort module for serial communication
