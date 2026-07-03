FROM mdnf1992/cpp-dev

RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh && \
    arduino-cli core update-index && \
    arduino-cli core install arduino:zephyr && \
    arduino-cli lib install "Arduino_RouterBridge"


RUN cd /tmp && git clone https://github.com/msgpack/msgpack-c.git && \
    cd msgpack-c && \
    git checkout cpp_master && \
    cmake . && \
    cmake --build . --target install && \
    rm -r /tmp/msgpack-c
