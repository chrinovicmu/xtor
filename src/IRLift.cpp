#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include "X2R_IR.hpp" 
#include "ELFLoader.hpp"
#include "utils.hpp"

namespace X2R_IR {


X2R_IR::Opcode decodeMnemonic(const std::string& mnem) {

    switch (fnv1a(mnem.c_str())) {

        case fnv1a("mov"):  
            return X2R_IR::Opcode::MOV;
        case fnv1a("add"):  
            return X2R_IR::Opcode::ADD;
        case fnv1a("sub"):  
            return X2R_IR::Opcode::SUB;
        case fnv1a("mul"):  
            return X2R_IR::Opcode::MUL;
        case fnv1a("div"):  
            return X2R_IR::Opcode::DIV;

        case fnv1a("ldr"):
        case fnv1a("load"): 
            return X2R_IR::Opcode::LOAD;

        case fnv1a("str"):
        case fnv1a("store"): 
            return X2R_IR::Opcode::STORE;

        case fnv1a("jmp"):  
            return X2R_IR::Opcode::JMP;

        case fnv1a("jz"):
        case fnv1a("jnz"):
        case fnv1a("je"):
        case fnv1a("jne"):  
            return X2R_IR::Opcode::CJMP;

        default:            
            return X2R_IR::Opcode::NOP;
    }
}

X2R_IR::IRProgram liftX86ToIR(const ElfLoad::ElfLoadResult& elf){

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

    /*enable detail mode */ 
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON); 

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
            const auto& ci = insn[x]; 

            /*map capstone mnemonic to IR opcode */ 
            std::string mnem = ci.mnemonic;

            irInst.opcode = decodeMnemonic(mnem); 

            /*handling operands 
             * capstonme stores operands in ci.details->x86.oprands array */ 

            /*destinatino operand (first operand) */ 
            if (ci.detail && ci.detail->x86.op_count >= 1) {

                const auto& op = ci.detail->x86.operands[0];

                switch (op.type) {

                    case X86_OP_REG:

                        irInst.dst = X2R_IR::Operand{
                            X2R_IR::OperandType::Register,
                            cs_reg_name(handle, op.reg),
                            0,
                            0
                        };
                        break;

                    case X86_OP_IMM:

                        irInst.dst = X2R_IR::Operand{
                            X2R_IR::OperandType::Immediate,
                            "",
                            op.imm,
                            0
                        };
                        break;

                    case X86_OP_MEM:

                        irInst.dst = X2R_IR::Operand{
                            X2R_IR::OperandType::Memory,
                            "",
                            0,
                            op.mem.disp
                        };
                        break;

                    default:

                        /* fallback (unsupported operand type) */ 
                        irInst.dst = X2R_IR::Operand{
                            X2R_IR::OperandType::None,
                            "",
                            0,
                            0
                        };

                        break;
                }
            }

            if(ci.detail->x86.op_count >= 2){

                const auto& op = ci.detail->x86.operands[1]; 

                switch(op.type){

                    case X86_OP_REG:

                        irInst.src = X2R_IR::Operand{
                            X2R_IR::OperandType::Register, 
                            cs_reg_name(handle, op.reg), 
                            0, 
                            0
                            }; 
                        break;

                    case X86_OP_IMM:

                        irInst.src = X2R_IR::Operand{
                            X2R_IR::OperandType::Immediate,
                            "",
                            op.imm, 
                            0
                        }; 
                        break; 

                    case X86_OP_MEM:
                    
                        irInst.src = X2R_IR::Operand{
                            X2R_IR::OperandType::Memory, 
                            "", 
                            0, 
                            op.mem.disp
                        }; 
                        break;  

                    default:
                        irInst.src = X2R_IR::Operand{
                            X2R_IR::OperandType::Memory, 
                            "",
                            0,
                            0
                        };  

                        break; 
                }
            }

            block.instructions.push_back(irInst); 
        }

        func.blocks.push_back(block); 
    }
    program.functions.push_back(func); 

    cs_free(insn, count); 
    cs_close(&handle); 

    return program; 
}

void PrintIRInstr(const IRInst& instr) {

    // --- opcode to string ---
    std::string opcode;
    switch (instr.opcode) {
        case Opcode::MOV:   opcode = "mov"; break;
        case Opcode::ADD:   opcode = "add"; break;
        case Opcode::SUB:   opcode = "sub"; break;
        case Opcode::MUL:   opcode = "mul"; break;
        case Opcode::DIV:   opcode = "div"; break;
        case Opcode::LOAD:  opcode = "ld";  break;
        case Opcode::STORE: opcode = "st";  break;
        case Opcode::JMP:   opcode = "jmp"; break;
        case Opcode::CJMP:  opcode = "cjmp"; break;
        case Opcode::NOP:   opcode = "nop"; break;
    }

    std::string dst;
    switch (instr.dst.type) {
        case OperandType::Register:
            dst = instr.dst.regName;
            break;
        case OperandType::Immediate:
            dst = std::to_string(instr.dst.imm);
            break;
        case OperandType::Memory:
            dst = "[" + std::to_string(instr.dst.memAddr) + "]";
            break;
    }

    std::string src;
    if (instr.src.has_value()) {
        const Operand& s = *instr.src;
        switch (s.type) {
            case OperandType::Register:
                src = s.regName;
                break;
            case OperandType::Immediate:
                src = std::to_string(s.imm);
                break;
            case OperandType::Memory:
                src = "[" + std::to_string(s.memAddr) + "]";
                break;
        }
    }

    if (!instr.label.empty()) {
        std::cout << instr.label << ":\n";
    }

    if (!src.empty())
        std::cout << opcode << " " << src << ", " << dst << "\n";
    else
        std::cout << opcode << " " << dst << "\n";
}

void PrintIRFunction(const IRFunction& fn){

    std::cout << "funtion " << fn.name << ":\n";

    for(const auto& block : fn.blocks){
        std::cout << "block " << block.name << "\n"; 

        for(const auto& instr : block.instructions){
            std::cout << "  "; 
            PrintIRInstr(instr); 
        }
        std::cout << "\n"; 
    }
}

void PrintIRProgram(const IRProgram& program) {

    std::cout << "program:\n";

    for (const auto& fn : program.functions) {
        std::cout << "\n";
        PrintIRFunction(fn);
    }
}
}
