#ifndef SDD_ESSENTIALS_H
#define SDD_ESSENTIALS_H

/**
  * All Rights Reserved To Lieuwe Vinkhuijzen
  * https://zenodo.org/record/3940936#.YOy93nUzZhE
**/

extern "C"
{
  #include <sddapi.h>
}
#include "dd_exchange.hpp"
#include <stdio.h>
#include <vector>

namespace interchange
{

typedef SddNode* SDD;

class SddSatEnum
{

  public:  
  
    static SddManager* Manager1;
    
    //void SetManager(SddManager* Manager)
    //{
      //Manager1 = Manager;
    //}
  
    struct sdd_model_iterator
    {
      SddNode* root;
      unsigned int i;
      int finished;
      unsigned int var_is_used;
    };

    struct sdd_mit_master
    {
      SddNode* set;
      struct sdd_model_iterator* nodes;
      SddLiteral* e;
      unsigned int finished;
    };
  
    static SddLiteral vtree_highest_var(Vtree* tree) 
    {
    	if (tree == 0) return 0;
	    else if (sdd_vtree_is_leaf(tree)) 
	    {
		    SddLiteral v = sdd_vtree_var(tree);
		    if (v % 2 == 0) return 0;
		    else return (((unsigned int)v)-1) / 2;
	    }
	    else 
	    {
		    SddLiteral s, p;
		    s = vtree_highest_var(sdd_vtree_left(tree));
		    p = vtree_highest_var(sdd_vtree_right(tree));
		    return s > p ? s : p;
	    }
    }
    
    static void sdd_mit_init_prime(struct sdd_mit_master* mas, Vtree* tree) 
    {
    	if (tree == 0 || sdd_vtree_is_leaf(tree)) return;
	    struct sdd_model_iterator* it = &(mas->nodes[sdd_vtree_position(tree)]);
	    Vtree* root_tree = (it->root == 0) ? 0 : sdd_vtree_of(it->root);
	    Vtree* right_tree = sdd_vtree_right(tree);
	    if (it->root == 0) 
		    sdd_get_iterator_rec(mas, 0, right_tree);
	    else if (root_tree == tree) 
	    {
		    SddNode* prime = sdd_node_elements(it->root)[2*(it->i)+1];
		    if (prime == sdd_manager_true(Manager1)) 
			    sdd_get_iterator_rec(mas, 0, right_tree);	
		    else 
			    sdd_get_iterator_rec(mas, prime, right_tree);
      } 
      else if (sdd_vtree_is_sub(root_tree, right_tree)) 
		    sdd_get_iterator_rec(mas, it->root, right_tree);
	    else 
        sdd_get_iterator_rec(mas, 0, right_tree);	
    }
    
    static void sdd_mit_init_sub(struct sdd_mit_master* mas, Vtree* tree) 
    {
    	if (tree == 0 || sdd_vtree_is_leaf(tree)) return;
	    struct sdd_model_iterator* it = &(mas->nodes[sdd_vtree_position(tree)]);
	    Vtree* root_tree = (it->root == 0) ? 0 : sdd_vtree_of(it->root);
	    Vtree* left_tree = sdd_vtree_left(tree);
	    if (it->root == 0) 
		    sdd_get_iterator_rec(mas, 0, left_tree);
	    else if (root_tree == tree) 
		    sdd_get_iterator_rec(mas, sdd_node_elements(it->root)[2*(it->i)], left_tree);
	    else if (sdd_vtree_is_sub(root_tree, left_tree)) 
		    sdd_get_iterator_rec(mas, it->root, left_tree);
	    else 
		    sdd_get_iterator_rec(mas, 0, left_tree);	
    }
    
    static void sdd_mit_skip_false_prime(struct sdd_model_iterator* it) 
    {
    	if (it->root != 0 && sdd_node_is_decision(it->root) && it->i < sdd_node_size(it->root)) {
		    SddNode* prime = sdd_node_elements(it->root)[2*(it->i)+1];
		    if (sdd_node_is_false(prime)) 
			    it->i++;
	    }
    }
    
