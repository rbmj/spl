/* SI 413 Fall 2015
 * Lab 7
 * This file contains the implementations of longer methods in the
 * AST class hierarchy.
 */

#include "ast.hpp"
#include <fstream>

/* Adds this node and all children to the output stream in DOT format. 
 * nextnode is the index of the next node to add. */
void AST::addToDot(ostream& out, int& nextnode) {
  int root = nextnode;
  ++nextnode;
  out << "\tn" << root << " [label=\"" << nodeLabel << "\"];" << endl;
  for (int i=0; i < children.size(); ++i) {
    int child = nextnode;
    children[i]->addToDot(out, nextnode);
    out << "\tn" << root << " -> n" << child << ";" << endl;
  }
}

/* Writes this AST to a .dot file as named. */
void AST::writeDot(const char* fname) {
  ofstream fout(fname);
  int nodes = 1;
  fout << "digraph AST {" << endl;
  addToDot (fout, nodes);
  fout << "}" << endl;
  fout.close();
}

// ArithOp constructor
ArithOp::ArithOp(Exp* l, Oper o, Exp* r) { 
  op = o;
  left = l;
  right = r;
  nodeLabel = "Exp:ArithOp:";
  switch(o) {
    case ADD: nodeLabel += '+'; break;
    case SUB: nodeLabel += '-'; break;
    case MUL: nodeLabel += '*'; break;
    case DIV: nodeLabel += '/'; break;
    case MOD: nodeLabel += '%'; break;
    default:  nodeLabel += "ERROR";
  }
  ASTchild(left);
  ASTchild(right);
}

// Evaluates an arithmetic operation
Value ArithOp::eval() {
  int l = left->eval().num();
  int r = right->eval().num();
  switch(op) {
    case ADD: return l + r;
    case SUB: return l - r;
    case MUL: return l * r;
    case DIV: 
      if (r != 0) return l / r;
      else if (!error) {
        error = true;
        errout << "ERROR: Divide by zero" << endl;
      }
      return Value();
    case MOD: return l % r;
    default:  return Value(); // shouldn't get here...
  }
}

void ArithOp::evalCode(codeGenContext& ctx) {
    right->evalCode(ctx);
    ctx.code.push_back("push eax");
    left->evalCode(ctx);
    ctx.code.push_back("pop ebx");
    switch(op) {
        case ADD: ctx.code.push_back("add eax, ebx"); break;
        case SUB: ctx.code.push_back("sub eax, ebx"); break;
        case MUL: ctx.code.push_back("imul ebx"); break;
        case DIV:
            ctx.code.push_back("xor edx, edx");
            ctx.code.push_back("idiv ebx");
            break;
        case MOD:
            ctx.code.push_back("xor edx, edx");
            ctx.code.push_back("idiv ebx");
            ctx.code.push_back("mov eax, edx");
            break;
        default:
            errout << "Unimplemented operator\n";
            exit(1);
    }
}

Value NegOp::eval() {
  return -(right->eval().num());
}

// Constructor for CompOp
CompOp::CompOp(Exp* l, Oper o, Exp* r) {
  op = o;
  left = l;
  right = r;
  nodeLabel = "Exp:CompOp:";
  switch(o) {
    case LT: nodeLabel += "<";  break;
    case GT: nodeLabel += ">";  break;
    case LE: nodeLabel += "<="; break;
    case GE: nodeLabel += ">="; break;
    case EQ: nodeLabel += "=";  break;
    case NE: nodeLabel += "!="; break;
    default: nodeLabel += "ERROR"; break;
  }
  ASTchild(left);
  ASTchild(right);
}

Value CompOp::eval() {
  int lhs = left->eval().num();
  int rhs = right->eval().num();
  switch (op) {
    case LT: return lhs < rhs;
    case GT: return lhs > rhs;
    case LE: return lhs <= rhs;
    case GE: return lhs >= rhs;
    case EQ: return lhs == rhs;
    case NE: return lhs != rhs;
    default: break;
  }
  return false;
}

void CompOp::evalCode(codeGenContext& ctx) {
    right->evalCode(ctx);
    ctx.code.push_back("push eax");
    left->evalCode(ctx);
    ctx.code.push_back("pop ebx");
    ctx.code.push_back("xor ecx, ecx");
    ctx.code.push_back("cmp eax, ebx");
    switch (op) {
        case LT: ctx.code.push_back("setl cl"); break;
        case GT: ctx.code.push_back("setg cl"); break;
        case LE: ctx.code.push_back("setle cl"); break;
        case GE: ctx.code.push_back("setge cl"); break;
        case EQ: ctx.code.push_back("sete cl"); break;
        case NE: ctx.code.push_back("setne cl"); break;
        default:
            errout << "Unimplemented operator\n";
            exit(1);
    }
    ctx.code.push_back("mov eax, ecx");
}
    

// Constructor for BoolOp
BoolOp::BoolOp(Exp* l, Oper o, Exp* r) {
  op = o;
  left = l;
  right = r;
  nodeLabel = "Exp:BoolOp:";
  if (o == AND) nodeLabel += "and";
  else nodeLabel += "or";
  ASTchild(left);
  ASTchild(right);
}

Value BoolOp::eval() {
  switch (op) {
      case AND: return left->eval().tf() && right->eval().tf();
      case OR:  return left->eval().tf() || right->eval().tf();
      default: break;
  }
  return false;
}

