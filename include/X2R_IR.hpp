#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include "ELFLoader.hpp"

namespace XTOR_IR{

class IRType; 
class IRValue; 
class IRInst; 
class IRBasicBlock;
class IRFunction; 
class IRProgram; 


enum class IRTypeKind{
    Void, 
    I1, I8, I16, I64, 
    F32, F64, 
    Ptr, 
}; 

enum class CallingConv{
    C, 
    Win64, 
    Fast, 
    Naked, 
}; 

enum class IcmpCond{
    EQ, NE, 
    SLT, SLE, SGT, SGE, //signed 
    ULT, ULE, UGT, UGE, //unsigned 
}; 

enum class FcmpCond {
    OEQ, ONE, OLT, OLE, OGT, OGE,  // ordered (no NaN)
    UNO, ORD,                        // unordered / ordered predicate
};

enum class Opcode{
    //Interger arithmetic
    ADD, SUB, MUL, SDIV, UDIV, SREM ,UREM, NEG, 
    //Bitwise / shift 
    AND, OR, NOT, SHL, LSHR, ASHR, 
    //Float arithmetic 
    FADD, FSUB, FMUL, FDIV, FNEG, 
    //Type conversion 
    ZEXT, SEXT, TRUNC
    FPEXT, FPTRUNC, FPTOSI, FPTOUI, SITOFP, UITOFP, 
    INTTOPTR, PTRTOINT, BITCAST, 
    //Memory 
    LOAD, STORE, ALLOCA, 
    //Comaparision , produces an Ii) 
    ICMP, FCMP, 
    //Branchless conditional move
    SELECT,
    //Control flow 
    JMP, CJMP, RET, CALL, PHI, 
    NOP, 
}; 

class IRType{

public:

    //static factories, the only way to contruct an IRType

    static IRType i1() { return IRType(IRTypeKind::I1);}
    static IRType i8() { return IRType(IRTypeKind::I8); }
    static IRType i16() { return IRType(IRTypeKind::I16);}
    static IRType i32() { return IRType(IRTypeKind::I32);}
    static IRType i64() { return IRType(IRTypeKind::I64);}
    static IRType f32() { return IRType(IRTypeKind::F32);}
    static IRType f64() { return IRType(IRTypeKind::F64);}
    static IRType ptr() { return IRType(IRTypeKind::Ptr);}
    static IRType void_() { return IRType(IRTypeKind::Void);}

    IRTypeKind kind() const {return m_kind; }


    //Return byte width fir RISC-V load/store width selction 
    uint32_t byteWidth() const{
        switch (m_kind){
            case IRTypeKind::I1:
            case IRTypeKind::I8:    return 1;
            case IRTypeKind::I16:   return 2; 
            case IRTypeKind::I32:
            case IRTypeKind::F32:   return 4; 
            case IRTypeKind::I64:
            case IRTypeKind::F64:
            case IRTypeKind::Ptr:   return 8; 
            case IRTypeKind::Void:
                throw std::logic_error("IRType::byteWidth called on void"); 
        }
        __builtin_unreachable(); 
    }

    bool isInteger()  const { return m_kind >= IRTypeKind::I1  && m_kind <= IRTypeKind::I64; }
    bool isFloat()    const { return m_kind == IRTypeKind::F32 || m_kind == IRTypeKind::F64; }
    bool isPointer()  const { return m_kind == IRTypeKind::Ptr;  }
    bool isVoid()     const { return m_kind == IRTypeKind::Void; }

    bool operator==(const IRType& o) const { return m_kind == o.m_kind; }
    bool operator!=(const IRType& o) const { return !(*this == o); }

    std::string toString() const;
private: 

    explicit IRType(IRTypeKind kind) : m_kind(kind) {}

    IRTypeKind m_kind; 
}; 

//IR virtual register 
class VReg{
public:
    VReg(uint32_t id, std::string debugName = " ")
        : m_id(id), m_debugName(std::move(debugName)) {}

    uint32_t id() const { return m_id;}
    const std::string debugName() const { return m_debugName;}

    bool operator==(const VReg& o) const { return m_id == o.m_id; }
    bool operator!=(const VReg& o) const { return !(*this == o); }

    std::string toString() const {
        return m_debugName.empty()
            ? "%vreg" + std::to_string(m_id)
            : "%" + m_debugName + "." + std::to_string(m_id);
    }

private:
    uint32_t m_id; 
    std::string m_debugName; 
}; 

class Immediate{
public:
    Immediate(int64_t value, IRType type)
        : m_value(value), m_type(type) {}

    int64_t value() const { return m_value; }
    const IRType& type() const {return m_type; }

