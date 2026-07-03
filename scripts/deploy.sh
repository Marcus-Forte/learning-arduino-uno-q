IP=192.168.178.188

scp build/ino_mcu/*.elf-zsk.bin arduino@$IP:/tmp/firmware.elf-zsk.bin
ssh arduino@$IP arduino-cli upload -i /tmp/firmware.bin  -b arduino:zephyr:unoq -v