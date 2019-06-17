/// @file callback_test.cpp
/// @brief Creates a callback class and has it print a message.
/// $Id: callback_test.cpp 26 2012-08-23 19:41:54Z mfms $

#include"callback_test.h"

void
MyHeuristicCallBackI::main() {
  getEnv().out() << "&&& Callback, rows = " << getNrows()
                 << ", cols = " << getNcols() << endl;
}

IloCplex::CallbackI *
MyHeuristicCallBackI::duplicateCallback() const {
  return (new (getEnv()) MyHeuristicCallBackI(*this));
}

IloCplex::Callback *
MyHeuristicCallBackI::createInstance() {
  return dynamic_cast<IloCplex::Callback *>
    ((new (getEnv()) MyHeuristicCallBackI(*this)));
}

//  [Last modified: 2006 03 09 at 22:21:29 GMT]
