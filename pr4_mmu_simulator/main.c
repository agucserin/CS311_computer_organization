
/* main.c 

Page Table Entry (PTE) format:

Width: 32 bits

Bit 31 ~ 12     : 20-bit physical page number for the 2nd-level page table node or the actual physical page.
Bit 1           : Dirty bit
Bit 0           : Valid bit

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"  /* DO NOT DELETE */


/* addr_t type is the 32-bit address type defined in util.h */

int total_accesses = 0;
int reads = 0;
int writes = 0;
int tlb_accesses = 0;
int tlb_hits = 0;
int tlb_misses = 0;
int page_walks = 0;
int page_faults = 0;

char valid = 0x1;
char dirty = 0x2;
//char getPTE = 0xFF; //1byte 짜리 주소
int GETaddr = 0xFFFFF000; //PTE에서 주소(PPN) 추출

struct rank_cell{
    int tag;
    struct rank_cell * prev;
    struct rank_cell * next;
};

void insert_cell(struct rank_cell ** tower, int tag, int index){
    struct rank_cell * first_cell = tower[index];
    
    //printf("insert_index : %d\n",index);

    if (first_cell->next == NULL){

        //printf("NULL\n");

        struct rank_cell * tmpp = (struct rank_cell * )malloc(sizeof(struct rank_cell));
        tmpp->tag = tag;

        first_cell->next = tmpp;

        tmpp->prev = first_cell;

        first_cell->prev = tmpp;
    }
    else{
        struct rank_cell * tmp = first_cell->next;
        while(1){
            if (tmp->tag == tag){ //찾음 -> 맨 앞으로 끌올
                if (tmp -> prev == first_cell){ //찾았는데 얘밖에 없음
                    break;
                }
                else if (tmp-> next != NULL){ //찾은 위치가 중간
                    struct rank_cell * pre = tmp->prev;
                    struct rank_cell * nex = tmp->next;

                    pre->next = nex;
                    nex->prev = pre;

                    struct rank_cell * fir_nex = first_cell->next;

                    first_cell->next = tmp;
                    tmp->prev = first_cell;
                    tmp->next = fir_nex;
                    fir_nex->prev = tmp;
                    break;
                }
                else{ //찾은 위치가 맨끝
                    //printf("찾은 위치가 맨끝\n");
                    struct rank_cell * pre = tmp->prev;
                    pre->next = NULL;

                    first_cell->prev = pre;

                    struct rank_cell * fir_nex = first_cell->next;

                    first_cell->next = tmp;
                    tmp->prev = first_cell;
                    tmp->next = fir_nex;
                    fir_nex->prev = tmp;
                    break;
                }
            }
            if (tmp->next == NULL){
                first_cell->prev = tmp;

                struct rank_cell * tmpp = (struct rank_cell * )malloc(sizeof(struct rank_cell));
                struct rank_cell * fir_nex = first_cell->next;
                tmpp->tag = tag;

                first_cell->next = tmpp;
                tmpp->prev = first_cell;

                tmpp->next = fir_nex;
                fir_nex->prev = tmpp;
                break;
            }
            tmp = tmp->next;
        }
    }
}

int evict_cell(struct rank_cell ** tower, int index){
    //printf("evict\n");
    struct rank_cell * first_cell = tower[index];

    struct rank_cell * toevict = first_cell->prev;

    first_cell->prev = toevict->prev;
    toevict->prev->next = NULL;

    int tmp = toevict->tag;

    free(toevict);
    return tmp;
}

int lg2(int a){
    int ans = 0;
    while(1){
        if (a < 2){
            break;
        }
        else{
            a = a >> 1;
            ans += 1;
        }
    }
    return ans;
}

int TLB_search(char ** TLB,struct rank_cell ** tower,char * binary, int tag_size,int cell_size,int height,int width, char rw){
    tlb_accesses += 1;

    int idx = 0;
    int two = 1;
    for (int i = 12; i < 12 + lg2(height); i++){
        if (binary[i] == '1'){
            idx += two;
        }
        two = two << 1;
    }

    int tg = 0;
    char tag[tag_size + 1];
    two = 1;
    for (int i = 12 + lg2(height); i < 32; i++){
        tag[i - 12 - lg2(height)] = binary[i];
        if (binary[i] == '1'){
            tg += two;
        }
        two = two << 1;
    }
    tag[tag_size] = '\0';

    //printf("index %d deci : %d binary : %s\n",idx,ret,binary);
    //printf("tag : %s\n",tag);

    char * list = TLB[idx];

    for (int i = 0; i < width; i++){
        char * cell = list + (i * cell_size);
        if ((cell[0] == '0' || cell[0] == 0)){ //invalid
            continue;
        }
        else{ //valid
            char tagg[tag_size + 1];
            for (int j = 2; j < tag_size + 2; j++){
                tagg[j - 2] = cell[j];
            }
            tagg[tag_size] = '\0';
            if (strcmp(tag,tagg) == 0){ // TLB Hit!
                //printf("hit\n");
                insert_cell(tower,tg,idx);
                tlb_hits += 1;
                if ((rw == 'w') && ((cell[1] == '0') || cell[1] == 0)){
                    cell[1] = '1';
                    return 2;
                }
                else{
                    return 1;
                }
            }
        }
    }
    tlb_misses += 1;
    return 0;
}

