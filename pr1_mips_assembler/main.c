#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 1000000

int hextoint(char * str){
	int ret = 0;
	int lenstr = strlen(str);
	for(int i = 2; i < lenstr; i++){
		int eight = 1;
		for(int j = 0; j < lenstr - i - 1; j++){
			eight *= 16;
		}
		char tmpp[2];
		tmpp[0] = str[i];
		tmpp[1] = '\0';
		int ans;
		if ((str[i] != '0') && (atoi(tmpp) == 0)){
			ans = str[i] - 87;
		}
		else{
			ans = atoi(tmpp);
		}
		ret += ans * eight; 
	}
	return ret;
}

int iflong(int tmp){
	if (tmp % 128 != 0){
		return 1;
	}
	return 0;
}

int findaddr(char * str, char ** strlis, int * addrlis, int gaso){
	for (int i = 0; i < gaso; i++){
		if(strcmp(str,strlis[i]) == 0){
			return addrlis[i];
		}
	}
}

void binary5(int a, char * bin){
	unsigned int tmp = 0x10;
	for (int i = 4; i >= 0; i--) {
        if ((a & tmp) == tmp){
            bin[4 - i] = '1';
        }
        else{
            bin[4 - i] = '0';
        }
        tmp >>= 1;
    }
    bin[5] = '\0';
}

void binary16(int a, char * bin){
	unsigned int tmp = 0x8000;
	for (int i = 15; i >= 0; i--) {
        if ((a & tmp) == tmp){
            bin[15 - i] = '1';
        }
        else{
            bin[15 - i] = '0';
        }
        tmp >>= 1;
    }
    bin[16] = '\0';
}

void binary18(int a, char * bin){
	unsigned int tmp = 0x20000;
	for (int i = 17; i >= 0; i--) {
        if ((a & tmp) == tmp){
            bin[17 - i] = '1';
        }
        else{
            bin[17 - i] = '0';
        }
        tmp >>= 1;
    }
    bin[18] = '\0';
}

void binary32(int a, char * bin){
	unsigned int tmp = 0x80000000;
	for (int i = 31; i >= 0; i--) {
        if ((a & tmp) == tmp){
            bin[31 - i] = '1';
        }
        else{
            bin[31 - i] = '0';
        }
        tmp >>= 1;
    }
    bin[32] = '\0';
}

