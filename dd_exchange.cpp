#include "dd_exchange.hpp"

using namespace sylvan;

//------------------------------------BDD-------------------------------------//

const Bdd 
Bdd::ddOne()
{
  return Bdd(sylvan_true);
}
    
const Bdd
Bdd::ddZero()
{
  return Bdd(sylvan_false);
}
    
Bdd
Bdd::ddVar(BDDVAR index)
{
  LACE_ME;
  return sylvan_ithvar(index);
}

int
Bdd::operator==(const Bdd& other) const
{
  return bdd == other.bdd;
}

Bdd&
Bdd::operator=(const Bdd& other) 
{
  bdd = other.bdd;
  return *this;
}
    
Bdd
Bdd::operator&(const Bdd& other) const
{
  LACE_ME;
  return Bdd(sylvan_and(bdd, other.bdd));
}

Bdd&
Bdd::operator&=(const Bdd& other) 
{
  LACE_ME;
  bdd = sylvan_and(bdd, other.bdd);
  return *this;
}

Bdd
Bdd::operator|(const Bdd& other) const
{
  LACE_ME;
  return Bdd(sylvan_or(bdd, other.bdd));
}

Bdd&
Bdd::operator|=(const Bdd& other) 
{
  LACE_ME;
  bdd = sylvan_or(bdd, other.bdd);
  return *this;
}

Bdd
Bdd::operator*(const Bdd& other) const
{
  LACE_ME;
  return Bdd(sylvan_and(bdd, other.bdd));
}

Bdd&
Bdd::operator*=(const Bdd& other)
{
  LACE_ME;
  bdd = sylvan_and(bdd, other.bdd);
  return *this;
}

Bdd
Bdd::operator!() const
{
  return Bdd(sylvan_not(bdd));
}

BDD
Bdd::GetDD() const
{
  return bdd;
}

Bdd
Bdd::Xnor(const Bdd& g) const
{
  LACE_ME;
  return sylvan_equiv(bdd, g.bdd);
}

Bdd
Bdd::Then() const
{
  return Bdd(sylvan_high(bdd));
}

Bdd
Bdd::RelNext(const Bdd& relation, const BddSet& cube) const
{
  LACE_ME;
  return sylvan_relnext(bdd, relation.bdd, cube.set.bdd);
}

size_t
Bdd::NodeCount() const
{
  return sylvan_nodecount(bdd);
}

int
Bdd::isOne() const
{
  return bdd == sylvan_true;
}

BDDVAR
Bdd::TopVar() const
{
  return sylvan_var(bdd);
}

//-----------------------------------SDD--------------------------------------//

const Sdd
Sdd::ddOne()
{
  return Sdd(sdd_manager_true(Manager));
}
    
const Sdd 
Sdd::ddZero()
{
  return Sdd(sdd_manager_false(Manager));
}
    
Sdd
Sdd::ddVar(SddLiteral var)
{
  return Sdd(sdd_manager_literal(var, Manager));
}

int 
Sdd::operator==(const Sdd& other) const
{
  return sdd == other.sdd;
}

Sdd&
Sdd::operator=(const Sdd& other) 
{
  sdd = other.sdd;
  return *this;
}
    
Sdd
Sdd::operator&(const Sdd& other) const
{
  return Sdd(sdd_conjoin(sdd, other.sdd, Manager));
}

Sdd&
Sdd::operator&=(const Sdd& other) 
{
  sdd = sdd_conjoin(sdd, other.sdd, Manager);
  return *this;
}

Sdd
Sdd::operator|(const Sdd& other) const
{
  return Sdd(sdd_disjoin(sdd, other.sdd, Manager));
}

Sdd&
Sdd::operator|=(const Sdd& other) 
{
  sdd = sdd_disjoin(sdd, other.sdd, Manager);
  return *this;
}

Sdd
Sdd::operator*(const Sdd& other) const
{
  LACE_ME;
  return Sdd(sdd_conjoin(sdd, other.sdd, Manager));
}

Sdd&
Sdd::operator*=(const Sdd& other)
{
  sdd = sdd_conjoin(sdd, other.sdd, Manager);
  return *this;
}

Sdd
Sdd::operator!() const
{
  return Sdd(sdd_negate(sdd, Manager));
}

SDD
Sdd::GetDD() const
{
  return sdd;
}

Sdd
Sdd::Xnor(const Sdd& g) const
{
  // sdd xnor g.sdd = -((sdd V g.sdd) ^ -(sdd ^ g.sdd))
  return Sdd(sdd_negate(sdd_conjoin(sdd_disjoin(sdd, g.sdd, Manager), sdd_negate(sdd_conjoin(sdd, g.sdd, Manager), Manager), Manager), Manager));
}

Sdd
Sdd::Then() const
{
  return Sdd(SddExtra::vtree_highest_var(sdd_manager_vtree(Manager)));
}
//XXX needs finish in touch
Sdd
Sdd::RelNext(const Sdd& relation, const SddSet& cube) const
{
  SDD Rn = sdd_conjoin(cube.set.sdd, relation.sdd, Manager);
  return Sdd(Rn);
}

size_t
Sdd::NodeCount() const
{
  return sdd_global_model_count(sdd, Manager);
}

int
Sdd::isOne() const
{
  return sdd == sdd_manager_true(Manager);
}

SddLiteral
Sdd::TopVar() const
{
  return sdd_node_literal(sdd);
}

//---------------------------------BDDSET-------------------------------------//

BDD 
BddSet::GetDD() const 
{ 
  return set.bdd; 
}

void 
BddSet::add(BDDVAR variable) 
{ 
  set *= Bdd::ddVar(variable); 
}

void 
BddSet::add(BddSet &other) 
{ 
  set *= other.set; 
}

bool 
BddSet::isEmpty() const 
{ 
  return set.isOne(); 
}
    
BDDVAR 
BddSet::TopVar() const 
{ 
  return set.TopVar();
}
    
BddSet 
BddSet::Next() const
{
  Bdd then = set.Then();
  return BddSet(then);
}
    
bool 
BddSet::contains(BDDVAR variable) const 
{
  if (isEmpty()) return false;
  else if (TopVar() == variable) return true;
  else return Next().contains(variable);
}
   
std::vector<BDDVAR> 
BddSet::toVector() const 
{
  std::vector<BDDVAR> result;
  Bdd x = set;
  while (!x.isOne()) 
  {
    result.push_back(x.TopVar());
    x = x.Then();
  }
  return result;
}

//---------------------------------SDDSET-------------------------------------//
  
SDD 
SddSet::GetDD() const 
{ 
  return set.sdd;
}

void 
SddSet::add(SddLiteral variable) 
{ 
  set *= Sdd::ddVar(variable); 
}

void 
SddSet::add(SddSet &other) 
{ 
  set *= other.set; 
}

bool 
SddSet::isEmpty() const 
{ 
  return set.isOne(); 
}

SddLiteral 
SddSet::TopVar() const 
{ 
  return set.TopVar(); 
}
    
SddSet 
SddSet::Next() const
{
  Sdd then = set.Then();
  return SddSet(then);
}

bool 
SddSet::contains(SddLiteral variable) const 
{
  if (isEmpty()) return false;
  else if (TopVar() == variable) return true;
  else return Next().contains(variable);;
  return false;
}

std::vector<SddLiteral> 
SddSet::toVector() const 
{
  std::vector<SddLiteral> result;
  Sdd x = set;
  printf("test1\n");
  while (!x.isOne()) 
  {
    result.push_back(x.TopVar());
    x = x.Then();
  }
  printf("test2\n");
  return result;
}
