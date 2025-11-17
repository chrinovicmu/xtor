#include "disasm.hpp"
#include <capstone/capstone.h>
#include <cstdio>
#include <iostream>


namespace Disasm {

void dumppX86_64(const ElfLoad::ElfLoadResult & elf)
{
    if(!elf.hasText()){
        std::cerr << "no .text segement available\n"; 
        return; 
    }

    csh handle; 
    cs_err err = cs_open(CS_ARCH_X86, CS_MODE_64, &handle); 
    if(err != CS_ERR_OK){
        std::cerr << "capstone open failed: " << cs_strerror(err) << "\n"; 
        return; 
    }

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON); 
    
    cs_insn* insn = nullptr; 

    // disassemble the entire .text segment in one call:
    //  - handle: disassembler instance
    //  - elf.text.data(): pointer to raw machine code bytes
    //  - elf.text.size(): total number of bytes to decode
    //  - elf.textAddr: virtual address where these bytes are mapped
    //  - 0: decode until end of buffer (no limit on instruction count)
    //  - &insn: output pointer where Capstone writes the decoded array

    size_t count = cs_disasm(
        csh handle, 
        elf.text.data(), 
        elf.text.size(), 
        elf.textAddr, 
        0, 
        &insn
    ); 

    if(count == 0)
    {
        std::cerr << "capstone failed to disassemble\n"; 
        cs_close(&handle); 
        return; 
    }

    for (size_t i = 0; i < count; i++) {
        // Print instruction address, mnemonic, and operand string.
        // Example output:
        //     0x401000: mov     rax, rbx
        std::printf(
            "0x%llx: %-8s %s\n",
            (unsigned long long)insn[i].address,   // virtual address of instruction
            insn[i].mnemonic,                      // instruction mnemonic
            insn[i].op_str                         // formatted operand string
        );
    }

    cs_free(insn, count);
    cs_close(&handle);
}

}
