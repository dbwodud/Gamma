#include "Gamma.h"

llvm::Value *NumExprAST::codegen() {
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(TheContext),Val);
}

llvm::Value *VarExprAST::codegen() {
	llvm::Value *v = NamedValues[Name];
	return v ? v : 0;
}

llvm::Value *BinaryExprAST::codegen() {
	llvm::Value *L = LHS->codegen();
	llvm::Value *R = RHS->codegen();
	if (L == 0 || R == 0)
		return 0;
	switch (Op) {
	case T_add: return Builder.CreateAdd(L, R, "addtmp");
	case T_sub: return Builder.CreateSub(L, R, "subtmp");
	case T_mul: return Builder.CreateMul(L, R, "multmp");
	case T_div: return Builder.CreateUDiv(L, R, "divtmp");
	default: return 0;
	}
}

llvm::Value *CallExprAST::codegen() {
	llvm::Function *CalleeF = TheModule->getFunction(Callee);
	if (!CalleeF) {

	}
	std::vector<llvm::Value *>ArgsV;
	for (unsigned i = 0, e = Args.size(); i != e; ++i) {
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back())
			return nullptr;
	}
	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// Int function
llvm::Function *PrototypeAST::codegen() {

	std::vector<llvm::Type *>Int(Args.size(), llvm::Type::getInt32Ty(TheContext));
	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), Int, false);
	llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

	unsigned Idx = 0;
	for (auto &Arg : F->args())
		Arg.setName(Args[Idx++]);

	return F;
}

llvm::Function *FunctionAST::codegen() {

	llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

	if (!TheFunction)
		TheFunction = Proto->codegen();
	if (!TheFunction)
		return nullptr;
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
	Builder.SetInsertPoint(BB);
	
	NamedValues.clear();
	for (auto &Arg : TheFunction->args())
		NamedValues[Arg.getName()] = &Arg;
	if (llvm::Value *RetVal = Body->codegen()) {
		// Finish off the function.
		Builder.CreateRet(RetVal);
		// Validate the generated code, checking for consistency.
		llvm::verifyFunction(*TheFunction);

		return TheFunction;
	}
	//Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}
