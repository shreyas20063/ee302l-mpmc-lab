main.hex: main.c
	sdcc -mmcs51 --xram-size 1024 main.c
	packihx main.ihx > main.hex

flash: main.hex
	../tools/flash51.sh main.hex

clean:
	rm -f *.ihx *.lk *.map *.mem *.rel *.rst *.sym *.lst *.asm