void TLB_insert(addr_t pte, char ** TLB,struct rank_cell ** tower, char * binary, int tag_size,int cell_size,int height,int width, char rw){

    int idx = 0;
    int two = 1;
    for (int i = 12; i < 12 + lg2(height); i++){
        if (binary[i] == '1'){
            idx += two;
        }
        two = two << 1;
    }

    int tg = 0;
    char tag[tag_size + 1];
    two = 1;
    for (int i = 12 + lg2(height); i < 32; i++){
        tag[i - 12 - lg2(height)] = binary[i];
        if (binary[i] == '1'){
            tg += two;
        }
        two = two << 1;
    }
    tag[tag_size] = '\0';



    //printf("index %d deci : %d binary : %s\n",idx,ret,binary);
    //printf("tag : %s index : %s\n",tag,index);

    char * list = TLB[idx];

    char inserted = 0;
    for (int i = 0; i < width; i++){
        char * cell = list + (i * cell_size);

        if (cell[0] == 0){ //invalid >> insert!
            insert_cell(tower,tg,idx);
            inserted = 1;
            cell[0] = 1;
            if ((rw == 'w') || ((pte & dirty) == dirty)){
            //if (rw == 'w'){
                cell[1] = '1';
            }
            else{   
                cell[1] = '0';
            }
            for (int j = 2; j < tag_size + 2; j++){
                cell[j] = tag[j - 2];
            }
            int tmp = 0x1000; //1 0000 0000 0000
            for (int j = tag_size + 2; j < tag_size + 22; j++){
                if ((pte & tmp) == tmp){
                    cell[j] = '1';
                }
                tmp <<= 1;
            }
            break;
        }
    }

    if (inserted == 0){ // FULL!
        int evicted_tag = evict_cell(tower,idx);

        for (int i = 0; i < width; i++){
            char * cell = list + (i * cell_size);

            int tmp_tag = 0;
            two = 1;
            for (int j = 2; j < tag_size + 2; j++){
                if (cell[j] == '1'){
                    tmp_tag += two;
                }
                two <<= 1;
            }

            if (tmp_tag == evicted_tag){
                insert_cell(tower,tg,idx);
                cell[0] = 1;
                if ((rw == 'w') || ((pte & dirty) == dirty)){
                //if (rw == 'w'){
                    cell[1] = '1';
                }
                else{
                    cell[1] = '0';
                }
                
                for (int j = 2; j < tag_size + 2; j++){
                    cell[j] = tag[j - 2];
                }
                int tmp = 0x1000; //1 0000 0000 0000
                for (int j = tag_size + 2; j < tag_size + 22; j++){
                    if ((pte & tmp) == tmp){
                        cell[j] = '1';
                    }
                    else{
                        cell[j] = '0';
                    }
                    tmp <<= 1;
                }
                /*if (((pte & dirty) == dirty) && ((cell[1] == '0') || (cell[1] == 0))){
                    int tg = 0;
                    int two = 1;
                    for (int k = 2; k < tag_size + 2; k++){
                        if (cell[k] == '1'){
                            tg += two;
                        }
                        two = two << 1;
                    }
                    printf("idx : %d tag : 0x%x\n",idx,tg);
                }*/
                break;
            }
        }
    }
}

int PageTable_walker(uint32_t read, int sec, char rw){
    page_walks += 1;

    //mem_write_word32(baseidx, 1);
    //break;

    if ((read & valid) == 0){ //page fault in 1st floor
        return 1;
    }
    else{ //2층 탐색
        addr_t secaddr = read & GETaddr;

        addr_t nodeidx = secaddr + sec * 4;
        uint32_t read = mem_read_word32(nodeidx);

        if ((read & valid) == 0){ //page fault in 2nd floor
            return 2;
        }
        if (rw == 'w'){
            read = read | dirty;
        }
        mem_write_word32(nodeidx, read);

        return read;
    }
}

