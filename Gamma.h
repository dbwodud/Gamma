
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
// standard library ==============================================================

#include<iostream>
#include<stdlib.h>
#include<ctype.h>
#include<string>
#include<map>
#include<vector>
#include<stack>
#include<list>
#include<memory>

// lexer.c =======================================================================

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

extern char buffer[32];
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
void T();
void F();
void E();
void LLparser();
void parser_init();
void S();
int DT();

// LRparser.cpp =======================================================================
/*
   class LRparser {
    std::stack<int> parser_stack;
    public:
        int lookahead;
            LRparser() {
                    lookahead = head->token_type;
                            token_p = head;
                                }
                                    void match(int n) {
                                            if (n == parser_stack.top()) {
                                                        parser_stack.pop();
                                                                }
                                                                        else {
                                                                                    printf("LRparser match Error");
                                                                                                exit(1);
                                                                                                        }
                                                                                                            }
                                                                                                                void shift() {
                                                                                                                        if (lookahead == T_eof)
                                                                                                                                    return;
                                                                                                                                            if (token_p->next) {
                                                                                                                                                        parser_stack.push(token_p->token_type);
                                                                                                                                                                    token_p = token_p->next;
                                                                                                                                                                                printf("%d\n", token_p->token_type);
                                                                                                                                                                                            lookahead = token_p->next->token_type;
                                                                                                                                                                                                    }
                                                                                                                                                                                                            else {
                                                                                                                                                                                                                        printf("parser Error");
                                                                                                                                                                                                                                    exit(1);
                                                                                                                                                                                                                                            }
                                                                                                                                                                                                                                                }
                                                                                                                                                                                                                                                    void reduce() {
                                                                                                                                                                                                                                                            switch (parser_stack.top()) {
                                                                                                                                                                                                                                                                    case T_stmt:
                                                                                                                                                                                                                                                                                parser_stack.pop();
                                                                                                                                                                                                                                                                                            switch (lookahead) {
                                                                                                                                                                                                                                                                                                        case T_int:
                                                                                                                                                                                                                                                                                                                        parser_stack.push(T_variable); parser_stack.push(T_int);
                                                                                                                                                                                                                                                                                                                                        break;
                                                                                                                                                                                                                                                                                                                                                    case T_char:
                                                                                                                                                                                                                                                                                                                                                                    parser_stack.push(T_variable); parser_stack.push(T_char);
                                                                                                                                                                                                                                                                                                                                                                                    break;
                                                                                                                                                                                                                                                                                                                                                                                                case T_variable:
                                                                                                                                                                                                                                                                                                                                                                                                                parser_stack.push(T_variable);
                                                                                                                                                                                                                                                                                                                                                                                                                                break;
                                                                                                                                                                                                                                                                                                                                                                                                                                            case T_if:
                                                                                                                                                                                                                                                                                                                                                                                                                                                            parser_stack.push(T_rparen); parser_stack.push(T_expr); parser_stack.push(T_lparen); parser_stack.push(T_if);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                            break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        case T_while:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        parser_stack.push(T_rparen); parser_stack.push(T_expr); parser_stack.push(T_lparen); parser_stack.push(T_while);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    default:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    printf("LRparser Reduce Error");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    case T_expr:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                parser_stack.pop();
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            switch (lookahead) {
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        case T_const:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    case T_variable:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    parser_stack.push(T_term);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                default:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                printf("LRparser Reduce Error");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                case T_term:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            parser_stack.pop();
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        switch (lookahead) {
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    case T_const:
            case T_variable:
                            parser_stack.push(T_form);
                                            break;
                                                        default:
                                                                        printf("LRparser Reduce Error");
                                                                                    }
            break;
                    case T_form:
                                parser_stack.pop();
                                            switch (lookahead) {
                                                            case T_variable:
                                                                                parser_stack.push(T_variable);
                                                                                                break;
                                                                                                            case T_const:
                                                                                                                parser_stack.push(T_const);
                                                                                                                                break;
                                                                                                                                            default:
                                                                                                                                                printf("LRparser Reduce Error");
                                                                                                                                                            }
            break;
                    default:
                                break;
                                        }
    }
    void accept() {
                if (parser_stack.top() == T_semicolon) {
                                parser_stack.pop();
                                        }
                        else {
                                        printf("Accept Error");
                                                    exit(1);
                                                            }
                            }
    void execute() {
                while (lookahead!=T_eof) {
                                parser_stack.push(T_stmt);
                                            switch (lookahead) {
                                                            case T_int:
                                                                            case T_char:
                                                                            case T_variable:
                                                                                shift();
                                                                                                reduce();
                                                                                                            }
                                                    }
                    }
};
*/
// Symaboltable.cpp ===================================================================
enum data_type {
        D_const,D_int, D_char,D_void,D_proc,D_class
};
/*
   class variable {
    std::string var_id;
        int Dtype;
        public:
            variable(std::string str, int dtype) {
                    var_id = str; Dtype = dtype;
                        }
                            std::string get_str() {
                                    return var_id;
                                        }
                                        };
                                        */

class table {
    protected:
            std::map<std::string,void *>sub_table;
                std::map<std::string,llvm::Value *>var_table;
    public:
                    void insert_subtable(std::string str, void * ptr);
                        void insert_variable(std::string str, llvm::Value *var);
                            std::string lookup(std::string str);
};

class global_table : public table {};

class class_table : public table {
        std::string class_id;
    public:
            class_table(std::string id);
                std::string get_id();
};

class fuc_table {
        std::string fuc_id;
            std::map<std::string, llvm::Value *>var_table;
    public:
                fuc_table(std::string id);
                    std::string get_id();

                        void insert_variable(std::string str, llvm::Value *var);
};

// Ast.cpp =================================================

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *>NamedValues;

class ExprAST {
    public:
            virtual ~ExprAST() = default;
                virtual llvm::Value *codegen() = 0;
};

class NumExprAST : public ExprAST {
        int Val;
    public:
            NumExprAST(int Val) :Val(Val) {}
                llvm::Value *codegen() override;
};

class VarExprAST : public ExprAST {
        std::string Name;
    public:
            VarExprAST(const std::string &Name) :Name(Name) {}
                llvm::Value *codegen() override;
};

class BinaryExprAST : public ExprAST {
        int Op;
            std::unique_ptr<ExprAST> LHS, RHS;
    public:
                BinaryExprAST(int Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
                    llvm::Value *codegen() override;
};
// Function Call
class CallExprAST : public ExprAST {
        std::string Callee;
            std::vector<std::unique_ptr<ExprAST>> Args;
    public:
                CallExprAST(const std::string &Name, std::vector<std::unique_ptr<ExprAST>> Args)
                            : Callee(Callee), Args(std::move(Args)) {}
                    llvm::Value *codegen();
                        const std::string &getName() const { return Callee; }
};
// ProtoTypeAST - 
class PrototypeAST {
        std::string Name;
            std::vector<std::string> Args;
    public:
                PrototypeAST(const std::string &Name, std::vector<std::string> Args)
                            :Name(Name), Args(std::move(Args)) {}
                    llvm::Function *codegen();
                        const std::string &getName() const { return Name; }
};

class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
            std::unique_ptr<ExprAST> Body;

    public:
                FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                                std::unique_ptr<ExprAST> Body)
                            : Proto(std::move(Proto)), Body(std::move(Body)) {}

                    const PrototypeAST& getProto() const;
                        const std::string& getName() const;
                            llvm::Function *codegen();
};
/*
    token type : keyword , identi , delimiter , Operator
    */
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
