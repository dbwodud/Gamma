#Gamma

명령어

g++ \`llvm-config --cxxflags --ldflags --system-libs --libs core\` Gamma.cpp lexer.cpp  LLparser.cpp $CLANG_LIBS \`llvm-config --libs --system-libs\` -o GAMMA

팩토리 패턴 사용

사용 환경 : Linux 

사용 라이브러리 : LLVM
