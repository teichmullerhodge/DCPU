#include <assert.h>
#include <stab.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ACC_MEMORY 512
#define INITIAL_INSTR_QUANTITY 36

#define STPINT_ADD(VAR, ADD)                                                   \
  VAR[0] = 'A';                                                                \
  VAR[1] = 'D';                                                                \
  VAR[2] = 'D';                                                                \
  VAR[3] = '\0';
#define STPINT_SUB(VAR, SUB)                                                   \
  VAR[0] = 'S';                                                                \
  VAR[1] = 'U';                                                                \
  VAR[2] = 'B';                                                                \
  VAR[3] = '\0';
#define STPINT_LOAD(VAR, LOAD)                                                 \
  VAR[0] = 'L';                                                                \
  VAR[1] = 'O';                                                                \
  VAR[2] = 'A';                                                                \
  VAR[3] = 'D';                                                                \
  VAR[4] = '\0';

typedef enum {

  LOAD,
  ADD,
  SUB,
  JMP,
  NOP

} DCPU_INST;

typedef struct {

  unsigned ACC;
  unsigned PC;
  size_t IC; // instruction counter

  bool STATUS_FLAG;

} DCPU_REG;

typedef struct {

  DCPU_INST INST;
  unsigned VALUE;

} LINE_INST;

typedef struct {

  DCPU_REG Register;
  LINE_INST *INSTRUCTIONS; // need to have a growth factor. Not implemented yet.
  size_t INSTRUCTIONS_SIZE;

} DCPU;

void INIT_DCPU(DCPU *CPU) {

  CPU->INSTRUCTIONS = malloc(sizeof(LINE_INST) * INITIAL_INSTR_QUANTITY);
  if (CPU->INSTRUCTIONS == NULL) {
    perror("Failed initializing DCPU! Memory allocation failed...\n");
    exit(-1);
  }
  CPU->Register.ACC = 0;
  CPU->Register.PC = 0;
  CPU->Register.STATUS_FLAG = true;
  CPU->INSTRUCTIONS_SIZE = INITIAL_INSTR_QUANTITY;
  return;
}

bool ASSIGN_INSTRUCTIONS(DCPU *CPU) {

  CPU->Register.IC++;
  if (CPU->Register.IC >= CPU->INSTRUCTIONS_SIZE) {

    printf("Performing reallocation...\n");
    CPU->INSTRUCTIONS = (LINE_INST *)realloc(
        CPU->INSTRUCTIONS, sizeof(LINE_INST) * (CPU->INSTRUCTIONS_SIZE * 2));
    if (CPU->INSTRUCTIONS == NULL) {
      perror("Realloc failed...exiting!\n");
      return false;
    }

    CPU->INSTRUCTIONS_SIZE *= 2;
  }
  return true;
}

void DCPU_EXEC(DCPU *CPU, DCPU_INST instruction, unsigned value) {

  if (instruction == NOP) {
    return;
  }

  switch (instruction) {
  case ADD:
    CPU->Register.ACC = CPU->Register.ACC + value <= MAX_ACC_MEMORY
                            ? CPU->Register.ACC + value
                            : CPU->Register.ACC;
    CPU->Register.STATUS_FLAG = ASSIGN_INSTRUCTIONS(CPU);
    break;
  case SUB:
    CPU->Register.ACC = (int)CPU->Register.ACC - (int)value >= 0
                            ? CPU->Register.ACC - value
                            : CPU->Register.ACC;
    CPU->Register.STATUS_FLAG = ASSIGN_INSTRUCTIONS(CPU);
    break;
  case LOAD:
    CPU->Register.ACC = value <= MAX_ACC_MEMORY ? value : CPU->Register.ACC;
    CPU->Register.STATUS_FLAG = ASSIGN_INSTRUCTIONS(CPU);
    break;
  default:
    break;
  }
}

void delay(long long T) {

  while (T > 0) {
    asm("nop");
    T--;
  }
}

void main_loop(DCPU *CPU) {

  srand(time(NULL));
  char instruction[5];
  while (1) {

    int randomInstruction = rand() % 3;
    int randomValue = rand() % MAX_ACC_MEMORY;
    if (randomInstruction == 0) {
      DCPU_EXEC(CPU, ADD, randomValue);
      STPINT_ADD(instruction, ADD);
    }
    if (randomInstruction == 1) {
      DCPU_EXEC(CPU, SUB, randomValue);
      STPINT_SUB(instruction, SUB);
    }
    if (randomInstruction == 2) {
      DCPU_EXEC(CPU, LOAD, randomValue);
      STPINT_LOAD(instruction, LOAD);
    }

    assert((int)CPU->Register.ACC >= 0 && CPU->Register.ACC <= MAX_ACC_MEMORY);
    assert(CPU->Register.IC < CPU->INSTRUCTIONS_SIZE);

    printf("[CPU INSTRUCTION] -> [%s, %d]\n", instruction, randomValue);
    printf("[CPU STATUS] -> ACC: %d, IC: %ld, IS: %ld\n", CPU->Register.ACC,
           CPU->Register.IC, CPU->INSTRUCTIONS_SIZE);
  }
}

int main(int argc, char **argv) {

  (void)argc;
  (void)argv;

  DCPU Cpu;
  INIT_DCPU(&Cpu);
  main_loop(&Cpu);
  return 0;
}
