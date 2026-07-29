# Hybrid RPC architecture

This document illustrates the current hybrid RPC application: a Linux-side gRPC service forwards requests to an Arduino-compatible MCU over a local bridge socket, and the MCU firmware actuates hardware such as LEDs and the LED matrix.

## Deployment diagram

```mermaid
flowchart LR
    subgraph Client["gRPC Client"]
        gRPCClient["gRPC client / test app"]
    end

    subgraph LinuxHost["UNO Q MPU"]
        Server["gRPC server\nsrc/bridge/main.cc"]
        Bridge["Bridge client\nsrc/bridge/bridge.cc"]
        RouterSock["Unix socket\n/var/run/arduino-router.sock"]
    end

    subgraph ArduinoDevice["UNO Q MCU"]
        RouterBridge["Arduino RouterBridge library"] --> Firmware["Firmware sketch\nsrc_mcu/rpc/rpc.ino"]
        Firmware --> |Draw LEDs| Hardware["LED matrix"]
    end

    gRPCClient -->|grpc.setMatrix| Server
    Server --> Bridge
    Bridge --> RouterSock
    RouterSock --> |setMatrix| RouterBridge
```

## Data flow

```mermaid
sequenceDiagram
    participant C as gRPC Client
    participant GS as Uno Q MPU gRPC Server
    participant B as Bridge Client
    participant R as Router Socket
    participant A as UNO Q MCU Firmware
    participant H as Hardware

    C->>GS: SetLed or SetMatrix(request)
    GS->>B: Forward method call
    B->>R: MsgPack request [type=0, msgid, method, params]
    R->>A: RouterBridge callback dispatch

    alt SetLed
        A->>H: digitalWrite(LED3_B, ...)
    else SetMatrix
        A->>H: matrix.draw(pixels)
    end

    A-->>R: Success response
    R-->>B: Acknowledged response
    B-->>GS: Result / error
    GS-->>C: gRPC status OK or failure
```

## Notes

- The Linux side uses gRPC for service RPCs and protobuf messages.
- The bridge layer serializes requests with MessagePack over a Unix domain socket.
- The Arduino sketch exposes callbacks such as set_led_state and set_matrix through the RouterBridge interface.
