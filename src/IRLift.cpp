#include <capstone/capstone.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include "XR2_IR.hpp" 
#include "ELFLoader.hpp"

X2R_IR::IRProgram liftX86ToIR(cont ElfLoad::ElfLoadResult& elf){

    csh handle; 
    cs_insn *insn;
    size_t count; 

    X2R_IR::IRProgram program; 
    X2R_IR::IRFunction func; 

    func.name = "lifted_function";

    /*initialise capston for x86-64 */
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        std::cerr << "Failed to initialize Capstone\n";
        return program;
    }

    // disassemble the .text section of the ELF
    // elf.text.data() = pointer to raw machine code
    // elf.text.size() = size of .text section
    // elf.textAddr = virtual address of the text section (used for instruction addresses)
    count = cs_disasm(handle, elf.text.data(), elf.text.size(), elf.textAddr, 0, &insn);

    if(count > 0)
    {
        X2R_IR::IRBasicBlock block; 
        block.name = "entry"; 

        for(size_t x =0; x < count; x++){
            X2R_IR::IRInst irInst; 

            /*reference to the current capstone instruction */ 
            const auto& ci = insn[i]; 

            /*map capstone mnemonic to IR opcode */ 
            std::string mnem = ci.mnemonic;

                        
            if (mnem == "mov") 
                irInst.opcode = MyIR::Opcode::MOV;
            else if (mnem == "add") 
                irInst.opcode = MyIR::Opcode::ADD;
            else if (mnem == "sub") 
                irInst.opcode = MyIR::Opcode::SUB;
            else if (mnem == "mul") 
                irInst.opcode = MyIR::Opcode::MUL;
            else if (mnem == "div") 
                irInst.opcode = MyIR::Opcode::DIV;
            else if (mnem == "ldr" || mnem == "load") // depending on architecture
                irInst.opcode = MyIR::Opcode::LOAD;
            else if (mnem == "str" || mnem == "store")
                irInst.opcode = MyIR::Opcode::STORE;
            else if (mnem == "jmp") 
                irInst.opcode = MyIR::Opcode::JMP;
            else if (mnem == "jz" || mnem == "jnz" || mnem == "je" || mnem == "jne") 
                irInst.opcode = MyIR::Opcode::CJMP;
            else 
                irInst.opcode = MyIR::Opcode::NOP; // fallback for unsupported instructions

        }
    }

}
