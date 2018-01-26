CXX=gcc

default: shell

shell.o : shell.c

clean :
	@-rm $(NAME)
