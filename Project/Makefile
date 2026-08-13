# IR remote cloner - build on macOS with sdcc
# (lab machines: open main.c in MCU 8051 IDE and compile there instead)

main.hex: main.c
	sdcc -mmcs51 --model-small --xram-size 1024 main.c
	packihx main.ihx > main.hex

flash: main.hex
	dfu-programmer at89c5131 erase
	dfu-programmer at89c5131 flash main.hex

clean:
	rm -f *.ihx *.lk *.map *.mem *.rel *.rst *.sym *.lst *.asm
