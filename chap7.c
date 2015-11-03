/* 
 *
 *  typedef struct mpc_ast_t {
 *      char* tag;
 *      char* contents;
 *      mpc_state_t state;
 *      int children_num;
 *      struct mpc_ast_t** children;
 *  } mpc_ast_t;
 *  - `tag` field is a string containing a list of all the rules used to parse that particular item
 *  - `contents` contains actual contents of the node, empty for branches (rules)
 *  -`state` contains information about what state the parser was in when the node was found, such as
 *    line number and column number
 *  To traverse the tree we need children_num and children. The type of `children` is mpc_ast_t**;
 *  We can access a child node by accessing this field using array notation. Because the mpc_ast_t* is a
 *  pointer to a struct we need to use -> notation rather than dots
 *
*/
