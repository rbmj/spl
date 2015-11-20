/* SI 413 Fall 2015
 * Lab 7
 * This is a C++ header file for the AST class hierarchy.
 * YOUR NAME HERE
 * PARTNER NAME HERE
 */

#ifndef AST_HPP
#define AST_HPP

#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <deque>
using namespace std;

#include "colorout.hpp"
#include "value.hpp"
#include "st.hpp"

// Declare the output streams to use everywhere
extern colorout resout;
extern colorout errout;

// Global variable to indicate if an error has occurred.
extern bool error;

// Global variable to indicate there is a human typing at a keyboard
extern bool showPrompt;

// This enum type gives codes to the different kinds of operators.
// Basically, each oper below such as DIV becomes an integer constant.
enum Oper {
  ADD, SUB,
  MUL, DIV, MOD,
  LT, GT, LE, GE, 
  EQ, NE,
  AND, OR, NOT
};

// These are forward declarations for the classes defined below.
// They show the class hierarchy.
class AST;
  class Stmt;
    class NullStmt;
    class Block;
    class IfStmt;
    class WhileStmt;
    class NewStmt;
    class Asn;
    class Write;
    class WriteStr;
    class Fun;
    class Return;
    class ExpStmt;
  class Exp;
    class Id;
    class Num;
    class BoolExp;
    class ArithOp;
    class CompOp;
    class BoolOp;
    class NegOp;
    class NotOp;
    class Read;
    class Funcall;
  class StrExp;

struct codeGenContext {
    codeGenContext* parent;
    vector<codeGenContext> children;
    vector<string> literals;
    map<string, int> identifiers;
    vector<string> code;
    deque<unsigned> labels;
    int numids;
    void addIdentifier(const string& s) {
        identifiers[s] = numids++;
    }
    bool hasIdentifier(const string& s) {
        if (identifiers.find(s) != identifiers.end()) return true;
        if (parent && parent->hasIdentifier(s)) return true;
        return false;
    }
    string getLitID(unsigned index) {
        ostringstream os;
        os << "SPLLIT_" << index;
        return os.str();
    }
    string getAsmID(const string& id) {
        if (!parent) {
            return "SPL_" + id; //global scope
        }
        else {
            auto it = identifiers.find(id);
            if (it != identifiers.end()) {
                ostringstream os;
                os << "ebp - " << (it->second+1)*4;
                return os.str();
            }
            return parent->getAsmID(id);
        }
    }
    string getLabel(unsigned index) { //label immediately before statement <index>
        ostringstream os;
        os << ".L" << index;
        return os.str();
    }
    bool hasFunction(const string& id) {
        codeGenContext* global_scope = parent ? parent : this;
        for (int i = 0; i < global_scope->children.size(); ++i) {
            if (global_scope->children[i].code[0] == id) {
                return true;
            }
        }
        return false;
    }

    void generateCode(const char*);
    codeGenContext(codeGenContext* p=NULL) : parent(p), numids(0) {}
};

/* The AST class is the super-class for abstract syntax trees.
 * Every type of AST (or AST node) has its own subclass.
 */
class AST {
  private:
    /* Adds this node and all children to the output stream in DOT format. 
     * nextnode is the index of the next node to add. */
    void addToDot(ostream& out, int& nextnode);

  protected:
    // These two protected fields determine the structure of the AST.
    string nodeLabel;
    vector<AST*> children;

    // Inserts a new AST node as a child of this one.
    // (where the new node is inserted depends on which subclass.)
    virtual void ASTchild(AST* child) = 0;

  public:
    /* Writes this AST to a .dot file as named. */
    void writeDot(const char* fname);

    /* Makes a new "empty" AST node. */
    AST() { nodeLabel = "EMPTY"; }
};

/* Every AST node that is not a Stmt is an Exp.
 * These represent actual computations that return something
 * (in particular, a Value object).
 */
class Exp :public AST {
  protected:
    // Inserts a new AST as a child of this one.
    void ASTchild(AST* child) { children.push_back(child); }

  public:
    /* This is the method that must be overridden by all subclasses.
     * It should perform the computation specified by this node, and
     * return the resulting value that gets computed. */
    virtual Value eval() {
      if (!error) {
        errout << "eval() not yet implemented for " 
               << nodeLabel << " nodes!" << endl;
        error = true;
      }
      return Value();
    }
    virtual void evalCode(codeGenContext&) {
        errout << "Error: Not Implemented for " << nodeLabel << endl;
        exit(1);
    }
};

