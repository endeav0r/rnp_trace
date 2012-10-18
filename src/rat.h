#ifndef rnp_aatree_HEADER
#define rnp_aatree_HEADER

#include <string.h>

/*
*  RainbowsAndPwnies AA Tree
*/

struct _rat_i {
    int level;
    void * data;
    struct _rat_i * left;
    struct _rat_i * right;
};

struct _rat {
    struct _rat_i * items;
    int (* cmp) (void *, void *);
};

struct _rat * rat_create  (int (* cmp) (void *, void *));
void          rat_destroy (struct _rat * rat);

int    rat_insert (struct _rat * rat, void * data, size_t data_len);
void * rat_search (struct _rat * rat, void * needle);

struct _rat_i * rat_i_insert  (struct _rat *   rat, 
                               struct _rat_i * tree,
                               struct _rat_i * new_rat_i);
struct _rat_i * rat_i_search  (struct _rat *   rat,
                               struct _rat_i * tree,
                               void *          needle);
struct _rat_i * rat_i_skew    (struct _rat_i *);
struct _rat_i * rat_i_split   (struct _rat_i *);
void            rat_i_destroy (struct _rat_i *);

#endif