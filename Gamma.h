
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
// KaleidoscopeJIT.h =============================================================
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
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
    T_NULL = -1,T_void, T_int, T_char, T_return, T_if, T_else, T_while,T_def, // Keyword
    T_assign, T_add, T_sub, T_mul, T_div, T_mod, T_equal, T_notequal,   // operation
    T_const, T_variable, // const,variable
    T_lbrace, T_rbrace, T_lbracket, T_rbracket, T_lparen, T_rparen, // { , } , [ , ] , ( , )
    T_peroid, // , 
    T_semicolon, // ;
    T_eof, // END of file
    T_stmt,T_expr,T_term,T_form
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
//extern char buffer[32];
extern struct token *head;
extern struct token *token_p;
extern struct token *tail;
extern struct token *lookahead;

// LLparser.cpp =====================================================================
/* NODE Tree 
class node {
int token_type;
    std::vector<node*> pointers;
    public:
        int getToken() {
            return token_type;
        }
        void setToken(int Token) {
            token_type = Token;
        }
        void setunToken(int Token) {
            node *ptr = new node;
            ptr->setToken(Token);
            this->pointers.push_back(ptr);
        }
        node *last_at() {
            return pointers.back();
        }
};
extern std::stack<node *> _stack;
extern std::vector<node *> tree_vector;
extern node *root;
extern node *present_ptr;
*/
void match(int terminal);
void parser_init();
void getNextToken();

// Ast ============================================================================

extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::map<std::string,llvm::Value *> NamedValues;

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
    CallExprAST(const std::string &Callee,std::vector<std::unique_ptr<ExprAST>>Args)
        :Callee(Callee),Args(std::move(Args)){}
   
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
    std::unique_ptr<ExprAST>Body;
    public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
    :Proto(std::move(proto)),Body(std::move(body)){}
    llvm::Function *codegen();
};

llvm::Value *LogErrorV(const char *str);
std::unique_ptr<ExprAST> LogError(const char *str);
std::unique_ptr<ExprAST> ParseParenExpr();
std::unique_ptr<ExprAST> ParseIdentifierExpr();
std::unique_ptr<ExprAST> ParsePrimary();

int GetToPrecedence();

std::unique_ptr<ExprAST>ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS);
std::unique_ptr<ExprAST>ParseExpression();
std::unique_ptr<PrototypeAST>ParsePrototype();
std::unique_ptr<FunctionAST>ParseDefinitition();
std::unique_ptr<FunctionAST>ParseTopLevelExpr();

void Driver();

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
