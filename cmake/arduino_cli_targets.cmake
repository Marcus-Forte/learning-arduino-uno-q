find_program(ARDUINO_CLI arduino-cli REQUIRED)

add_custom_target(arduino_compile_mcu ALL
  COMMAND ${ARDUINO_CLI} --no-color compile
          --verbose
          -b arduino:zephyr:unoq
          ${CMAKE_SOURCE_DIR}/src_mcu/
          --build-path ${CMAKE_BINARY_DIR}/ino_mcu
  VERBATIM
  COMMENT "Compile Arduino sketch from src_mcu"
)