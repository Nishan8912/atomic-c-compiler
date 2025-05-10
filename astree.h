#ifndef ASTREE_H
#define ASTREE_H

#include <stdio.h>
#include "symtable.h"

// AST Node Types
typedef enum {
    AST_PROGRAM,
    AST_VARDECL,
    AST_FUNCTION,
    AST_SBLOCK,
    AST_FUNCALL,
    AST_ASSIGNMENT,
    AST_WHILE,
    AST_DOWHILE,       // NEW: do-while loop
    AST_IFELSE,
    AST_EXPRESSION,
    AST_UNARY,         // NEW: unary operators
    AST_VARREF,
    AST_CONSTANT,
    AST_ARGUMENT,
    AST_RELEXPR,
    AST_ARRAYREF,
    AST_ARRAYASSIGN
} ASTNodeType;

#define ASTNUMCHILDREN 3

typedef struct astnode_s {
    ASTNodeType type;
    DataType valType;
    SymbolKind varKind;
    int ival;
    char *strval;
    int strNeedsFreed;
    struct astnode_s *next;
    struct astnode_s *child[ASTNUMCHILDREN];
} ASTNode;

ASTNode* newASTNode(ASTNodeType type);
void freeASTree(ASTNode* tree);
void printASTree(ASTNode* tree, int level, FILE *out);
void genCodeFromASTree(ASTNode* tree, int hval, FILE *out);

#endif
