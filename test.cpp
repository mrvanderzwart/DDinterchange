#include "dd_exchange.hpp"

#include <iostream>

#include "assert.h"

using namespace sylvan;

SddManager* SddEssentials::Manager = sdd_manager_create(3,0);

int main()
{
  dd one = DecisionDiagram::ddOne();
  dd zero = DecisionDiagram::ddZero();

  //leaves
  //assert(one != zero);
  assert(one == !zero);
  assert(!one == zero);
  assert(!(!one) == one);
  assert(!(!zero) == zero);
  
  dd a = DecisionDiagram::create(1);
  dd b = DecisionDiagram::create(2);
  dd c = DecisionDiagram::create(3);
  
  //associativity
  assert((a&(b&c)) == ((a&b)&c));
  assert(((a&b)&c) == (a&b&c));
  assert((a|(b|c)) == ((a|b)|c));
  assert(((a|b)|c) == (a|b|c));
  
  //commutativity
  assert((a&b) == (b&a));
  assert((a|b) == (b|a));
  
  //distributivity
  assert((a&(b|c)) == (a&b|a&c));
  assert((a|(b&c)) == ((a|b)&(a|c)));
  
  //demorgan
  assert(!(a|b) == (!a&!b));
  assert(!(a&b) == (!a|!b));
}
