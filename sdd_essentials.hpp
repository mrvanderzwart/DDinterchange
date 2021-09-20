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
#include <stdio.h>

typedef SddNode* SDD;

class SddExtra
{
  public:
  
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
      int* e;
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
    
    static void sdd_mit_init_prime(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) 
    {
    	if (tree == 0 || sdd_vtree_is_leaf(tree)) return;
	    struct sdd_model_iterator* it = &(mas->nodes[sdd_vtree_position(tree)]);
	    Vtree* root_tree = (it->root == 0) ? 0 : sdd_vtree_of(it->root);
	    Vtree* right_tree = sdd_vtree_right(tree);
	    if (it->root == 0) 
		    sdd_get_iterator_rec(mas, 0, right_tree, Manager);
	    else if (root_tree == tree) 
	    {
		    SddNode* prime = sdd_node_elements(it->root)[2*(it->i)+1];
		    if (prime == sdd_manager_true(Manager)) 
			    sdd_get_iterator_rec(mas, 0, right_tree, Manager);	
		    else 
			    sdd_get_iterator_rec(mas, prime, right_tree, Manager);
      } 
      else if (sdd_vtree_is_sub(root_tree, right_tree)) 
		    sdd_get_iterator_rec(mas, it->root, right_tree, Manager);
	    else 
        sdd_get_iterator_rec(mas, 0, right_tree, Manager);	
    }
    
    static void sdd_mit_init_sub(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) 
    {
    	if (tree == 0 || sdd_vtree_is_leaf(tree)) return;
	    struct sdd_model_iterator* it = &(mas->nodes[sdd_vtree_position(tree)]);
	    Vtree* root_tree = (it->root == 0) ? 0 : sdd_vtree_of(it->root);
	    Vtree* left_tree = sdd_vtree_left(tree);
	    if (it->root == 0) 
		    sdd_get_iterator_rec(mas, 0, left_tree, Manager);
	    else if (root_tree == tree) 
		    sdd_get_iterator_rec(mas, sdd_node_elements(it->root)[2*(it->i)], left_tree, Manager);
	    else if (sdd_vtree_is_sub(root_tree, left_tree)) 
		    sdd_get_iterator_rec(mas, it->root, left_tree, Manager);
	    else 
		    sdd_get_iterator_rec(mas, 0, left_tree, Manager);	
    }
    
    static void sdd_mit_skip_false_prime(struct sdd_model_iterator* it) 
    {
    	if (it->root != 0 && sdd_node_is_decision(it->root) && it->i < sdd_node_size(it->root)) {
		    SddNode* prime = sdd_node_elements(it->root)[2*(it->i)+1];
		    if (sdd_node_is_false(prime)) 
			    it->i++;
	    }
    }
    
    static void sdd_get_iterator_rec(struct sdd_mit_master* mas, SddNode* root, Vtree* tree, SddManager* Manager) 
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
		    sdd_mit_init_prime(mas, tree, Manager);
		    sdd_mit_init_sub(mas, tree, Manager);
	    }   
    }
    
    static struct sdd_mit_master sdd_get_iterator(SddNode* set, SddManager* Manager) 
    {
    	struct sdd_mit_master mas;
	    mas.set = set;
	    mas.e = (int*) malloc(sizeof(int) * sdd_manager_var_count(Manager));
	    for (int i = 0; i < sdd_manager_var_count(Manager); i++) 
		    mas.e[i] = 0;
	    mas.finished = 0;
	    mas.nodes = (struct sdd_model_iterator*) malloc(sizeof(struct sdd_model_iterator) * 
	                 2*sdd_manager_var_count(Manager));
	    sdd_get_iterator_rec(&mas, set, sdd_manager_vtree(Manager), Manager);
	    return mas;
    }
    
    static void sdd_mit_free(struct sdd_mit_master mas) 
    {
    	free(mas.e);
	    free(mas.nodes);
    }
    
    static void sdd_next_model_rec(struct sdd_mit_master* mas, Vtree* tree, SddManager* Manager) 
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

	    sdd_next_model_rec(mas, sdd_vtree_right(tree), Manager);
	    SddLiteral primeid = sdd_vtree_position(sdd_vtree_right(tree));
	    if (mas->nodes[primeid].finished == 1) {
		    SddLiteral subid   = sdd_vtree_position(sdd_vtree_left(tree));
		    sdd_next_model_rec(mas, sdd_vtree_left(tree), Manager);
		    if (mas->nodes[subid].finished == 0) 
			    sdd_get_iterator_rec(mas, mas->nodes[primeid].root, sdd_vtree_right(tree), Manager);
		    else if (it->root != 0 && sdd_vtree_of(it->root) == tree) 
		    {
			    it->i++;
			    sdd_mit_skip_false_prime(it);
			    if (it->i < sdd_node_size(it->root)) 
			    {
				    sdd_mit_init_prime(mas, tree, Manager);
				    sdd_mit_init_sub(mas, tree, Manager);
			    }
			    else 
				    it->finished = 1;
		    }
		    else
			    it->finished = 1;
	    }
    }
    
    static void sdd_next_model(struct sdd_mit_master* mas, SddManager* Manager) 
    {
    	sdd_next_model_rec(mas, sdd_manager_vtree(Manager), Manager);
	    mas->finished = mas->nodes[sdd_vtree_position(sdd_manager_vtree(Manager))].finished;
    }
    
    static void set_enum(SddNode* set, SddManager* Manager, void (*funcptr)(int*))
    {
      if (sdd_node_is_false(set)) 
		    printf("    (empty)\n");
	    else 
	    {
		    struct sdd_mit_master mas;
		    for (mas = sdd_get_iterator(set, Manager); mas.finished == 0; sdd_next_model(&mas, Manager)) 
		    {
			    (*funcptr)(mas.e);
		    }
		    sdd_mit_free(mas);
	    }
    }

};

#endif
