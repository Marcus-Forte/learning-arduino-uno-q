#pragma once

#include <array>
#include <cstdint>
#include <msgpack.hpp>
#include <mutex>

class Bridge {
public:
    Bridge();
    ~Bridge();

    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;

    // Format: [type=0 (Request), msgid=1, method="set_led_state", params=[led_state]]
    bool SetLedState(bool led_state);

    // Format: [type=0 (Request), msgid, method="set_matrix", params=[104-element pixel array]]
    bool SetMatrix(const std::array<uint8_t, 104>& pixels);

private:
    bool EnsureConnectedLocked();
    void MarkDisconnectedLocked();

    bool connected_;
    int socket_fd_;
    std::uint32_t next_msgid_;
    std::mutex socket_mu_;
    msgpack::unpacker unpacker_;
};