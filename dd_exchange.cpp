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

int
Bdd::operator!=(const Bdd& other) const
{
  return bdd != other.bdd;
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
Bdd::operator-(const Bdd& other) const
{
  LACE_ME;
  return sylvan_equiv(bdd, other.bdd);
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

//-----------------------------------SDD--------------------------------------//

const Sdd
Sdd::ddOne()
{
  return Sdd(sdd_manager_true(SddExtra::Manager));
}
    
const Sdd 
Sdd::ddZero()
{
  return Sdd(sdd_manager_false(SddExtra::Manager));
}
    
Sdd
Sdd::ddVar(SddLiteral var)
{
  return Sdd(sdd_manager_literal(var, SddExtra::Manager));
}

int 
Sdd::operator==(const Sdd& other) const
{
  return sdd == other.sdd;
}

int
Sdd::operator!=(const Sdd& other) const
{
  return sdd != other.sdd;
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
  return Sdd(sdd_conjoin(sdd, other.sdd, SddExtra::Manager));
}

Sdd&
Sdd::operator&=(const Sdd& other) 
{
  sdd = sdd_conjoin(sdd, other.sdd, SddExtra::Manager);
  return *this;
}

Sdd
Sdd::operator|(const Sdd& other) const
{
  return Sdd(sdd_disjoin(sdd, other.sdd, SddExtra::Manager));
}

Sdd&
Sdd::operator|=(const Sdd& other) 
{
  sdd = sdd_disjoin(sdd, other.sdd, SddExtra::Manager);
  return *this;
}

Sdd
Sdd::operator-(const Sdd& other) const
{
  return Sdd(sdd_disjoin(sdd_conjoin(sdd, other.sdd, SddExtra::Manager), sdd_conjoin(sdd_negate(sdd, SddExtra::Manager), sdd_negate(other.sdd, SddExtra::Manager), SddExtra::Manager), SddExtra::Manager));
}

Sdd
Sdd::operator*(const Sdd& other) const
{
  return Sdd(sdd_conjoin(sdd, other.sdd, SddExtra::Manager));
}

Sdd&
Sdd::operator*=(const Sdd& other)
{
  sdd = sdd_conjoin(sdd, other.sdd, SddExtra::Manager);
  return *this;
}

Sdd
Sdd::operator!() const
{
  return Sdd(sdd_negate(sdd, SddExtra::Manager));
}

SDD
Sdd::GetDD() const
{
  return sdd;
}

Sdd
Sdd::RelNext(const Sdd& relation, const SddSet& state) const
{
  int n_variables = sdd_manager_var_count(SddExtra::Manager);
  SDD Rn = sdd_conjoin(sdd, relation.sdd, SddExtra::Manager);
  int *exists_map = (int*)malloc((n_variables+1)*sizeof(int));
  SddLiteral *variable_map = (SddLiteral*)malloc((n_variables+1)*sizeof(SddLiteral));
  
  for (int i = 1; i <= n_variables; i++)
  {
    exists_map[i] = 0;
    variable_map[i] = 0;
  }
  
  for (int i = 0; i < n_variables/2; i++)
  {
    exists_map[state.set[i]] = 1;    
    variable_map[state.set[i+n_variables/2]] = state.set[i];  
  }

  SDD Sn = sdd_exists_multiple(exists_map, Rn, SddExtra::Manager);  
  return sdd_rename_variables(Sn, variable_map, SddExtra::Manager);
  
  free(exists_map);
  free(variable_map);
}

size_t
Sdd::NodeCount() const
{
  return sdd_model_count(sdd, SddExtra::Manager);
}

//---------------------------------BDDSET-------------------------------------//

BDD
BddSet::GetDD() const
{
  return set.GetDD();
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
BddSet::contains(BDDVAR variable) const 
{
  Bdd then = Bdd(sylvan_high(set.GetDD()));
  if (set.GetDD() == sylvan_true) return false;
  else if (sylvan_var(set.GetDD()) == variable) return true;
  else return BddSet(then).contains(variable);
}
   
std::vector<BDDVAR> 
BddSet::toVector() const 
{
  std::vector<BDDVAR> result;
  Bdd x = set;
  while (x.GetDD() != sylvan_true) 
  {
    result.push_back(sylvan_var(x.GetDD()));
    x = Bdd(sylvan_high(x.GetDD()));
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