class StrExp :public AST {
    private:
        string s;
    public:
        StrExp(string a) : s(escape(a.substr(1, a.size()-2))) { nodeLabel = "StrExp:" + a; }
        string& getVal() { return s; }
        void ASTchild(AST* child) { children.push_back(child); }
        static string escape(const string& a) {
            string ret = "";
            bool esc = false;
            for (char c : a) {
                if (esc) {
                    switch (c) {
                        case 'n': ret = ret + '\n'; break;
                        case 't': ret = ret + '\t'; break;
                        default: ret = ret + c; break;
                    }
                    esc = false;
                }
                else {
                    if (c == '\\') {
                        esc = true;
                    }
                    else {
                        ret = ret + c;
                    }
                }
            }
            if (esc) {
                ret = ret + '\\';
            }
            return ret;
        }
        void evalCode(codeGenContext& ctx) {
            codeGenContext* global_scope = ctx.parent ? ctx.parent : &ctx;
            global_scope->literals.push_back(s);
            string id = global_scope->getLitID(global_scope->literals.size()-1);
            ctx.code.push_back("lea eax, [" + id + "]");
        }
};

/* An identifier, i.e. variable or function name. */
class Id :public Exp {
  private:
    string val;

  public:
    // Constructor from a C-style string
    Id(const char* v) { 
      val = v;
      nodeLabel = "Exp:Id:" + val;
    }

    // Returns a reference to the stored string value.
    string& getVal() { return val; }
    Value eval();
    void evalCode(codeGenContext& ctx) {
      if (!ctx.hasIdentifier(val)) {
        errout << "Undefined identifier " << val << endl;
        exit(1);
      }
      ctx.code.push_back("mov eax, [" + ctx.getAsmID(val) + "]");
    }
};

/* A literal number in the program. */
class Num :public Exp {
  private:
    int val;

  public:
    Num(int v) { 
      val = v;
      // Converting integers to strings is a little annoying...
      ostringstream label;
      label << "Exp:Num:" << val;
      nodeLabel = label.str();
    }

    // To evaluate, just return the number!
    Value eval() { return val; }
    void evalCode(codeGenContext& ctx) {
      ostringstream os;
      os << "mov eax, " << val;
      ctx.code.push_back(os.str());
    }
};

/* A literal boolean value like "true" or "false" */
class BoolExp :public Exp {
  private:
    bool val;

  public:
    BoolExp(bool v) { 
      val = v;
      nodeLabel = "Exp:Bool:";
      if (v) nodeLabel += "true";
      else nodeLabel += "false";
    }
    Value eval() { return val; }
    void evalCode(codeGenContext& ctx) {
      ostringstream os;
      os << "mov eax, " << (val ? 1 : 0);
      ctx.code.push_back(os.str());
    }
};

/* A binary opration for arithmetic, like + or *. */
class ArithOp :public Exp {
  private:
    Oper op;
    Exp* left;
    Exp* right;

  public:
    ArithOp(Exp* l, Oper o, Exp* r);

    Value eval();
    void evalCode(codeGenContext& ctx);
};

/* A binary operation for comparison, like < or !=. */
class CompOp :public Exp {
  private:
    Oper op;
    Exp* left;
    Exp* right;

  public:
    CompOp(Exp* l, Oper o, Exp* r);

    Value eval();
    void evalCode(codeGenContext& ctx);
};

/* A binary operation for boolean logic, like "and". */
class BoolOp :public Exp {
  private:
    Oper op;
    Exp* left;
    Exp* right;

  public:
    BoolOp(Exp* l, Oper o, Exp* r);
    Value eval();
    void evalCode(codeGenContext& ctx);
};

/* This class represents a unary negation operation. */
class NegOp :public Exp {
  private:
    Exp* right;

  public:
    NegOp(Exp* r) { 
      nodeLabel = "Exp:NegOp";
      right = r;
      ASTchild(right);
    }
    Value eval();
    void evalCode(codeGenContext& ctx) {
        right->evalCode(ctx);
        ctx.code.push_back("neg eax");
    }
};

/* This class represents a unary "not" operation. */
class NotOp :public Exp {
  private:
    Exp* right;

  public:
    NotOp(Exp* r) { 
      nodeLabel = "Exp:NotOp";
      right = r;
      ASTchild(right);
    }
    Value eval() {
        return !(right->eval().tf());
    }
    void evalCode(codeGenContext& ctx) {
        right->evalCode(ctx);
        ctx.code.push_back("neg eax");
        ctx.code.push_back("sbb eax, eax");
        ctx.code.push_back("inc eax");
    }
};

/* A read expression. */
class Read :public Exp {
  public:
    Read() { nodeLabel = "Exp:Read"; }
    Value eval() {
      int x;
      std::cout << "read> ";
      std::cin >> x;
      return Value(x);
    }
    void evalCode(codeGenContext& ctx) {
        ctx.code.push_back("call read");
    }
};