void PageFault_handler(int floor, uint32_t read, addr_t baseidx, int sec, char rw){
    addr_t new_page = get_new_physical_page();
    page_faults += 1;
    if (floor == 1){
        //char tmppp = 0xFFFFFF;
        //printf("fir : %d sec : %d\n",fir,sec);

        addr_t new_node = get_new_page_table_node();
        //int adddr = new_node << 12;

        //printf("node addr : 0x%x\n",new_node);

        read = read | new_node | valid;
        mem_write_word32(baseidx, read);

        addr_t nodeidx = new_node + sec * 4;
        
        //read = mem_read_word32(nodeidx);
        
        //new_page = new_page << 12;

        if (rw == 'w'){
            //read = read | new_page | valid;
            read = new_page | valid | dirty;
        }
        else{
            //read = read | new_page | valid | dirty;
            read = new_page | valid;
        }

        mem_write_word32(nodeidx, read);
    }
    else{
        addr_t node_addr = read & GETaddr;

        addr_t nodeidx = node_addr + sec * 4;

        if (rw == 'w'){
            //read = read | new_page | valid;
            read = new_page | valid | dirty;
        }
        else{
            //read = read | new_page | valid | dirty;
            read = new_page | valid;
        }
        

        mem_write_word32(nodeidx, read);
    }
}

