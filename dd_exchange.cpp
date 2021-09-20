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
Bdd::RelNext(const Bdd& relation, const BddSet& state) const
{
  LACE_ME;
  return sylvan_relnext(bdd, relation.bdd, state.set.bdd);
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
  return Sdd(sdd_disjoin(sdd_conjoin(sdd, g.sdd, Manager), sdd_conjoin(sdd_negate(sdd, Manager), sdd_negate(g.sdd, Manager), Manager), Manager));
}

Sdd
Sdd::RelNext(const Sdd& relation, const SddSet& state) const
{
  std::vector<SddLiteral> SourceVars;  
  std::vector<SddLiteral> TargetVars;
  
  for (int i = 0; i < sdd_manager_var_count(Manager)/2; i++)
    SourceVars.push_back(state.set[i]);
  for (int i = sdd_manager_var_count(Manager)/2; i < sdd_manager_var_count(Manager); i++)
    TargetVars.push_back(state.set[i]);

  SDD Rn = sdd_conjoin(sdd, relation.sdd, Manager);
  int *exists_map = (int*)malloc(sdd_manager_var_count(Manager)*sizeof(SddLiteral));
  SddLiteral *variable_map = (SddLiteral*)malloc(sdd_manager_var_count(Manager)*sizeof(SddLiteral));
  for (unsigned i = 0; i < SourceVars.size(); i++)
  {
    exists_map[SourceVars[i]] = 1;    
    variable_map[TargetVars[i]] = SourceVars[i];  
  }
  
  SDD Sn = sdd_exists_multiple(exists_map, Rn, Manager);  
  return sdd_rename_variables(Sn, variable_map, Manager);
}

int
Sdd::isOne() const
{
  return sdd == sdd_manager_true(Sdd::Manager);
}

size_t
Sdd::NodeCount() const
{
  return sdd_model_count(sdd, Manager);
}

//---------------------------------BDDSET-------------------------------------//

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
BddSet::contains(BDDVAR variable) const 
{
  Bdd then = set.Then();
  if (set.isOne()) return false;
  else if (set.TopVar() == variable) return true;
  else return BddSet(then).contains(variable);
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

void 
SddSet::add(SddLiteral variable) 
{ 
  set.push_back(variable);
}

void 
SddSet::add(SddSet &other) 
{ 
  for (unsigned i = 0; i < other.set.size(); i++)
    set.push_back(other.set[i]);
}

bool 
SddSet::contains(SddLiteral variable) const 
{
  return std::count(set.begin(), set.end(), variable);
}

std::vector<SddLiteral> 
SddSet::toVector() const 
{
  return set;
}
