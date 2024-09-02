/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*                                                             */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	uint32_t pc = CURRENT_STATE.PC;
    uint32_t* reg = CURRENT_STATE.REGS;

    uint32_t idx = (pc - MEM_TEXT_START) / 4;

    instruction inst = INST_INFO[idx];

    short op = inst.opcode;

    int pced = 0;
    if (op == 0){
        short func = inst.func_code;
        unsigned char rs = inst.r_t.r_i.rs;
        if (func == 8){ //JR
            pced = 1;
            CURRENT_STATE.PC = reg[rs];
        }
        else{ //Rtype;
            unsigned char rt = inst.r_t.r_i.rt;
            unsigned char rd = inst.r_t.r_i.r_i.r.rd;
            if (func == 33){ //ADDU
                reg[rd] = reg[rt] + reg[rs];
            }
            else if (func == 36){ //AND
                reg[rd] = reg[rt] & reg[rs];
            }
            else if (func == 39){ //NOR
                reg[rd] = ~(reg[rt] | reg[rs]);
            }
            else if (func == 37){ //OR
                reg[rd] = reg[rt] | reg[rs];
            }
            else if (func == 43){ //SLTU
                if (reg[rt] > reg[rs]){
                    reg[rd] = 1;
                }
                else{
                    reg[rd] = 0;
                }
            }
            else if (func == 0){ //SLL
                unsigned char sh = inst.r_t.r_i.r_i.r.shamt;
                reg[rd] = reg[rt] << sh;
            }
            else if (func == 2){ //SRL
                unsigned char sh = inst.r_t.r_i.r_i.r.shamt;
                reg[rd] = reg[rt] >> sh;
            }
            else if (func == 35){ //SUBU
                reg[rd] = reg[rs] - reg[rt];
            }

            //printf("rd[%d]: %x | rs[%d]: %x | rt[%d]: %x\n",rd,reg[rd],rs,reg[rs],rt,reg[rt]);

        }
        
    }
    else if ((op == 2) || (op == 3)){ //jtype
        pced = 1;
        uint32_t target = inst.r_t.target << 2;
        if (op == 3){ //jal
            reg[31] = pc + 4;
            CURRENT_STATE.PC = target;
        }
        else{
            uint32_t tmp = 0xF0000000;
            pc = pc & tmp;
            CURRENT_STATE.PC = target | pc;
        }

    }
    else{ //I type
        unsigned char rs = inst.r_t.r_i.rs;
        unsigned char rt = inst.r_t.r_i.rt;
        short imm = inst.r_t.r_i.r_i.imm;

        if (op == 9){ //addiu
            reg[rt] = reg[rs] + imm;
        }
        else if (op == 12){ //andi
            reg[rt] = reg[rs] & imm;
        }
        else if (op == 4){ //beq
            if (reg[rs] == reg[rt]){
                pced = 1;
                CURRENT_STATE.PC += imm * 4 + 4;
            }
        }
        else if (op == 5){ //bne
            if (reg[rs] != reg[rt]){
                pced = 1;
                CURRENT_STATE.PC += imm * 4 + 4;
            }
        }
        else if (op == 15){ //lui
            reg[rt] = imm << 16;
        }
        else if (op == 35){ //lw
            reg[rt] = mem_read_32(reg[rs] + imm);
        }
        else if (op == 13){ //ori
            reg[rt] = reg[rs] | imm;
        }
        else if (op == 11){ //sltiu
            if (imm > reg[rs]){
                reg[rt] = 1;
            }
            else{
                reg[rt] = 0;
            }
        }
        else if (op == 43){ //sw
            mem_write_32(reg[rs] + imm,reg[rt]);
        }

        //printf("rs[%d]: %x | rt[%d]: %x\n",rs,reg[rs],rt,reg[rt]);

    }

    if (pced == 0) CURRENT_STATE.PC += 4;

    if ((CURRENT_STATE.PC - MEM_TEXT_START) / 4 > NUM_INST){
        CURRENT_STATE.PC -= 4;
        RUN_BIT = FALSE;
    }
}
