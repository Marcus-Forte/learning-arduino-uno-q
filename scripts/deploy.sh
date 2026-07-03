# Upload firmware to Arduino Uno Q1 MCU

IP=192.168.178.188

# Deploy MPU App
scp build/learning_arduino_q1 arduino@$IP:/tmp/learning_arduino_q1


# Deploy MCU App
ssh arduino@$IP mkdir -p /tmp/firmware
scp build/ino_mcu/*.elf-zsk.bin arduino@$IP:/tmp/firmware/firmware.elf-zsk.bin
ssh arduino@$IP arduino-cli upload -i /tmp/firmware/ -b arduino:zephyr:unoq -v