#pragma once
#define MEM_SIZE 565536
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

uint8_t memory[MEM_SIZE];

typedef struct z80_cpu {
    //registri principale
    uint8_t A,F;
    uint8_t B,C;
    uint8_t D,E;
    uint8_t H,L;

    //registri ombra che permettono di salvare lo stato dei registri senza dover ricorrere allo stack
    uint8_t AA,FF;
    uint8_t BB,CC;
    uint8_t DD,EE;
    uint8_t HH,LL;

    //registri indice, utilizzati per l'indirizzamento indicizzato soprattutto per i base index
    uint16_t IX;
    uint16_t IY;

    uint16_t SP; //stack pointer
    uint16_t PC; //program counter
    
    //stato halted
    int halted; 

    //flags
    uint8_t flags;

} z80_cpu;


#define FLAG_C  0x01  // Carry
#define FLAG_N  0x02  // Subtract
#define FLAG_PV 0x04  // Parity/Overflow
#define FLAG_H  0x10  // Half Carry
#define FLAG_Z  0x40  // Zero
#define FLAG_S  0x80  // Sign

void update_flags_add(z80_cpu* CPU, uint8_t a, uint8_t b, uint16_t result);
void update_flags_sub(z80_cpu* CPU, uint8_t a, uint8_t b, uint16_t result);

//creiamo degli helper per i registri combinati: BC, DE, HL, AF
uint16_t get_AF(z80_cpu* CPU){
    return (CPU->A << 8) | CPU->F;
}

uint16_t get_BC(z80_cpu* CPU){
    return (CPU->B << 8) | CPU->C;
}

uint16_t get_DE(z80_cpu* CPU){
    return (CPU->D << 8) | CPU->E;
}

uint16_t get_HL(z80_cpu* CPU){
    return (CPU->H << 8) | CPU->L;
}


uint16_t set_AF(z80_cpu* CPU, uint16_t val){
    CPU->A = val >> 8;
    CPU->F = val & 0xFF; 
}

uint16_t set_BC(z80_cpu* CPU, uint16_t val){
    CPU->B = val >> 8;
    CPU->C = val & 0xFF; 
}

uint16_t set_DE(z80_cpu* CPU, uint16_t val){
    CPU->D = val >> 8;
    CPU->E = val & 0xFF; 
}

uint16_t set_HL(z80_cpu* CPU, uint16_t val){
    CPU->H = val >> 8;
    CPU->L = val & 0xFF; 
}

void CPU_reset(z80_cpu* CPU);
uint8_t fetch(z80_cpu* CPU);
void z80_step(z80_cpu* CPU);
void push(z80_cpu* CPU, uint16_t val);
uint16_t pop(z80_cpu * CPU);
uint16_t fetch_word(z80_cpu *CPU);
