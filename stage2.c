#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "all_stages.h"


/*The functions that belong to Stage 2 - which aims to parse the line, encode it, and export the output files*/

void stage2_start()	/*the function make output files, and update IC and DC*/
{
	FILE* f1;
	FILE* f2;
	FILE* f3;

	f1 = fopen("ps.ent", "a");
	fclose(f1);

	f2 = fopen("ps.ext", "a");
	fclose(f2);

	f3 = fopen("ps.o", "a");
	fprintf(f3, "%d\t%d\n", IC, DC); 
	fclose(f3);

	IC = 100;
}


void stage2_pars_row(char myrow[], char label[], char is_label[])	/*the function pars the row for second time, with stage 2 algoritm*/
{
	int i, len, lenword;
	char word[MAX_WORD], row[REST_ROW], nlabel[REST_ROW], temp[REST_ROW], rest_row[REST_ROW], is_pars[6] = FALSE, is_intruction[6] = FALSE;

	strcpy(row, myrow);
	len = strlen(row);
	IN_IC = IC;

	if (len == 0)		/*check if empty row*/
		return;

	strcpy(temp, row);
	get_word(temp, word);	/*the first word in the row*/
	lenword = strlen(word);

	if (strcmp(is_label, FALSE) == 0)	/*check if label*/
	{
		if (word[lenword-1] == ':')
		{
			strcpy(rest_row, row);
			get_rest_row(row, rest_row);
			strcpy(temp, row);
			get_word(temp, nlabel);
	
			lenword = strlen(nlabel);
			substring(nlabel, nlabel, 0, lenword-1);
			strcpy(is_label, TRUE);
			stage2_pars_row(rest_row, nlabel, is_label);	 /*check the rest of the row*/
		}
	}
	for(i=0; i<9; i++)	/*check if instruction word*/
	{
		if (check_opcode_group(word) != 0)
		{
			get_rest_row(temp, row);
			stage2_coding_instruction(temp);
			strcpy(is_intruction, TRUE);
			return;
		}
	}
	if (strcmp(is_intruction, FALSE) == 0)
	{
		if (word[0] == ';')			/*check if comment statement*/
		{
			strcpy(is_pars, TRUE);
			return;
		}
		if (word[0] == '.')		/*check if directive word*/
		{
			stage2_coding_directive(label, word, row);
			strcpy(is_pars, TRUE);
		}
		if (strcpy(is_pars, FALSE) == 0)
			fprintf(stderr, "Line is invalid");
	}
}


void stage2_coding_directive(char label[], char word[], char row[])	/*the function coding directive statement*/
{
	if (strcmp(word, ".define") == 0)	/*check if macro*/
		return;
	if (strcmp(word, ".entry") == 0)	/*check if entry*/
	{
		get_rest_row(row, row);
		stage2_pars_entry(row);
	}
	else if (strcmp(word, ".extern") == 0)
	{
		return;
	}
	else 		 /*data or string*/
		return;		/*already coded in stage1 */
}


void stage2_pars_entry(char row[])		/*the function coding entry statement and add information to the ps.ent*/
{
	symbol *s = S_head;
	FILE* f;
	int val;
	char symbol[MAX_WORD];

	get_word(row, symbol);				
	
	while(s)
	{
		if (strcmp(s -> name, symbol) == 0)
		{
			f = fopen("ps.ent", "a");
			val = s -> val;
			fprintf(f, "%s\t0%d\n", symbol, val); 
			fclose(f);
			return;
		}
		s = s -> next;
	}
	fprintf(stderr, "This symbol is not exists in the symbols table - %s\n", symbol);
}

void stage2_pars_ext(char symbol[], int ic)	/*the function coding extern statement and add information to the ps.ext*/
{
	int i;
	int code[CODE_ROW];
	FILE* f;

	f = fopen("ps.ext", "a");
	fprintf(f, "%s\t0%d\n", symbol, ic); 
	fclose(f);

	for(i=0; i<CODE_ROW-1; i++)
		code[i] = 0;
	code[CODE_ROW-1] = 1;
	insert_to_code_table(ic, code);
}

