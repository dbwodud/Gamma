#include "Gamma.h"

std::map<int,int>BinopPrecedence;


int cnt = 1;

std::vector<std::string> sym_list;
std::vector<std::string>::iterator iter;

void parser_init() {
	lookahead = head;
    sym_list=Symbol_table.get_vector();
    iter = sym_list.begin();
    BinopPrecedence[T_equal]=1;
    BinopPrecedence[T_notequal]=1;
    BinopPrecedence[T_add]=2;
    BinopPrecedence[T_sub]=3;
    BinopPrecedence[T_mul]=4;
    BinopPrecedence[T_div]=5;
}

void getNextToken(){
    lookahead=lookahead->next;
    cnt++;
}

llvm::Function *getFunction(std::string Name){
    if(auto *F = TheModule->getFunction(Name))
        return F;

    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    // If no existing prototype exists, return null.
    return nullptr;
}

void match(int terminal) {
	if (lookahead->token_type == terminal) {
		lookahead = lookahead->next;
		cnt++;
	} 
	else {
		printf("count : %d\n", cnt);
		printf("Terminal : %d\n",terminal);
		printf("Token_type : %d\n",lookahead->token_type);
		printf("match Error");
		exit(1);
	}
}

llvm::Value *NumberExprAST::codegen(){
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheContext),Val);
}

llvm::Value *VariableExprAST::codegen(){
    llvm::Value *V = NamedValues[Name];
    if(!V)
        LogErrorV("Unknown variable name");
    return V;
}

llvm::Value *BinaryExprAST::codegen(){
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();

    if(!L || !R)
        return nullptr;
    switch(Op){
        case T_add:
            return Builder.CreateAdd(L,R,"addtmp");
        case T_sub:
            return Builder.CreateSub(L,R,"subtmp");
        case T_mul:
            return Builder.CreateMul(L,R,"multmp");
        case T_div:
            return Builder.CreateFDiv(L,R,"divtmp");
        case T_equal:
            L = Builder.CreateICmpEQ(L,R,"eqtmp");
            return Builder.CreateZExt(L,llvm::Type::getInt32Ty(TheContext),"booltmp");
        case T_notequal:
            L = Builder.CreateICmpNE(L,R,"neqtmp");
            return Builder.CreateZExt(L,llvm::Type::getInt32Ty(TheContext),"booltmp");
        default:
            return 0;
    }
}

