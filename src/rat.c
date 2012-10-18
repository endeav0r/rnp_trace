#include "rat.h"

#include <stdlib.h>

struct _rat * rat_create (int (* cmp) (void *, void *))
{
	struct _rat * rat;

	rat = (struct _rat *) malloc(sizeof(struct _rat));
	rat->cmp = cmp;
	rat->items = NULL;

	return rat;
}


void rat_destroy (struct _rat * rat)
{
	rat_i_destroy(rat->items);
	free(rat);
}


int rat_insert (struct _rat * rat, void * data, size_t data_len)
{
	struct _rat_i * rat_i;

	rat_i = (struct _rat_i *) malloc(sizeof(struct _rat_i));
	rat_i->level = 0;
	rat_i->data  = malloc(data_len);
	memcpy(rat_i->data, data, data_len);
	rat_i->left  = NULL;
	rat_i->right = NULL;

	rat_i = rat_i_insert(rat, rat->items, rat_i);

	if (rat_i == NULL)
		return -1;

	rat->items = rat_i;
	return 0;
}


void * rat_search (struct _rat * rat, void * needle)
{
	struct _rat_i * rat_i;

	rat_i = rat_i_search(rat, rat->items, needle);
	if (rat_i != NULL)
		return rat_i->data;
	else
		return NULL;
}


struct _rat_i * rat_i_insert (struct _rat * rat, 
							  struct _rat_i * tree,
							  struct _rat_i * new_rat_i)
{
	if (tree == NULL)
		tree = new_rat_i;
	else if (rat->cmp(new_rat_i->data, tree->data) < 0)
		tree->left = rat_i_insert(rat, tree->left, new_rat_i);
	else if (rat->cmp(new_rat_i->data, tree->data) > 0)
		tree->right = rat_i_insert(rat, tree->right, new_rat_i);
	else
		return NULL;

	//tree = rat_i_skew(tree);
	//tree = rat_i_split(tree);

	return tree;
}


struct _rat_i * rat_i_search (struct _rat *   rat,
							  struct _rat_i * tree,
							  void *          needle)
{
	if (tree == NULL)
		return NULL;
	if (rat->cmp(needle, tree->data) == 0)
		return tree;
	else if (rat->cmp(needle, tree->data) < 0)
		return rat_i_search(rat, tree->left, needle);
	else if (rat->cmp(needle, tree->data) > 0)
		return rat_i_search(rat, tree->right, needle);
	return NULL;
}


struct _rat_i * rat_i_skew (struct _rat_i * tree)
{
	struct _rat_i * L;

	if (tree == NULL)
		return NULL;
	else if (tree->left == NULL)
		return tree;
	else if (tree->left->level == tree->level){
		L = tree->left;
		tree->left = tree->right;
		tree->right = L;
		return L;
	}
	return tree;
}


struct _rat_i * rat_i_split (struct _rat_i * tree)
{
	struct _rat_i * R;

	if (tree == NULL)
		return NULL;
	else if ((tree->right == NULL) || (tree->right->right == NULL))
		return NULL;
	else if (tree->right->right->level == tree->level) {
		R = tree->right;
		tree->right = R->left;;
		R->left = tree;
		R->level++;
		return R;
	}
	return tree;
}


void rat_i_destroy (struct _rat_i * tree)
{
	if (tree == NULL)
		return;
	rat_i_destroy(tree->left);
	rat_i_destroy(tree->right);
	free(tree->data);
	free(tree);
}