This project was setup using the ESP32 board therefore the corresponding toolchain needs to be installed 
Add the espresif toolchain to zephyr : west espressif install
Bring the toolchain up to date : west espressif update

Additionally the environment variables need to be setup, please check your paths,
Linux:
export ESPRESSIF_TOOLCHAIN_PATH="${HOME}/.espressif/tools/zephyr/xtensa-esp32-elf"
export PATH=$ESPRESSIF_TOOLCHAIN_PATH/bin:$PATH

Optionally you could set the default toolchain to espressif, but this should be taken care of in the CMakeLists.txt
export ZEPHYR_TOOLCHAIN_VARIANT="espressif"

For more details checkout the following page: https://docs.zephyrproject.org/2.6.0/boards/xtensa/esp32/doc/index.html

The Micro-USB Port exposes a UART interface which will be available as /dev/ttyUSB<n> and uses a Baudrate of 115200 
Example Minicom command:
minicom -b 115200 -D /dev/ttyUSB0
