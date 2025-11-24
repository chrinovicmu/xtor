#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include "ELFLoader.hpp"

namespace X2R_IR {

enum class OperandType {
    Register, 
    Immediate, 
    Memory,
    None
}; 

struct Operand {
    OperandType type; 
    std::string regName;
    int64_t imm; 
    int64_t memAddr; 
}; 

enum class Opcode{
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    LOAD,
    STORE,
    JMP,
    CJMP, /*conditional jump */ 
    NOP 
}; 

struct IRInst{
    Opcode opcode;
    Operand dst; 
    std::optional<Operand> src;
    std::string label; 
}; 

/*basic Block */ 
struct IRBasicBlock{
    std::string name;
    std::vector<IRInst> instructions; 
}; 

struct IRFunction{
    std::string name; 
    std::vector<IRBasicBlock> blocks; 
}; 

struct IRProgram{
    std::vector<IRFunction> functions; 
}; 
void PrintIRInstr(const IRInst& instr);
void PrintIRFunction(const IRFunction& fn);
void PrintIRProgram(const IRProgram& program);

IRProgram liftX86ToIR(const ElfLoad::ElfLoadResult& elf); 

}//namespace X2R_IR 
