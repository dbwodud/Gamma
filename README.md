"# Gamma" 

명령어

g++ \`llvm-config --cxxflags --ldflags --system-libs --libs core\` Gamma.cpp lexer.cpp  LLparser.cpp $CLANG_LIBS \`llvm-config --libs --system-libs\` -o GAMMA
