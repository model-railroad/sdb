# Software Defined Blocks

# Overview

### Goal

"Software Defined Blocks" (SDB) is a way to emulate block activation for a train model railroad:
A C++ program running on an embedded [ESP32](https://www.espressif.com/en/products/socs/esp32)
monitors sensors, and sends block activation signals to a server such as
[JMRI](https://www.jmri.org/) or MQTT.

Requirements:

- An embedded [ESP32](https://www.espressif.com/en/products/socs/esp32) board.
- Sensors (Time-of-Flight, infrared, camera).
- A [JMRI](https://www.jmri.org/) server with JSON or MQTT.
- A Wifi network to communicate with JMRI.


### Hardware Target

- [ESP32](https://www.espressif.com/en/products/socs/esp32) board:
    - Generic ESP32.
    - ESP32-CAM.

### Supported Sensors

- [VL53L0X](https://www.adafruit.com/product/3317) ToF Distance Sensor.
- BD20 Block Occupation Sensor and similar.
- Infrared reflective Sensor.
- Camera (only on ESP32-CAM).

### Other Resources

Blog posts describing this project:

- [Presentation of "Software Defined Blocks"](https://www.alfray.com/trains/blog/train/2023-01-29_sdb_software_defined_blocks.html)
- [Code Design](https://www.alfray.com/trains/blog/train/2023-01-31_sdb_overall_module_design.html)
- [MQTT Support](https://www.alfray.com/trains/blog/train/2023-02-24_sdb_mqtt.html)


# Development Phases

The project is going to be split in two phases, each with their own MVP.

### Phase 1: ESP32 + ToF sensor + Wifi + JmriJSON.

This phase is known to be entirely doable.
I expect most of phase 1 will be dedicated to the overall structure I want to achieve for the
program.

There are some unknowns (f.ex. how is configuration achieved, how is state reported) but that’s
because there are multiple choices, not due to lack of knowledge.

The MVP for phase 1 is the
[Stockton’s Trolley Automation](https://www.alfray.com/trains/blog/randall/2018-11-22_more_automated_routes.html)
as described in 2018.

### Phase 2: ESP32-CAM + image analysis (OpenCV or something else).

This phase is known to be purely experimental.
The camera analysis part has some obvious challenges -- should I be using OpenCV or duplicate
algorithms, can they even achieve what I want, what do I even want, etc. There’s an interesting
open research area here, yet overall it should be achievable, and once achieved the stability of
the solution remains to be measured.

To be clear, phase 2 is the “interesting” and novel part. It is based on my “Grade Crossing Camera”
project from a couple years ago, which I toyed with but never fully finished.
So it’s time to reuse what I did last time and finish it.


# Overall Module Design

SDB is designed around “modules”.

### Modules

Each module should have:

- An init method called when the main starts.
- A start method called once all other modules have been initialized.
- A loop method called as part of the main loop.
    - The loop method will return the max time it wants to sleep before being called again.
      The loop method may be invoked sooner than requested, but not later.
- Its own thread-safe message queue.

Each module is free to start a thread or do their process in the main loop.

The main loop will call all the modules’ loop methods, and sleep the minimum time requested by
all modules (similar to a classic Arduino sketch main loop).

Care should be taken to avoid having two modules use the same hardware resource -- e.g. the same
GPIO pin, or the same I2C controller, etc. One way to ensure this would be to have a global
“hardware resource allocator”, e.g. an object which “owns” all hardware resources and panics if a
resource is reused without being freed first. This seems overkill in the context of the phase 1 MVP.

### Module Manager

Modules are registered in a global ModuleManager singleton.

The module manager is also responsible for maintaining a thread-safe message queue per module.

Each module has a unique identifier code name which is 2 characters long.

### Message Queue

Inter-module communication is done by posting a message on another target module via the manager
rather than directly. Modules regularly pool their own message queue.

Although we could use a notification object to wake up a thread when a message is posted, I do not
anticipate having to do that in the first version.

The goal of the message queue is to simplify the need for synchronization if two threaded modules
call each other, and to simplify the dependency between modules. We want to avoid one module
calling methods from another module directly.

The downside of a message queue is that it is an inefficient mechanism to share large amounts of
data by copy, which we posit is not needed in phase 1. This would not work to share image buffers,
but it’s enough to share e.g. pointers.

### Data Store

The message queue is also a poor way to share frequently changing data between modules, as well as
to handle configuration data.

Thus the current implementation is to have a singleton global dictionary as a Data Store.
Some keys are transient data, whereas others are backed up to Non-Volatile Storage (NVS).

Whereas it makes more sense for each module to have its own message queue, the data store is a
global singleton. Accessors must be thread safe as they can be used by any threaded module.

The store is potentially accessible by many threads. We want to ensure consistency in reads &
writes, not to mention that dictionaries/maps typically do not have thread safe updates. Thus we
expect to have to use locks around read/writes.

Using a global dictionary is a bit more expensive than writing just global variables. However it is
deemed an adequate compromise to prevent strong module interdependence.

That means modules should consider store accesses to be expensive. As an example, we can anticipate
that the wifi / http module would update the sensor threshold configuration by writing a maximum of
a handful of values, then send a message to the adequate module to reload their configuration; that
module would read values once from the store and keep them in their own object attributes.

We probably want an API that allows a module to read & write multiple values with a single
synchronized access, even possibly a get/condition/update access.

### Block Logic

The core of the project is defining blocks, and the conditions that trigger them.

We need to elaborate on the possible conditions here.

We have 2 options here:

- Use a dedicated module that has the configuration data, reads the state of sensors from the store, and then updates the block state in the store.
- There is no dedicated module; instead each sensor module has its own logic to update block state in the store.

In both cases, a separate JMRI JSON/MQTT module reads that block state from the store, and sends the JSON/MQTT to JMRI when a change is detected.

We’ll start with a dedicated “block module”.

The configuration consists of output blocks (JMRI system names).

For each block, an input is configured with its own data.
For a ToF sensor, that data is the distance threshold, and whether the block is occupied or free
when the sensor is below that threshold.
To be clear, that means the ToF module only needs to export distance information, and the block
module is the one reading the sensor(s) distance and computing the state of the block.
That also means the trigger logic will be depending on the type of sensor, and that logic lives in
the block module rather than the sensor module.

We may want to include debounce capabilities in there in case the measurements are noisy.

### Foundations

- Module Manager
- Message Queue (accessed via Module Manager).
- Data Store.


### Expect modules

- Display.
    - Owns the OLED + its I2C driver.
    - Read store for variables to display (e.g. a simple location code +
      value).
- Wifi + HTTP.
    - Owns the wifi, handling reconnections.
    - Owns sending a web page over it.
    - Read store for states to display on that web page.
    - On configuration change, write to store + notify modules.
- ToF sensor.
    - Manages i2c sensors.
    - Optional: filter/smooth the readings, discard spikes.
    - Regularly reads sensor values and updates them into the store.
- Block Module.
    - Regularly reads store for sensor states.
    - Updates blocks states in store.
    - Notified by Wifi / HTTP if configuration has changed.
    - Notify JMRI JSON module when there’s a change.
    - Notify JMRI MQTT module when there’s a change.
    - (we’ll support both protocols upfront)
- JMRI JSON/MQTT Module(s).
  - Manages its own socket to the JMRI server.
  - Notified by Wifi / HTTP if configuration has changed.
  - Notified by Block module if blocks states have changed.


### Source Organization

The main entry point is `sdb.ino`, a cpp file. We're keeping the "ino" suffix in line with the
arduino sketch conventions. This entry point sets up the global Module Manager and its modules.

Supported C++ level is C++17.

Each module is implemented in its own `src/*.h` header file, one class per header file.
There is no `.cpp` file. Eventually these will be converted to modules on C++20 support
is enabled in the ESP-IDF toolchain.


# Building and Deploying

### Source

```
$ git clone git@bitbucket.org:alf-labs/sdb.git
```

### VSCode Setup Instructions

In VSCode, we use the "Arduino" extension, and _not_ the "Espressif IDF" extension.
Add these 2 extensions:

- [Arduino for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino) by Microsoft
- [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) by Microsoft

The first run:

- Use Extensions > Arduino > Install.
- Open Workspace > …/sdb/sdb.code-workspace
- When asked by the Arduino Extension: “Use bundled arduino-cli”.
  If not, can be done in the extension settings.

To connect your ESP32 and deploy to it:

- F1 > Arduino Board Manager
- Section Additional URLs > Workspace > Add Item
- Add this URL to the board list:
  “https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json”

Alternatively, on Cygwin, the same can be done from the command line:

```
$ ./_arduino_cli.sh config init --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
$ ./_arduino_cli.sh core update-index
```

Finally, you need to select your board model:

- F1 > Arduino Board Manager
    - Search for “esp”
    - Install esp32 by Espressif, version 2.06
- F1 > Arduino Board Config
    - Should be picked up from the sdb project settings.
    - Current is `Heltec Wifi Kit 32, PSRAM disabled, 240 MHz`.

Actual build and deploy is done via the command line script indicated below.


### CLion Instructions

**Preparation**:

- Build once using `_compile.sh`.
- Run `_gen_cmakelists.sh`.

This parses the `build` from arduino-cli and generates the `CMakeLists.txt` file needed by CLion.
You only need to do that once before opening the project with CLion and later when adding new
source files to the project.

In CLion:

- Use the Welcome Wizard or File > Open, select `sdb/CMakeLists.txt`, and chose to _Open as Project_.
- In the _Open Project Wizard_, keep the default "Debug" profile with default MinGW.
- Once project is loaded, open `sdb.ino` or any of the `src/*.h`.

This `CMakeLists.txt` defines a custom cross-compiling toolchain, as well as one project "SDB" with
all the sources.
The main `sdb.ino` is treated as a C++ 11 file, and all `src/` headers are included.

This config is only used for editing sources.

Actual build and deploy is done via the command line script indicated below.

Note: after adding new directories or source files to the project, run `_compile.sh` and
`_gen_cmakelists.sh` to update `CMakeLists.txt`. CLion should then refresh its project view.


### Build and Deploy

The git repo comes with an aptly named `_compile_and_monitor.sh` script for Linux, MSYS, or Cygwin.
There's an equivalent `_compile_and_monitor.bat` for CMD. A few variants are provided:

- `_compile.sh`: Compiles using arduino-cli; does not upload to ESP32; does not monitor.
- `_compile_and_monitor.sh`: The main script to compile, upload, and monitor the stdout.
- `_compile_and_debug.sh`: A variation building with debug info, without any optimizations.
  Also preforms upload and monitor stdout.
- `_compile_and_monitor.bat`: A short version for usage with CMD on Windows. Has less capabilities.


**Preparation**:

- On Windows, connect your ESP32 and use the Device Manager to figure which COM port is in use.
- On Linux, use `lsusb` or `dmesg` to find the appropriate `/dev/ttyS` port.
- **Edit `sketch.yaml` and `_compile_and_monitor.bat` to properly set the USB port**.
- The default is `COM5`.
  Adjust the value to match your USB port.

The first compilation takes about forever, and it unfortunately doesn't list which files are
being compiled as it goes. You can add `--verbose` to list them, but then it's _really_ too verbose.

**Warning**: The HTML-to-gz generation is only done in the shell script.

**Tip**: On Windows, a default install of VS Code or CLion will likely run a PowerShell terminal.
The ".bat" script can be used for that.
Alternatively, if you install Git for Windows, you can change the terminal type in VS Code or
CLion and select the `MSYS Git Bash` option to run the ".sh" script.

# VL53L0X Wiring

- VL53L0X VIN ⇒ to ESP32 5V (or 3V3, technically both work).
- VL53L0X GND ⇒ to ESP32 GND.
- VL53L0X SCL ⇒ to ESP32 SCL 22.
- VL53L0X SDA ⇒ to ESP32 SDA 21.
- VL53L0X GPIO ⇒ n/c
- VL53L0X xshut ⇒ ESP32 GPIO 18 for ToF0, GPIO 23 for ToF1.
  - ⇒ In this case, ToF sensors change from I2C address 0x29 to 0x30 + 0x31 respectively.
- VL53L0X xshut ⇒ n/c (an option if only one ToF sensor is used)
  - If xshut is not connected, can only have one ToF at I2C 0x29, and need to disable the address-change logic in `mod_tof.h`.
- ESP32 GPIO 19 ⇒ LED to 200 Ohm resistor to GND (optional).
- ESP32 GPIO 36 ⇒ Leave floating (pull-up). Connect to GND to force WiFi in AP mode and ignore STA info stored in NVS.


# License

[GNU General Public License (GPL) version 3](https://www.gnu.org/licenses).

SDB is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

SDB is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see https://www.gnu.org/licenses/.


~~
