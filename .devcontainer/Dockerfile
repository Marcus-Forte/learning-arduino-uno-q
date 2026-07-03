FROM mdnf1992/cpp-dev

RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh && \
    arduino-cli core update-index && \
    arduino-cli core install arduino:zephyr && \
    arduino-cli lib install "Arduino_RouterBridge"