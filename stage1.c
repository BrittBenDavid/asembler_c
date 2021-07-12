#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "all_stages.h"

/*The functions that belong to Stage 1 - which aims to perform initial parsing of rows, and update the symbol table, data table ,IC and DC*/

void stage1_pars_row(char myrow[], char label[], char is_label[]) 	/*the function pars the row for first time, with stage 1 algoritm*/
{
	int i, len, lenword;
	char word[MAX_WORD], rest_row[REST_ROW], nlabel[MAX_WORD], row[REST_ROW], temp[REST_ROW], is_pars[6] = FALSE;

	strcpy(row, myrow);
	len = strlen(row);
	strcpy(SAME_MEM_WORD, FALSE);

	if (len == 0)			/*check if empty row*/
		return;
	strcpy(temp, row);
	get_word(temp, word);		/*the first word in the row*/
	lenword = strlen(word);

	for(i=0; i<9; i++)		/*check if instruction statement*/
	{
		if (check_opcode_group(word) != 0)
		{
			stage1_pars_instruction(row, label, is_label);
			return;
		}
	}

	if (word[0] == ';')		/*check if comment statement*/
	{
		strcpy(is_pars, TRUE);
		return;
	}

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
			stage1_pars_row(rest_row, nlabel, is_label); /*check the rest of the row*/
		}
	}
	if (word[0] == '.') 			/*check if directive statement*/
	{
		stage1_pars_directive(temp, word, label, is_label); 
		strcpy(is_pars, TRUE);
	}
	if (strcpy(is_pars, FALSE) == 0)	/*the row is invalid, no good parser, return an error*/
	{
		fprintf(stderr, "Line is invalid");
	}
}


void stage1_pars_macro(char row[])	/*the function pars macro statement*/
{
	char c;
	int i, ind, val, len;
	char nval[MAX_WORD];

	len = strlen(row);

	for(i = len; i >=0; i--)
	{
		c = row[i];
		if(c == '=')
		{
			ind = i+1;
		}
	}
	substring(row, nval, ind, len);
	val = atof(nval);	
	stage1_insert_to_symbol_table(row, "", FALSE, "macro", val); 
}


void stage1_pars_directive(char myrow[], char word[], char label[], char is_label[])	/*the function pars directive statement*/
{

	char row[REST_ROW];
	strcpy(row, myrow);

	if (strcmp(word, ".define") == 0)	/*check if macro*/
	{
		get_rest_row(row, row);
		stage1_pars_macro(row); 
	}
	else if (strcmp(word, ".entry") == 0)	/*entry statement - will check in stage2*/
	{
		return;
	}			
	else if (strcmp(word, ".extern") == 0)	/*external statement*/
	{
		get_rest_row(row, row);
		get_word(row, word);
		insert_to_symbol_table(word, 0, "extern");
	}
	else if (strcmp(word, ".string") == 0)	/*string statement*/
	{
		get_rest_row(row, row);
		if (strcmp(is_label, TRUE) == 0)
		{
			stage1_insert_to_symbol_table(row, label, is_label, "string", 0);
		}
		stage1_pars_string(row);	
	}
	else if (strcmp(word, ".data") == 0)	/*data statement*/
	{
		get_rest_row(row, row);
		if (strcmp(is_label, TRUE) == 0)
		{
			stage1_insert_to_symbol_table(row, label, is_label, "data", 0);
		}
		stage1_pars_data(row);	
	}
}


void stage1_insert_to_symbol_table(char row[], char label[], char is_label[], char type[], int macro_val)	/*the function use the insret_to_symbol_table function, and insert all the symbols as per the requirements*/
{
	int val;
	char name[MAX_WORD];
	symbol *s = S_head;

	if (strcmp(type, "extern") == 0)
		val = 0; 		 /*an external val*/

	if (strcmp(is_label, TRUE) == 0)
		strcpy(name, label);
	if (strcmp(type, "macro") == 0)
	{
		val = macro_val;	/*the value of the macro*/
		get_word(row, name);
	}
	else if (strcmp(type, "code") == 0)	
		val = IC;
	else /*(strcmp(type, "data") == 0)*/	
		val = DC;

	if(!s)				/*check the current symbol table and enter the new symbol*/
	{
		insert_to_symbol_table(name, val, type);
	}
	else
	{
		while(s)		/*check were to enterd and if the symbol is not exists*/
		{
			if (strcmp(s -> name, name) == 0)
			{
				fprintf(stderr, "This symbol is already exists in the symbols table - %s\n", name);
				return;
			}
			s = s -> next;
		}	
		insert_to_symbol_table(name, val, type);
	}
}


