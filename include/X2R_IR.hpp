#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>


namespace X2R_IR {

enum class OperandType {
    Register, 
    Immediate, 
    Memory
}; 

struct Operand {
    OperandType type; 
    std::string regName;
    int64_t imm; 
    uint64_t memAddr; 
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

}
