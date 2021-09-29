#include "dd_exchange.hpp"

#include <iostream>

#include "assert.h"

using namespace sylvan;

SddManager* Sdd::Manager = sdd_manager_create(3,0);

void test_xnor(int a, int b) {
  SDD aa, bb;
  if (a == 1) aa = sdd_manager_true(Sdd::Manager);
  else        aa = sdd_manager_false(Sdd::Manager);
  if (b == 1) bb = sdd_manager_true(Sdd::Manager);
  else        bb = sdd_manager_false(Sdd::Manager);

  std::cout << "aa   = " << sdd_node_is_true(aa) << std::endl;
  std::cout << "bb   = " << sdd_node_is_true(bb) << std::endl;
  SDD res = sdd_disjoin(sdd_conjoin(aa, bb, Sdd::Manager), 
                        sdd_conjoin(sdd_negate(aa, Sdd::Manager), 
                                    sdd_negate(bb, Sdd::Manager), 
                                    Sdd::Manager), Sdd::Manager);
  std::cout << "XNOR = " << sdd_node_is_true(res) << std::endl << std::endl;
}

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
  
  dd a = DecisionDiagram::ddVar(1);
  dd b = DecisionDiagram::ddVar(2);
  dd c = DecisionDiagram::ddVar(3);
  
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


  test_xnor(0, 0);
  test_xnor(0, 1);
  test_xnor(1, 0);
  test_xnor(1, 1);
}
