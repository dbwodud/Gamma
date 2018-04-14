// Gamma.cpp : 응용 프로그램의 진입점을 정의합니다.
//
#include "Gamma.h"
// JIT File ============================
//#include "llvm/Support/TargetSelect.h"
//#include "llvm/Target/TargetMachine.h"
//#include "KaleidoscopeJIT.h"
//======================================
//static std::unique_ptr<KaleidoscopeJIT> TheJIT;

int main(int argc,char *argv[]) {
	std::cout << argv[1] << std::endl;
	if (argc == 1) {
		std::cout << "No input File" << std::endl;
		exit(1);
	}
	FILE *fp = fopen(argv[1], "r");
	lexer(fp);
	fclose(fp);
	printTokens();
	LLparser();

	//llvm::InitializeNativeTarget();
	//InitializeNativeTargetAsmPrinter();
	//InitializeNativeTargetAsmParser();
	//TheJIT = std::make_unique<KaleidoscopeJIT>();
	
	return 0;
}