/* A Stmt is anything that can be evaluated at the top level such
 * as I/O, assignments, and control structures.
 * The last child of any statement is the next statement in sequence.
 */
class Stmt :public AST {
  private:
    // Pointer to the next statement in sequence.
    Stmt* next;

  protected:
    // Inserts a new AST as a child of this one.
    void ASTchild(AST* child) {
      // This inserts before the last thing in the vector,
      // i.e., just before the "next" statement
      children.insert(children.end()-1, child);
    }

  public:
    /* This static method is for building sequences of statements by the
     * parser. It takes two statements, and appends one at the end of the other.
     * The returned value is a pointer to the new statement representing
     * the sequence.
     */
    static Stmt* append(Stmt* a, Stmt* b);

    /* Default constructor. The next statement will be set to NullStmt. */
    Stmt ();

    // This constructor sets the next statement manually.
    Stmt (Stmt* nextStmt) {
      if (nextStmt != NULL) children.push_back(nextStmt);
      next = nextStmt;
    }

    // Getter and setter for the next statement in sequence.
    Stmt* getNext() { return next; }
    void setNext(Stmt* nextStmt) { 
      children.back() = nextStmt; 
      next = nextStmt;
    }

    // This should only be false in the NullStmt class.
    bool hasNext() { return next != NULL; }

    /* This is the command that must be implemented everywhere to
     * execute this Stmt - that is, do whatever it is that this statement
     * says to do. */
    virtual void exec() {
      if (!error) {
        errout << "exec() not yet implemented for " 
               << nodeLabel << " nodes!" << endl;
        error = true;
      }
    }

    virtual void execCode(codeGenContext&) {
        errout << "Code Generation not implemented for " << nodeLabel << endl;
        exit(1);
    }
};

/* This class is necessary to terminate a sequence of statements. */
class NullStmt :public Stmt {
  public:
    NullStmt() :Stmt(NULL) { 
      nodeLabel = "Stmt:Null";
    }

    // Nothing to execute!
    void exec() { }
    void execCode(codeGenContext&) {}
};

/* This is a statement for a block of code, i.e., code enclosed
 * in curly braces { and }.
 * Eventually, this is where scopes will begin and end.
 */
class Block :public Stmt {
  private:
    Stmt* body;

  public:
    Block(Stmt* b) { 
      nodeLabel = "Stmt:Block";
      body = b;
      ASTchild(body);
    }
    void exec() {
      for (Stmt* p = body; p; p = p->getNext()) {
        p->exec();
      }
    }
    void execCode(codeGenContext& ctx) {
        for (Stmt* p = body; p; p = p->getNext()) {
            p->execCode(ctx);
        }
    }
};

/* This class is for "if" AND "ifelse" statements. */
class IfStmt :public Stmt {
  private:
    Exp* clause;
    Stmt* ifblock;
    Stmt* elseblock;

  public:
    IfStmt(Exp* e, Stmt* ib, Stmt* eb) { 
      nodeLabel = "Stmt:If";
      clause = e;
      ifblock = ib;
      elseblock = eb;
      ASTchild(clause);
      ASTchild(ifblock);
      ASTchild(elseblock);
    }
    void exec() {
      if (clause->eval().coerceBool()) {
        if (ifblock) ifblock->exec();
      }
      else {
        if (elseblock) elseblock->exec();
      }
    }
    void execCode(codeGenContext& ctx) {
        clause->evalCode(ctx);
        ctx.code.push_back("test eax, eax");
        ctx.code.push_back("jz ELSE");
        unsigned placeHold = ctx.code.size() - 1;
        if (ifblock) ifblock->execCode(ctx);
        ctx.code.push_back("jmp END");
        ctx.labels.push_back(ctx.code.size());
        ctx.code[placeHold] = "jz " + ctx.getLabel(ctx.code.size());
        placeHold = ctx.code.size()-1;
        ctx.code.push_back("nop");
        if (elseblock) elseblock->execCode(ctx);
        ctx.labels.push_back(ctx.code.size());
        ctx.code[placeHold] = "jmp " + ctx.getLabel(ctx.code.size());
    }
};

/* Class for while statements. */
class WhileStmt :public Stmt {
  private:
    Exp* clause;
    Stmt* body;
   
  public:
    WhileStmt(Exp* c, Stmt* b) { 
      nodeLabel = "Stmt:While";
      clause = c;
      body = b;
      ASTchild(clause);
      ASTchild(body);
    }
    void exec() {
      while (clause->eval().coerceBool()) {
        if (body) body->exec();
      }
    }
    void execCode(codeGenContext& ctx) {
        ctx.code.push_back("jmp COND");
        ctx.labels.push_back(ctx.code.size());
        unsigned placeHold = ctx.code.size();
        body->execCode(ctx);
        ctx.labels.push_back(ctx.code.size());
        ctx.code[placeHold-1] = "jmp " + ctx.getLabel(ctx.code.size());
        clause->evalCode(ctx);
        ctx.code.push_back("test eax, eax");
        ctx.code.push_back("jnz " + ctx.getLabel(placeHold));
    }
};

