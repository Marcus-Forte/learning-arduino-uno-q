FROM mdnf1992/cpp-dev as dev

# Install ino compilation toolchain
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh && \
    arduino-cli core update-index && \
    arduino-cli core install arduino:zephyr && \
    arduino-cli lib install "Arduino_RouterBridge"


# Install msgpack for RPC
RUN apt-get update && apt-get install -y libmsgpack-cxx-dev libgpiod-dev && \
    rm -rf /var/lib/apt/lists/*

FROM dev AS build

COPY . /app
WORKDIR /app

RUN mkdir -p build && cd build && \
    cmake --preset preset .. && \
    cmake --build . --target install


FROM debian:trixie-slim AS runtime
COPY --from=build /app/build/install /app