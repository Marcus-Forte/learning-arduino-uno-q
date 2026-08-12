#include "mcu_service_simulator.hh"
#include <print>

grpc::Status
McuServiceSimulator::SetLed(grpc::ServerContext * /*context*/,
                            const LedRequest * /*request*/,
                            google::protobuf::Empty * /*response*/) {
  std::print("SetLed called on McuServiceSimulator\n");
  return grpc::Status::OK;
}

grpc::Status
McuServiceSimulator::SetMatrix(grpc::ServerContext * /*context*/,
                               const MatrixRequest *request,
                               google::protobuf::Empty * /*response*/) {
  std::print("SetMatrix called on McuServiceSimulator\n");
  if (request->pixels_size() != 104) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "pixels must contain exactly 104 values");
  }

  for (int row = 0; row < 8; ++row) {
    for (int column = 0; column < 13; ++column) {
      const auto pixel = request->pixels(row * 13 + column);
      std::print("{}", pixel == 0 ? "  " : "##");
    }
    std::print("\n");
  }

  return grpc::Status::OK;
}