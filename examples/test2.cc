#include "../dd_interchange/sylvan_dd.h"
#include <stdio.h>
#include <assert.h>

using namespace sylvan;

SddManager* Sdd::Manager = sdd_manager_create(4, 0);

void test()
{
  /* test initialization of SDDs */
  
  Dd one = Dd::ddOne(); //true SDD
  Dd zero = Dd::ddZero(); //false SDD

  assert(one.GetDD() == sdd_manager_true(Sdd::Manager)); //check if really true
  assert(one.isOne()); //check if really true with function
  assert(zero.GetDD() == sdd_manager_false(Sdd::Manager)); //check if really false
  
  /* test variable initialization of SDDs */

  Dd a = Dd::ddVar(1); //create SDD variable x_0
  Dd b = Dd::ddVar(2); //create SDD variable x_1

  // check if a is really the Boolean formula x_0
  assert(a.TopVar() == 1);
  //assert(a.Then() == one);

  // check if b is really the Boolean formula x_1
  assert(b.TopVar() == 2);
  //assert(b.Then() == one);
  
  /* test Boolean operators */
  
  // negation
  Dd not_a = !a;
  assert((!not_a) == a);
  
  // conjunction and disjunction 
  Dd a_and_b = a & b;
  Dd not_not_a_or_not_b = !(!a | !b);
  assert(a_and_b == not_not_a_or_not_b);
}

int main()
{
  /* Init BDD package */
  lace_init(0, 0);
  lace_startup(0, NULL, NULL);
  LACE_ME;
  sylvan_set_sizes(1ULL<<22, 1ULL<<27, 1ULL<<20, 1ULL<<26);
  sylvan_init_package();
  sylvan_init_bdd();

  test();
}
