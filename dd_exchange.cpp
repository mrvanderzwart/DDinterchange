#include "dd_exchange.hpp"

using namespace interchange;

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

const Bdd
Bdd::ddVar(BDDVAR index)
{
  return Bdd(sylvan_ithvar(index));
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

double
Bdd::SatCount(const BddSet &variables) const
{
  LACE_ME;
  return sylvan_satcount(bdd, variables.set.bdd);
}

size_t
Bdd::NodeCount() const
{
  return sylvan_nodecount(bdd);
}

void
Bdd::SetEnum(const BddSet& SourceVars, void (*funcptr)(void*,std::vector<BDDVAR>,uint8_t*,int), void *ctx)
{
  BDD B = bdd;
  int size = SourceVars.size();
  uint8_t *arr = (uint8_t*)malloc(size);
  
  BDD V = mtbdd_enum_all_first(B, SourceVars.GetDD(), arr, nullptr);
  while (V != sylvan_false)
  {
    (*funcptr)(ctx, SourceVars.toVector(), arr, size);
    V = mtbdd_enum_all_next(B, SourceVars.GetDD(), arr, nullptr);
  }
  free(arr);
}

//-----------------------------------SDD--------------------------------------//

const Sdd
Sdd::ddOne()
{
  return Sdd(sdd_manager_true(SddSatEnum::Manager));
}
    
const Sdd 
Sdd::ddZero()
{
  return Sdd(sdd_manager_false(SddSatEnum::Manager));
}

const Sdd
Sdd::ddVar(SddLiteral index)
{
  return Sdd(sdd_manager_literal(index, SddSatEnum::Manager));
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
  return Sdd(sdd_conjoin(sdd, other.sdd, SddSatEnum::Manager));
}

Sdd&
Sdd::operator&=(const Sdd& other) 
{
  sdd = sdd_conjoin(sdd, other.sdd, SddSatEnum::Manager);
  return *this;
}

Sdd
Sdd::operator|(const Sdd& other) const
{
  return Sdd(sdd_disjoin(sdd, other.sdd, SddSatEnum::Manager));
}

Sdd&
Sdd::operator|=(const Sdd& other) 
{
  sdd = sdd_disjoin(sdd, other.sdd, SddSatEnum::Manager);
  return *this;
}

Sdd
Sdd::operator-(const Sdd& other) const
{
  return Sdd(sdd_disjoin(sdd_conjoin(sdd, other.sdd, SddSatEnum::Manager), sdd_conjoin(sdd_negate(sdd, SddSatEnum::Manager), sdd_negate(other.sdd, SddSatEnum::Manager), SddSatEnum::Manager), SddSatEnum::Manager));
}

Sdd
Sdd::operator*(const Sdd& other) const
{
  return Sdd(sdd_conjoin(sdd, other.sdd, SddSatEnum::Manager));
}

Sdd&
Sdd::operator*=(const Sdd& other)
{
  sdd = sdd_conjoin(sdd, other.sdd, SddSatEnum::Manager);
  return *this;
}

Sdd
Sdd::operator!() const
{
  return Sdd(sdd_negate(sdd, SddSatEnum::Manager));
}

SDD
Sdd::GetDD() const
{
  return sdd;
}

Sdd
Sdd::RelNext(const Sdd& relation, const SddSet& state) const
{
  int n_variables = sdd_manager_var_count(SddSatEnum::Manager);
  SDD Rn = sdd_conjoin(sdd, relation.sdd, SddSatEnum::Manager);
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

  SDD Sn = sdd_exists_multiple(exists_map, Rn, SddSatEnum::Manager);  
  return sdd_rename_variables(Sn, variable_map, SddSatEnum::Manager);
  
  free(exists_map);
  free(variable_map);
}

SddModelCount
Sdd::SatCount(SddSet& variables) const
{
  return sdd_model_count(sdd, SddSatEnum::Manager);
}

SddSize
Sdd::NodeCount() const
{
  return sdd_count(sdd);
}

void 
Sdd::SetEnum(const SddSet& SourceVars, void (*funcptr)(void*,std::vector<SddLiteral>,SddLiteral*,int), void *ctx)
{
  SddSatEnum::set_enum(sdd, SourceVars.toVector(), (*funcptr), ctx);
}

//---------------------------------BDDSET-------------------------------------//

BDD
BddSet::GetDD() const
{
  return set.GetDD();
}

size_t 
BddSet::size() const 
{
  if (set.GetDD() == sylvan_true) return 0;
  else return 1 + BddSet(Bdd(sylvan_high(set.GetDD()))).size();
}

void 
BddSet::add(BDDVAR &variable) 
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
SddSet::add(SddLiteral &variable) 
{ 
	static int init = 0;
	if (init < sdd_manager_var_count(SddSatEnum::Manager))
	{
		init++;
		variable++;
	}
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
