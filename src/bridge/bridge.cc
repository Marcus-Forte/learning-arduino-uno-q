#include "bridge.hh"

#include <msgpack.hpp>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

// Unix socket for router
static const char *g_socket = "/var/run/arduino-router.sock";

namespace {
constexpr std::size_t kRecvChunk = 4096;
constexpr int kSocketTimeoutSec = 3;
} // namespace

bool Bridge::EnsureConnectedLocked() {
  if (connected_ && socket_fd_ >= 0) {
    return true;
  }

  socket_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    std::cerr << "socket() failed: " << std::strerror(errno) << '\n';
    return false;
  }

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, g_socket, sizeof(addr.sun_path) - 1);

  if (connect(socket_fd_, reinterpret_cast<const sockaddr *>(&addr),
              sizeof(addr)) < 0) {
    std::cerr << "connect() failed: " << std::strerror(errno) << '\n';
    close(socket_fd_);
    socket_fd_ = -1;
    return false;
  }

  timeval timeout{};
  timeout.tv_sec = kSocketTimeoutSec;
  timeout.tv_usec = 0;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    std::cerr << "setsockopt(SO_RCVTIMEO) failed: " << std::strerror(errno)
              << '\n';
  }
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    std::cerr << "setsockopt(SO_SNDTIMEO) failed: " << std::strerror(errno)
              << '\n';
  }

  std::cerr << "[Bridge] Connected to router socket " << g_socket << '\n';

  unpacker_.remove_nonparsed_buffer();
  connected_ = true;
  return true;
}

void Bridge::MarkDisconnectedLocked() {
  connected_ = false;
  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  unpacker_.remove_nonparsed_buffer();
}

Bridge::Bridge() : connected_(false), socket_fd_(-1), next_msgid_(1) {
  const bool ok = EnsureConnectedLocked();
  if (!ok) {
    std::cerr << "Bridge startup: router socket not connected yet" << '\n';
  }
}

Bridge::~Bridge() {
  std::lock_guard<std::mutex> lock(socket_mu_);
  MarkDisconnectedLocked();
}

bool Bridge::SetLedState(bool led_state) {
  std::lock_guard<std::mutex> lock(socket_mu_);
  if (!EnsureConnectedLocked()) {
    std::cerr << "Bridge socket is not connected" << '\n';
    return false;
  }

  const std::uint32_t msgid = next_msgid_++;
  std::cerr << "[Bridge] -> set_led_state msgid=" << msgid
            << " value=" << (led_state ? "true" : "false") << '\n';

  msgpack::sbuffer request;
  msgpack::packer<msgpack::sbuffer> packer(request);

  packer.pack_array(4);
  packer.pack(0); // request type
  packer.pack(msgid);
  packer.pack(std::string_view{"set_led_state"});
  packer.pack_array(1);
  packer.pack(led_state);

  const char *data = request.data();
  std::size_t remaining = request.size();
  while (remaining > 0) {
    const ssize_t sent = send(socket_fd_, data, remaining, 0);
    if (sent < 0) {
      std::cerr << "[Bridge] send(set_led_state) failed msgid=" << msgid << ": "
                << std::strerror(errno) << '\n';
      MarkDisconnectedLocked();
      return false;
    }
    data += sent;
    remaining -= static_cast<std::size_t>(sent);
  }
  std::cerr << "[Bridge] set_led_state msgid=" << msgid
            << " sent, waiting for response" << '\n';

  while (true) {
    msgpack::object_handle handle;
    if (!unpacker_.next(handle)) {
      unpacker_.reserve_buffer(kRecvChunk);
      const ssize_t received =
          recv(socket_fd_, unpacker_.buffer(), kRecvChunk, 0);
      if (received <= 0) {
        std::cerr << "[Bridge] recv(set_led_state) failed msgid=" << msgid
                  << ": " << std::strerror(errno) << '\n';
        MarkDisconnectedLocked();
        return false;
      }
      unpacker_.buffer_consumed(static_cast<std::size_t>(received));
      std::cerr << "[Bridge] recv(set_led_state) bytes=" << received
                << " msgid=" << msgid << '\n';
      continue;
    }

    try {
      const msgpack::object &obj = handle.get();
      if (obj.type != msgpack::type::ARRAY || obj.via.array.size < 2) {
        // Ignore unrelated malformed frames and keep parsing stream.
        continue;
      }

      int message_type = 0;
      obj.via.array.ptr[0].convert(message_type);
      if (message_type != 1) {
        std::cerr << "[Bridge] skip non-response frame type=" << message_type
                  << " while waiting msgid=" << msgid << '\n';
        // Ignore requests/notifications on the same stream.
        continue;
      }

      std::uint32_t response_msgid = 0;
      obj.via.array.ptr[1].convert(response_msgid);
      if (response_msgid != msgid) {
        std::cerr << "[Bridge] skip response for other msgid=" << response_msgid
                  << " while waiting msgid=" << msgid << '\n';
        // Ignore stale responses from previous calls.
        continue;
      }

      if (obj.via.array.size != 4) {
        std::cerr << "Invalid response format" << '\n';
        return false;
      }

      if (!obj.via.array.ptr[2].is_nil()) {
        std::string err;
        try {
          obj.via.array.ptr[2].convert(err);
        } catch (...) {
          err = "<non-string error>";
        }
        std::cerr << "Router returned an error: " << err << '\n';
        return false;
      }

      std::cerr << "[Bridge] <- set_led_state msgid=" << msgid << " ok" << '\n';
      return true;
    } catch (const std::exception &e) {
      std::cerr << "MessagePack decode failed (set_led_state msgid=" << msgid
                << "): " << e.what() << '\n';
      return false;
    }
  }
}

