#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "all_stages.h"

int IC = 100;
int IN_IC = 100;
int DC = 0;
char SAME_MEM_WORD[MAX_WORD] = FALSE;

char* GROUP1[] = {"mov", "cmp", "add", "sub", "lea", "", "", "", ""};
char* GROUP2[] = {"not", "clr", "inc", "dec", "jmp", "bne", "red", "prn", "jsr"};
char* GROUP3[] = {"rts", "stop", "", "", "", "", "", "", ""};

symbol *S_head = NULL;
data *D_head = NULL;
coding *C_head = NULL;


int main(int argc, char *argv[])
{
	char *fname;
	char myrow[ROW], temprow[ROW], is_label[] = FALSE, label[MAX_WORD];
	FILE *fp;

	if (argc != 2)		/*check if we get the file*/
	{	
		fprintf(stderr, "Usage: ./prog file_name\n");
		exit(1);
	}
	fname = argv[1];
	fp = fopen(fname, "r");
	if (!fp)
	{
		fprintf(stderr, "can not open file %s\n", fname);
		exit(1);
	}
	
	/*~~~~~~~~~~~stage 1~~~~~~~~~~~~~~*/

	while (!feof(fp))	/*read all the file - stage1*/
	{
		if (fgets(myrow, ROW, fp) != NULL) /*pars every line*/
		{		
			strcpy(temprow, myrow);
			strcpy(is_label, FALSE);	/*initialization is label befor parsing the row*/
			stage1_pars_row(temprow, label, is_label);	/*pars with stage 1 algorithm*/
		}			
	}
	stage1_finish();	/*finish function for stage 1*/
	
	fclose(fp);		/*close the file and open again to pars every line from the first line*/

	/*~~~~~~~~~~~stage 2~~~~~~~~~~~~~~*/

	fp = fopen(fname, "r");

	stage2_start();
	while (!feof(fp))	/*read all the file again- stage2*/
	{
		if (fgets(myrow, ROW, fp) != NULL) /*pars every line*/
		{
			strcpy(is_label, FALSE);	/*initialization is label befor parsing the row*/	
			stage2_pars_row(myrow, label, is_label);	/*pars with stage 2 algorithm*/
		}
	}
	stage2_finish_outfput();	/*finish function for stage 2 and all the algorithm*/

	fclose(fp);	/*close the file*/

	printf("finish\n");
	return 0;
}

