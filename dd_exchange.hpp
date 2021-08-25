#ifndef DD_EXCHANGE_H
#define DD_EXCHANGE_H

#include <sylvan.h>
#include "sdd_essentials.hpp"

#include <vector>
#include <stdio.h>

namespace sylvan
{

template <typename T1, typename T2>
class DD;
class Bdd;
class Sdd;
class BddSet;
class SddSet;

/* dd_choice set to true for Bdd
 * dd_choice set to false for Sdd
*/
const bool dd_choice = false;
typedef std::conditional<dd_choice,
                         DD<Bdd,BDDVAR>,
                         DD<Sdd,SddLiteral>>::type DecisionDiagram;
typedef std::conditional<dd_choice,Bdd,Sdd>::type dd;
typedef std::conditional<dd_choice,BDDVAR,SddLiteral>::type ddvar;
typedef std::conditional<dd_choice,BddSet,SddSet>::type ddset;

template <typename T1, typename T2>
class DD
{
  public:  
    /* FACTORY */
    static T1 create(T1 DD)
    {
      return T1(DD);
    }    
    static const T1 ddOne()
    {
      return T1::ddOne();
    }    
    static const T1 ddZero()
    {
      return T1::ddZero();
    }
    static T1 ddVar(T2 var)
    {
      return T1::ddVar(var);
    }
    /* */
    
    virtual int operator==(const T1& other) const = 0;
    virtual T1&  operator= (const T1& other) = 0;
    virtual T1   operator& (const T1& other) const = 0;
    virtual T1&  operator&=(const T1& other) = 0;
    virtual T1   operator| (const T1& other) const = 0;
    virtual T1&  operator|=(const T1& other) = 0;
    virtual T1   operator* (const T1& other) const = 0;
    virtual T1&  operator*=(const T1& other) = 0;
    virtual T1   operator! () const = 0;   
    virtual T1 Xnor(const T1& g) const = 0;
    virtual T1 Then() const = 0;    
    virtual size_t NodeCount() const = 0;  
    virtual int isOne() const = 0;
};

class Bdd : public DD<Bdd,BDDVAR>
{ 
  friend class BddSet;

  public:
    Bdd() 
    { 
      /* Init BDD package only once */
      static bool init_bdd = init();
      bdd = sylvan_false; sylvan_protect(&bdd); 
    }
    Bdd(const BDD from) : bdd(from) { sylvan_protect(&bdd); }
    Bdd(const Bdd &from) : bdd(from.bdd) { sylvan_protect(&bdd); }
    Bdd(const uint32_t var) { bdd = sylvan_ithvar(var); sylvan_protect(&bdd); }

    ~Bdd() { sylvan_unprotect(&bdd); }
    
    bool init()
    {
      lace_init(0, 0);
      lace_startup(0, NULL, NULL);
      LACE_ME;
      sylvan_set_sizes(1ULL<<22, 1ULL<<27, 1ULL<<20, 1ULL<<26);
      sylvan_init_package();
      sylvan_init_bdd();
      return true;
    }
    
    static const Bdd ddOne();
    static const Bdd ddZero();
    static Bdd ddVar(BDDVAR index);
    
    int  operator==(const Bdd& other) const override;
    Bdd& operator= (const Bdd& other) override;
    Bdd  operator& (const Bdd& other) const override;
    Bdd& operator&=(const Bdd& other) override;
    Bdd  operator| (const Bdd& other) const override;
    Bdd& operator|=(const Bdd& other) override;
    Bdd  operator* (const Bdd& other) const override;
    Bdd& operator*=(const Bdd& other) override;
    Bdd  operator! () const override;
    
    BDD GetDD() const;
    Bdd Xnor(const Bdd& g) const override;
    Bdd Then() const override;
    Bdd RelNext(const Bdd& relation, const BddSet& cube) const;
    
    size_t NodeCount() const;
    
    int isOne() const override;
    
    BDDVAR TopVar() const;
    
  private:
    BDD bdd;
};

class Sdd : public DD<Sdd,SddLiteral>
{ 
  friend class SddSet;

  public:
    Sdd() { sdd = sdd_manager_false(Manager); sdd_ref(sdd, Manager); }
    Sdd(const SDD from) : sdd(from) { sdd_ref(from, Manager); }
    Sdd(const SddLiteral from) { sdd = sdd_manager_literal(from, Manager); sdd_ref(sdd, Manager); }
    Sdd(const Sdd& from) : sdd(from.sdd) { sdd_ref(from.sdd, Manager); }

    ~Sdd() { }
    
    static const Sdd ddOne();
    static const Sdd ddZero();
    static Sdd ddVar(SddLiteral index);
    
    int  operator==(const Sdd& other) const override;
    Sdd& operator= (const Sdd& other) override;
    Sdd  operator& (const Sdd& other) const override;
    Sdd& operator&=(const Sdd& other) override;
    Sdd  operator| (const Sdd& other) const override;
    Sdd& operator|=(const Sdd& other) override;
    Sdd  operator* (const Sdd& other) const override;
    Sdd& operator*=(const Sdd& other) override;
    Sdd  operator! () const override;
    
    SDD GetDD() const;
    Sdd Xnor(const Sdd& g) const override;
    Sdd Then() const override;
    Sdd RelNext(const Sdd& relation, const SddSet& cube) const;
    
    size_t NodeCount() const override;
    
    int isOne() const override;
    
    SddLiteral TopVar() const;
    
    static SddManager* Manager;
    
  private:
    SDD sdd;
};

template <typename T1, typename T2>
class VariableSet
{
  public:
    virtual void add(T2 variable) = 0;
    virtual void add(T1& other) = 0;
    virtual bool isEmpty() const = 0;
    virtual T2 TopVar() const = 0;
    virtual T1 Next() const = 0;
    virtual bool contains(T2 variable) const = 0;
    virtual std::vector<T2> toVector() const = 0;
};

class BddSet : public VariableSet<BddSet,BDDVAR>
{
  friend class Bdd;
  Bdd set;

  public:
    BddSet() : set(Bdd::ddOne()) {}
    BddSet(const Bdd& other) : set(other) {}
    BddSet(const BddSet& other) : set(other.set) {}
    
    BDD GetDD() const;
    void add(BDDVAR variable) override;
    void add(BddSet& other) override;
    bool isEmpty() const override;
    BDDVAR TopVar() const override;
    BddSet Next() const override;
    bool contains(BDDVAR variable) const override;
    std::vector<BDDVAR> toVector() const override;
};

class SddSet : public VariableSet<SddSet,SddLiteral>
{
  friend class Sdd;
  Sdd set;
  
  public:
    SddSet() : set(Sdd::ddOne()) {}
    SddSet(const Sdd& other) : set(other) {}
    SddSet(const SddSet& other) : set(other.set) {}
  
    SDD GetDD() const;
    void add(SddLiteral variable) override;
    void add(SddSet &other) override;
    bool isEmpty() const override;
    SddLiteral TopVar() const override;
    SddSet Next() const override;
    bool contains(SddLiteral variable) const override;
    std::vector<SddLiteral> toVector() const override;
};

}

#endif
