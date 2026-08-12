"""FastMCP tools for controlling the Arduino UNO Q LED matrix over gRPC."""

import os

import grpc
from fastmcp import FastMCP

import mcu_pb2
import mcu_pb2_grpc


MATRIX_WIDTH = 13
MATRIX_HEIGHT = 8
MATRIX_PIXEL_COUNT = MATRIX_WIDTH * MATRIX_HEIGHT
GRPC_TARGET = os.getenv("MCU_GRPC_TARGET", "192.168.178.188:50051")

mcp = FastMCP("Arduino LED Matrix")


@mcp.tool()
def draw_led_matrix(pixels: list[int]) -> str:
    """Draw an 8-by-13 LED matrix frame in row-major order.

    Supply exactly 104 unsigned integer pixel values. Use 0 for off and 1 for
    on when working with a monochrome display.
    """
    if len(pixels) != MATRIX_PIXEL_COUNT:
        raise ValueError(
            f"Expected {MATRIX_PIXEL_COUNT} pixels for a "
            f"{MATRIX_HEIGHT}x{MATRIX_WIDTH} matrix, received {len(pixels)}."
        )
    if any(pixel < 0 or pixel > 0xFFFFFFFF for pixel in pixels):
        raise ValueError("Each pixel must be an unsigned 32-bit integer.")

    with grpc.insecure_channel(GRPC_TARGET) as channel:
        stub = mcu_pb2_grpc.McuServiceStub(channel)
        try:
            stub.SetMatrix(mcu_pb2.MatrixRequest(pixels=pixels))
        except grpc.RpcError as exc:
            raise RuntimeError(
                f"SetMatrix RPC to {GRPC_TARGET} failed: "
                f"{exc.code().name} - {exc.details()}"
            ) from exc

    return f"Drew {MATRIX_HEIGHT}x{MATRIX_WIDTH} LED matrix frame."


if __name__ == "__main__":
    mcp.run()