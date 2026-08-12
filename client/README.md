# Proto Generation

Generate Python protobuf and gRPC stubs from ../proto/mcu.proto:

uv run python -m grpc_tools.protoc -I../proto --python_out=. --grpc_python_out=. ../proto/mcu.proto

This creates:
- mcu_pb2.py
- mcu_pb2_grpc.py

Then run the client:

uv run main.py

# MCP Server

Run the FastMCP server over stdio:

uv run server.py

It exposes `draw_led_matrix(pixels)`, which accepts 104 unsigned integer
pixels in row-major order for the 8x13 LED matrix. Set `MCU_GRPC_TARGET` to
override the default gRPC target (`192.168.178.188:50051`).

For an MCP client configuration using `uv`:

```json
{
	"mcpServers": {
		"arduino-led-matrix": {
			"command": "uv",
			"args": ["run", "server.py"],
			"cwd": "${workspaceFolder}/tmp"
		}
	}
}
```
