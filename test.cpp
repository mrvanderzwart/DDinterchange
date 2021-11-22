#include "dd_exchange.hpp"

#include "assert.h"

using namespace interchange;

void test_initialization()
{
  dd one = dd::ddOne();
  dd zero = dd::ddZero();

  //leaves
  assert(one != zero);
  assert(one == !zero);
  assert(!one == zero);
  assert(!(!one) == one);
  assert(!(!zero) == zero);
}

void test_xnor(int a, int b)
{
  dd aa, bb, res;
  if (a == 1) aa = dd::ddOne();
  else        aa = dd::ddZero();
  if (b == 1) bb = dd::ddOne();
  else        bb = dd::ddZero();
  if (a == b) res = dd::ddOne();
  else        res = dd::ddZero();
  
  dd compute_res = aa - bb;
  
  assert(compute_res == res);
}

void test_operators()
{
  dd a = dd::ddVar(1);
  dd b = dd::ddVar(2);
  dd c = dd::ddVar(3);
  
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
  
  test_xnor(0,0);
  test_xnor(0,1);
  test_xnor(1,0);
  test_xnor(1,1);
}

void test_transition()
{
  vset sources;
  vset targets;
  vset variables;
  
  for (int i = 0; i < 2; i++)
  {  
    static ddvar var = 0;
  
    ddvar source = var++;
    ddvar target = var++;
  
    sources.add(source);
    targets.add(target);
  }
  variables.add(sources);
  variables.add(targets);
  
  std::vector<ddvar> vars = sources.toVector();

  dd state_prev = dd::ddOne(); 
  state_prev = !dd::ddVar(vars[0])&
               !dd::ddVar(vars[1]);
  
  dd state_next = dd::ddOne();
  state_next = dd::ddVar(vars[0])&
               dd::ddVar(vars[1]);

  dd relation = !dd::ddVar(vars[0]) & dd::ddVar(vars[0]+1) &
                !dd::ddVar(vars[1]) & dd::ddVar(vars[1]+1);

  assert(state_prev.RelNext(relation, variables) == state_next);
  assert(state_prev.RelNext(relation, variables).SatCount(variables) == state_next.SatCount(variables));
}

int main()
{
  test_initialization();  
  test_operators();  
  test_transition();
}
