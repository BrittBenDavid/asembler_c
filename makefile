prog: all_stages.o stage1.o stage2.o assembler.o 
	gcc -g -Wall -ansi -pedantic all_stages.o stage1.o stage2.o assembler.o -o prog -lm


assembler.o: assembler.c all_stages.o stage1.o stage2.o all_stages.h
	gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o -lm

all_stages.o: all_stages.c all_stages.h
	gcc -c -Wall -ansi -pedantic all_stages.c -o all_stages.o -lm

stage1.o: stage1.c all_stages.o all_stages.h
	gcc -c -Wall -ansi -pedantic stage1.c -o stage1.o -lm

stage2.o: stage2.c all_stages.o all_stages.h
	gcc -c -Wall -ansi -pedantic stage2.c -o stage2.o -lm