void stage2_coding_instruction(char myrow[])	/*the function coding instruction statement - the first memmory word, and after the added memmory word that we need to add*/
{
	char opcode[MAX_WORD], row[REST_ROW];;
	int code[CODE_ROW];
	int opcode_num[OPCODE_BITS];
	int dst[OPERAND_BITS], src_dst[OPERAND_BITS+ARE_BITS], are[ARE_BITS];
	int opcode_group, i, j;

	strcpy(row, myrow);
	
	/*bits 10-13 (= 0-3 in the arr code) - unused bits*/
	for (i=0; i<CODE_ROW; i++)					
		code[i] = 0;	

	/*bits 6-9 (= 4-7 in the arr code) - opcode word*/
	get_word(row, opcode);		
	get_rest_row(row, row);
	stage2_coding_opcode(opcode, opcode_num);

	for (i=4, j=0; j<OPCODE_BITS; i++, j++)
		code[i] = opcode_num[j];

	/*check operands*/
	opcode_group = check_opcode_group(opcode);
	if(opcode_group == 1)			/*group 1 - there is src and dst operand*/
	{
		stage2_coding_group1(IN_IC, row, src_dst);
		for (i=8, j=0; j<OPERAND_BITS*3; i++, j++)	/*src and dst bits*/
			code[i] = src_dst[j];
	}
	else if(opcode_group == 2)		/*group 2 - only dst operand*/
	{
		stage2_coding_group2(IN_IC, row, dst);
		for (i=10, j=0; j<OPERAND_BITS; i++, j++)
			code[i] = dst[j];
	}
	else if (opcode_group == 3)		/*group 3 - no operand - unused bits*/
	{
		for (i=8; i<OPERAND_BITS*2; i++)		
			code[i] = 0;
	}

	stage2_check_are("absolute", are);	/*instruction row is always absolute*/
	for (i=12, j=0; j<ARE_BITS; i++, j++)		/*check a-r-e bits*/
		code[i] = are[j];

	insert_to_code_table(IN_IC, code);
	IC ++;
}


int* stage2_coding_opcode(char opcode[], int opcode_num[])  /*the function coding the opcode*/
{
	if (strcmp(opcode, "mov") == 0)
		change_to_bit(0, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "cmp") == 0)
		change_to_bit(1, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "add") == 0)
		change_to_bit(2, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "sub") == 0)
		change_to_bit(3, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "not") == 0)
		change_to_bit(4, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "clr") == 0)
		change_to_bit(5, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "lea") == 0)
		change_to_bit(6, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "inc") == 0)
		change_to_bit(7, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "dec") == 0)
		change_to_bit(8, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "jmp") == 0)
		change_to_bit(9, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "bne") == 0)
		change_to_bit(10, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "red") == 0)
		change_to_bit(11, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "prn") == 0)
		change_to_bit(12, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "jsr") == 0)
		change_to_bit(13, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "rts") == 0)
		change_to_bit(14, opcode_num, OPCODE_BITS);
	else if (strcmp(opcode, "stop") == 0)
		change_to_bit(15, opcode_num, OPCODE_BITS);
	return opcode_num;
}



void stage2_coding_group1(int IN_IC, char row[], int add_src_and_dst[]) /*the function coding group 1 statement - add src and dst operand*/
{
	int i, j;
	char src[MAX_WORD], dst[MAX_WORD], rest_row[REST_ROW];
	int add_src[OPERAND_BITS], add_dst[OPERAND_BITS];


	get_word(row, src);
	get_rest_row(row, rest_row);
	get_word(rest_row, dst);

	for(i=0; i<9; i++)	/*check if it is opcode*/
	{
		if (check_opcode_group(src) != 0)
		{
			strcpy(src, dst);
			get_rest_row(rest_row, rest_row);
			get_word(rest_row, dst);
		}
	} 

	if (src[0] == 'r' && dst[0] == 'r')  		/*src and dst are hamsters*/
	{
		stage2_add_src_and_dst_hamster(IN_IC, src, dst);
		change_to_bit(3, add_src, OPERAND_BITS);
		change_to_bit(3, add_dst, OPERAND_BITS);
	}
	else if (src[0] == 'r' && dst[0] != 'r')	/*only src is an hamster*/
	{
		stage2_add_src_hamster(IN_IC, src);
		change_to_bit(3, add_src, OPERAND_BITS);
		stage2_coding_not_hamster(IN_IC+1, dst, add_dst);
	}
	else if (src[0] != 'r' && dst[0] == 'r')	/*only dst is an hamster*/
	{
		stage2_coding_not_hamster(IN_IC, src, add_src);
		stage2_add_dst_hamster(IN_IC+1, dst);
		change_to_bit(3, add_dst, OPERAND_BITS);
	}
	else if (src[0] != 'r' && dst[0] != 'r')	/*no operand hamster*/
	{
		stage2_coding_not_hamster(IN_IC, src, add_src);
		stage2_coding_not_hamster(IN_IC+1, dst, add_dst);
	}
	
	/*fix the address of the src and dst operand if it is group 1*/
	for(i=0, j=0; j<OPERAND_BITS; i++, j++)
		add_src_and_dst[i] = add_src[j];
	for(i=OPERAND_BITS, j=0; j<OPERAND_BITS; i++, j++)
		add_src_and_dst[i] = add_dst[j];
}


