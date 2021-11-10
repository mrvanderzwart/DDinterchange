#include "dd_exchange.hpp"

#include "assert.h"

using namespace interchange;

void test_initialization()
{
  dd one = Factory::ddOne();
  dd zero = Factory::ddZero();

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
  if (a == 1) aa = Factory::ddOne();
  else        aa = Factory::ddZero();
  if (b == 1) bb = Factory::ddOne();
  else        bb = Factory::ddZero();
  if (a == b) res = Factory::ddOne();
  else        res = Factory::ddZero();
  
  dd compute_res = aa - bb;
  
  assert(compute_res == res);
}

void test_operators()
{
  dd a = Factory::ddVar(1);
  dd b = Factory::ddVar(2);
  dd c = Factory::ddVar(3);
  
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

  dd state_prev = Factory::ddOne(); 
  state_prev = !Factory::ddVar(vars[0])&
               !Factory::ddVar(vars[1]);
  
  dd state_next = Factory::ddOne();
  state_next = Factory::ddVar(vars[0])&
               Factory::ddVar(vars[1]);

  dd relation = !Factory::ddVar(vars[0]) & Factory::ddVar(vars[0]+1) &
                !Factory::ddVar(vars[1]) & Factory::ddVar(vars[1]+1);

  assert(state_prev.RelNext(relation, variables) == state_next);
  assert(state_prev.RelNext(relation, variables).SatCount(variables) == state_next.SatCount(variables));
}

int main()
{
  test_initialization();  
  test_operators();  
  test_transition();
}
