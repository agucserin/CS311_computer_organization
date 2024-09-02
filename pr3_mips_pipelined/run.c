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
instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
//int i = 0;
void process_instruction(){
    //printf("%d WB\n",i);
    WB_Stage();
    //printf("%d MEM\n",i);
    MEM_Stage();
    //printf("%d EX\n",i);
    EX_Stage();
    //printf("%d ID\n",i);
    ID_Stage();
    //printf("%d IF\n",i);
    IF_Stage();
    //i++;
}

void IF_Stage(){
    if (RUN_BIT == FALSE){
        
        CURRENT_STATE.PIPE[IF_STAGE] = 0;
        return;
    }
    if(CURRENT_STATE.IF_ID_stall == 1){
        CURRENT_STATE.IF_ID_stall = 0;
        return;
    }
    else if(CURRENT_STATE.IF_ID_flush == 1){
        CURRENT_STATE.IF_ID_flush = 0;
        //CURRENT_STATE.IF_ID_flushed = 1;
        //printf("####\n");
        CURRENT_STATE.flushed = 1;
        CURRENT_STATE.PIPE[IF_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.flushed == 1){
        CURRENT_STATE.flushed = 0;
        //printf("IF 1 : 0x%x\n",CURRENT_STATE.MEM_WB_NPC);
        CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.MEM_WB_NPC;
        CURRENT_STATE.PC = CURRENT_STATE.MEM_WB_NPC;

    }
    else if(CURRENT_STATE.IF_ID_NPC != 0){
        CURRENT_STATE.PC = CURRENT_STATE.IF_ID_NPC;
    }
    
    if(CURRENT_STATE.jumped == 1){
        CURRENT_STATE.jumped = 0;
    }
    uint32_t pc = CURRENT_STATE.PC;
    CURRENT_STATE.PC += 4;

    //printf("IF2 : 0x%x %d\n",pc,NUM_INST);

    //printf("%d\n",(pc + 4 - MEM_TEXT_START) / 4);
    if (((pc + 4 - MEM_TEXT_START) / 4 > NUM_INST)){
        CURRENT_STATE.PIPE[IF_STAGE] = 0;
        CURRENT_STATE.IF_ID_NPC = 0;
        //printf("no more inst\n");
        return;
    }
    else{
        CURRENT_STATE.PIPE[IF_STAGE] = pc;
    }
    uint32_t* reg = CURRENT_STATE.REGS;
    uint32_t idx = (pc - MEM_TEXT_START) / 4;

    CURRENT_STATE.IF_ID_INST = idx;
    CURRENT_STATE.IF_ID_NPC = pc + 4;

    CURRENT_STATE.MEM_WB_FORWARD_REG = 32;

    CURRENT_STATE.IF_ID_ready = 1;
}
void ID_Stage(){
    /*
    CURRENT_STATE.ID_EX_REG1 = 0;
    CURRENT_STATE.ID_EX_REG2 = 0;
    CURRENT_STATE.ID_EX_IMM = 0;
    CURRENT_STATE.ID_EX_DEST = 32;
    CURRENT_STATE.ID_EX_opcode = 63;*/
    if(CURRENT_STATE.IF_ID_ready == 0  || RUN_BIT == FALSE){
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.ID_EX_stall == 1){
        CURRENT_STATE.ID_EX_stall = 0;
        CURRENT_STATE.IF_ID_stall = 1;
        return;
    }
    else if(CURRENT_STATE.ID_EX_flush == 1){
        CURRENT_STATE.ID_EX_flush = 0;
        CURRENT_STATE.IF_ID_flush = 1;
        CURRENT_STATE.ID_EX_opcode = 63;
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.jumped == 1){
        //printf("IDD : pc : 0x%x\n",CURRENT_STATE.PC);
        //printf("IDD : pc : 0x%x\n",CURRENT_STATE.IF_ID_NPC);
        CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
        CURRENT_STATE.ID_EX_opcode = 63;
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.flushed == 1){
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        return;
    }
    CURRENT_STATE.ID_EX_REG1 = 0;
    CURRENT_STATE.ID_EX_REG2 = 0;
    CURRENT_STATE.ID_EX_IMM = 0;
    CURRENT_STATE.ID_EX_DEST = 32;
    CURRENT_STATE.ID_EX_opcode = 63;
    /*else if(CURRENT_STATE.ID_EX_opcode == 35){
        instruction inst = INST_INFO[CURRENT_STATE.IF_ID_INST];
        unsigned char rt = inst.r_t.r_i.rt;
        unsigned char rs = inst.r_t.r_i.rs;

        
        short op = inst.opcode;
        if(op == 0){
            if(rt == CURRENT_STATE.ID_EX_DEST || rs == CURRENT_STATE.ID_EX_DEST){
                CURRENT_STATE.IF_ID_stall == 1;
                return; 
            }
        }
        else{
            if(rs == CURRENT_STATE.ID_EX_DEST){
                CURRENT_STATE.IF_ID_stall == 1;
                return; 
            }
        }
        //printf("%d %d %d\n",rs, rt, CURRENT_STATE.ID_EX_DEST);
        
    }*/

    
    

    uint32_t pc = CURRENT_STATE.IF_ID_NPC;
    instruction inst = INST_INFO[CURRENT_STATE.IF_ID_INST];
    uint32_t* reg = CURRENT_STATE.REGS;

    //printf("ID\n");

    CURRENT_STATE.PIPE[ID_STAGE] = CURRENT_STATE.PIPE[IF_STAGE];

    unsigned char rs = 32;
    unsigned char rt = 32;
    unsigned char rd = 32;
    short imm;
    short func;
    uint32_t jtar;

    short op = inst.opcode;
    if (op == 0){
        func = inst.func_code;
        rs = inst.r_t.r_i.rs;
        rt = inst.r_t.r_i.rt;
        rd = inst.r_t.r_i.r_i.r.rd;
        
        CURRENT_STATE.ID_EX_DEST = rd;
    }
    else if ((op == 2) || (op == 3)){ //jtype
        jtar = inst.r_t.target << 2;
        
    }
    else{ //I type
        rs = inst.r_t.r_i.rs;
        rt = inst.r_t.r_i.rt;
        imm = inst.r_t.r_i.r_i.imm;

        CURRENT_STATE.ID_EX_DEST = rt;
    }
    

    //printf("ID NPC : 0x%x\n",pc);
    CURRENT_STATE.ID_EX_NPC = pc;

    //forwarding
    CURRENT_STATE.ID_EX_REG1 = reg[rs];
    CURRENT_STATE.ID_EX_REG2 = reg[rt];
    if (rs == CURRENT_STATE.EX_MEM_FORWARD_REG){
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    }
    else if (rs == CURRENT_STATE.MEM_WB_FORWARD_REG){
        //printf("ID rs : lw to sw %d %d\n",rs,CURRENT_STATE.MEM_WB_FORWARD_REG);
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }
    if (rt == CURRENT_STATE.EX_MEM_FORWARD_REG){
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    }
    else if (rt == CURRENT_STATE.MEM_WB_FORWARD_REG){
        //printf("ID  rt: lw to sw\n");
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }

    //printf("0x%x 0x%x\n",CURRENT_STATE.ID_EX_REG1,CURRENT_STATE.ID_EX_REG2);

    CURRENT_STATE.ID_EX_IMM = imm;
    //CURRENT_STATE.ID_EX_INST = CURRENT_STATE.IF_ID_INST;

    CURRENT_STATE.ID_EX_opcode = inst.opcode;
    CURRENT_STATE.ID_EX_func = inst.func_code;
    CURRENT_STATE.ID_EX_shamt = inst.r_t.r_i.r_i.r.shamt;
    CURRENT_STATE.jtar = jtar;
    CURRENT_STATE.ID_EX_ready = 1;

    CURRENT_STATE.ID_EX_INST = CURRENT_STATE.IF_ID_INST;
    pc -= 4;
    //printf("ID 0x%x: reg1 value : %d(%d) reg2 value : %d(%d) %d\n",pc,CURRENT_STATE.ID_EX_REG1,rs,CURRENT_STATE.ID_EX_REG2,rt,CURRENT_STATE.ID_EX_DEST);
}
void EX_Stage(){
    if(CURRENT_STATE.ID_EX_ready == 0  || RUN_BIT == FALSE){
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.EX_MEM_stall == 1){
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
        CURRENT_STATE.EX_MEM_stall = 0;
        CURRENT_STATE.ID_EX_stall = 1;

        CURRENT_STATE.EX_MEM_NPC = 0;
        CURRENT_STATE.EX_MEM_ALU_OUT = 0;
        CURRENT_STATE.EX_MEM_W_VALUE = 0;
        CURRENT_STATE.EX_MEM_BR_TARGET = 0;
        CURRENT_STATE.EX_MEM_BR_TAKE = 0;
        CURRENT_STATE.EX_MEM_DEST = 32;
        CURRENT_STATE.EX_MEM_opcode = 63;
        CURRENT_STATE.EX_MEM_func = 0;
        return;
    }
    else if(CURRENT_STATE.EX_MEM_flush == 1){
        CURRENT_STATE.EX_MEM_flush = 0;
        CURRENT_STATE.ID_EX_flush = 1;
        CURRENT_STATE.MEM_WB_opcode = 63;
        CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.MEM_WB_NPC;
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.ID_EX_opcode == 63){
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
        CURRENT_STATE.EX_MEM_NPC = 0;
        CURRENT_STATE.EX_MEM_ALU_OUT = 0;
        CURRENT_STATE.EX_MEM_W_VALUE = 0;
        CURRENT_STATE.EX_MEM_BR_TARGET = 0;
        CURRENT_STATE.EX_MEM_BR_TAKE = 0;
        CURRENT_STATE.EX_MEM_DEST = 32;
        CURRENT_STATE.EX_MEM_opcode = 63;
        CURRENT_STATE.EX_MEM_func = 0;
        return;
    }
    else if(CURRENT_STATE.flushed == 1){
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
        CURRENT_STATE.EX_MEM_NPC = 0;
        CURRENT_STATE.EX_MEM_ALU_OUT = 0;
        CURRENT_STATE.EX_MEM_W_VALUE = 0;
        CURRENT_STATE.EX_MEM_BR_TARGET = 0;
        CURRENT_STATE.EX_MEM_BR_TAKE = 0;
        CURRENT_STATE.EX_MEM_DEST = 32;
        CURRENT_STATE.EX_MEM_opcode = 63;
        CURRENT_STATE.EX_MEM_func = 0;
        return;
    }
    CURRENT_STATE.EX_MEM_NPC = 0;
    CURRENT_STATE.EX_MEM_ALU_OUT = 0;
    CURRENT_STATE.EX_MEM_W_VALUE = 0;
    CURRENT_STATE.EX_MEM_BR_TARGET = 0;
    CURRENT_STATE.EX_MEM_BR_TAKE = 0;
    CURRENT_STATE.EX_MEM_DEST = 32;
    CURRENT_STATE.EX_MEM_opcode = 63;
    CURRENT_STATE.EX_MEM_func = 0;

    uint32_t pc = CURRENT_STATE.ID_EX_NPC;
    uint32_t reg1 = CURRENT_STATE.ID_EX_REG1;
    uint32_t reg2 = CURRENT_STATE.ID_EX_REG2;
    short imm = CURRENT_STATE.ID_EX_IMM;
    unsigned char dest = CURRENT_STATE.ID_EX_DEST;
    uint32_t* reg = CURRENT_STATE.REGS;
    //instruction inst = INST_INFO[CURRENT_STATE.ID_EX_INST];
    short op = CURRENT_STATE.ID_EX_opcode;

    uint32_t jtar = CURRENT_STATE.jtar;

    //printf("##\n");
    //printf("0x%d 0x%d\n",CURRENT_STATE.MEM_WB_FORWARD_REG,INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rt);
    if (CURRENT_STATE.MEM_WB_FORWARD_REG == INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rs){
        reg1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }
    if (CURRENT_STATE.MEM_WB_FORWARD_REG == INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rt){
        reg2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }

    //printf("EX %d\n",dest);
    CURRENT_STATE.PIPE[EX_STAGE] = CURRENT_STATE.PIPE[ID_STAGE];
    //printf("%d %d---\n",reg1,reg2);

    uint32_t out = 0;
    short outed = 0;
    short func;
    int pced = 0;
    if (op == 0){
        func = CURRENT_STATE.ID_EX_func;
        if (func == 8){ //JR
            CURRENT_STATE.PC = reg1;
            CURRENT_STATE.jumped = 1;
            //printf("EX : 0x%x\n",CURRENT_STATE.PC);
        }
        //printf("%d---\n",func);
        if (func == 33){ //ADDU
            out = reg2 + reg1;
            outed = 1;
            //printf("%d\n",out);
        }
        else if (func == 36){ //AND
            out = reg2 & reg1;
            outed = 1;
        }
        else if (func == 39){ //NOR
            out = ~(reg2 | reg1);
            outed = 1;
        }
        else if (func == 37){ //OR
            out = reg2 | reg1;
            outed = 1;
        }
        else if (func == 43){ //SLTU
            if (reg2 > reg1){
                out = 1;
                outed = 1;
            }
            else{
                out = 0;
                outed = 1;
            }
        }
        else if (func == 0){ //SLL
            short sh = CURRENT_STATE.ID_EX_shamt;
            out = reg2 << sh;
            //printf("sh : %d 0x%x 0x%x 0x%x\n",sh,reg2,out,reg1);
            outed = 1;
        }
        else if (func == 2){ //SRL
            short sh = CURRENT_STATE.ID_EX_shamt;
            out = reg2 >> sh;
            outed = 1;
        }
        else if (func == 35){ //SUBU
            out = reg1 - reg2;
            outed = 1;
        }
        //printf("rd[%d]: %x | rs[%d]: %x | rt[%d]: %x\n",rd,reg[rd],rs,reg[rs],rt,reg[rt]);
        
    }
    else if ((op != 2) && (op != 3)){ //I type
        if (op == 9){ //addiu
            out = reg1 + imm;
            outed = 1;
        }
        else if (op == 12){ //andi
            out = reg1 & imm;
            outed = 1;
        }
        else if (op == 4){ //beq
            if (reg1 == reg2){
                CURRENT_STATE.EX_MEM_BR_TAKE = 1;
                CURRENT_STATE.EX_MEM_BR_TARGET = pc + imm * 4;
            }
            else{
                CURRENT_STATE.EX_MEM_BR_TAKE = 0;
            }
        }
        else if (op == 5){ //bne
            if (reg1 != reg2){
                CURRENT_STATE.EX_MEM_BR_TAKE = 1;
                CURRENT_STATE.EX_MEM_BR_TARGET = pc + imm * 4;
            }
            else{
                CURRENT_STATE.EX_MEM_BR_TAKE = 0;

            }
        }
        else if (op == 15){ //lui
            out = imm << 16;
            //printf("out : 0x%x\n",out);
            outed = 1;
        }
        else if (op == 13){ //ori
            out = reg1 | imm;
            //printf("out : 0x%x\n",out);
            outed = 1;
        }
        else if (op == 11){ //sltiu
            if (imm > reg1){
                out = 1;
            }
            else{
                out = 0;
            }
            outed = 1;
        }
        else if (op == 35){ //lw
            out = reg1 + imm;
            outed = 1;
            
            
        }
        else if (op == 43){ //sw
            out = reg1 + imm;
            CURRENT_STATE.EX_MEM_W_VALUE = reg2;
            //printf("%d\n",reg2);
            outed = 1;
        }
        //printf("rs[%d]: %x | rt[%d]: %x\n",rs,reg[rs],rt,reg[rt]);

    }
    else if ((op == 2) || (op == 3)){ //jtype
        if (op == 3){ //jal
            reg[31] = pc;
            //printf("EX JAL : 0x%x\n",pc);
            CURRENT_STATE.PC = jtar;
            pc = CURRENT_STATE.PC;
            CURRENT_STATE.jumped = 1;
        }
        else{
            uint32_t tmp = 0xF0000000;
            pc = pc & tmp;
            CURRENT_STATE.PC = jtar | pc;
            pc = CURRENT_STATE.PC;
            CURRENT_STATE.jumped = 1;
        }

    }
    
    //printf("EX NPC : 0x%x\n",pc);
    //printf("EX op  : %d\n",CURRENT_STATE.ID_EX_opcode);
    CURRENT_STATE.EX_MEM_NPC = pc;
    CURRENT_STATE.EX_MEM_ALU_OUT = out;
    
    if(outed == 1){
        CURRENT_STATE.EX_MEM_DEST = dest;
        CURRENT_STATE.EX_MEM_FORWARD_VALUE = out;
    }
    CURRENT_STATE.EX_MEM_FORWARD_REG = dest;
    CURRENT_STATE.EX_MEM_opcode = CURRENT_STATE.ID_EX_opcode;
    CURRENT_STATE.EX_MEM_func = func;

    CURRENT_STATE.EX_MEM_ready = 1;

    
    //printf("EX : reg1 value : %d reg2 value : %d = %d\n",reg1,reg2,out);
}
void MEM_Stage(){//branch miss에 대한 flush 진행
    if(CURRENT_STATE.EX_MEM_ready == 0 || RUN_BIT == FALSE){
        CURRENT_STATE.PIPE[MEM_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.MEM_WB_stall == 1){
        //printf("stall\n");
        CURRENT_STATE.MEM_WB_stall = 0;
        CURRENT_STATE.EX_MEM_stall = 1;
        return;
    }
    else if(CURRENT_STATE.flushed == 1){
        //printf("MEMFLUSH\n");
        CURRENT_STATE.PIPE[MEM_STAGE] = 0;
        return;
    }
    else if(CURRENT_STATE.EX_MEM_opcode == 63){
        CURRENT_STATE.MEM_WB_NPC = 0; 
        CURRENT_STATE.MEM_WB_ALU_OUT = 0; 
        CURRENT_STATE.MEM_WB_MEM_OUT = 0; 
        CURRENT_STATE.MEM_WB_BR_TAKE = 0;
        CURRENT_STATE.MEM_WB_DEST = 32;
        CURRENT_STATE.MEM_WB_opcode = 63;
        CURRENT_STATE.MEM_WB_func = 0;
        CURRENT_STATE.PIPE[MEM_STAGE] = 0;
        return;
    }

    

    CURRENT_STATE.MEM_WB_NPC = 0; 
    CURRENT_STATE.MEM_WB_ALU_OUT = 0; 
    CURRENT_STATE.MEM_WB_MEM_OUT = 0; 
    CURRENT_STATE.MEM_WB_BR_TAKE = 0;
    CURRENT_STATE.MEM_WB_DEST = 32;
    CURRENT_STATE.MEM_WB_opcode = 63;
    CURRENT_STATE.MEM_WB_func = 0;

    uint32_t pc = CURRENT_STATE.EX_MEM_NPC;
    uint32_t aluout = CURRENT_STATE.EX_MEM_ALU_OUT;
    uint32_t w = CURRENT_STATE.EX_MEM_W_VALUE;
    uint32_t brtar = CURRENT_STATE.EX_MEM_BR_TARGET;
    uint32_t brtak = CURRENT_STATE.EX_MEM_BR_TAKE;
    unsigned char dest = CURRENT_STATE.EX_MEM_DEST;
    short op = CURRENT_STATE.EX_MEM_opcode;
    short func = CURRENT_STATE.EX_MEM_func;

    //printf("MEM : 0x%x\n",CURRENT_STATE.PIPE[EX_STAGE]);
    CURRENT_STATE.PIPE[MEM_STAGE] = CURRENT_STATE.PIPE[EX_STAGE];

    uint32_t* reg = CURRENT_STATE.REGS;

    if (op == 35){ //lw
        //printf("mem read : 0x%x\n",aluout);
        CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(aluout);
        if(INST_INFO[CURRENT_STATE.ID_EX_INST].opcode == 0){
            if ((dest == INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rs) || (dest == INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rt)){
                //printf("MEM : stall!\n");
                CURRENT_STATE.EX_MEM_stall = 1;
                //printf("dest : %d\n",dest);
            }
        }
        else{
            if (dest == INST_INFO[CURRENT_STATE.ID_EX_INST].r_t.r_i.rs){
                //printf("MEM : stall!\n");
                CURRENT_STATE.EX_MEM_stall = 1;
                //printf("dest : %d\n",dest);
            }
        }
    }
    else if (op == 43){ //sw
        
        mem_write_32(aluout,w);
    }
    else if ((op == 4) || (op == 5)){ //branch
        if (brtak == 1){
            pc = brtar;
            //printf("MEM : 0x%x\n",brtar);
            CURRENT_STATE.EX_MEM_flush = 1;
            CURRENT_STATE.MEM_WB_flush = 1;
        }
    }
    if (dest != 0){
        //printf("WB %d\n",CURRENT_STATE.MEM_WB_FORWARD_REG);
        CURRENT_STATE.MEM_WB_FORWARD_REG = dest;
        if (op == 35){
            CURRENT_STATE.MEM_WB_FORWARD_VALUE = CURRENT_STATE.MEM_WB_MEM_OUT;
        }
        else{
             CURRENT_STATE.MEM_WB_FORWARD_VALUE = aluout;
        }
    }

    //printf("MEM NPC : 0x%x\n",pc);
    CURRENT_STATE.MEM_WB_NPC = pc;
    CURRENT_STATE.MEM_WB_ALU_OUT = aluout;
    CURRENT_STATE.MEM_WB_BR_TAKE = brtak;
    CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.MEM_WB_opcode = op;
    CURRENT_STATE.MEM_WB_func = CURRENT_STATE.EX_MEM_func;

    CURRENT_STATE.MEM_WB_ready = 1;
}
void WB_Stage(){
    if(CURRENT_STATE.MEM_WB_ready == 0){
        return;
    }
    if(CURRENT_STATE.MEM_WB_flush == 1){
        CURRENT_STATE.PC = CURRENT_STATE.MEM_WB_NPC; //branch take 됐으니 이 주소부터 시작하세요
        //printf("WB : br : 0x%x %d\n",CURRENT_STATE.PC,CURRENT_STATE.flushed);
        CURRENT_STATE.MEM_WB_flush = 0;
    }
    else if(CURRENT_STATE.MEM_WB_opcode == 63){
        CURRENT_STATE.PIPE[WB_STAGE] = 0;
        return;
    }

    uint32_t aluout = CURRENT_STATE.MEM_WB_ALU_OUT; 
    uint32_t memout = CURRENT_STATE.MEM_WB_MEM_OUT; 
    uint32_t brtak = CURRENT_STATE.MEM_WB_BR_TAKE;
    unsigned char dest = CURRENT_STATE.MEM_WB_DEST;
    short op = CURRENT_STATE.MEM_WB_opcode;
    short func = CURRENT_STATE.MEM_WB_func;

    CURRENT_STATE.PIPE[WB_STAGE] = CURRENT_STATE.PIPE[MEM_STAGE];

    uint32_t* reg = CURRENT_STATE.REGS;

    if (op == 35){
        reg[dest] = memout;
        CURRENT_STATE.MEM_WB_FORWARD_REG = dest;
    }
    else if (((op == 0) && (func != 8))|| (op == 9) || (op == 12) || (op == 15) || (op == 13) || (op == 11)){
        reg[dest] = aluout;
        //printf("WB : $%d : 0x%x\n",dest,aluout);
    }
    
    
    //printf("0x%x %d\n",CURRENT_STATE.MEM_WB_NPC,NUM_INST);
    //printf("WB NPC : 0x%x\n",CURRENT_STATE.MEM_WB_NPC);
    INSTRUCTION_COUNT++;
    //printf("total inst : %d\n",INSTRUCTION_COUNT);
    if (((CURRENT_STATE.MEM_WB_NPC + 4) - MEM_TEXT_START) / 4 > NUM_INST){
        CURRENT_STATE.PC = CURRENT_STATE.MEM_WB_NPC;
        //printf("runbit false\n");
        RUN_BIT = FALSE;
    }
    //printf("r6 : 0x%x r7 : 0x%x r15 : 0x%x\n",reg[6],reg[7],reg[dest]);
    /*
    else if (INSTRUCTION_COUNT >= MAX_INSTRUCTION_NUM){
        CURRENT_STATE.PC += 4;
    }*/
}