bool Bridge::SetMatrix(const std::array<uint8_t, 104> &pixels) {
  std::lock_guard<std::mutex> lock(socket_mu_);
  if (!EnsureConnectedLocked()) {
    std::cerr << "Bridge socket is not connected" << '\n';
    return false;
  }

  const std::uint32_t msgid = next_msgid_++;
  std::cerr << "[Bridge] -> set_matrix msgid=" << msgid
            << " pixels=104 first4=[" << static_cast<int>(pixels[0]) << ","
            << static_cast<int>(pixels[1]) << "," << static_cast<int>(pixels[2])
            << "," << static_cast<int>(pixels[3]) << "]" << '\n';

  msgpack::sbuffer request;
  msgpack::packer<msgpack::sbuffer> packer(request);

  // [type=0, msgid, "set_matrix", [pixels_bin]]
  packer.pack_array(4);
  packer.pack(0); // request type
  packer.pack(msgid);
  packer.pack(std::string_view{"set_matrix"});
  packer.pack_array(1); // params: one argument
  packer.pack_bin(pixels.size());
  packer.pack_bin_body(reinterpret_cast<const char *>(pixels.data()),
                       pixels.size());

  const char *data = request.data();
  std::size_t remaining = request.size();
  while (remaining > 0) {
    const ssize_t sent = send(socket_fd_, data, remaining, 0);
    if (sent < 0) {
      std::cerr << "[Bridge] send(set_matrix) failed msgid=" << msgid << ": "
                << std::strerror(errno) << '\n';
      MarkDisconnectedLocked();
      return false;
    }
    data += sent;
    remaining -= static_cast<std::size_t>(sent);
  }
  std::cerr << "[Bridge] set_matrix msgid=" << msgid
            << " sent, waiting for response" << '\n';

  while (true) {
    msgpack::object_handle handle;
    if (!unpacker_.next(handle)) {
      unpacker_.reserve_buffer(kRecvChunk);
      const ssize_t received =
          recv(socket_fd_, unpacker_.buffer(), kRecvChunk, 0);
      if (received <= 0) {
        std::cerr << "[Bridge] recv(set_matrix) failed msgid=" << msgid << ": "
                  << std::strerror(errno) << '\n';
        MarkDisconnectedLocked();
        return false;
      }
      unpacker_.buffer_consumed(static_cast<std::size_t>(received));
      std::cerr << "[Bridge] recv(set_matrix) bytes=" << received
                << " msgid=" << msgid << '\n';
      continue;
    }

    try {
      const msgpack::object &obj = handle.get();
      if (obj.type != msgpack::type::ARRAY || obj.via.array.size < 2) {
        continue;
      }

      int message_type = 0;
      obj.via.array.ptr[0].convert(message_type);
      if (message_type != 1) {
        std::cerr << "[Bridge] skip non-response frame type=" << message_type
                  << " while waiting msgid=" << msgid << '\n';
        continue;
      }

      std::uint32_t response_msgid = 0;
      obj.via.array.ptr[1].convert(response_msgid);
      if (response_msgid != msgid) {
        std::cerr << "[Bridge] skip response for other msgid=" << response_msgid
                  << " while waiting msgid=" << msgid << '\n';
        continue;
      }

      if (obj.via.array.size != 4) {
        std::cerr << "Invalid response format" << '\n';
        return false;
      }

      if (!obj.via.array.ptr[2].is_nil()) {
        std::string err;
        try {
          obj.via.array.ptr[2].convert(err);
        } catch (...) {
          err = "<non-string error>";
        }
        std::cerr << "Router returned an error: " << err << '\n';
        return false;
      }

      std::cerr << "[Bridge] <- set_matrix msgid=" << msgid << " ok" << '\n';
      return true;
    } catch (const std::exception &e) {
      std::cerr << "MessagePack decode failed (set_matrix msgid=" << msgid
                << "): " << e.what() << '\n';
      return false;
    }
  }
}