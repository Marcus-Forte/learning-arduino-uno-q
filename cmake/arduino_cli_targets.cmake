find_program(ARDUINO_CLI arduino-cli REQUIRED)

add_custom_target(arduino_compile_mcu ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build/ino_mcu
  COMMAND ${ARDUINO_CLI} --no-color compile
          --verbose
          -b arduino:zephyr:unoq
          --export-binaries
          ${CMAKE_SOURCE_DIR}/src_mcu/
          --build-path ${CMAKE_BINARY_DIR}/ino_mcu
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  USES_TERMINAL
  VERBATIM
  COMMENT "Compile Arduino sketch from src_mcu"
)