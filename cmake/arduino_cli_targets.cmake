find_program(ARDUINO_CLI arduino-cli REQUIRED)
include(GNUInstallDirs)

function(add_arduino_cli_compile_target sketch_folder)
  if(IS_ABSOLUTE "${sketch_folder}")
    set(sketch_folder_abs "${sketch_folder}")
  else()
    set(sketch_folder_abs "${CMAKE_CURRENT_SOURCE_DIR}/${sketch_folder}")
  endif()

  get_filename_component(sketch_folder_name "${sketch_folder_abs}" NAME)
  set(sketch_binary "${CMAKE_BINARY_DIR}/ino_${sketch_folder_name}/ino_${sketch_folder_name}.elf-zsk.bin")

  add_custom_target(arduino_compile_${sketch_folder_name} ALL
    COMMAND ${ARDUINO_CLI} --no-color compile
            # --verbose
            -b arduino:zephyr:unoq
            ${sketch_folder_abs}
            --build-path ${CMAKE_BINARY_DIR}/ino_${sketch_folder_name}
            --output-dir ${CMAKE_BINARY_DIR}/ino_${sketch_folder_name}
            --build-property build.project_name=ino_${sketch_folder_name}
    # VERBATIM
    COMMENT "Compile Arduino sketch from ${sketch_folder_abs}"
  )

  install(FILES "${sketch_binary}"
          DESTINATION "${CMAKE_INSTALL_BINDIR}/ino_${sketch_folder_name}")
endfunction()