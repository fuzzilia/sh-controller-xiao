# curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/bin sh
arduino-cli config init
arduino-cli config add board_manager.additional_urls "https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json"
arduino-cli update
arduino-cli core install Seeeduino:nrf52