#define ROW 50
#define MAX_WORD 20
#define REST_ROW 40
#define CODE_ROW 14

#define OPCODE_BITS 4
#define OPERAND_BITS 2
#define ARE_BITS 2
#define SIGNED_NUM 14
#define CODE_VAL 12
#define HAMSTER_NUM 3

#define TRUE "truee"
#define FALSE "false"

extern int IC;
extern int IN_IC;
extern int DC;
extern char SAME_MEM_WORD[MAX_WORD];

extern char* GROUP1[];
extern char* GROUP2[];
extern char* GROUP3[];


typedef struct symbol_table	/*symbol table - linked list*/
{
	char name[MAX_WORD];
	int val;			/*val or address*/
	char type[MAX_WORD];		/*macro or directive or instruction*/
	struct symbol_table* next;
} symbol;


typedef struct data_table 	/*data table - linked list*/
{
	int dc;
	int val[CODE_ROW];
	struct data_table* next;
} data;


typedef struct code_table 	/*data table - linked list*/
{
	int ic;
	int code[CODE_ROW];
	struct code_table* next;
} coding;



extern symbol *S_head;
extern data *D_head;
extern coding *C_head;


/*symbol table functions*/
void insert_to_symbol_table(char name[], int val, char type[]);
void print_symbol_table();


/*data table functions*/
void insert_to_data_table(int dc, int val[]);
void print_data_table(); 


/*code table functions*/
void insert_to_code_table(int ic, int code[]);
void print_code_table();



/*usefulness functions*/
int check_opcode_group(char opcode[]);
void change_to_bit(int val, int bits[], int num_bits);
void coding_signed_bits(int num, int arr[], int len);
void substring(char s[], char sub[], int from, int to);
int findspace(char row[]);
int find_not_space(char row[]);
void get_word(char row[], char word[]);
int find_end_row(char row[]);
void get_rest_row(char row[], char restrow[]);



/*The functions that belong to Stage 1 - which aims to perform initial parsing of rows, and update the symbol table, data table ,IC and DC*/

void stage1_pars_row(char row[], char label[], char is_label[]);
void stage1_insert_to_symbol_table(char row[], char label[], char is_label[], char type[], int macro_val); /*update symbol table*/
void stage1_pars_macro(char row[]);	/*macro*/
void stage1_pars_directive(char row[], char word[], char label[], char is_label[]);				/*directive statement*/
void stage1_pars_string(char row[]);	/*string*/
void stage1_pars_data(char row[]);	/*data*/
void stage1_pars_instruction(char row[], char word[], char is_label[]);	/*instruction statement*/
void stage1_pars_address(char row[]);	/*memory place, update IC*/
void stage1_finish();			/*final update IC for data statement*/



/*The functions that belong to Stage 2 - which aims to parse the line, encode it, and export the output files*/
void stage2_start();
void stage2_pars_row(char row[], char label[], char is_label[]);
void stage2_coding_directive(char label[], char word[], char row[]);
void stage2_pars_entry(char row[]);			/*entry*/
void stage2_pars_ext(char symbol[], int ic);		/*external*/
void stage2_coding_instruction(char row[]);		/*instruction statement*/
int* stage2_coding_opcode(char opcode[], int opcode_num[]);	/*opcode*/
void stage2_coding_group1(int ic, char row[], int add_src_and_dst[]);/*src and dst operand*/
void stage2_coding_group2(int ic, char row[], int add_dst[]); /*dst operand*/
void get_hamster(char hamster[], int coding_hamster[]);
void stage2_add_src_hamster(int ic, char src[]);
void stage2_add_dst_hamster(int ic, char dst[]);
void stage2_add_src_and_dst_hamster(int ic, char src[], char dst[]);
void stage2_coding_not_hamster(int ic, char operand[], int address[]);	/*cheack the place in memory for method of addressing - שיטת מיעון */
void stage2_coding1_immediate(int ic, char operand[], char is_pars[]);	/*method 1 - immediate address*/
void stage2_coding2_direct(int ic, char operand[], char is_pars[]);	/*method 2 - direct address*/
void stage2_coding3_fixed_index(int ic, char operand[], char is_pars[]);/*method 3 - fixed index address*/
void stage2_add_val_mem_word(int ic, int val, char type[]);
void stage2_check_are(char type[], int bits[]);
void stage2_finish_outfput();	/*write into the object file*/
char stage2_coding_base4(int n1, int n2);	/*coding object file to base code*/


