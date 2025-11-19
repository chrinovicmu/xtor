#include "ELFLoader.hpp"
#include "disasm.hpp"

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <elf-file>\n";
        return 1;
    }

    std::filesystem::path elfPath = argv[1];

    auto elf = ElfLoad::loadElfFile(elfPath);
    if (!elf) {
        std::cerr << "Failed to load ELF\n";
        return 1;
    }

    std::cout << "ELF loaded successfully: " << elf->path << "\n";
    
    Disasm::dumpX86_64(*elf);
    
    return 0;
}
