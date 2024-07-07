#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AVLNode {
    int height;
    int sub_tree_size;

    struct AVLNode *left;
    struct AVLNode *right;
    struct AVLNode *parent;
    
    // store a reference to hashNode(used in ZSet), used as a secondary index in the AVL tree, value is primary index, set to same value if not used for all nodes
    void * scnd_index;

    float value;
} AVLNode;

// configure the compare function for the secondary index, meant to see if two secondary indexes are equal
int compare_scnd_index(void * scnd_index1, void * scnd_index2);

AVLNode * avl_init(void * snd_index, float value);
AVLNode * avl_search_float(AVLNode * tree, float value);
AVLNode * avl_insert(AVLNode * tree, void * scnd_index, float value);
AVLNode * avl_delete(AVLNode * tree,  void * scnd_index, float value);
void avl_free(AVLNode * tree);
void avl_print(AVLNode * tree);