void TLB_content(char ** TLB,int height,int width,int tag_size, int cell_size){
    printf("TLB Content:\n-------------------------------------\n");
    printf("    ");
    for (int j = 0; j < width ; j++){
        printf("      WAY[%d]",j);
    }
    printf("\n");
    for (int i = 0; i < height ; i++){
        printf("SET[%d]:   ",i);
        for (int j = 0; j < width ; j++){
            char * cell = TLB[i] + (j * cell_size);
            int tg = 0;
            int two = 1;
            for (int k = 2; k < tag_size + 2; k++){
                if (cell[k] == '1'){
                    tg += two;
                }
                two = two << 1;
            }
            int ppn = 0;
            two = 1;
            for (int k = tag_size + 2; k < tag_size + 22; k++){
                if (cell[k] == '1'){
                    ppn += two;
                }
                two <<= 1;
            }
            char v = '1';
            if ((cell[0] == 0) || (cell[0] == '0')){
                v = '0';
            }
            char d = '1';
            if ((cell[1] == 0) || (cell[1] == '0')){
                d = '0';
            }
            printf(" (v=%c tag=0x%05x ppn=0x%05x d=%c) |",v,tg,ppn,d);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {

	init(); /* DO NOT DELETE. */

    char * tlbcon = "-c";
    char * dump = "-x";

    int TLB_configuration = 0;
    int Dump_TLBcontent = 0;

    char * inputfile;

    char * NoT;
    char * Asso;

    int Num_entry;
    int Assoc;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],tlbcon) == 0){ //has -c
            TLB_configuration = 1;
        }
        if (strcmp(argv[i],dump) == 0){ //has -x
            Dump_TLBcontent = 1;
        }
        if (strstr(argv[i], "/") != NULL){ //input 파일 경로
            inputfile = (char *)malloc(strlen(argv[i]) + 1);
            strcpy(inputfile,argv[i]);
        }
        if (strstr(argv[i], ":") != NULL){
            NoT = strtok(argv[i], ":");
            Asso = strtok(NULL, ":");
            Num_entry = atoi(NoT);
            Assoc = atoi(Asso);
        }
    }

    int height = Num_entry / Assoc;
    int width = Assoc;
    int tag_size = 20 - lg2(height);

    int cell_size = 22 + tag_size;

    char ** TLB;

    //printf("inputfile : %s\n",inputfile);

    TLB = (char **)malloc(sizeof(char *) * height);
    for (int i = 0; i < height; i++){
        TLB[i] = (char *)malloc(cell_size * width);
    }
    struct rank_cell ** RankTower = (struct rank_cell **)malloc(sizeof(struct rank_cell *) * height);
    for (int i = 0; i < height; i++){
        RankTower[i] = (struct rank_cell *)malloc(sizeof(struct rank_cell));
    }
    char * hea = (char *)malloc(strlen(inputfile) + 4);
    hea[0] = '.';
    hea[1] = '/';
    hea[2] = '\0';
    strcat(hea,inputfile);

    //printf("file : %s\n", hea);

    FILE * fp = fopen(hea, "r");

    char * buffer = (char *)malloc(20);

    addr_t baseaddr = page_table_base_addr();

    //printf("base addr : 0x%x\n",baseaddr);
    while(1){
        if (fgets(buffer,20,fp) == 0){
            break;
        }

        total_accesses += 1;

        char * RorW;
        char * addr;

        RorW = strtok(buffer, " ");
        addr = strtok(NULL, " ");
        char rw;
        if (strcmp(RorW,"R") == 0){
            rw = 'r';
            reads += 1;
        }
        else{
            rw = 'w';
            writes += 1;
        }

        char * binary = (char *)malloc(33 * sizeof(char));
        if(addr[strlen(addr) - 1] == '\n'){
            addr[strlen(addr) - 1] = '\0';
        }
        int len = strlen(addr);
        int ret = 0;
        for(int i = 2; i < len; i++){
            int eight = 1;
            for(int j = 0; j < len - i - 1; j++){
                eight *= 16;
            }
            char tmpp[2];
            tmpp[0] = addr[i];
            tmpp[1] = '\0';
            int ans;
            if ((addr[i] != '0') && (atoi(tmpp) == 0)){
                ans = addr[i] - 87;
            }
            else{
                ans = atoi(tmpp);
            }
            ret += ans * eight; 
        }
        
        unsigned int tmp = 0x80000000;
        for (int i = 31; i >= 0; i--) {
            if ((ret & tmp) == tmp){
                binary[i] = '1';
            }
            else{
                binary[i] = '0';
            }
            tmp >>= 1;
        }
        binary[32] = '\0';

        //printf("binary %s\n",binary);
        /*printf("%s\n",addr);
        int idx = 0;
        int two = 1;
        for (int i = 12; i < 12 + lg2(height); i++){
            if (binary[i] == '1'){
                idx += two;
            }
            two = two << 1;
        }
        int tg = 0;
        two = 1;
        for (int k = 12 + lg2(height); k < tag_size + 12 + lg2(height); k++){
            if (binary[k] == '1'){
                tg += two;
            }
            two = two << 1;
        }
        printf("rw : %c idx %d : 0x%05x\n",rw ,idx ,tg);*/



        int res = TLB_search(TLB, RankTower, binary, tag_size, cell_size, height, width, rw);
        if (res == 0){ // TLB miss

            int two = 1;
            int fir = 0;
            for (int i = 22; i < 32; i++){ // 1층
                if (binary[i] == '1'){
                    fir += two;
                }
                two <<= 1;
            }
            two = 1;
            int sec = 0;
            for (int i = 12; i < 22; i++){ // 2층
                if (binary[i] == '1'){
                    sec += two;
                }
                two <<= 1;
            }

            addr_t baseidx = baseaddr + fir * 4;
            uint32_t read = mem_read_word32(baseidx);
            res = PageTable_walker(read ,sec, 'r');

            if ((res != 1) && (res != 2)){
                TLB_insert(res, TLB, RankTower, binary, tag_size, cell_size, height, width, rw);
            }
            else{ //pagefault
                PageFault_handler(res, read, baseidx, sec, rw);

                tlb_accesses += 1;
                tlb_misses += 1;

                read = mem_read_word32(baseidx);
                res = PageTable_walker(read ,sec, 'r');
                TLB_insert(res, TLB, RankTower, binary, tag_size, cell_size, height, width, rw);
                
            }    
        }
        else if (res == 2){ //TLB hit 지만 write이라 pagetable upgrade 하러 감
            int two = 1;
            int fir = 0;
            for (int i = 22; i < 32; i++){ // 1층
                if (binary[i] == '1'){
                    fir += two;
                }
                two <<= 1;
            }
            two = 1;
            int sec = 0;
            for (int i = 12; i < 22; i++){ // 2층
                if (binary[i] == '1'){
                    sec += two;
                }
                two <<= 1;
            }

            addr_t baseidx = baseaddr + fir * 4;
            uint32_t read = mem_read_word32(baseidx);

            PageTable_walker(read ,sec, 'w');
        }
        free(binary);
    }
    
    //printf("abcd\n");

    dump_page_table_area();

    if (TLB_configuration == 1){
        cdump(Num_entry, Assoc);
    }
    if (Dump_TLBcontent == 1){
        sdump(total_accesses, reads, writes, tlb_accesses,
        tlb_hits, tlb_misses, page_walks, page_faults);
    }
    if (TLB_configuration == 1){
        TLB_content(TLB, height, width, tag_size, cell_size);
    }

    for (int i = 0; i < height; i++){
        free(TLB[i]);
    }
    for (int i = 0; i < height; i++){
        struct rank_cell * tmp = RankTower[i];

        tmp = tmp->prev;
        while (tmp != NULL){
            if (tmp->prev == RankTower[i]){
                free(tmp);
                break;
            }
            tmp = tmp->next;
        }

        free(RankTower[i]);
    }
    free(TLB);
    free(hea);
    return 0;
}
