/* SI 413 Fall 2015
 * Lab 7
 * C++ header file for the Value class.
 */

#ifndef VALUE_HPP
#define VALUE_HPP

#include <iostream>
#include <cstdlib>
using namespace std;

// Forward declaration of the Lambda AST class so we can have
// pointers to Lambda objects.
class Lambda;

// This gives the type of what's stored in the Value object.
// NONE_T means nothing has been set yet.
enum VType {
  NUM_T, BOOL_T, FUN_T, NONE_T
};

class Value {
  private:
    // The value is either an int, a bool, or a pointer to a Lambda.
    union {
      int num;
      bool tf;
      Lambda* func;
    } val;

    VType type;

  public:
    Value() :type(NONE_T) { }
    Value(int n) :type(NUM_T) { val.num = n; }
    Value(bool b) :type(BOOL_T) { val.tf = b; }
    Value(Lambda* ptr) :type(FUN_T) { val.func = ptr; }

    VType getType() { return type; }
    void setType(VType t) { type = t; }

    int num() { return val.num; }

    bool tf() { return val.tf; }

    bool coerceBool() {
      bool branch;
      if (getType() == NUM_T) {
        branch = (num() != 0);
      }
      else if (getType() == BOOL_T) {
        branch = tf();
      }
      return branch;
    }


    Lambda* func() { return val.func; }

    /* Writes a representation of this Value object to the
     * named output stream, according to the stored type.
     */
    void writeTo(ostream& out) {
      switch(type) {
        case NUM_T: out << val.num; break;
        case BOOL_T: out << (val.tf ? "true" : "false"); break;
        case FUN_T: out << "lambda expression"; break;
        case NONE_T: cout << "UNSET"; break;
      }
    }

    /* This function makes equality testing work using the == operator.
     * So for instance if we have
     * Value v1, v2;
     * then we can write
     * v1 == v2
     * and get a bool back only if they have the same type and value.
     * Note that != is NOT implemented.
     */
    bool operator==(const Value& other) {
      if (type != other.type) {
          return false;
      }
      switch(type) {
        case NUM_T: return val.num == other.val.num;
        case BOOL_T: return val.tf == other.val.tf;
        case FUN_T: return val.func == other.val.func;
        case NONE_T: return true;
      }
    }
};


#endif // VALUE_HPP
