#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "all_stages.h"


/*symbol table functions*/
void insert_to_symbol_table(char name[], int val, char type[])	/*the function insert symbol to symbol table*/
{
	symbol *curr_s;
	symbol *new_s = NULL;
	new_s = (symbol*) malloc(sizeof(symbol));

	strcpy(new_s -> name, name);
	new_s -> val = val;
	strcpy(new_s -> type, type);
	new_s -> next = NULL;

	if(!S_head)	/*if the symbol table is empty*/
	{
		S_head = new_s;
	}
	else
	{
		curr_s = S_head;
		while(curr_s -> next) /*insert to the end of the table*/
			curr_s = curr_s -> next;
		curr_s -> next = new_s;
	}
}


void print_symbol_table()	 /*the function print symbol table*/
{
	symbol *s = S_head;
	while(s) 
	{
		printf("\n%s\t%d\t%s", s -> name, s -> val, s -> type);
		s = s -> next;
	}
	printf("\n");
}


/*data table functions*/
void insert_to_data_table(int dc, int val[])	/*the function insert data to data table*/
{
	int i;
	data *curr_d;
	data *new_d = NULL;
	new_d = (data*) malloc(sizeof(data));

	new_d -> dc = dc;
	for (i=0; i<CODE_ROW; i++)
		new_d -> val[i] = val[i];
	new_d -> next = NULL;

	if(!D_head)	/*if the data table is empty*/
	{
		D_head = new_d;
	}
	else
	{
		curr_d = D_head;
		while(curr_d -> next) /*insert to the end of the table*/
			curr_d = curr_d -> next;
		curr_d -> next = new_d;
	}
	DC ++;
}


void print_data_table()		 /*the function print data table*/
{
	int i;
	data *d = D_head;

	while (d) 
	{
		printf("\n%d\t", d -> dc);
		for(i=0; i<CODE_ROW; i++)
			printf("%d", d -> val[i]);
		d = d -> next;
	}
	printf("\n");
}


/*code table functions*/
void insert_to_code_table(int ic, int code[])	/*the function insert code to code table*/
{
	int i;
	coding *curr_c;
	coding *new_c = NULL;
	new_c = (coding*) malloc(sizeof(coding));

	new_c -> ic = ic;
	for (i=0; i<CODE_ROW; i++)
		new_c -> code[i] = code[i];
	new_c -> next = NULL;

	if(!C_head)	/*if the code table is empty*/
	{
		C_head = new_c;
	}
	else
	{
		curr_c = C_head;
		while(curr_c -> next) /*insert to the end of the table*/
			curr_c = curr_c -> next;
		curr_c -> next = new_c;
	}

}

void print_code_table()		 /*the function print code table*/
{
	int i;
	coding *c = C_head;

	while(c) 
	{
		printf("\n%d\t", c -> ic);
		for(i=0; i<CODE_ROW; i++)
			printf("%d", c -> code[i]);
		c = c -> next;
	}
	printf("\n");
}



/*useful functions*/

int check_opcode_group(char opcode[])		/*the function check what group instruction word belongs to*/
{
	int i;
	for(i=0; i<9; i++)
	{
		if(strcmp(opcode, GROUP1[i])	== 0)	/*the instruction word belongs to group number 1*/
			return 1;
		else if(strcmp(opcode, GROUP2[i]) == 0)	/*the instruction word belongs to group number 2*/
			return 2;
		else if(strcmp(opcode, GROUP3[i]) == 0)	/*the instruction word belongs to group number 3*/
			return 3;
	}
	return 0;
}

void change_to_bit(int val, int bits[], const int nbits)	/*the function save the bits in the array that we get*/
{
	int i, j;
	int temp_bits[nbits];

	for(i=0; i<nbits; i++)
	{
		if(val > 0)
		{
			temp_bits[i] = val%2;
			val = val/2;
		}
		else
		{
			temp_bits[i] = 0;
		}
	}

	for(i=nbits-1, j=0; j<nbits; i--, j++)
		bits[i] = temp_bits[j];
}


void coding_signed_bits(int num, int arr[], int len)		/*the function coding signed values*/
{
	int i;
	char first_one[6] = TRUE;


	if (num >= 0)
		change_to_bit(num, arr, len);
	else
	{
		num = num * -1;
		change_to_bit(num, arr, len);
		for(i=len-1; i>=0; i--)
			if (strcmp(first_one, TRUE) == 0)
			{
				if (arr[i] == 0)
					continue;
				else if (arr[i] == 1)
					strcpy(first_one, FALSE);
			}
			else
			{
				if (arr[i] == 0)
					arr[i] = 1;
				else if (arr[i] == 1)
					arr[i] = 0;
			}
	}
}


void substring(char s[], char sub[], int from, int to) 		/*the function cat row form 'from' index to 'to' index*/
{
	int c = 0;
	while(from < to)
	{
		sub[c] = s[from];
		c++;
		from++;
	}
	sub[c] = '\0';
}


int findspace(char row[])		/*the function find the next space in the row end return his index*/
{
	int len = strlen(row);
	int i;
	for (i = 0; i < len; i++)
	{
		if (row[i] == ' ' || row[i] == '\t' || row[i] == '\n' || row[i] == ',' || row[i] == EOF) 
			return i;
	}
	return 0;
}


int find_not_space(char row[])		/*the function find the next char that is not space in the row end return his index*/
{
	int len = strlen(row);
	int i;

	for (i = 0; i < len; i++)
	{
		if (row[i] != ' ' && row[i] != '\t' && row[i] != '\n' && row[i] != ',' && row[i] != EOF) 
			return i;
	}
	return 0;
}


void get_word(char row[], char word[])		/*the function find the first word in the row*/
{	
	int s, e, len;	/*start of word and end of word*/
	char rest_row[REST_ROW];

	len = strlen(row);
	
	s = find_not_space(row);
	e = 0;
	if (s == 0)
	{
		e = findspace(row);
		if (e != 0)
			substring(row, word, s, e);
		else
		{
			len = strlen(row);
			substring(row, word, s, len);
		}
	}
	else
	{
		substring(row, rest_row, s, len);
		e = findspace(rest_row);
		substring(rest_row, word, 0, e);
	}

}

int find_end_row(char row[])		/*the function find the index that is the end of the row*/
{
	int i, len = strlen(row);

	for(i = 0; i < len; i ++)
	{
		if (row[i] == '\n' || row[i] != EOF)
			return i;
	}
}

void get_rest_row(char row[], char rest_row[])		/*the function find the rest of the row, after the first word in the row*/
{	
	int s, e;	/*start of word and end of word*/

	e = strlen(row);

	s = findspace(row);
	substring(row, rest_row, s+1, e);
	s = find_not_space(rest_row);
	if(s == 0)
		return;
	else
		substring(rest_row, rest_row, s, e);
}


