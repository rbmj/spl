/* SI 413 Fall 2015
 * Lab 7
 * C++ header file for the SymbolTable class
 */

#ifndef ST_HPP
#define ST_HPP

#include <iostream>
#include <map>
#include <string>
using namespace std;

#include "value.hpp"

// Global variable to indicate if an error has occurred.
extern bool error;

// Declare the output streams to use everywhere
extern colorout resout;
extern colorout errout;

/* This class represents a simple global symbol table.
 * Later we will extend it to support dynamic scoping.
 */
class SymbolTable {
  private:
    // The actual map. It is declared private, so it can only
    // be accessed via the public methods below.
    map<string,Value> bindings;

  public:
    // Creates a new, empty symbol table
    SymbolTable() { }

    // Returns the Value bound to the given name.
    Value lookup(string name) {
      if (bindings.count(name) > 0) return bindings[name];
      if (!error) {
        error = true;
        errout << "ERROR: No binding for variable " << name << endl;
      }
      return Value();
    }

    // Creates a new name-value binding
    void bind(string name, Value val = Value()) {
      bindings[name] = val;
      // YOU HAVE TO WRITE THE ERROR CHECKING!
    }

    // Re-defines the value bound to the given name.
    void rebind(string name, Value val) {
      bindings[name] = val;
      // YOU HAVE TO WRITE THE ERROR CHECKING!
    }
};

#endif // ST_HPP
