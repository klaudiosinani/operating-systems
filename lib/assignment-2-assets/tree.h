#ifndef TREE_H
#define TREE_H

/*
 * Data structure definitions
 */

#define NODE_NAME_SIZE 16
/* tree node structure */
struct tree_node {
  unsigned          nr_children;
  char              name[NODE_NAME_SIZE];
  struct tree_node  *children;
};


/*
 * Helper Functions
 */

/* returns the root node of the tree defined in a file */
struct tree_node *get_tree_from_file(const char *filename);

void print_tree(struct tree_node *root);

#endif /* TREE_H */
