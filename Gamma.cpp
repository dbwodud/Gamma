#include "Gamma.h"


/// putchard - putchar that takes a double and returns 0.
extern "C" int putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" int printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

symbol_table Symbol_table;

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
std::unique_ptr<llvm::Module> TheModule;
std::map<std::string,llvm::AllocaInst *> NamedValues;
std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
std::map<std::string,std::unique_ptr<PrototypeAST>> FunctionProtos;

void InitializeModuleAndPassManager(){

    TheModule = llvm::make_unique<llvm::Module>("my cool jit", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

    TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());
    TheFPM->add(llvm::createInstructionCombiningPass());
    TheFPM->add(llvm::createReassociatePass());
    TheFPM->add(llvm::createGVNPass());
    TheFPM->add(llvm::createCFGSimplificationPass());
    TheFPM->add(llvm::createPromoteMemoryToRegisterPass());
    TheFPM->add(llvm::createInstructionCombiningPass());
    TheFPM->add(llvm::createReassociatePass());
    TheFPM->doInitialization();
}

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
    TheJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
    InitializeModuleAndPassManager();
    
    Driver();
    return 0;
}
