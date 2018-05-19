#include "Gamma.h"

std::map<int,int>BinopPrecedence;

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
std::unique_ptr<llvm::Module> TheModule;
std::map<std::string,llvm::Value *> NamedValues;

int cnt = 1;

std::vector<std::string> sym_list;
std::vector<std::string>::iterator iter;

void parser_init() {
	lookahead = head;
    sym_list=Symbol_table.get_vector();
    iter = sym_list.begin();
    BinopPrecedence[T_add]=10;
    BinopPrecedence[T_sub]=20;
    BinopPrecedence[T_mul]=30;
    BinopPrecedence[T_div]=40;
}

void getNextToken(){
    lookahead=lookahead->next;
    cnt++;
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
            return Builder.CreateUDiv(L,R,"divtmp");
        default:
            return 0;
    }
}

llvm::Value *CallExprAST::codegen(){
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
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
    llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

    if(!TheFunction)
        TheFunction=Proto->codegen();
    if(!TheFunction)
        return nullptr;
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

//Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[Arg.getName()] = &Arg;
   
    if (llvm::Value *RetVal = Body->codegen()) {
                // Finish off the function.
        Builder.CreateRet(RetVal);

                       // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
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
    if(lookahead->token_type != T_rparen){
        while(1){
            if(auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if(lookahead->token_type==T_rparen)
                break;

            match(T_peroid);
        }
    }

    match(T_rparen);

    return llvm::make_unique<CallExprAST>(IdName,std::move(Args));
}

std::unique_ptr<ExprAST> ParsePrimary(){
    switch(lookahead->token_type){
        default:
            return LogError("unknown token when expecting an expression");
        case T_variable:
            return ParseIdentifierExpr();
        case T_const:
            return ParseNumberExpr();
        case T_lparen:
            return ParseParenExpr();
    }
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
        match(T_variable);
        ArgNames.push_back(*iter);
        iter++;
        if(lookahead->token_type==T_peroid){
            match(T_peroid);
        }else{
            break;
        }
    }

    match(T_rparen);
    
    return llvm::make_unique<PrototypeAST>(FnName,std::move(ArgNames));
}

std::unique_ptr<FunctionAST>ParseDefinition(){
    std::unique_ptr<PrototypeAST> Proto = ParsePrototype();
    if(!Proto)
        return nullptr;
    if(lookahead->token_type==T_lbrace){
        match(T_lbrace);
        auto E = ParseExpression(); match(T_semicolon);
        match(T_rbrace);
        return llvm::make_unique<FunctionAST>(std::move(Proto),std::move(E));
    }
    return nullptr;
}

std::unique_ptr<FunctionAST>ParseTopLevelExpr(){
    if(auto E = ParseExpression()){
        std::unique_ptr<PrototypeAST> Proto = llvm::make_unique<PrototypeAST>("__annon_expr",std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(move(Proto),std::move(E));
    }
    return nullptr;
}

void HandleDefinition(){
    if(auto FnAST = ParseDefinition()){
        if(auto *FnIR = FnAST->codegen()){
            std::cout<<"Read function definition."<<std::endl;
            FnIR->print(llvm::errs());
            fprintf(stderr,"\n");
        }
    }else{
        getNextToken();
    }
}

void HandleTopLevelExpression(){
    if(auto FnAST=ParseTopLevelExpr()){
        if(auto *FnIR = FnAST->codegen()){
            fprintf(stderr,"Read top-level expression:");
            FnIR->print(llvm::errs());
            fprintf(stderr,"\n");
            std::cout<<"Parsed a top-level expr"<<std::endl;
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
