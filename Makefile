project:ir_decode.o led.o servo.o cJSON.o curses.o
	gcc curses.o ir_decode.o led.o servo.o cJSON.o -lwiringPi -lm -lcurses -o project
clean:
	rm -f *.o

