NAME = vrc6test
EMULATOR = fceux

$(NAME).nes: *.c *.s *.h Makefile
	cc65 --target nes vrc6test.c
	
	ca65 --target nes vrc6test.s
	ca65 --target nes reset.s
	ca65 --target nes joystick.s
	
	ld65 --mapfile map -C $(NAME).cfg -o $(NAME).nes reset.o vrc6test.o joystick.o nes.lib

clean:
	rm -f ./$(NAME).s
	rm -f ./*.o
	rm -f ./$(NAME).nes

play: $(NAME).nes
	$(EMULATOR) $(NAME).nes

