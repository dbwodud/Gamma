#include "Gamma.h"
symbol_table Symbol_table;

int main(int argc,char *argv[]) {
    std::cout << argv[1] << std::endl;
    if (argc == 1) {
        std::cout << "No input File" << std::endl;
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");

    
    lexer(fp);
    printTokens();


    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    TheModule=llvm::make_unique<llvm::Module>("My Cool jit",TheContext);
    Driver();

    return 0;
}
