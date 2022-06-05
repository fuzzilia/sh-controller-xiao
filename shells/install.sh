# curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/bin sh
arduino-cli config init
arduino-cli config add board_manager.additional_urls "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
arduino-cli update
arduino-cli core install adafruit:nrf52
arduino-cli lib install "Adafruit DotStar"
arduino-cli lib install "Adafruit BusIO"
arduino-cli lib install "Adafruit MPR121"
pip3 install adafruit-nrfutil --user