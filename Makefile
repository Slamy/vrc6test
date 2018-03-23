NAME = vrc6test
EMULATOR = fceux

$(NAME).nes: *.c *.s
	cc65 --target nes -Oi $(NAME).c --add-source
	ca65 $(NAME).s
	ca65 reset.s
	ld65 --mapfile map -C $(NAME).cfg -o $(NAME).nes reset.o $(NAME).o nes.lib

clean:
	rm -f ./$(NAME).s
	rm -f ./*.o
	rm -f ./$(NAME).nes

play: $(NAME).nes
	$(EMULATOR) $(NAME).nes

