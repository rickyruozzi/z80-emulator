#include "z80.h"

#define SET_FLAG(cpu, f)    ((cpu)->flags |=  (f))
#define RESET_FLAG(cpu, f)  ((cpu)->flags &= ~(f))
#define TEST_FLAG(cpu, f)   ((cpu)->flags &   (f))

void update_flags_add(z80_cpu* CPU, uint8_t a, uint8_t b, uint16_t result) {
    RESET_FLAG(CPU, FLAG_N);                                          // ADD resetta N
    (result & 0xFF) == 0 ? SET_FLAG(CPU, FLAG_Z) : RESET_FLAG(CPU, FLAG_Z);   // Zero
    result & 0x80        ? SET_FLAG(CPU, FLAG_S) : RESET_FLAG(CPU, FLAG_S);   // Sign
    result > 0xFF        ? SET_FLAG(CPU, FLAG_C) : RESET_FLAG(CPU, FLAG_C);   // Carry
    // Overflow: segno di a e b uguali, segno risultato diverso
    ((~(a ^ b)) & (a ^ result) & 0x80) ? SET_FLAG(CPU, FLAG_PV) : RESET_FLAG(CPU, FLAG_PV);
    // Half carry: carry dal bit 3 al bit 4
    ((a & 0xF) + (b & 0xF)) > 0xF ? SET_FLAG(CPU, FLAG_H) : RESET_FLAG(CPU, FLAG_H);
}

void update_flags_sub(z80_cpu* CPU, uint8_t a, uint8_t b, uint16_t result) {
    SET_FLAG(CPU, FLAG_N);                                            // SUB setta N
    (result & 0xFF) == 0 ? SET_FLAG(CPU, FLAG_Z) : RESET_FLAG(CPU, FLAG_Z);   // Zero
    result & 0x80        ? SET_FLAG(CPU, FLAG_S) : RESET_FLAG(CPU, FLAG_S);   // Sign
    result > 0xFF        ? SET_FLAG(CPU, FLAG_C) : RESET_FLAG(CPU, FLAG_C);   // Carry
    ((a ^ b) & (a ^ result) & 0x80) ? SET_FLAG(CPU, FLAG_PV) : RESET_FLAG(CPU, FLAG_PV);
    ((a & 0xF) < (b & 0xF))         ? SET_FLAG(CPU, FLAG_H) : RESET_FLAG(CPU, FLAG_H);
}

void CPU_reset(z80_cpu* CPU){
    memset(CPU, 0, sizeof(z80_cpu)); //azzera il register file della CPU
    CPU->PC = 0X0000;
    CPU->SP = 0XFFFF; //reset di PC e SP ai valori di default
}

uint8_t fetch(z80_cpu* CPU){
    return memory[CPU->PC++]; //recupera l'istruzione scritta nel byte successivo
}


