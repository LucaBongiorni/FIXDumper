SRC = FixDumper.C Usage.C GetOpt.C
OBJ = $(SRC:.C=.o)
CFLAGS = -g -O 
LIB = 

CC = g++
LD = g++
FixDumper: $(OBJ)
	$(LD) $(OBJ) $(LIB) -o $@
	cp $@ ../bin

clean:
	rm *.o

$(OBJ): %.o: %.C
	$(CC) -c $(CFLAGS) $< -o $@

