
#include "../include/KaleidoscopeJIT.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"

// standard library ================================================================
#include<iostream>
#include<stdlib.h>
#include<ctype.h>
#include<string>
#include<map>
#include<vector>
#include<stack>
#include<list>
#include<memory>
#include<cassert>
#include<exception>
// Symboltable.cpp =================================================================
class symbol_table{
    std::vector<std::string>symbol_list;
    std::map<std::string,int>symbol_tree;
public:
    void insert(std::string str,int token_type){
        symbol_list.push_back(str);
        symbol_tree.insert(std::pair<std::string,int>(str,token_type));
    }
    int lookup(std::string str){
        if(symbol_tree[str]){
            return 1;
        }
        return 0;
    }
    std::vector<std::string> get_vector(){
        return symbol_list;
    }
};

extern symbol_table Symbol_table;    

// lexer.cpp =======================================================================

enum token_type {
    T_NULL = -1,T_void, T_int, T_char, T_return, T_if, T_else, T_for,T_def, // Keyword
    T_assign, T_add, T_sub, T_mul, T_div, T_mod, T_equal, T_notequal, T_cmpULT, T_cmpUGT,   // operation
    T_const, T_variable, // const,variable 17
    T_lbrace, T_rbrace, T_lbracket, T_rbracket, T_lparen, T_rparen, // { , } , [ , ] , ( , )
    T_peroid, // , 
    T_semicolon, // ;
    T_eof, // END of file
    T_colon
};

struct token {
    int token_type;
    struct token *next;
};

struct symbol {
    int datatype;
    char *idNname;
    struct symbol *next;
};

void initToken();
void insertToken(int value);
void printTokens();
void lexer(FILE *fp);
void isbraket(char ch);
void isop(char aheadch, FILE *fp);
void init_buffer();

extern std::string buffer;
extern struct token *head;
extern struct token *token_p;
extern struct token *tail;
extern struct token *lookahead;

void match(int terminal);
void parser_init();
void getNextToken();

// Ast ============================================================================

extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::map<std::string,llvm::Value *> NamedValues;
extern std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
llvm::Function *getFunction(std::string Name);
void InitializeModuleAndPassManager(void);

class ExprAST{
public:
    virtual ~ExprAST(){}
    virtual llvm::Value *codegen() = 0;
};

class NumberExprAST:public ExprAST{
    int Val;
public:
    NumberExprAST(int Val) : Val(Val){}
    llvm::Value *codegen() override;
};

class VariableExprAST:public ExprAST{
    std::string Name;
public:
    VariableExprAST(const std::string &Name) : Name(Name){}
    llvm::Value *codegen() override;
};

class BinaryExprAST : public ExprAST{
    int Op;
    std::unique_ptr<ExprAST> LHS,RHS;
    public:
    BinaryExprAST(int op,std::unique_ptr<ExprAST>LHS,std::unique_ptr<ExprAST>RHS)
        :Op(op),LHS(move(LHS)),RHS(move(RHS)){}
    llvm::Value *codegen() override;    
};

class CallExprAST : public ExprAST{
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
    public:
    CallExprAST(const std::string &Callee,std::vector<std::unique_ptr<ExprAST>>args)
        :Callee(Callee),Args(std::move(args)){}
   
    llvm::Value *codegen() override;
};

class PrototypeAST{
    std::string Name;
    std::vector<std::string> Args;
    public:
    PrototypeAST(const std::string &name
            ,std::vector<std::string>args):Name(name)
                                           ,Args(std::move(args)){}
    llvm::Function *codegen();
    const std::string &getName() const{return Name;}
};

class FunctionAST{
    std::unique_ptr<PrototypeAST>Proto;
    std::vector<std::unique_ptr<ExprAST>>Bodys;
    public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::vector<std::unique_ptr<ExprAST>>bodys)
    :Proto(std::move(proto)),Bodys(std::move(bodys)){}
    llvm::Function *codegen();
};

class IfExprAST : public ExprAST{
    std::unique_ptr<ExprAST> Cond,Then,Else;
  public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,std::unique_ptr<ExprAST> Else) : Cond(std::move(Cond)),Then(std::move(Then)),Else(std::move(Else)){}
    llvm::Value *codegen() override;
};

class ForExprAST : public ExprAST{
    std::string VarName;
    std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
  ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
             std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
             std::unique_ptr<ExprAST> Body)
    : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
      Step(std::move(Step)), Body(std::move(Body)) {}
    llvm::Value *codegen() override;
};

extern std::map<std::string,std::unique_ptr<PrototypeAST>> FunctionProtos;

llvm::Value *LogErrorV(const char *str);
std::unique_ptr<ExprAST> LogError(const char *str);
std::unique_ptr<ExprAST> ParseParenExpr();
std::unique_ptr<ExprAST> ParseIdentifierExpr();
std::unique_ptr<ExprAST> ParsePrimary();

int GetToPrecedence();

std::unique_ptr<ExprAST>ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS);
std::unique_ptr<ExprAST>ParseExpression();
std::unique_ptr<ExprAST>ParseIfExpr();
std::unique_ptr<PrototypeAST>ParsePrototype();
std::unique_ptr<FunctionAST>ParseDefinitition();
std::unique_ptr<FunctionAST>ParseTopLevelExpr();

void Driver();

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
