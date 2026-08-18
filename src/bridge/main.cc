#include <grpcpp/grpcpp.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <memory>
#include <string>

#include "bridge.hh"

#include "mcu_service_simulator.hh"

#include "google/protobuf/empty.pb.h"
#include "mcu.grpc.pb.h"

class McuServiceImpl final : public McuService::Service {
public:
  grpc::Status SetLed(grpc::ServerContext * /*context*/,
                      const LedRequest *request,
                      google::protobuf::Empty * /*response*/) override {
    std::cout << "SetLed called, on=" << (request->on() ? "true" : "false")
              << '\n';
    if (!bridge_.SetLedState(request->on())) {
      std::cerr << "Failed to forward set_led_state to router" << '\n';
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to call router socket");
    }

    return grpc::Status::OK;
  }

  grpc::Status SetMatrix(grpc::ServerContext * /*context*/,
                         const MatrixRequest *request,
                         google::protobuf::Empty * /*response*/) override {
    std::cout << "SetMatrix called, pixels_size=" << request->pixels_size()
              << '\n';
    if (request->pixels_size() != 104) {
      std::cerr << "SetMatrix rejected: expected 104 pixels, got "
                << request->pixels_size() << '\n';
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                          "pixels must contain exactly 104 values");
    }

    std::array<uint8_t, 104> pixels{};
    for (int i = 0; i < request->pixels_size(); ++i) {
      const uint32_t value = request->pixels(i);
      if (value > 255u) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                            "pixel values must be in range 0..255");
      }
      pixels[static_cast<std::size_t>(i)] = static_cast<uint8_t>(value);
    }

    if (!bridge_.SetMatrix(pixels)) {
      std::cerr << "Failed to forward set_matrix to router" << '\n';
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Failed to call router socket");
    }
    std::cout << "SetMatrix completed" << '\n';

    return grpc::Status::OK;
  }

private:
  Bridge bridge_;
};

int main(int argc, char *argv[]) {
  bool simulation = false;
  int opt;
  while ((opt = getopt(argc, argv, "sh")) != -1) {
    if (opt == 's') {
      simulation = true;
    } else if (opt == 'h' || opt == '?') {
      std::cout << "Usage: " << argv[0] << " [-s] [-h]\n"
                << "  -s  Run with the simulated MCU service\n"
                << "  -h  Show this help message\n";
      return opt == 'h' ? 0 : 1;
    }
  }

  const std::string server_address("0.0.0.0:50051");
  std::shared_ptr<McuService::Service> service;
  if (simulation) {
    service = std::make_shared<McuServiceSimulator>();
  } else {
    service = std::make_shared<McuServiceImpl>();
  }

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(service.get());

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "Failed to start gRPC server on " << server_address << '\n';
    return 1;
  }

  std::cout << "gRPC server listening on " << server_address << '\n';
  server->Wait();
  return 0;
}