void z80_step(z80_cpu* CPU){
    uint8_t opcode = fetch(CPU);
    switch(opcode){
        case 0x00 :break; //NOP

        //Caricamenti nel rgistro
        case 0x3E: CPU->A = fetch(CPU); break;
        case 0x06 :CPU->B = fetch(CPU); break;
        case 0x0E :CPU->C = fetch(CPU); break;
        case 0x16: CPU->D = fetch(CPU); break;
        case 0x1E: CPU->E = fetch(CPU); break; 
        case 0x26: CPU->H = fetch(CPU); break; 
        case 0x2E: CPU->L = fetch(CPU); break;

        //Copia tra registri
        case 0x78: CPU->A = CPU->B; break;  
        case 0x79: CPU->A = CPU->C; break;  
        case 0x7A: CPU->A = CPU->D; break;  
        case 0x7B: CPU->A = CPU->E; break;  
        case 0x7C: CPU->A = CPU->H; break;  
        case 0x7D: CPU->A = CPU->L; break;  
        case 0x47: CPU->B = CPU->A; break;  
        case 0x41: CPU->B = CPU->C; break;  
        case 0x4F: CPU->C = CPU->A; break;  
        case 0x57: CPU->D = CPU->A; break;  
        case 0x5F: CPU->E = CPU->A; break;  
        case 0x67: CPU->H = CPU->A; break;  
        case 0x6F: CPU->L = CPU->A; break;  

        // add A,n
        case 0xC6:
            uint8_t val = fetch(CPU);
            uint16_t result = (uint16_t)CPU->A + val;
            update_flags_add(CPU, CPU->A, val, result); //aggiorna le flag delle operazioni
            CPU->A = result & 0xFF; // pulisce il valore
            break;

        // sub A,n
        case 0xD6:
            uint8_t val = fetch(CPU);
            uint16_t result = (uint16_t) - val; 
            update_flags_sub(CPU, CPU->A, val, result);
            CPU->A = result & 0xFF;
            break;

        // ADD A + r
        case 0x87: result = (uint16_t)CPU->A + CPU->A; update_flags_add(CPU, CPU->A, CPU->A, result); CPU->A = result & 0xFF; break; 
        case 0x80: result = (uint16_t)CPU->A + CPU->B; update_flags_add(CPU, CPU->A, CPU->B, result); CPU->A = result & 0xFF; break; 
        case 0x81: result = (uint16_t)CPU->A + CPU->C; update_flags_add(CPU, CPU->A, CPU->C, result); CPU->A = result & 0xFF; break; 
        case 0x82: result = (uint16_t)CPU->A + CPU->D; update_flags_add(CPU, CPU->A, CPU->D, result); CPU->A = result & 0xFF; break; 
        case 0x83: result = (uint16_t)CPU->A + CPU->E; update_flags_add(CPU, CPU->A, CPU->E, result); CPU->A = result & 0xFF; break; 

        //SUB A - r
        case 0x97: result = (uint16_t)CPU->A - CPU->A; update_flags_sub(CPU, CPU->A, CPU->A, result); CPU->A = result & 0xFF; break; 
        case 0x90: result = (uint16_t)CPU->A - CPU->B; update_flags_sub(CPU, CPU->A, CPU->B, result); CPU->A = result & 0xFF; break; 
        case 0x91: result = (uint16_t)CPU->A - CPU->C; update_flags_sub(CPU, CPU->A, CPU->C, result); CPU->A = result & 0xFF; break; 
        case 0x92: result = (uint16_t)CPU->A - CPU->D; update_flags_sub(CPU, CPU->A, CPU->D, result); CPU->A = result & 0xFF; break; 
        case 0x93: result = (uint16_t)CPU->A - CPU->E; update_flags_sub(CPU, CPU->A, CPU->E, result); CPU->A = result & 0xFF; break; 

        //Jump to N
        case 0xc3:
            CPU->PC = fetch(CPU->PC); //imposta il pc scritto sul valore nell'indirizzo indicato da pc+1
            break;

        // jump to n if flag_zero is activated
        case 0xCA:
            uint16_t addr = fetch_word(CPU); 
            if (TEST_FLAG(CPU, FLAG_Z)) CPU->PC = addr;
            break;  
        
        case 0xC2:
            uint16_t addr = fetch_word(CPU);
            if(!TEST_FLAG(CPU, FLAG_Z)) CPU->PC = addr; 
            break;

        case 0xDA : 
            uint16_t addr = fetch_word(CPU);
            if(TEST_FLAG(CPU, FLAG_C)) CPU->PC = addr;
            break;

        case 0xD2:
            uint16_t addr = fetch_word(CPU);
            if(!TEST_FLAG(CPU, FLAG_C)) CPU->PC = addr; 
            break;

        default:
            printf("Opcode non implementato");
            exit(1);
    }
}

void push(z80_cpu *cpu, uint16_t val){
    cpu->SP--;
    memory[cpu->SP] = (val >> 8);      // primo byte

    cpu->SP--;
    memory[cpu->SP] = (val & 0xFF);    // secondo byte
}

uint16_t pop(z80_cpu * CPU){
    uint8_t bb = memory[CPU->SP]; //byte basso
    CPU->SP++;
    uint8_t ba = memory[CPU->SP]; //byte alto
    CPU->SP++;

    uint16_t val = ba << 8; //shift per i primi 8 bit
    return val | bb; //aggiungiamo gli ultimi 8 bit
}

uint16_t fetch_word(z80_cpu *CPU){
    //LITTLE ENDIAN => salviamo prima il byte basso
    uint8_t basso = memory[CPU->PC++]; 
    uint8_t alto = memory[CPU->PC++];
    return ((uint16_t)alto << 8) | basso ; //restituiamo la word
}

