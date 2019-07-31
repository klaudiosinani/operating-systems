#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "tree.h"

#define BUFF_SIZE 1024

static void __print_tree(struct tree_node *root, int level) {
  int i;
  for (i=0; i<level; i++)
    printf("\t");
  printf("%s\n", root->name);

  for (i=0; i < root->nr_children; i++){
    __print_tree(root->children + i, level + 1);
  }
}

void print_tree(struct tree_node *root) {
  __print_tree(root, 0);
}

static char * read_line(FILE *file, char *buff, size_t buff_size) {
  char *ret;
  size_t ret_len;

  ret = fgets(buff, buff_size, file);

  /* sanity check */
  if (ret == NULL){
    return ret;
  }

  ret_len = strlen(ret);
  if (ret_len == buff_size - 1){
    fprintf(stderr, "line too long: %s\n", buff);
    exit(1);
  }

  if (ret_len > 0)
    buff[ret_len - 1] = '\0'; /* remove \n */

  return ret;
}

static char *
read_empty_line(FILE *file, char *buff, size_t buff_size) {
  char *ret;
  ret = read_line(file, buff, buff_size);
  if (ret != NULL && strlen(ret) != 0){
    fprintf(stderr, "expecting an empty line: %s", buff);
    exit(1);
  }

  return ret;
}

static char *
read_non_empty_line(FILE *file, char *buff, size_t buff_size) {
  char *ret;
  ret = read_line(file, buff, buff_size);

  if (ret == NULL){
    fprintf(stderr, "unexpected EOF\n");
    exit(1);
  }

  if (strlen(ret) == 0){
    fprintf(stderr, "Unexpected empty line\n");
    exit(1);
  }

  return ret;
}

static char *
find_block_start(FILE *file, char *buff, size_t buff_size) {
  char *line;
  for (;;){
    line = read_line(file, buff, buff_size);
    if (line == NULL) /* EOF */
      break;
    if (strlen(line) == 0 || line[0] == '#')
      continue;  /* comment or empty line */
    else
      break;
  }

  return line;
}

/*
 * recursively parse tree file, creating nodes
 */
static struct tree_node *
parse_node(FILE *file, struct tree_node *node) {
  char buff[BUFF_SIZE], *name, *num_str;
  unsigned nr_children;
  int i;

  name = find_block_start(file, buff, BUFF_SIZE);
  if (name == NULL){ /* EOF */
     /* empty file, do nothing */
    if (node == NULL)
      return NULL;
    /* otherwise, terminate parsing */
    fprintf(stderr, "expecting: %s and got EOF\n", node->name);
    exit(1);
  }

  /* If no node given, allocate one -- this is used for root
   * If node is given, check that the names match */
  if (node == NULL){
    node = calloc(1, sizeof(struct tree_node));
    if (node == NULL){
      fprintf(stderr, "node allocation failed\n");
      exit(1);
    }
    snprintf(node->name, NODE_NAME_SIZE, "%s", name);
  } else if (strncmp(node->name, name, NODE_NAME_SIZE) != 0){
    fprintf(stderr, "nodes must be placed in a DFS order\n");
    fprintf(stderr, "expecting: %s and got: %s\n", node->name, name);
    exit(1);
  }

  /* read number of children */
  num_str = read_non_empty_line(file, buff, BUFF_SIZE);
  nr_children = node->nr_children = atol(num_str);

  /* allocate children */
  if (nr_children != 0){
    node->children = malloc(sizeof(struct tree_node)*nr_children);
    if (node->children == NULL){
      fprintf(stderr, "allocate children failed\n");
      exit(1);
    }
  }

  /* read children names */
  for (i=0; i<nr_children; i++){
    name = read_non_empty_line(file, buff, BUFF_SIZE);
    snprintf(node->children[i].name, NODE_NAME_SIZE, "%s", name);
    //printf("%s\n", node->children[i].name);
  }

  read_empty_line(file, buff, BUFF_SIZE);

  /* parse children */
  for (i=0; i<nr_children; i++){
    parse_node(file, &node->children[i]);
  }

  return node;
}


struct tree_node *
get_tree_from_file(const char *filename) {
  FILE *file;
  assert(BUFF_SIZE >= NODE_NAME_SIZE);
  struct tree_node *root;

  file = fopen(filename, "r");
  if (file == NULL){
    perror(filename);
    exit(1);
  }

  root = parse_node(file, NULL);

  fclose(file);

  return root;
}
