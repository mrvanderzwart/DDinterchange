#ifndef DD_EXCHANGE_H
#define DD_EXCHANGE_H

#include <sylvan.h>
#include "sdd_sat_enum.hpp"

#include <algorithm>

using namespace sylvan;

namespace interchange
{

template <typename T>
class DD;
class Bdd;
class Sdd;
class BddSet;
class SddSet;

inline bool init_bdd = false;

/* dd_choice set to 1 for Bdd
 * dd_choice set to 0 for Sdd
*/
const int dd_choice = 1;

typedef std::conditional<dd_choice,DD<Bdd>,DD<Sdd>>::type DecisionDiagram;
typedef std::conditional<dd_choice,Bdd,Sdd>::type dd;
typedef std::conditional<dd_choice,BDDVAR,SddLiteral>::type ddvar;
typedef std::conditional<dd_choice,BddSet,SddSet>::type ddset;

template <typename T>
class DD
{
  public:  
    /* FACTORY */
    
    /**
     * return the Decision Diagram representing "True"
    */
    static const T ddOne()
    {
      return T::ddOne();
    }   
    
    /**
     * return the Decision Diagram representing "False"
    */ 
    static const T ddZero()
    {
      return T::ddZero();
    }
    
     /**
     * @brief returns a new Decision Diagram of a variable
     * @param dd the new variable
     * the variable must be of type BDDVAR or SddLiteral which is wrapped in 
     * type ddvar
    */
    static const T ddVar(auto var)
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
      if (not init_bdd)
        init();
      bdd = sylvan_false; sylvan_protect(&bdd); 
    }
    Bdd(const BDD from) : bdd(from) {  }
    Bdd(const Bdd &from) : bdd(from.bdd) { sylvan_protect(&bdd); }
    Bdd(const uint32_t var) 
    { 
      if (not init_bdd)
        init();
      bdd = sylvan_ithvar(var); 
      sylvan_protect(&bdd); 
    }

    ~Bdd() { sylvan_unprotect(&bdd); }
    
    /**
     * initializes the bdd package
     * called by static bool in constructor to only run once
    */
    void init()
    {
      lace_init(0, 0);
      lace_startup(0, NULL, NULL);
      LACE_ME;
      sylvan_set_sizes(1ULL<<22, 1ULL<<27, 1ULL<<20, 1ULL<<26);
      sylvan_init_package();
      sylvan_init_bdd();
      init_bdd = true;
    }
    
    static const Bdd ddOne();
    static const Bdd ddZero();
    static const Bdd ddVar(BDDVAR index);
    
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
    
    /**
     * @brief gets the BDD of this Bdd
    */
    BDD GetDD() const;
    
    /**
     * @brief computes the application of a transition relation to this set
     * @param relation the transition relation to apply
     * @param cube the variables that are in the transition relation
    */
    Bdd RelNext(const Bdd& relation, const BddSet& cube) const; 
    
    /**
     * @brief gets the number of satisfiable assignments of the Bdd
     * @param variables the variables in the Bdd
    */
    double SatCount(const BddSet &variables) const;
    
    /**
     * @brief gets the number of nodes in this Bdd
    */
    size_t NodeCount() const;
    
    /**
     * @brief iterates through all satisfying assignments and call a callback
     * function on each assignment
     * @param SourceVars the variables of the satisfying assignment
     * @param (*funcptr)() the callback function
     * @param *ctx the class from which the function is called
    */
    void SetEnum(const BddSet& SourceVars, void (*funcptr)(void*,std::vector<BDDVAR>,uint8_t*,int), void *ctx);
    
  private:
    BDD bdd;
};

class Sdd : public DD<Sdd>
{ 

  public:
  
    Sdd() { sdd = sdd_manager_false(SddSatEnum::Manager); sdd_ref(sdd, SddSatEnum::Manager); }
    Sdd(const SDD from) : sdd(from) { sdd_ref(from, SddSatEnum::Manager); }
    Sdd(const SddLiteral from) { sdd = sdd_manager_literal(from, SddSatEnum::Manager); sdd_ref(sdd, SddSatEnum::Manager); }
    Sdd(const Sdd& from) : sdd(from.sdd) { sdd_ref(from.sdd, SddSatEnum::Manager); }

    ~Sdd() { }
    
    static const Sdd ddOne();
    static const Sdd ddZero();
    static const Sdd ddVar(SddLiteral index);
    
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
    
    /**
     * @brief gets the Sdd of this Sdd
    */
    SDD GetDD() const;
    
    /**
     * @brief computes the application of a transition relation to this set
     * @param relation the transition relation to apply
     * @param cube the variables that are in the transition relation
    */
    Sdd RelNext(const Sdd& relation, const SddSet& cube) const;  
    
    /**
     * @brief gets the number of satisfiable assignments of the Sdd
     * @param the variables in the Sdd (ignored)
    */
    SddModelCount SatCount(SddSet& variables) const;
    
    /**
     * @brief gets the number of nodes in this Sdd
    */
    SddSize NodeCount() const;
    
    /**
     * @brief iterates through all satisfying assignments and call a callback
     * function on each assignment
     * @param SourceVars the variables of the satisfying assignment
     * @param (*funcptr)() the callback function
     * @param *ctx the class from which the function is called
    */
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
