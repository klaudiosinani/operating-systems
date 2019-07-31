#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

int main(int argc, char *argv[]) {
  struct tree_node *root;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
    exit(1);
  }

  root = get_tree_from_file(argv[1]);
  print_tree(root);

  return 0;
}
