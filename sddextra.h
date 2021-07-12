#ifndef SYLVAN_OBJ_H
#define SYLVAN_OBJ_H

extern "C"
{
  #include "../../../../Downloads/libsdd-2.0/sddapi.h"
}

class SddExtra
{
  public:
    SddLiteral vtree_highest_var(Vtree* tree) const;
    void sdd_mit_init_prime(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) const;
    void sdd_mit_init_sub(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) const;
    void sdd_mit_skip_false_prime(struct sdd_model_iterator* it) const;
    void sdd_get_iterator_rec(struct sdd_mit_master* mas, SddNode* root, Vtree* tree, SddManager* Manager) const;
    struct sdd_mit_master sdd_get_iterator(SddNode* set, SddManager* Manager) const;
    void sdd_mit_free(struct sdd_mit_master mas) const;
    void sdd_next_model_rec(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) const;
    void sdd_next_model(struct sdd_mit_master* mas, SddManager* Manager) const;
    void set_enum(SddNode* set, SddManager* Manager, void (*funcptr)(int*)) const;
    
  private:

};

#endif