llvm::Value *CallExprAST::codegen(){
    llvm::Function *CalleeF = getFunction(Callee);
    if(!CalleeF)
        return LogErrorV("Unknow Function referenced");

    if(CalleeF->arg_size()!=Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<llvm::Value *>ArgsV;
    for(unsigned i = 0,e=Args.size();i!=e;++i){
        ArgsV.push_back(Args[i]->codegen());
        if(!ArgsV.back())
            return nullptr;
    }
    return Builder.CreateCall(CalleeF,ArgsV,"calltmp");
}

llvm::Function *PrototypeAST::codegen(){
    std::vector<llvm::Type *>Integers(Args.size(),llvm::Type::getInt32Ty(TheContext));
    llvm::FunctionType *FT = 
        llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext),Integers,false);
    llvm::Function *F = 
        llvm::Function::Create(FT,llvm::Function::ExternalLinkage,Name,TheModule.get());

    unsigned Idx=0;
    for(auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

llvm::Function *FunctionAST::codegen(){
    auto &P = *Proto;
    FunctionProtos[Proto->getName()] = std::move(Proto);
    llvm::Function *TheFunction = getFunction(P.getName());
    if(!TheFunction)
        return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    for(auto &Arg : TheFunction->args())
        NamedValues[Arg.getName()] = &Arg;
    for(auto &Body : Bodys){
        if (Bodys.back()==Body) {
            Builder.CreateRet(Body->codegen());

            verifyFunction(*TheFunction);
            TheFPM->run(*TheFunction);
            return TheFunction;
        }else{
            Body->codegen();
        }
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

llvm::Value *IfExprAST::codegen(){
    llvm::Value *CondV = Cond->codegen();
    if(!CondV)
        return nullptr;
    CondV = Builder.CreateICmpNE(CondV, llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheContext),0), "ifcond");

    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
    Builder.SetInsertPoint(ThenBB);

    llvm::Value *ThenV = Then->codegen();
    if (!ThenV)
        return nullptr;

    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    llvm::Value *ElseV = Else->codegen();
    if (!ElseV)
        return nullptr;

    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(llvm::Type::getInt32Ty(TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

std::unique_ptr<ExprAST> LogError(const char *str){
    fprintf(stderr,"LogError : %s \n",str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *str){
    LogError(str);
    return nullptr;
}

llvm::Value *LogErrorV(const char *str){
    LogError(str);
    return nullptr;
}

std::unique_ptr<ExprAST>ParseNumberExpr(){
    std::string str=*iter;
    iter++;
    std::unique_ptr<ExprAST> result = llvm::make_unique<NumberExprAST>(atoi(str.c_str()));
    match(T_const);
    return std::move(result);
}

std::unique_ptr<ExprAST>ParseParenExpr(){
    match(T_lparen);
    std::unique_ptr<ExprAST> V = ParseExpression();
    if(!V)
        return nullptr;
    match(T_rparen);
    
    return V;
}

std::unique_ptr<ExprAST>ParseIdentifierExpr(){
    std::string IdName = *iter;
    iter++;
    match(T_variable);
    if(lookahead->token_type != T_lparen)
        return llvm::make_unique<VariableExprAST>(IdName);
    match(T_lparen);

    std::vector<std::unique_ptr<ExprAST>> Args;
    while(1){
        if(lookahead->token_type==T_rparen)
            break;
        
        if(auto Arg = ParseExpression())
            Args.push_back(std::move(Arg));
        else
            return nullptr;

        if(lookahead->token_type==T_peroid)
            match(T_peroid);
    }
    match(T_rparen);
    return llvm::make_unique<CallExprAST>(IdName,std::move(Args));
}

std::unique_ptr<ExprAST>ParseIfExpr(){
    match(T_if);
    auto Cond = ParseExpression();
    if(!Cond)
        return nullptr;
    match(T_colon);
    auto Then = ParseExpression();
    if(!Then)
        return nullptr;
    match(T_semicolon);
    match(T_else);
    match(T_colon);
    auto Else = ParseExpression();
    if(!Else)
        return nullptr;

    return llvm::make_unique<IfExprAST>(std::move(Cond),std::move(Then),std::move(Else));
}

std::unique_ptr<ExprAST> ParsePrimary(){
    switch(lookahead->token_type){
        default:
              return nullptr;
        case T_variable:
            return ParseIdentifierExpr();
        case T_const:
            return ParseNumberExpr();
        case T_lparen:
            return ParseParenExpr();
        case T_if:
            return ParseIfExpr();
        case T_semicolon:
            match(T_semicolon);
            break;
    }
    return nullptr;
}

int GetTokPrecedence(){
    int TokPrec = BinopPrecedence[lookahead->token_type];
    if(TokPrec<=0)return -1;
    return TokPrec;
}

std::unique_ptr<ExprAST>ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS){
    while(1){
        int TokPrec = GetTokPrecedence();
        if(TokPrec < ExprPrec)
            return LHS;
        int BinOp = lookahead->token_type;
        getNextToken();

        auto RHS = ParsePrimary();
        if(!RHS)
            return nullptr;

        int NextPrec = GetTokPrecedence();
        if(TokPrec<NextPrec){
            RHS = ParseBinOpRHS(TokPrec+1,std::move(RHS));
            if(!RHS)
                return nullptr;
        }

        LHS = llvm::make_unique<BinaryExprAST>(BinOp,std::move(LHS),std::move(RHS));
    }
}

std::unique_ptr<ExprAST>ParseExpression(){
    std::unique_ptr<ExprAST> LHS = ParsePrimary();
    if(!LHS)
        return nullptr;
    return ParseBinOpRHS(0,std::move(LHS));
}

std::unique_ptr<PrototypeAST>ParsePrototype(){
    match(T_def); match(T_variable);

    std::string FnName = *iter;
    iter++;

    match(T_lparen);

    std::vector<std::string> ArgNames;
    while(1){
        if(lookahead->token_type==T_rparen){
            match(T_rparen);
            break;
        }
        match(T_variable);
        ArgNames.push_back(*iter);
        iter++;
        if(lookahead->token_type==T_peroid){
            match(T_peroid);
        }
    }
    
    return llvm::make_unique<PrototypeAST>(FnName,std::move(ArgNames));
}

std::unique_ptr<FunctionAST>ParseDefinition(){
    std::unique_ptr<PrototypeAST> Proto = ParsePrototype();
    if(!Proto)
        return nullptr;
    if(lookahead->token_type==T_lbrace){
        match(T_lbrace);
        std::vector<std::unique_ptr<ExprAST>>bodys;
        while(1){
            if(auto E = ParseExpression()){
                bodys.push_back(std::move(E));
                match(T_semicolon);
            }else{
                break;
            }
        }
        match(T_rbrace);
        return llvm::make_unique<FunctionAST>(std::move(Proto),std::move(bodys));
    }
    return nullptr;
}

std::unique_ptr<FunctionAST>ParseTopLevelExpr(){
    std::unique_ptr<PrototypeAST> Proto = llvm::make_unique<PrototypeAST>("__anon_expr",std::vector<std::string>());
    std::vector<std::unique_ptr<ExprAST>>bodys;
    while(1){
        if(auto E = ParseExpression()){
            bodys.push_back(std::move(E));
        }else{
            break;
        }
    }
    if(bodys.size()==0)
        return nullptr;
    return llvm::make_unique<FunctionAST>(std::move(Proto),std::move(bodys));
}

void HandleDefinition(){
    if(auto FnAST = ParseDefinition()){
        if(auto *FnIR = FnAST->codegen()){
            fprintf(stderr, "Read function definition:");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
            TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();
        }
    }else{
        getNextToken();
    }
}

void HandleTopLevelExpression(){
    if(auto FnAST=ParseTopLevelExpr()){
        if(FnAST->codegen()){
            auto H = TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();
            auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "Function not found");
            int (*FP)() = (int (*)())(intptr_t)llvm::cantFail(ExprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %d\n", FP());
            TheJIT->removeModule(H); 
        }
    }else{
        getNextToken();
    }
}

void Driver(){
    parser_init();
    while(1){
        switch(lookahead->token_type){
        case T_eof:
            return;
        case T_def:
            HandleDefinition();
            break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}