void BoolOp::evalCode(codeGenContext& ctx) {
    left->evalCode(ctx);
    ctx.code.push_back("test eax, eax");
    ctx.code.push_back("jz/jnz DUMMY");
    unsigned placeHold = ctx.code.size() - 1;
    right->evalCode(ctx);
    ctx.labels.push_back(ctx.code.size());
    ctx.code[placeHold] = (op == AND ? "jz " : "jnz ") + ctx.getLabel(ctx.code.size());
}


// Appends b to the end of a and returns the result.
Stmt* Stmt::append(Stmt* a, Stmt* b) {
  if (! a->hasNext()) return b;
  Stmt* last = a;
  while (last->getNext()->hasNext()) last = last->getNext();
  last->setNext(b);
  return a;
}

// Default constructor for Stmt.
// This HAS to be declared here because it uses NullStmt, which
// hasn't been defined yet in the header file!
Stmt::Stmt() {
  next = new NullStmt();
  children.push_back(next);
}

std::map<std::string, Value> varmap;

void NewStmt::exec() {
    if (varmap.find(lhs->getVal()) != varmap.end()) {
        error = true;
        std::cerr << "ERROR: Variable already bound\n";
    }
    else {
        varmap[lhs->getVal()] = rhs->eval();
    }
}

void NewStmt::execCode(codeGenContext& ctx) {
    rhs->evalCode(ctx);
    if (ctx.hasIdentifier(lhs->getVal())) {
        errout << "ERROR: Variable already bound\n";
        exit(1);
    }
    ctx.addIdentifier(lhs->getVal());
    ctx.code.push_back("mov [" + ctx.getAsmID(lhs->getVal()) + "], eax");
}

void Asn::exec() {
    if (varmap.find(lhs->getVal()) == varmap.end()) {
        error = true;
        std::cerr << "ERROR: Can't rebind; not yet bound!\n";
    }
    else {
        varmap[lhs->getVal()] = rhs->eval();
    }
}

void Asn::execCode(codeGenContext& ctx) {
    rhs->evalCode(ctx);
    if (!ctx.hasIdentifier(lhs->getVal())) {
        errout << "ERROR: Undefined variable\n";
        exit(1);
    }
    ctx.code.push_back("mov [" + ctx.getAsmID(lhs->getVal()) + "], eax");
}

Value Id::eval() {
    if (varmap.find(val) == varmap.end()) {
        error = true;
        std::cerr << "ERROR: Can't reference, not yet bound!\n";
        return Value(0);
    }
    return varmap[val];
}

//from http://stackoverflow.com/questions/874134
inline bool ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void codeGenContext::generateCode(const char* fname_c) {
    code.push_back("call exit");
    string fname(fname_c);
    if (ends_with(fname, ".spl")) {
        auto it = fname.rbegin();
        *(it++) = 'm';
        *(it++) = 's';
        *(it++) = 'a';
    }
    else {
        fname += ".asm";
    }
    ofstream out(fname.c_str());
    out << "[BITS 32]\n"
        << "extern exit\n"
        << "extern write\n"
        << "extern writestr\n"
        << "extern writebool\n"
        << "extern writelf\n"
        << "extern read\n"
        << "global _start\n";
    out << "\nsection .rodata\n";
    for (unsigned i = 0; i < literals.size(); ++i) {
        out << getLitID(i) << ": db `" << literals[i] << "\\0`\n";
    }
    out << "\nsection .bss\n";
    for (auto& i : identifiers) {
        out << getAsmID(i.first) << ": resb 4\n";
    }
    out << "\nsection .text\n\n";
    for (int i = 0; i < children.size(); ++i) {
        out << "global " << children[i].code[0] << '\n';
        out << children[i].code[0] << ":\n";
        for (int j = 1; j < children[i].code.size(); ++j) {
            if (children[i].labels.front() == j) {
                children[i].labels.pop_front();
                out << getLabel(j) << ":\n";
            }
            out << '\t' << children[i].code[j] << '\n';
        }
        out << ".RET:\n";
        out << "\tmov esp, ebp\n";
        out << "\tpop ebp\n";
        out << "\tret\n\n";
    }
    out << "_start:\n";
    for (unsigned i = 0; i < code.size(); ++i) {
        if (labels.front() == i) {
            labels.pop_front();
            out << getLabel(i) << ":\n";
        }
        out << '\t' << code[i] << '\n';
    }
    out.close();
}

void Fun::execCode(codeGenContext& ctx) {
    if (ctx.parent) {
        std::cerr << "ERROR: No nested function declarations!\n";
        exit(1);
    }
    if (ctx.hasFunction(getName())) {
        cerr << "ERROR: Attempted to redefine function " << getName() << '\n';
        exit(1);
    }
    ctx.children.push_back(codeGenContext(&ctx));
    codeGenContext& childctx = ctx.children.back();
    childctx.code.push_back(getName());
    childctx.code.push_back("push ebp");
    childctx.code.push_back("mov ebp, esp");
    childctx.addIdentifier(getVar());
    childctx.code.push_back("push eax");
    childctx.code.push_back("sub esp, DUMMY");
    unsigned placeholder = childctx.code.size()-1;
    body->execCode(childctx);
    ostringstream os;
    os << "sub esp, " << (childctx.identifiers.size()-1)*4;
    childctx.code[placeholder] = os.str();
}
