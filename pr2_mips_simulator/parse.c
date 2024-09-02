/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*                                                             */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

char * rtype = "000000";
char * addiu = "001001";
char * andi = "001100";
char * beq = "000100";
char * bne = "000101";
char * lui = "001111";
char * lw = "100011";
char * ori = "001101";
char * sltiu = "001011";
char * sw = "101011";
char * j = "000010";
char * jal = "000011";

char * addu = "100001";
char * and = "100100";
char * nor = "100111";
char * or = "100101";
char * sltu = "101011";
char * sll = "000000";
char * srl = "000010";
char * subu = "100011";
char * jr = "001000";

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
	/** Implement this function */

	char opcode[7];
	for (int i = 0; i < 6; i++){
		opcode[i] = buffer[i];
	}
	opcode[6] = '\0';

	instr.opcode = (short)(fromBinary(opcode));

	if (strcmp(rtype,opcode) == 0){
		char funcode[7];
		
		for (int i = 0; i < 6; i++){
			funcode[i] = buffer[i + 26];
		}
		funcode[6] = '\0';
		
		instr.func_code = (short)(fromBinary(funcode));

		if (strcmp(jr,funcode) == 0){
			char rs[6];
			for (int i = 0; i < 5; i++){
				rs[i] = buffer[i + 6];
			}
			rs[5] = '\0';
			instr.r_t.r_i.rs = (char)(fromBinary(rs));
		}
		else{ // R-type
			if ((strcmp(srl,funcode) == 0) || (strcmp(sll,funcode) == 0)){
				char sa[6];
				for (int i = 0; i < 5; i++){
					sa[i] = buffer[i + 21];
				}
				sa[5] = '\0';
				instr.r_t.r_i.r_i.r.shamt = (char)(fromBinary(sa));
			}
			char rs[6];
			char rt[6];
			char rd[6];
			for (int i = 0; i < 5; i++){
				rs[i] = buffer[i + 6];
				rt[i] = buffer[i + 11];
				rd[i] = buffer[i + 16];
			}
			rs[5] = '\0';
			rt[5] = '\0';
			rd[5] = '\0';

			instr.r_t.r_i.rs = (char)(fromBinary(rs));
			instr.r_t.r_i.rt = (char)(fromBinary(rt));

			instr.r_t.r_i.r_i.r.rd = (char)(fromBinary(rd));
		}
	}
	else if ((strcmp(addiu,opcode) == 0) || (strcmp(andi,opcode) == 0) || (strcmp(beq,opcode) == 0) || \
	(strcmp(bne,opcode) == 0) || (strcmp(lui,opcode) == 0) || (strcmp(lw,opcode) == 0) || \
	(strcmp(ori,opcode) == 0) || (strcmp(sltiu,opcode) == 0) || (strcmp(sw,opcode) == 0)){
		char rs[6];
		char rt[6];
		for (int i = 0; i < 5; i++){
			rs[i] = buffer[i + 6];
			rt[i] = buffer[i + 11];
		}
		rs[5] = '\0';
		rt[5] = '\0';

		char imm[17];
		for (int i = 0; i < 16; i++){
			imm[i] = buffer[i + 16];
		}
		imm[16] = '\0';

		instr.r_t.r_i.rs = (char)(fromBinary(rs));
		instr.r_t.r_i.rt = (char)(fromBinary(rt));
		instr.r_t.r_i.r_i.imm = (short)(fromBinary(imm));
	}
	else{ // J-type
		char target[27];
		for (int i = 0; i < 26; i++){
			target[i] = buffer[i + 6];
		}
		target[26] = '\0';

		instr.r_t.target = (uint32_t)(fromBinary(target));
	}

    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */
	int data = fromBinary((char *)buffer);
	mem_write_32(MEM_DATA_START + index, (uint32_t) data);
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
		printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
		printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

		switch(INST_INFO[i].opcode)
		{
			//I format
			case 0x9:		//ADDIU
			case 0xc:		//ANDI
			case 0xf:		//LUI	
			case 0xd:		//ORI
			case 0xb:		//SLTIU
			case 0x23:		//LW	
			case 0x2b:		//SW
			case 0x4:		//BEQ
			case 0x5:		//BNE
			printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
			break;

				//R format
			case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
			printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
			printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
			printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
			break;

				//J format
			case 0x2:		//J
			case 0x3:		//JAL
			printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
			break;

			default:
			printf("Not available instruction\n");
			assert(0);
		}

		printf("\n");

    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
