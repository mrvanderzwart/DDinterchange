#ifndef DD_EXCHANGE_H
#define DD_EXCHANGE_H

#include <sylvan.h>
#include "sdd_essentials.hpp"

#include <algorithm>

namespace sylvan
{

template <typename T>
class DD;
class Bdd;
class Sdd;
class BddSet;
class SddSet;

/* dd_choice set to 1 for Bdd
 * dd_choice set to 0 for Sdd
*/
const int dd_choice = 0;
typedef std::conditional<dd_choice,DD<Bdd>,DD<Sdd>>::type DecisionDiagram;
typedef std::conditional<dd_choice,Bdd,Sdd>::type dd;
typedef std::conditional<dd_choice,BDDVAR,SddLiteral>::type ddvar;
typedef std::conditional<dd_choice,BddSet,SddSet>::type ddset;
typedef std::conditional<dd_choice,uint8_t*,SddLiteral*>::type printtype;

template <typename T>
class DD
{
  public:  
    /* FACTORY */
    static T create(T dd)
    {
      return T(dd);
    }    
    static const T ddOne()
    {
      return T::ddOne();
    }    
    static const T ddZero()
    {
      return T::ddZero();
    }
    static T ddVar(auto var)
    {
      return T::ddVar(var);
    }
    /* */

    virtual int operator==(const T& other) const = 0;
    virtual int operator!=(const T& other) const = 0;
    virtual T&  operator= (const T& other) = 0;
    virtual T   operator& (const T& other) const = 0;
    virtual T&  operator&=(const T& other) = 0;
    virtual T   operator| (const T& other) const = 0;
    virtual T&  operator|=(const T& other) = 0;
    virtual T   operator- (const T& other) const = 0;
    virtual T   operator* (const T& other) const = 0;
    virtual T&  operator*=(const T& other) = 0;
    virtual T   operator! () const = 0;   
};

class Bdd : public DD<Bdd>
{ 

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
    int  operator!=(const Bdd& other) const override;
    Bdd& operator= (const Bdd& other) override;
    Bdd  operator& (const Bdd& other) const override;
    Bdd& operator&=(const Bdd& other) override;
    Bdd  operator| (const Bdd& other) const override;
    Bdd& operator|=(const Bdd& other) override;
    Bdd  operator- (const Bdd& other) const override;
    Bdd  operator* (const Bdd& other) const override;
    Bdd& operator*=(const Bdd& other) override;
    Bdd  operator! () const override;
    
    BDD GetDD() const;
    Bdd RelNext(const Bdd& relation, const BddSet& cube) const;   
    size_t NodeCount() const;
    void SetEnum(const BddSet& SourceVars, void (*funcptr)(void*,std::vector<BDDVAR>,uint8_t*,int), void *ctx);
    
  private:
    BDD bdd;
};

class Sdd : public DD<Sdd>
{ 

  public:
  
    Sdd() { sdd = sdd_manager_false(SddEssentials::Manager); sdd_ref(sdd, SddEssentials::Manager); }
    Sdd(const SDD from) : sdd(from) { sdd_ref(from, SddEssentials::Manager); }
    Sdd(const SddLiteral from) { sdd = sdd_manager_literal(from, SddEssentials::Manager); sdd_ref(sdd, SddEssentials::Manager); }
    Sdd(const Sdd& from) : sdd(from.sdd) { sdd_ref(from.sdd, SddEssentials::Manager); }

    ~Sdd() { }
    
    static const Sdd ddOne();
    static const Sdd ddZero();
    static Sdd ddVar(SddLiteral index);
    
    int  operator==(const Sdd& other) const override;
    int  operator!=(const Sdd& other) const override;
    Sdd& operator= (const Sdd& other) override;
    Sdd  operator& (const Sdd& other) const override;
    Sdd& operator&=(const Sdd& other) override;
    Sdd  operator| (const Sdd& other) const override;
    Sdd& operator|=(const Sdd& other) override;
    Sdd  operator- (const Sdd& other) const override;
    Sdd  operator* (const Sdd& other) const override;
    Sdd& operator*=(const Sdd& other) override;
    Sdd  operator! () const override;
    
    SDD GetDD() const;
    Sdd RelNext(const Sdd& relation, const SddSet& cube) const;  
    size_t NodeCount() const;
    void SetEnum(const SddSet& SourceVars, void (*funcptr)(void*,std::vector<SddLiteral>,SddLiteral*,int), void *ctx);

  private:
    SDD sdd;
};

template <typename T1, typename T2>
class VariableSet
{
  public:
    virtual void add(T2 &variable) = 0;
    virtual void add(T1 &other) = 0;
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
    
    BDD GetDD() const;
    size_t size() const;
    
    void add(BDDVAR &variable) override;
    void add(BddSet &other) override;
    bool contains(BDDVAR variable) const override;
    std::vector<BDDVAR> toVector() const override;
};

class SddSet : public VariableSet<SddSet,SddLiteral>
{
  friend class Sdd;
  std::vector<SddLiteral> set;
  
  public:
    SddSet() : set() {}
    
    void add(SddLiteral &variable) override;
    void add(SddSet &other) override;
    bool contains(SddLiteral variable) const override;
    std::vector<SddLiteral> toVector() const override;
};

}

#endif