void stage2_coding_group2(int IN_IC, char row[], int add_dst[]) 	/*the function coding group 2 statement - add only dst operand*/
{
int i;
	char dst[MAX_WORD];
	get_word(row, dst);

	for(i=0; i<9; i++)	/*check if it is opcode*/
	{
		if (check_opcode_group(dst) != 0)
		{
			get_rest_row(row, row);
			get_word(row, dst);
		}
	} 
	if (dst[0] == 'r') 	/*method 4 - direct hamster address*/
	{
		stage2_add_dst_hamster(IN_IC+1, row);
		change_to_bit(3, add_dst, OPERAND_BITS);
	}
	else
		stage2_coding_not_hamster(IN_IC, dst, add_dst);
}


void get_hamster(char hamster[], int coding_hamster[]) 	/*the function get the number of the hamster of the addressing and coding it*/
{
	int num_h;
	substring(hamster,hamster ,1 ,2);	/*this is the num of the hamster*/	
	num_h = atof(hamster);
	change_to_bit(num_h, coding_hamster, HAMSTER_NUM);	
}

void stage2_add_src_hamster(int IN_IC, char src[]) 	/*the function add and coding source hamster*/
{
	int i, j;
	int code[CODE_ROW], coding_hamster[HAMSTER_NUM];
	get_hamster(src, coding_hamster);

	IN_IC ++;
	for(i=0; i<CODE_ROW; i++)
		code[i] = 0;
	for(i=6, j=0; j<HAMSTER_NUM; i++, j++)
		code[i] = coding_hamster[j];

	insert_to_code_table(IN_IC, code);
	IC ++;
}

void stage2_add_dst_hamster(int ic, char dst[])		/*the function add and coding destination hamster*/
{
	int i, j;
	int code[CODE_ROW], coding_hamster[HAMSTER_NUM];
	get_hamster(dst, coding_hamster);
	
	IN_IC ++;
	for(i=0; i<CODE_ROW; i++)
		code[i] = 0;
	for(i=9, j=0; j<HAMSTER_NUM; i++, j++)
		code[i] = coding_hamster[j];
	insert_to_code_table(IN_IC, code);
	IC ++;
}

void stage2_add_src_and_dst_hamster(int IN_IC, char src[], char dst[])	/*the function add and coding source and destiniation hamsteres*/
{
	int i, j;
	int code[CODE_ROW], coding_src[HAMSTER_NUM], coding_dst[HAMSTER_NUM];

	IN_IC ++;
	get_hamster(src, coding_src);
	get_hamster(dst, coding_dst);
	
	for(i=0; i<CODE_ROW; i++)
		code[i] = 0;
	for(i=6, j=0; j<HAMSTER_NUM; i++, j++)
		code[i] = coding_src[j];
	for(i=9, j=0; j<HAMSTER_NUM; i++, j++)
		code[i] = coding_dst[j];
	
	insert_to_code_table(IN_IC, code);
	IC ++;
}


void stage2_coding_not_hamster(int IN_IC, char operand[], int address[]) 	/*the function cheack the place in memory for method of addressing that is not hamster and coding them*/
{
	int i, cnt = 0, len;
	char is_pars[] = FALSE;
	len = strlen(operand);

	if (operand[0] == '#')			/*method 1 - immediate address*/
	{
		stage2_coding1_immediate(IN_IC, operand, is_pars);
		change_to_bit(0, address, OPERAND_BITS);
	}

	if (strcmp(is_pars, FALSE) == 0) 	/*method 3- fixed index address*/
	{
		for(i=0; i<len; i++)
		{
			if(operand[i] == '[' || operand[i] == ']')
				cnt ++;
		}
		if(cnt == 2)			/*it is a fixed index address*/
		{			
			stage2_coding3_fixed_index(IN_IC, operand, is_pars);
			change_to_bit(2, address, OPERAND_BITS);
			return;
		}
	}

	if	(strcmp(is_pars, FALSE) == 0)	/*method 2 - direct address*/
	{	
		stage2_coding2_direct(IN_IC, operand, is_pars);
		change_to_bit(1, address, OPERAND_BITS);
	}
}


void stage2_coding1_immediate(int IN_IC, char operand[], char is_pars[])	/*the function coding method 1 - immediate address*/
{
	int val, len;
	symbol *s = S_head;

	len = strlen(operand);
	substring(operand, operand, 1, len);
	while (s)
	{
		if (strcmp(s -> name, operand) == 0)
		{
			val = s -> val;
			if (strcmp(s -> type, "extern") == 0)	
				stage2_pars_ext(operand, IN_IC+1);
			strcpy(is_pars, TRUE);
		}
		s = s -> next;
	}
	if (strcmp(is_pars, FALSE) == 0)
	{
		val = atof(operand);
	}
	stage2_add_val_mem_word(IN_IC, val, "absolute");	/*we add anothr mem word for the immediate number*/
	strcpy(is_pars, TRUE);
}


