
#include "ELFLoader.hpp"
#include "disasm.hpp"

int main(int argc, char** argv)
{
    auto elf = ElfLoad::loadElfFile(argv[1]);
    if (!elf) {
        std::cerr << "failed to load ELF\n";
        return 1;
    }

    Disasm::dumpX86_64(*elf);
}