    static void sdd_get_iterator_rec(struct sdd_mit_master* mas, SddNode* root, Vtree* tree) 
    {
    	struct sdd_model_iterator* it = &(mas->nodes[sdd_vtree_position(tree)]);
	    it->root = root;
	    it->i = 0;
	    it->finished = 0;
	    it->var_is_used = 0;
	    sdd_mit_skip_false_prime(it);
	    if (sdd_vtree_is_leaf(tree)) 
	    {
		    SddLiteral var = sdd_vtree_var(tree);
		    if (it->root != 0)
		    {
			    it->i = sdd_node_literal(it->root) > 0 ? 1 : 0;
			    it->var_is_used = 1;
		    }
	      mas->e[(var-1)/2] = it->i;
	    }
	    else 
	    {
		    sdd_mit_init_prime(mas, tree);
		    sdd_mit_init_sub(mas, tree);
	    }   
    }
    
    static struct sdd_mit_master sdd_get_iterator(SddNode* set) 
    {
    	struct sdd_mit_master mas;
	    mas.set = set;
	    mas.e = (SddLiteral*) malloc(sizeof(SddLiteral) * sdd_manager_var_count(Manager1));
	    for (int i = 0; i < sdd_manager_var_count(Manager1); i++) 
		    mas.e[i] = 0;
	    mas.finished = 0;
	    mas.nodes = (struct sdd_model_iterator*) malloc(sizeof(struct sdd_model_iterator) * 
	                 2*sdd_manager_var_count(Manager1));
	    sdd_get_iterator_rec(&mas, set, sdd_manager_vtree(Manager1));
	    return mas;
    }
    
    static void sdd_mit_free(struct sdd_mit_master mas) 
    {
    	free(mas.e);
	    free(mas.nodes);
    }
    
    static void sdd_next_model_rec(struct sdd_mit_master* mas, Vtree* tree) 
    {
    	SddLiteral treeid = sdd_vtree_position(tree);
	    struct sdd_model_iterator* it = &(mas->nodes[treeid]);
	    if (it->finished == 1) return;
	    if (sdd_vtree_is_leaf(tree)) 
	    {
		    SddLiteral var_sdd = sdd_vtree_var(tree);
		    SddLiteral var = (var_sdd - 1) / 2;
		    if (it->root != 0) 
			    it->finished = 1;
		    else if (var_sdd % 2 == 0) 
			    it->finished = 1;
		    else if (it->var_is_used && it->i == 0) 
		    {
			    it->i++;
			    mas->e[var] = 1;
		    }
		    else 
			    it->finished = 1;
		    return;
	    }

	    sdd_next_model_rec(mas, sdd_vtree_right(tree));
	    SddLiteral primeid = sdd_vtree_position(sdd_vtree_right(tree));
	    if (mas->nodes[primeid].finished == 1) {
		    SddLiteral subid   = sdd_vtree_position(sdd_vtree_left(tree));
		    sdd_next_model_rec(mas, sdd_vtree_left(tree));
		    if (mas->nodes[subid].finished == 0) 
			    sdd_get_iterator_rec(mas, mas->nodes[primeid].root, sdd_vtree_right(tree));
		    else if (it->root != 0 && sdd_vtree_of(it->root) == tree) 
		    {
			    it->i++;
			    sdd_mit_skip_false_prime(it);
			    if (it->i < sdd_node_size(it->root)) 
			    {
				    sdd_mit_init_prime(mas, tree);
				    sdd_mit_init_sub(mas, tree);
			    }
			    else 
				    it->finished = 1;
		    }
		    else
			    it->finished = 1;
	    }
    }
    
    static void sdd_next_model(struct sdd_mit_master* mas) 
    {
    	sdd_next_model_rec(mas, sdd_manager_vtree(Manager1));
	    mas->finished = mas->nodes[sdd_vtree_position(sdd_manager_vtree(Manager1))].finished;
    }
    
    static void set_enum(SddNode* set, std::vector<SddLiteral> sources, void (*funcptr)(void*,std::vector<SddLiteral>,SddLiteral*,int), void *ctx)
    {
      if (sdd_node_is_false(set)) 
		    printf("    (empty)\n");
	    else 
	    {
		    struct sdd_mit_master mas;
		    for (mas = sdd_get_iterator(set); mas.finished == 0; sdd_next_model(&mas)) 
		    {
			    (*funcptr)(ctx, sources, mas.e, sources.size());
		    }
		    sdd_mit_free(mas);
	    }
    }

};

}

#endif
