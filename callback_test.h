/// @file callback_test.h
/// @brief Header for a test of the callback mechanism.
/// $Id: callback_test.h 26 2012-08-23 19:41:54Z mfms $

using namespace std;

#include <string>
#include <iostream>
#include <ilcplex/ilocplex.h>
#include <stdlib.h>

class MyHeuristicCallBackI: public IloCplex::HeuristicCallbackI {
public:
  virtual void main();
  virtual IloCplex::CallbackI * duplicateCallback() const;
  virtual ~MyHeuristicCallBackI() {}
  IloCplex::Callback * createInstance(); 
};

//  [Last modified: 2006 03 09 at 22:27:28 GMT]
