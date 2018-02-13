Member 1: Nicholas Kelton
Member 2: Davis Phillips
Member 3: Bryce Vokus


 p1-Kelton-Phillips-Vokus.tar contents:
	README
	shell.c		//main implementation, including all helper functions 
	Makefile
	report.txt
	
Completed using: linprog

To build:
$> make

To cleam:
$> make clean 

Known bugs:
1. Zombie processes are allowed to run free
2. When esleep is run after io, random memory garbage appended to esleep CMD argument
3. An extra element is appended to a line where the final argument is &. The argument is always empty 

To do:
1. Still need to do piping
2. Still need to implement background processes
3. Still need to rememdy memory leaks 

Addtional comments:
-We used a dynamic two dimensional array for the commmands and dynamically allocated space at each element.

-We decided not to use util.h and util.c files which lead to some disorganization and messy coding practices. In
the future, we will work on improving readability and orderliness.

-Because project involved performing a number of functionalities, sometimes functions were written for too specific
of purposes. This meant that other, very similiar functions needed to be written. In the future, we would like to
revise our code so that such functions are consolidated into one. This would simplify the code immensly. 