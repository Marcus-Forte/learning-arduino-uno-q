#pragma once

#include "mcu.grpc.pb.h"

class McuServiceSimulator final : public McuService::Service {
public:
  grpc::Status SetLed(grpc::ServerContext *context,
                      const LedRequest *request,
                      google::protobuf::Empty *response) override;

  grpc::Status SetMatrix(grpc::ServerContext *context,
                         const MatrixRequest *request,
                         google::protobuf::Empty *response) override;
};