    std::string toString() const{
        return std::to_string(m_value); 
    }
private:
    int64_t m_value; 
    IRType m_type; 
}; 

//A reference to a symbol from the ELF symbol table
//almost always Ptr(value is an address)
class GlobalRef{
public:
    GlobalRef(std::string name, IRType type)
        : m_name(nstd::move(name)), m_type(type) {}

    const std::string& name() const { return m_name; }
    const IRType& type() const { return m_type; }

    std:string toString() const { return "@" + name; }

private:
    std::string m_name; 
    IRType m_type; 
}; 

class IRValue{

public: 

    static IRValue makeVReg(uint32_t id, IRType ty, std::string name = ""){
        return IRValue(VReg(id, std::move(name)), ty); 
    }

    static IRValue makeImm(int64_t v, IRType ty){
        return IRValue(Immediate(v, ty), ty); 
    }

    static IRValue makeGlobal(std::string name, IRType ty){
        return IRValue(GlobalRef(std::move(name), ty), ty); 
    }

    const IRType& type() const { return m_type; }

    bool isVReg() const { return std::holds_alternative<VReg>(m_payload); }
    bool isImm() const { return std::holds_alternative<Immediate>(m_payload);}
    bool isGlobal() const { return std::holds_alternative<GlobalRef>(m_payload);}

    const VReg& asVReg() const { return std::get<VReg>(m_payload); }
    const Immediate& asImm() const { return std::get<Immediate>(m_payload); }
    const GlobalRef& asGlobal() const { return std::get<GlobalRef>(m_payload); }

    std::string toString() const;
private:

    using Payload = std::variant<VReg, Immediate, GlobalRef>; 

    IRValue(Payload payload, IRType type)
        : m_payload(std::move(payload), m_type(type)) {}

    Payload m_payload; 
    IRType m_type; 
}; 

//Descibes alignment and volatility of a memory access. 
class MemFlags{
public:
    explicit MemFlags(uint32_t alignment = 1, bool isVolatile = false)
        : m_aligment(alignment), m_isVolatile(isVolatile){
        //alignment must be power of 2 
        if(alignment == 0 || (alignment & (alignment -1)) != 0)
            throw std::invalid_argument("MemFlags: alignment must be a power of two"); 
    }

    uint32_t alignment() const { return m_aligment; }
    bool isVolatile() const { return m_isVolatile; }

private:
    uint32_t m_aligment; 
    bool m_isVolatile; 
}; 

class IRInst{

public:

    //Factories (one per opcode family) 


    //Binary integer/float arithmetic: dst = lhs op rhs 
    static IRInst makeBinop(Opcode op, VReg dst, IRType ty, 
                            IRValue lhs, IRValue rhs); 

    //Unary: dst = op src (NEG, NOT, FNEG)
    static IRInst makeUnop(Opcode op, VReg dst, IRType ty, 
                           IRValue src); 

    //Integer comparision : results an I1 
    static IRInst makeIcomp(IcmpCond cond, VReg dst, 
                            IRValue lhs, IRValue rhs); 

    //Float comparision
    static IRInst makeFcmp(FcmpCond cond, VReg dst, 
                           IRValue lhs, IRValue rhs); 

    //Type cast (ZEXT, SEXT, TRUNC, FPEXT... BITCAST)
    static IRInst makeCast(Opcode castOp, VReg dst, 
                           IRType dstTy, IRValue src); 
    //dst = ptr 
    static IRInst makeLoad(VReg dst, IRType loadTy, 
                           IRValue ptr, MemFlags flags = MemFlags{}); 

    // *ptr = val 
    static IRInst makeStore(IRValue val, IRValue ptr, 
                            MemFlags flags = MemFlags{}); 

    //dst = alloca <bytes> - allocates on the stack frame 
    static IRInst makeAlloca(VReg dst, uint64_t bytes, uint32_t alignment = 8); 

    //unconditional branch 
    static IRInst makeJmp(std::string TargetBlock); 

    //Conditional branch 
    static IRInst makeCjmp(IRValue cond, 
                           std::string trueBlock, 
                           std::string falseBlock); 

    //return (with optional value)
    static IRInst makeRet(std::optional<IRValue> retVal = std::nullopt); 

    //function call 
    static IRInst makeCall(std::optional<VReg> dst, ,IRType retTy, 
                           IRValue callee, 
                           std::vector<IRValue> args, 
                           CallingConv cc = CallingConv::C, 
                           bool tailCall = false); 

    //branchless select: dst = cond ? trueVal : falseVal
    static IRInst makeSelect(VReg dst, IRType ty, 
                             IRValue cond, 
                             IRValue trueVal, IRValue falseVal); 

    static IRInst makeNop(); 

    Opcode opcode() const { return m_opcode; }
    const IRType& resultType() const { return m_resultType; }
    uint64_t sourceAddr() const { return ,_sourceAddr; }
    bool hasResult() const { return m_dst.has_value(); }


}
}