void stage2_coding2_direct(int IN_IC, char operand[], char is_pars[])	/*the function coding method 2 - direct address*/
{
	symbol *s = S_head;
	int val = 0;
	char type[MAX_WORD] = "";

	while (s)
	{
		if (strcmp(s -> name, operand) == 0)
		{
			val = s -> val;
			strcpy(type, s -> type);
			if (strcmp(s -> type, "extern") == 0)		/*check if extern*/
				stage2_pars_ext(operand, IN_IC+1);	
			strcpy(is_pars, TRUE);
		}
		s = s -> next;
	}
	
	if (strcmp(is_pars, TRUE) == 0)
		stage2_add_val_mem_word(IN_IC, val, type);	/*we add another mem word for the direct number*/
}

void stage2_coding3_fixed_index(int IN_IC, char operand[], char is_pars[])	/*the function coding method 3 - fixed index address*/
{
	int i, from, to, len, add, ind;
	char address[MAX_WORD], index[MAX_WORD], type[MAX_WORD];
	char index_symbol[6] = FALSE;
	symbol *s = S_head;

	len = strlen(operand);

	for(i=0; i<len; i++)
	{
		if (operand[i] == '[')
			from = i;
		else if(operand[i] == ']')
			to = i;
	}
	substring(operand, address, 0, from);	/*get the address name*/
	substring(operand, index, from+1, to);	/*get the index name*/

	while(s != NULL)
	{
		if(strcmp(s -> name, address) == 0)
		{
			add = s -> val;
			strcpy(type, s -> type);
			if (strcmp(s -> type, "extern") == 0)	
				stage2_pars_ext(operand, IN_IC+1);
			stage2_add_val_mem_word(IN_IC, add, type);	/*we add another mem word for the address of the array*/
			strcpy(is_pars, TRUE);
		}
		if(strcmp(s -> name, index) == 0) 
		{
			ind = s -> val;
			if (strcmp(s -> type, "extern") == 0)	
				stage2_pars_ext(operand, IN_IC+1);
			stage2_add_val_mem_word(IN_IC+1, ind, "absolute");
			strcpy(index_symbol, TRUE);
			strcpy(is_pars, TRUE);
		}
		s = s -> next;
	}

	if (strcmp(index_symbol, FALSE) == 0)	/*the index is not a symbol, it is a real num*/
	{
		ind = atof(index);
		stage2_add_val_mem_word(IN_IC+2, ind, "absolute"); 	/*we add another mem word for the index of the number in the array*/
	}
}


void stage2_add_val_mem_word(int IN_IC, int val, char type[])	/*the function add a new memmory word that is not an hamster*/
{
	int i, j;
	int code[CODE_ROW], coding_num[CODE_VAL], are[ARE_BITS];

	IN_IC ++;

	stage2_check_are(type, are);
	coding_signed_bits(val, coding_num, CODE_VAL);

	for(i=0; i<CODE_VAL; i++)
		code[i] = coding_num[i];

	for(i=CODE_VAL, j=0; j<ARE_BITS; i++, j++)
		code[i] = are[j];

	insert_to_code_table(IN_IC, code);
	IC ++;
}



void stage2_check_are(char type[], int bits[])		/*the function check what is the a-r-e definition of the statement*/
{
	int val;
	if (strcmp(type, "extern") == 0)	/*external*/
		val = 1;
	else if (strcmp(type, "data") == 0 || strcmp(type, "string") == 0 || strcmp(type, "entry") == 0 || strcmp(type, "code") == 0)
		val = 2;			/*rekocatable*/
	else
		val = 0; 			/*absolute*/
	change_to_bit(val, bits, ARE_BITS);
}



void stage2_finish_outfput()	/*the function write into the object file*/
{
	coding *c = C_head;
	FILE* f;
	int i, ic=100;
	int bits[CODE_ROW];
	char conv;

	while(ic < IC+DC+5)
	{
		while(c != NULL)
		{
			if (ic == c -> ic)
			{ 
				f = fopen("ps.o", "a");
				fprintf(f, "\n0%d\t", ic); 
				for (i=0; i<CODE_ROW; i++)
					bits[i] = c->code[i];
				i=0;
				while(i<CODE_ROW)
				{
					conv = stage2_coding_base4(bits[i], bits[i+1]);
					fprintf(f, "%c", conv); 
					i += 2;
				}
				fclose(f); 
			}
			c = c -> next;	
		}
		ic++;
		c = C_head;
	}
}


char stage2_coding_base4(int n1, int n2)	 /*the function coding object file to base 4 code*/
{
	if (n1 == 0 && n2 == 0)
		return '*';
	else if (n1 == 0 && n2 == 1)
		return '#';
	else if (n1 == 1 && n2 == 0)
		return '%';
	else /*(n1 == 1 && n2 == 1)*/
		return '!';
}



