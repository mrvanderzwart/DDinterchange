#ifndef SYLVAN_DD_H
#define SYLVAN_DD_H

#include <iostream>
#include <stdint.h>
#include <vector>

#include "../src/sylvan.h"
#include "sddextra.h"
extern "C"
{
  #include "../../../../Downloads/libsdd-2.0/sddapi.h"
}

const bool DDchoice = true; //SDD
//const bool DDchoice = false; //BDD

using namespace std;

namespace sylvan
{

typedef SddNode* SDD;
typedef std::conditional<DDchoice,SDD,BDD>::type DD;

class DecisionDiagram
{
  public:
    void start() { cout << "hallo\n"; };
    
};

class BddSet;

class Bdd : public DecisionDiagram
{ 
  friend class BddSet;

  public:
    Bdd() { bdd = sylvan_false; sylvan_protect(&bdd); }
    Bdd(const BDD from) : bdd(from) { sylvan_protect(&bdd); }
    Bdd(const Bdd &from) : bdd(from.bdd) { sylvan_protect(&bdd); }
    Bdd(const uint32_t var) { bdd = sylvan_ithvar(var); sylvan_protect(&bdd); }
    Bdd(const bool leaf) : bdd(leaf?sylvan_true:sylvan_false) { sylvan_protect(&bdd); }
    ~Bdd() { sylvan_unprotect(&bdd); }
    
    static const Bdd ddOne();
    static const Bdd ddZero();
    static Bdd ddVar(uint32_t index);
    Bdd& operator= (const Bdd& other);
    Bdd  operator& (const Bdd& other) const;
    Bdd& operator&=(const Bdd& other);
    Bdd  operator| (const Bdd& other) const;
    Bdd& operator|=(const Bdd& other);
    Bdd  operator* (const Bdd& other) const;
    Bdd& operator*=(const Bdd& other);
    Bdd  operator! ()                 const;
    
    BDD GetDD() const;
    Bdd Xnor(const Bdd& g) const;
    Bdd Then() const;
    Bdd RelNext(const Bdd& relation, const BddSet& cube) const;
    
    size_t NodeCount() const;
    
    int isOne() const;
    
    uint32_t TopVar() const;
    
  private:
    BDD bdd;
};

class BddSet
{
  friend class Bdd;
  Bdd set;
  
  public:
    BddSet() : set(Bdd::ddOne()) {}
    BddSet(const Bdd& other) : set(other) {}
    BddSet(const BddSet& other) : set(other.set) {}
  
    BDD GetDD() const { return set.bdd; }
    void add(BDDVAR variable) { set *= Bdd::ddVar(variable); }
    void add(BddSet &other) { set *= other.set; }
    bool isEmpty() const { return set.isOne(); }
    uint32_t TopVar() const { return set.TopVar(); }
    BddSet Next() const
    {
      Bdd then = set.Then();
      return BddSet(then);
    }
    bool contains(uint32_t variable) const 
    {
      if (isEmpty()) return false;
      else if (TopVar() == variable) return true;
      else return Next().contains(variable);
    }
    std::vector<uint32_t> toVector() const 
    {
      std::vector<uint32_t> result;
      Bdd x = set;
      while (!x.isOne()) {
        result.push_back(x.TopVar());
        x = x.Then();
      }
      return result;
    }

  private:
    
};

class SddSet;

class Sdd : public DecisionDiagram
{ 
  friend class SddSet;

  public:
  
    Sdd() { sdd = sdd_manager_false(Manager); sdd_ref(sdd, Manager); }
    Sdd(const SDD from) : sdd(from) { sdd_ref(from, Manager); }
    Sdd(const SddLiteral from) { sdd = sdd_manager_literal(from, Manager); sdd_ref(sdd, Manager); }
    Sdd(const Sdd& from) : sdd(from.sdd) { sdd_ref(from.sdd, Manager); }
    ~Sdd() { sdd_deref(sdd, Manager); }
    
    static const Sdd ddOne();
    static const Sdd ddZero();
    static Sdd ddVar(SddLiteral index);
    Sdd& operator= (const Sdd& other);
    Sdd  operator& (const Sdd& other) const;
    Sdd& operator&=(const Sdd& other);
    Sdd  operator| (const Sdd& other) const;
    Sdd& operator|=(const Sdd& other);
    Sdd  operator* (const Sdd& other) const;
    Sdd& operator*=(const Sdd& other);
    Sdd  operator! ()                 const;
    
    SDD GetDD() const;
    Sdd Xnor(const Sdd& g) const;
    Sdd Then() const;
    Sdd RelNext(const Sdd& relation, const SddSet& cube) const;
    
    size_t NodeCount() const;
    
    int isOne() const;
    
    SddLiteral TopVar() const;
    
    static SddManager* Manager;
    
  private:
    SDD sdd;
    SddExtra ex;
};

class SddSet
{
  friend class Sdd;
  Sdd set;
  
  public:
    SddSet() : set(Sdd::ddOne()) {}
    SddSet(const Sdd& other) : set(other) {}
    SddSet(const SddSet& other) : set(other.set) {}
  
    SDD GetDD() const { return set.sdd; }
    void add(SddLiteral variable) { set *= Sdd::ddVar(variable); }
    void add(SddSet &other) { set *= other.set; }
    bool isEmpty() const { return set.isOne(); }
    SddLiteral TopVar() const { return set.TopVar(); }
    SddSet Next() const
    {
      Sdd then = set.Then();
      return SddSet(then);
    }
    bool contains(SddLiteral variable) const 
    {
      if (isEmpty()) return false;
      else if (TopVar() == variable) return true;
      else return Next().contains(variable);;
      return false;
    }
    std::vector<SddLiteral> toVector() const 
    {
      std::vector<SddLiteral> result;
      Sdd x = set;
      while (!x.isOne()) {
        result.push_back(x.TopVar());
        x = x.Then();
      }
      return result;
    }

  private:
    
};

}

#endif
