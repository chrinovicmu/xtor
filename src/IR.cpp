#include "IR.hpp"

namespace XTOR_IR{

IRInst IRInst::makeCast(Opcode, castOp, VReg dst, IRType dstTy, IRValue src){
    IRInst inst;
    inst.m_opcode     = castOp;     //  TRUNC, ZEXT, SEXT, BITCAST...
    inst.m_dst        = dst;
    inst.m_resultType = dstTy;      // the type AFTER the cast 
    inst.m_operands.push_back(std::move(src));  // operands[0] = value BEFORE the cast
    return inst;
}

IRInst IRInst::makeMov(VReg dst, IRType ty, IRValue src) {
    IRInst inst;
    inst.m_opcode     = Opcode::MOV;
    inst.m_dst        = dst;
    inst.m_resultType = ty;
    inst.m_operands.push_back(std::move(src));
    return inst;
}

IRInst IRInst::makeBinop(Opcode op, VReg dst, IRType ty, IRValue lhs, IRValue rhs) {
    IRInst inst;
    inst.m_opcode     = op;
    inst.m_dst        = dst;
    inst.m_resultType = ty;
    inst.m_operands.push_back(std::move(lhs));  // operands[0]
    inst.m_operands.push_back(std::move(rhs));  // operands[1]
    return inst;
}

IRInst IRInst::makeIcmp(IcmpCond cond, VReg dst, IRValue lhs, IRValue rhs) {
    IRInst inst;
    inst.m_opcode     = Opcode::ICMP;
    inst.m_dst        = dst;
    inst.m_resultType = IRType::i1();   // comparisons always produce i1
    inst.m_icmpCond   = cond;
    inst.m_operands.push_back(std::move(lhs));
    inst.m_operands.push_back(std::move(rhs));
    return inst;
}

IRInst IRInst::makeFcmp(FcmpCond cond, VReg dst, IRValue lhs, IRValue rhs) {
    IRInst inst;
    inst.m_opcode     = Opcode::FCMP;
    inst.m_dst        = dst;
    inst.m_resultType = IRType::i1();
    inst.m_fcmpCond   = cond;
    inst.m_operands.push_back(std::move(lhs));
    inst.m_operands.push_back(std::move(rhs));
    return inst;
}

IRInst IRInst::makeLoad(VReg dst, IRType loadTy, IRValue ptr, MemFlags flags) {
    IRInst inst;
    inst.m_opcode     = Opcode::LOAD;
    inst.m_dst        = dst;
    inst.m_resultType = loadTy;
    inst.m_memFlags   = flags;
    inst.m_operands.push_back(std::move(ptr));   // operands[0] = address
    return inst;
}

IRInst IRInst::makeStore(IRValue val, IRValue ptr, MemFlags flags) {
    IRInst inst;
    inst.m_opcode     = Opcode::STORE;
    inst.m_resultType = IRType::void_();   // STORE produces no value
    inst.m_memFlags   = flags;
    // dst is left empty (std::nullopt) — STORE has no result VReg.
    inst.m_operands.push_back(std::move(val));   // operands[0] = value to store
    inst.m_operands.push_back(std::move(ptr));   // operands[1] = address
    return inst;
}

IRInst IRInst::makeAlloca(VReg dst, uint64_t bytes, uint32_t alignment) {
    IRInst inst;
    inst.m_opcode      = Opcode::ALLOCA;
    inst.m_dst         = dst;
    inst.m_resultType  = IRType::ptr();   // alloca always produces a pointer
    inst.m_allocaBytes = bytes;
    inst.m_memFlags    = MemFlags(alignment);
    return inst;
}

IRInst IRInst::makeJmp(std::string targetBlock) {
    IRInst inst;
    inst.m_opcode      = Opcode::JMP;
    inst.m_resultType  = IRType::void_();
    inst.m_branchTrue  = std::move(targetBlock);
    return inst;
}

IRInst IRInst::makeCjmp(IRValue cond, std::string trueBlock, std::string falseBlock) {
    IRInst inst;
    inst.m_opcode      = Opcode::CJMP;
    inst.m_resultType  = IRType::void_();
    inst.m_branchTrue  = std::move(trueBlock);
    inst.m_branchFalse = std::move(falseBlock);
    inst.m_operands.push_back(std::move(cond));   // operands[0] = i1 condition
    return inst;
}

IRInst IRInst::makeRet(std::optional<IRValue> retVal) {
    IRInst inst;
    inst.m_opcode     = Opcode::RET;
    inst.m_resultType = IRType::void_();
    if (retVal.has_value())
        inst.m_operands.push_back(std::move(*retVal));   // operands[0] = return value
    return inst;
}

IRInst IRInst::makeCall(std::optional<VReg> dst, IRType retTy, IRValue callee,
                        std::vector<IRValue> args, CallingConv cc, bool tailCall) {
    IRInst inst;
    inst.m_opcode     = Opcode::CALL;
    inst.m_dst        = dst;          // empty for void-returning calls
    inst.m_resultType = retTy;
    inst.m_callConv   = cc;
    inst.m_isTailCall = tailCall;
    inst.m_operands.push_back(std::move(callee));   // operands[0] = callee
    for (auto& a : args)
        inst.m_operands.push_back(std::move(a));     // operands[1..N] = arguments
    return inst;
}

IRInst IRInst::makeSelect(VReg dst, IRType ty, IRValue cond,
                          IRValue trueVal, IRValue falseVal) {
    IRInst inst;
    inst.m_opcode     = Opcode::SELECT;
    inst.m_dst        = dst;
    inst.m_resultType = ty;
    inst.m_operands.push_back(std::move(cond));     // operands[0]
    inst.m_operands.push_back(std::move(trueVal));  // operands[1]
    inst.m_operands.push_back(std::move(falseVal)); // operands[2]
    return inst;
}

IRInst IRInst::makeNop() {
    IRInst inst;
    inst.m_opcode     = Opcode::NOP;
    inst.m_resultType = IRType::void_();
    return inst;
}

//  IRInst observer methods


const VReg& IRInst::dst() const {
    if (!m_dst.has_value())
        throw std::logic_error("IRInst::dst() called on instruction with no result");
    return *m_dst;
}

const IRValue& IRInst::operand(size_t i) const {
    if (i >= m_operands.size())
        throw std::out_of_range("IRInst::operand() index out of range");
    return m_operands[i];
}

const IcmpCond& IRInst::icmpCond() const {
    if (!m_icmpCond.has_value())
        throw std::logic_error("IRInst::icmpCond() called on non-ICMP instruction");
    return *m_icmpCond;
}

const FcmpCond& IRInst::fcmpCond() const {
    if (!m_fcmpCond.has_value())
        throw std::logic_error("IRInst::fcmpCond() called on non-FCMP instruction");
    return *m_fcmpCond;
}

const std::string& IRInst::branchTrue() const {
    if (m_branchTrue.empty())
        throw std::logic_error("IRInst::branchTrue() called on non-branch instruction");
    return m_branchTrue;
}

const std::string& IRInst::branchFalse() const {
    if (m_branchFalse.empty())
        throw std::logic_error("IRInst::branchFalse() called on non-CJMP instruction");
    return m_branchFalse;
}

CallingConv IRInst::callConv() const {
    if (!m_callConv.has_value())
        throw std::logic_error("IRInst::callConv() called on non-CALL instruction");
    return *m_callConv;
}

bool IRInst::isTailCall() const { return m_isTailCall; }

const MemFlags& IRInst::memFlags() const {
    if (!m_memFlags.has_value())
        throw std::logic_error("IRInst::memFlags() called on non-memory instruction");
    return *m_memFlags;
}

uint64_t IRInst::allocaBytes() const {
    if (!m_allocaBytes.has_value())
        throw std::logic_error("IRInst::allocaBytes() called on non-ALLOCA instruction");
    return *m_allocaBytes;
}

}