void stage1_pars_string(char row[])	/*the function pars a string statement*/
{
	int i, len;
	int val[CODE_ROW], bits[CODE_ROW];
	char string[MAX_WORD];
	char c;

	strcpy(string, row);
	len = strlen(string);

	for (i=0; i<len-3; i++)
	{
		c = string[i+1];
		val[i] = (int)c;
		change_to_bit(val[i], bits, CODE_ROW);
		insert_to_data_table(DC, bits);
	}
	change_to_bit(0, bits, CODE_ROW);
	insert_to_data_table(DC, bits);
}


void stage1_pars_data(char row[])	/*the function pars a data statement*/
{
	int val;
	int bits[CODE_ROW];
	char is_symbol[] = FALSE;
	symbol *s = S_head;
	char name_s[MAX_WORD], temp[REST_ROW];

	strcpy(temp, row);

	while(!(strcmp(temp, "") == 0))
	{
		get_word(temp, name_s);
		if (find_end_row(temp) == findspace(temp))
			strcpy(temp, "");
		else
			get_rest_row(temp, temp);
	
		s = S_head;
		while(s)
		{
			if (strcmp(s -> name, name_s) == 0)
			{
				val = s -> val;
				strcpy(is_symbol, TRUE);
			}
			s = s -> next;
		}
		if (strcmp(is_symbol, FALSE) == 0)
		{
			val = atof(name_s);
		}
		coding_signed_bits(val, bits, SIGNED_NUM);
		insert_to_data_table(DC, bits);
	}			
}


void stage1_pars_instruction(char row[], char label[], char is_label[])		/*the function pars instruction statement*/
{
	int opcode_group;
	char opcode[MAX_WORD], temp1[REST_ROW], temp2[REST_ROW];

	strcpy(temp1, row);
	
	if (strcmp(is_label, TRUE) == 0)	/*if this is a labael we will insert to symbol table*/
	{	
		stage1_insert_to_symbol_table(row, label, is_label, "code", 0);
	}
	else
		get_rest_row(temp1, temp1);
	
	get_word(temp1, opcode);
	get_rest_row(temp1, temp1);
	IC ++;
	opcode_group = check_opcode_group(opcode);
	if(opcode_group == 1)
	{
		stage1_pars_address(temp1);
		get_rest_row(temp1, temp2);
		stage1_pars_address(temp2);
	}
	else if(opcode_group == 2)
		stage1_pars_address(temp1);
	else if(opcode_group == 3)
		return;
	else if(opcode_group == 0)
		fprintf(stderr, "This instruction word does not exists");
}


void stage1_pars_address(char row[])	/*the function cheack the place in memory for method of addressing (שיטת מיעון) */
{
	symbol *s = S_head;	
	int i, lenword, cnt = 0;
	char operand[MAX_WORD], temp[REST_ROW];
	char found_add[6];

	strcpy(found_add, FALSE);
	strcpy(temp, row);
	get_word(temp, operand);

	if (operand[0] == '#')	/*method 1 - immediate address*/
	{
		IC ++;
		strcpy(found_add, TRUE);
	}
	if (strcmp(found_add, FALSE) == 0) 
	{
		while(s)	/*method 2 - direct address*/
		{
			if (strcmp(s -> name, operand) == 0)
			{
				IC ++;
				strcpy(found_add, TRUE);
			}
			s = s -> next;
		}
		
	}
	if (strcmp(found_add, FALSE) == 0) /*method 3 - fixed index address*/
	{
		lenword = strlen(operand); 
		for(i=0; i<lenword; i++)
		{
			if((operand[i] == '[') || (operand[i] == ']'))
			{
				cnt ++;
			}
		}
		if(cnt == 2)
		{
			IC += 2;
			strcpy(found_add, TRUE);
		}
	}

	if (strcmp(found_add, FALSE) == 0 && operand[0] == 'r') /*method 4 - direct hamster address*/
	{	
		if(strcmp(SAME_MEM_WORD, FALSE) == 0)
		{
			IC ++;
			strcpy(SAME_MEM_WORD, TRUE);
		}
		strcpy(found_add, TRUE);
	}
	if (strcmp(found_add, FALSE) == 0)
		IC ++;		/*symbol that is not in the table*/
}


void stage1_finish()	/*the function pars update the addresses after we have done stage1, cheack the IC and DC*/
{
	symbol *s = S_head;
	data *d = D_head;
	int curr_dc;

	while(s)
	{
		if (strcmp(s -> type, "data") == 0 || strcmp(s -> type, "string") == 0)
			s -> val += IC;
		s = s -> next;
	}

	while(d)
	{
		curr_dc = d -> dc;
		insert_to_code_table(curr_dc + IC, d -> val);
		d = d -> next;
	}		 
}