int main(int argc, char* argv[]){

	//RTYPE
	char * ADDU = "addu";
	char * AND = "and";
	char * NOR = "nor";
	char * OR = "or";
	char * SLTU = "sltu";
	char * SLL = "sll";
	char * SRL = "srl";
	char * SUBU = "subu";

	//ITYPE
	char * ADDIU = "addiu";
	char * ANDI = "andi";
	char * BEQ = "beq";
	char * BNE = "bne";
	char * LUI = "lui";
	char * LW = "lw";
	char * LA = "la";
	char * ORI = "ori";
	char * SLTIU = "sltiu";
	char * SW = "sw";

	//JTYPE
	char * J = "j";
	char * JAL = "jal";
	char * JR = "jr";

	char * data = ".data";
	char * text = ".text";

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n");
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else{
		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));

		if(freopen(file, "r",stdin)==0){
			printf("File open Error!\n");
			exit(1);
		}

		char *** pars;
		pars = (char ***)malloc(sizeof(char **) * BUF_SIZE);
		for (int i = 0; i < BUF_SIZE; i++){
			pars[i] = (char **)malloc(sizeof(char *) * 4);
			for (int j = 0; j < 4; j++){
				pars[i][j] = (char *) malloc(sizeof(char) * 100);  
			}
		}

		char ** datalis; //data 이름 저장
		datalis = (char **)malloc(sizeof(char *) * BUF_SIZE);
		for (int i = 0; i < BUF_SIZE; i++){
			datalis[i] = (char *)malloc(sizeof(char *) * 30);
		}
		
		int * dataaddrlis = (int *)malloc(sizeof(char *) * BUF_SIZE); //data 주소 저장
		int * datavallis = (int *)malloc(sizeof(char *) * BUF_SIZE); //data 값 저장

		char ** labellis; //label 이름 저장
		labellis = (char **)malloc(sizeof(char *) * BUF_SIZE);
		for (int i = 0; i < BUF_SIZE; i++){
			labellis[i] = (char *)malloc(sizeof(char *) * 30);
		}
		
		int * labeladdrlis = (int *)malloc(sizeof(char *) * BUF_SIZE); //label 주소 저장		

		char buf[1000];


		int line = 0;
		int idx;
		int check;
		while(fgets(buf,1000,stdin) > 0){
			idx = 0;
			check = 0;
			int gaso = 0;
			for (int i = 0; i < strlen(buf); i++){
				if ((buf[i] == '\t') || (buf[i] == '\n') || (buf[i] == ' ') || (buf[i] == ',') || (buf[i] == '\0')){
					if (check != 0) {
						pars[line][idx][check] = '\0';
						idx++;
					}
					check = 0;
					//printf("%c",buf[i]);
				}
				else{
					if(check == 0){
						gaso++;
						pars[line][idx][check] = buf[i];
						check++;
					}
					else{
						pars[line][idx][check] = buf[i];
						check++;
					}
				}
			}
			line++;
		}


		int isdata = 0;

		int dataaddr = 0x10000000;
		int instaddr = 0x400000;
		int len;
		int datagaso = 0;
		int wordgaso = 0;
		int labelgaso = 0;
		int instgaso = 0;
		int tmp = 0;
		for(int i0 = 0; i0 < line; i0++){
			char ** strlis = pars[i0];
			if (strcmp(strlis[0],text) == 0){
				isdata = 1;
			}
			else if ((isdata == 0) && (strcmp(data,strlis[0]) != 0)){ //data area
				len = strlen(strlis[0]);
				if (strlis[0][len - 1] == ':'){ //data:
					strlis[0][len - 1] = '\0';
					tmp = 2;
					datalis[datagaso] = strlis[0];
					dataaddrlis[datagaso] = dataaddr;
					datagaso++;
				}
				else{ //걍 .word
					tmp = 1;
				}
				if (strlis[tmp][1] == 'x'){
					datavallis[wordgaso] = hextoint(strlis[tmp]);
				}
				else{
					datavallis[wordgaso] = atoi(strlis[tmp]);
				}
				wordgaso++;
				dataaddr += 4;
			}
			else if (strcmp(data,strlis[0]) != 0){
				len = strlen(strlis[0]);
				if (strlis[0][len - 1] == ':'){ //label:
					strlis[0][len - 1] = '\0';
					labellis[labelgaso] = strlis[0];
					labeladdrlis[labelgaso] = instaddr;
					labelgaso++;
				}
				else{
					if (strcmp(strlis[0],LA) == 0){
						int dataaddr = findaddr(strlis[2],datalis,dataaddrlis,datagaso);
						if(iflong(dataaddr) == 1){
							instaddr += 4;
							instgaso += 4;
						}
						instaddr += 4;
						instgaso += 4;
					}
					else{
						instgaso += 4;
						instaddr += 4;
					}
				}
			}
		}

		/*
		for(int i = 0; i < datagaso; i++){
			printf("%s ",datalis[i]);
		}
		printf("\n");
		for(int i = 0; i < datagaso; i++){
			printf("0x%x ",dataaddrlis[i]);
		}
		printf("\n");
		for(int i = 0; i < wordgaso; i++){
			printf("0x%x ",datavallis[i]);
		}
		printf("\n");

		for(int i = 0; i < labelgaso; i++){
			printf("%s ",labellis[i]);
		}
		printf("\n");
		for(int i = 0; i < labelgaso; i++){
			printf("0x%x ",labeladdrlis[i]);
		}
		printf("\n");*/

		file[strlen(file)-1] ='o';
		freopen(file,"w",stdout);

		char howinst[33];
		char howdata[33];

		binary32(wordgaso * 4,howdata);
		binary32(instgaso,howinst);

		printf("%s",howinst);
		printf("%s",howdata);

		instaddr = 0x400000;
		for(int i = 0; i < line; i++){
			char ** strlis = pars[i];
			char output[32] = {'0',};
			char output2[32] = {'0',};
			int type = -1;
			if (strcmp(ADDU, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '0';
				output[29] = '0';
				output[30] = '0';
				output[31] = '1';
			}
			else if (strcmp(AND, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '0';
				output[29] = '1';
				output[30] = '0';
				output[31] = '0';

			}
			else if (strcmp(NOR, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '0';
				output[29] = '1';
				output[30] = '1';
				output[31] = '1';

			}
			else if (strcmp(OR, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '0';
				output[29] = '1';
				output[30] = '0';
				output[31] = '1';

			}
			else if (strcmp(SLTU, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '1';
				output[29] = '0';
				output[30] = '1';
				output[31] = '1';
			}
			else if (strcmp(SLL, strlis[0]) == 0){
				type = 3;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '0';
				output[27] = '0';
				output[28] = '0';
				output[29] = '0';
				output[30] = '0';
				output[31] = '0';

			}
			else if (strcmp(SRL, strlis[0]) == 0){
				type = 3;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '0';
				output[27] = '0';
				output[28] = '0';
				output[29] = '0';
				output[30] = '1';
				output[31] = '0';

			}
			else if (strcmp(SUBU, strlis[0]) == 0){
				type = 0;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				output[26] = '1';
				output[27] = '0';
				output[28] = '0';
				output[29] = '0';
				output[30] = '1';
				output[31] = '1';

			}
			//
			else if (strcmp(ADDIU, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '0';
				output[4] = '0';
				output[5] = '1';

			}
			else if (strcmp(ANDI, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '1';
				output[4] = '0';
				output[5] = '0';

			}
			else if (strcmp(BEQ, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '1';
				output[4] = '0';
				output[5] = '0';

			}
			else if (strcmp(BNE, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '1';
				output[4] = '0';
				output[5] = '1';

			}
			else if (strcmp(LUI, strlis[0]) == 0){
				type = 5;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '1';
				output[4] = '1';
				output[5] = '1';

			}
			else if (strcmp(LW, strlis[0]) == 0){
				type = 6;
				output[0] = '1';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '1';
				output[5] = '1';

			}
			else if (strcmp(LA, strlis[0]) == 0){
				type = 4;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '1';
				output[4] = '1';
				output[5] = '1';
			}
			else if (strcmp(ORI, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '1';
				output[4] = '0';
				output[5] = '1';

			}
			else if (strcmp(SLTIU, strlis[0]) == 0){
				type = 1;
				output[0] = '0';
				output[1] = '0';
				output[2] = '1';
				output[3] = '0';
				output[4] = '1';
				output[5] = '1';

			}
			else if (strcmp(SW, strlis[0]) == 0){
				type = 6;
				output[0] = '1';
				output[1] = '0';
				output[2] = '1';
				output[3] = '0';
				output[4] = '1';
				output[5] = '1';

			}
			//
			else if (strcmp(J, strlis[0]) == 0){
				type = 2;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '1';
				output[5] = '0';

			}
			else if (strcmp(JAL, strlis[0]) == 0){
				type = 2;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '1';
				output[5] = '1';

			}
			else if (strcmp(JR, strlis[0]) == 0){
				type = 7;
				output[0] = '0';
				output[1] = '0';
				output[2] = '0';
				output[3] = '0';
				output[4] = '0';
				output[5] = '0';

				for(int j = 11; j < 32; j++){
					output[j] = '0';
				}
				output[28] = '1';
			}

			if(type != -1){
				int two = 0;
				if(type == 0){ //R format
					int rdd = atoi(strlis[1] + 1);
					int rss = atoi(strlis[2] + 1);
					int rtt = atoi(strlis[3] + 1);
					char rs[6];
					char rt[6];
					char rd[6];
					binary5(rss,rs);
					binary5(rtt,rt);
					binary5(rdd,rd);

					for(int j = 0; j < 5; j++){
						output[j + 6] = rs[j];
						output[j + 11] = rt[j];
						output[j + 16] = rd[j];
						output[j + 21] = '0';
					}
				}
				else if(type == 1){ //I format
					int rss = atoi(strlis[1] + 1);
					int rtt = atoi(strlis[2] + 1);
					int imm;
					int off = 0;
					if (strlis[3][1] == 'x'){
						imm = hextoint(strlis[3]);
					}
					else{
						char tmp[2];
						tmp[0] = '0';
						tmp[1] = '1';
						if ((atoi(strlis[3]) == 0) && (strcmp(tmp,strlis[3]) != 0)){
							int labeladdr = findaddr(strlis[3],labellis,labeladdrlis,labelgaso);
							imm = labeladdr - (instaddr + 4);
							off = 1;
						}
						else{
							imm = atoi(strlis[3]);
						}
					}
					char rs[6];
					char rt[6];
					char im[17];
					binary5(rss,rs);
					binary5(rtt,rt);
					binary16(imm,im);

					
					if(off == 0){	
						for(int j = 0; j < 5; j++){
							output[j + 11] = rs[j];
							output[j + 6] = rt[j];
						}
						for(int j = 0; j < 16; j++){
							output[j + 16] = im[j];
						}
					}
					else{
						for(int j = 0; j < 5; j++){
							output[j + 11] = rt[j];
							output[j + 6] = rs[j];
						}
						char offset[19];
						/*
						if (imm < 0){
							int immp = imm * (-1);
							binary18(immp,offset);
							for (int j = 0; j < 18; j++){
								if (offset[j] == '0'){
									offset[j] = '1';
								}
								else{
									offset[j] = '0';
								}
							}
							char carry = '1';
							for (int j = 17; j >=0; j--){
								if (offset[j] == '1'){
									if (carry == '1'){ //1 + 1
										offset[j] = '0';
									}
									else{ //1 + 0
										offset[j] = '1';
									}
								}
								else{
									if (carry == '1'){ //0 + 1
										offset[j] = '1';
										carry = '0';
									}
									else{ //0 + 0
										offset[j] = '0';
									}
								}
							}
						}
						else{*/
						binary18(imm,offset);
						
						for(int j = 0; j < 16; j++){
							output[j + 16] = offset[j];
						}
					}
				}
				else if(type == 2){ //J format
					int labeladdr = findaddr(strlis[1],labellis,labeladdrlis,labelgaso);
					char addr[32];
					binary32(labeladdr,addr);
					output[6] = '0';
					output[7] = '0';
					output[8] = '0';
					output[9] = '0';
					for(int j = 4; j < 26; j++){
						output[j + 6] = addr[j + 4];
					}
				}
				else if(type == 3){ //shift type
					int rdd = atoi(strlis[1] + 1);
					int rtt = atoi(strlis[2] + 1);
					int saa;
					if (strlis[3][1] == 'x'){
						saa = hextoint(strlis[3]);
					}
					else{
						saa = atoi(strlis[3]);
					}
					char rt[6];
					char rd[6];
					char sa[6];
					binary5(rtt,rt);
					binary5(rdd,rd);
					binary5(saa,sa);

					for(int j = 0; j < 5; j++){
						output[j + 6] = '0';
						output[j + 11] = rt[j];
						output[j + 16] = rd[j];
						output[j + 21] = sa[j];
					}
				}
				else if(type == 4){ //la
					int rss = atoi(strlis[1] + 1);
					char rs[6];
					char addr[32];

					binary5(rss,rs);
					int dataddr = findaddr(strlis[2],datalis,dataaddrlis,datagaso);
					binary32(dataddr,addr);

					for(int j = 0; j < 5; j++){
						output[j + 11] = rs[j];
						output[j + 6] = '0';
					}
					for(int j = 0; j < 16; j++){
						output[j + 16] = addr[j];
					}
					output[0] = '0';
					output[1] = '0';
					output[2] = '1';
					output[3] = '1';
					output[4] = '1';
					output[5] = '1';
					if(iflong(dataddr) == 1){
						two = 1;
						output2[0] = '0';
						output2[1] = '0';
						output2[2] = '1';
						output2[3] = '1';
						output2[4] = '0';
						output2[5] = '1';
						for(int j = 0; j < 5; j++){
							output2[j + 6] = rs[j];
							output2[j + 11] = rs[j];
						}
						for(int j = 0; j < 16; j++){
							output2[j + 16] = addr[j + 16];
						}
					}
				}
				else if(type == 5){ //lui
					int rtt = atoi(strlis[1] + 1);
					int imm;
					if (strlis[2][1] == 'x'){
						imm = hextoint(strlis[2]);
					}
					else{
						imm = atoi(strlis[2]);
					}
					char rt[6];
					char im[17];
					binary5(rtt,rt);
					binary16(imm,im);

					for(int j = 0; j < 5; j++){
						output[j + 6] = '0';
						output[j + 11] = rt[j];
					}
					for(int j = 0; j < 16; j++){
						output[j + 16] = im[j];
					}
				}
				else if(type == 6){ //lw
					int rtt = atoi(strlis[1] + 1);
					char tmp[100];
					int j;
					for(j = 0; j < strlen(strlis[2]); j++){
						if (strlis[2][j] == '('){
							break;
						}
						else{
							tmp[j] = strlis[2][j];
						}
					}
					tmp[j] = '\0';

					int off = atoi(tmp);
					strlis[2][strlen(strlis[2]) - 1] = '\0';
					int rss = atoi(strlis[2] + j + 2);

					char rs[6];
					char rt[6];
					char of[17];
					binary5(rss,rs);
					binary5(rtt,rt);
					binary16(off,of);
					for(int j = 0; j < 5; j++){
						output[j + 11] = rt[j];
						output[j + 6] = rs[j];
					}
					for(int j = 0; j < 16; j++){
						output[j + 16] = of[j];
					}
				}
				else{
					int rss = atoi(strlis[1] + 1);
					char rs[6];
					binary5(rss,rs);
					for(int j = 0; j < 5; j++){
						output[j + 6] = rs[j];
					}
				}
				//printf("%s\n",strlis[0]);
				for(int j = 0; j < 32;j++){
					/*
					if(j == 6){
						printf(" ");
					}*/
					printf("%c",output[j]);
				}
				if ((type == 4) && (two == 1)){
					for(int j = 0; j < 32; j++){
						/*
						if(j == 6){
							printf(" ");
						}*/
						printf("%c",output2[j]);
					}
					instaddr += 4;
				}
				//printf("\n");

				instaddr += 4;
			}
		}

		for (int i = 0; i < wordgaso; i++){
			int data = datavallis[i];
			char dataa[33];
			binary32(data,dataa);
			printf("%s",dataa);
		}

		/*
		// For output file write 
		// You can see your code's output in the sample_input/example#.o 
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer*/
		
		

		//If you use printf from now on, the result will be written to the output file.

		//printf("Hello World!\n"); 


	}
	return 0;
}

