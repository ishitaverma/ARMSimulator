/*
Developer's Name: Nitika Saran 2014068 , Ishita Verma 2014051
Developer's Email id: nitika14068@iiitd.ac.in , ishita14051@iiitd.ac.in
Date: March 21, 2015
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Register file
static unsigned int R[16];
//flags
static int N,C,V,Z;
//memory
static unsigned char MEM[4000];
//intermediate datapath and control path signals
static unsigned int data[16][100];
static unsigned int instruction_word;
static unsigned int f,cond,opcode;
static unsigned int rn,rd,rm;
static unsigned int S,I;
static unsigned int shift;
static unsigned int operand1,operand2;
int acc;
int offset;
int *sp;
int *d,*n;

int read_word(char *mem, unsigned int address) {
  int *data;
  data =  (int*) (mem + address);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int *data_p;
  data_p = (int*) (mem + address);
  *data_p = data;
}


// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
  int i;
  for (i = 0; i < 16; ++i){
    R[i]=0;
  }
  memset(MEM,4000,0);
  N=0;C=0;V=0;Z=0;
  S=0;
  operand1=0;
  operand2=0;
  instruction_word=0;
  acc=0;
}


//load_program_memory reads the input memory, and pupulates the instruction 
// memory
void load_program_memory(char *file_name) {
  FILE *fp;
  unsigned int address, instruction;
  fp = fopen(file_name, "r");
  if(fp == NULL) {
    printf("Error opening input mem file\n");
    exit(1);
  }
  while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    write_word(MEM, address, instruction);
  }
  fclose(fp);
}

//writes the data memory in "data_out.mem" file
void write_data_memory() {
  FILE *fp;
  unsigned int i;
  fp = fopen("data_out.mem", "w");
  if(fp == NULL) {
    printf("Error opening dataout.mem file for writing\n");
    return;
  }
  
  for(i=0; i < 4000; i = i+4){
    fprintf(fp, "%x %x\n", i, read_word(MEM, i));
  }
  fclose(fp);
}

//should be called when instruction is swi_exit
void swi_exit() {
  write_data_memory();
  exit(0);
}


short check(){
  if(cond==14) return 1;
  if(cond==0) return Z;
  if(cond==1) return (!Z);
  if(cond==2) return C;
  if(cond==3) return (!C);
  if(cond==4) return N;
  if(cond==5) return (!N);
  if(cond==6) return V;
  if(cond==7) return (!V);
  if(cond==8) return (C & (!Z));
  if(cond==9) return ((!C) | Z);
  if(cond==10) return (N==V);
  if(cond==11) return (N);
  if(cond==12) return ((!Z) & (N==V));
  if(cond==13) return (Z | (N!=V));
  return 0;
}

int process(){
  if(opcode==0){
    printf("AND operation\n");
   return operand1 & operand2;
  }
  if(opcode==1) return operand1^operand2;
  if(opcode==2){
    printf("Subtract operation\n");
   return operand1-operand2;
  }
  if(opcode==3) return operand2-operand1;
  if(opcode==4){ 
    printf("Add operation\n");
    return operand1+operand2;
  }
  if(opcode==5) return operand1+operand2+C;
  if(opcode==6) return operand1-operand2+C-1;
  if(opcode==7) return operand2-operand1+C-1;
  if(opcode==8){
    S=1;
    return operand1 & operand2;
  }
  if(opcode==9){
    S=1;
    return operand1^operand2;
  }
  if(opcode==10){
    printf("CMP instruction\n");
    S=1;
    return operand1-operand2;
  }
  if(opcode==11){
    S=1;
    return operand1+operand2;
  }
  if(opcode==12){ 
    printf("OR operation\n");
    return operand1|operand2;
  }
  if(opcode==13) {
    printf("MOV operation\n");
    return operand2;
  }
  if(opcode==14) return operand1 & (~operand2);
  return ~operand2;
}


void branch(){
  if (offset>>23){
    offset = offset | 0xff000000;
  }
  printf("Offset : %d\n",offset);
  offset = offset<<2;
  R[15]+=offset;
}

void mem(){
  if(f!=1){
    printf("No memory opeation required\n");
    return;
  }
  if(opcode==24){   
    printf("LDR instruction\n");  
    acc= data[rn][offset>>2];
  }
  else if(opcode==25){
    printf("STR instruction\n");
    data[rn][offset>>2]=R[rd];
  }
}



//reads from the instruction memory and updates the instruction register
void fetch() {
  instruction_word=read_word(MEM,R[15]);
  printf("Instruction Word fetched: %x\n",instruction_word );
  R[15]+=4;
}


//reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode() {
  unsigned int mask;
  cond=instruction_word>>28;
  f=(instruction_word & (1<<27));
  f = f>>26;
  mask =(instruction_word & (1<<26));
  mask = mask >> 26;
  f=f+mask;
  printf("Condition code : %u\n", cond);
  printf("format : %u\n",f);
 
  if(f==0){
    I = instruction_word & (1<<25);
    mask = 15<<21;
    opcode = mask & instruction_word;
    opcode = opcode >> 21;
    S= instruction_word & (1<<20);
    if(S) printf("Set condition codes\n");
    mask = 15 << 16 ;
    rn = mask & instruction_word;
    rn = rn >> 16;
    mask = 15 << 12;
    rd = mask & instruction_word;
    rd = rd >> 12;
    operand1=R[rn];
    printf("Data Processing Instruction\n");
    printf("Opcode : %u\n", opcode);
    printf("Rd : %d\n", rd);
    printf("Rn : %d\n", rn);
    //printf("Operand1 : %u\n",operand1 );
    if(I){
      mask = 4095;
      operand2= mask & instruction_word;
      printf("Immediate operand2 : %u\n", operand2);
    }
    else{
      mask = 15;
      rm = instruction_word & mask;
      operand2=R[rm];
      printf("Rm : %d\n", rm);
      printf("Operand2 : %u\n", operand2);
    }
  }
  else if(f==1){
    mask = 63 << 20 ;
    opcode=mask & instruction_word;
    opcode = opcode >> 20;
    mask = 15 << 16;
    rd = instruction_word & mask;
    rd = rd >> 16;
    mask = 15 << 12;
    rn = mask & instruction_word;
    rn = rn >> 12;
    mask = 4095 ;
    offset= mask & instruction_word;
    printf("Data Tranfer Instruction\n");
    printf("Opcode : %u\n",opcode );
    printf("Rd : %d\n", rd);
    printf("Rn : %d\n", rn);
    printf("Offset : %u\n",offset);
  }
  else if(f==2){
    mask = 16777215;
    offset = mask & instruction_word; 
    printf("Branch Instruction\n");
 //   printf("Offset : %d\n",offset );  
  }
  else printf("Exit Instruction\n");
}



//executes the ALU operation based on ALUop
void execute() {
  if(!check()){
    printf("Condition not matched.\n");
    return;
  }
  if(f==0){
    acc=process();
    if(S){
      if(acc<0) N=1;
      else N=0;
      if(acc==0) Z=1;
      else Z=0;
    }
  }
  if(f==2) branch();
  if(f==3) {
    printf("XXXXXXXXXXXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    swi_exit();
  }
}




//writes the results back to register file
void write_back() {
  if(f==0 && acc>0){
   R[rd]=acc;
  // printf("wierddd'\n");
 }
  if(f==1 && opcode==24){
    R[rd]=acc;
  }
}


void run_armsim() {
  int i,j=100;
  while(1){
    fetch();
    decode();
    execute();
    mem();
    write_back();
    for(i=0;i<16;i++)
      printf("R%d: %u\n",i,R[i]);
    printf("N: %d Z: %d \n", N,Z);
    printf("-------------------------\n");
  }
}

int main(int argc, char** argv) {
  char* prog_mem_file; 
  if(argc < 2) {
    printf("Incorrect number of arguments. Please invoke the simulator \n\t./myARMSim <input mem file> \n");
    exit(1);
  }
  
  //reset the processor
  reset_proc();
  //load the program memory
  load_program_memory(argv[1]);
  //run the simulator
  run_armsim();

  return 1;
}