/* A "new" statement creates a new binding of the variable to the
 * stated value.  */
class NewStmt :public Stmt {
  private:
    Id* lhs;
    Exp* rhs;

  public:
    NewStmt(Id* l, Exp* r) { 
      nodeLabel = "Stmt:New";
      lhs = l;
      rhs = r;
      ASTchild(lhs);
      ASTchild(rhs);
    }
    void exec();
    void execCode(codeGenContext& ctx);
};

/* An assignment statement. This represents a RE-binding in the symbol table. */
class Asn :public Stmt {
  private:
    Id* lhs;
    Exp* rhs;
   
  public:
    Asn(Id* l, Exp* r) { 
      nodeLabel = "Stmt:Asn";
      lhs = l;
      rhs = r;
      ASTchild(lhs);
      ASTchild(rhs);
    }
    void exec();
    void execCode(codeGenContext& ctx);
};

/* A write statement. */
class Write :public Stmt {
  private:
    Exp* val;
    bool newline;
  public:
    Write(Exp* v, bool nl = true) { 
      nodeLabel = "Stmt:Write";
      val = v;
      ASTchild(val);
      newline = nl;
    }

    void exec() {
      Value res = val->eval();
      if (!error) {
        res.writeTo(resout); 
        if (newline) resout << '\n';
      }
      getNext()->exec();
    }
    void execCode(codeGenContext& ctx) {
        val->evalCode(ctx);
        ctx.code.push_back("call write");
        if (newline) ctx.code.push_back("call writelf");
    }
};

class WriteStr :public Stmt {
  private:
    StrExp* myval;
    bool newline;
  public:
    WriteStr(StrExp* v, bool nl = true) {
      nodeLabel = "Stmt:Write";
      myval = v;
      ASTchild(myval);
      newline = nl;
    }
    void exec() {
      string res = myval->getVal();
      resout << res;
      if (newline) resout << '\n';
      getNext()->exec();
    }
    void execCode(codeGenContext& ctx) {
        myval->evalCode(ctx);
        ctx.code.push_back("call writestr");
        if (newline) ctx.code.push_back("call writelf");
    }
};

/* A lambda expression consists of a parameter name and a body. */
class Fun :public Stmt {
  private:
    Id* name;
    Id* var;
    Stmt* body;

  protected:
    void writeLabel(ostream& out) { out << "fun:exp" << flush; }

  public:
    Fun(Id* n, Id* v, Stmt* b) { 
      nodeLabel = "Exp:Fun";
      name = n;
      var = v;
      body = b;
      ASTchild(var);
      ASTchild(body);
    }

    // These getter methods are necessary to support actually calling
    // the lambda sometime after it gets created.
    string& getName() { return name->getVal(); }
    string& getVar() { return var->getVal(); }
    Stmt* getBody() { return body; }
    void execCode(codeGenContext& ctx);
};

/* A function call consists of the function name, and the actual argument.
 * Note that all functions are unary. */
class Funcall :public Exp {
  private:
    Id* fun;
    Exp* arg;
  
  public:
    Funcall(Id* f, Exp* a) { 
      nodeLabel = "Exp:Funcall";
      fun = f;
      arg = a;
      ASTchild(fun);
      ASTchild(arg);
    }
    void evalCode(codeGenContext& ctx) {
        arg->evalCode(ctx);
        bool found = false;
        string name = fun->getVal();
        if (!ctx.hasFunction(name)) {
            std::cerr << "Use of undeclared function " << name << '\n';
            exit(1);
        }
        ctx.code.push_back("call " + name);
    }
};

class Return : public Stmt {
    private:
        Exp* arg;
    public:
        Return(Exp* a) {
            arg = a;
            nodeLabel = "Stmt:Return";
            ASTchild(arg);
        }
        void execCode(codeGenContext& ctx) {
            if (!ctx.parent) {
                cerr << "Cannot return from global scope\n";
                exit(1);
            }
            arg->evalCode(ctx);
            ctx.code.push_back("jmp .RET");
        }
};

class ExpStmt : public Stmt {
    private:
        Exp* arg;
    public:
        ExpStmt(Exp* a) {
            arg = a;
            nodeLabel = "Stmt:ExpStmt";
            ASTchild(arg);
        }
        void execCode(codeGenContext& ctx) {
            arg->evalCode(ctx);
        }
};

#endif //AST_HPP
