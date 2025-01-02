OBJS_COMMON = start.o

all: tests/copper-001.bin tests/sprite-001.bin

%.o: %.c
	clang --target=riscv32 -march=rv32imc -std=c99 -I. -O3 -Os -mno-relax -Wall -Werror -c $< -o $@

%.o: %.S
	clang --target=riscv32 -march=rv32imc -mno-relax -Wall -Werror -c $< -o $@

%.elf: %.o $(OBJS_COMMON) system.ld
	ld.lld -T system.ld $(OBJS_COMMON) $< -o $@ -M

%.bin: %.elf
	riscv64-unknown-elf-objcopy --keep-section=.text --keep-section=.data --output-target=binary $< $@

%.vh: %.bin
	hexdump -v -e '4/4 "%08x " "\n"' $< > $@

clean:
	rm -f *.o *.elf *.bin *.vh tests/*.o tests/*.elf tests/*.bin tests/*.vh

