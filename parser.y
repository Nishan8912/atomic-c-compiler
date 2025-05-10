%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "astree.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(const char *s);
void yylex_destroy(void);

ASTNode* astRoot = NULL;
int debug = 0;
int currentOffset = 16;
%}

%union {
    int ival;
    char* str;
    struct astnode_s* astnode;
}

%start wholeprogram

%type <astnode> wholeprogram globals functions function program statements statement assignment funcall arguments argument expression ifthenelse whileloop dowhile returnstmt boolexpr localvars localdecl parameters paramdecl functionbody
%type <astnode> vardecl

%token <str> ID STRING
%token <ival> NUMBER
%token <ival> KWLOCAL KWPROGRAM KWCALL KWFUNCTION KWGLOBAL KWINT KWSTRING
%token <ival> KWIF KWTHEN KWELSE KWWHILE KWDO KWRETURN KWRETURNVAL RELOP
%token <ival> EQUALS COMMA SEMICOLON LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET ADDOP

%left ADDOP

%%

wholeprogram:
    globals functions program {
        astRoot = newASTNode(AST_PROGRAM);
        astRoot->child[0] = $1;
        astRoot->child[1] = $2;
        astRoot->child[2] = $3;
    }
;

globals:
    /* empty */ { $$ = NULL; }
  | KWGLOBAL vardecl SEMICOLON globals {
        $2->next = $4;
        $$ = $2;
    }
;

vardecl:
    KWINT ID {
        addSymbol(symtable, $2, S_GLOBAL, T_INT, 0, 0, K_GLOBAL);
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_INT; $$->varKind = K_GLOBAL; $$->strNeedsFreed = 1;
    }
  | KWSTRING ID {
        addSymbol(symtable, $2, S_GLOBAL, T_STRING, 0, 0, K_GLOBAL);
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_STRING; $$->varKind = K_GLOBAL; $$->strNeedsFreed = 1;
    }
  | KWINT ID LBRACKET NUMBER RBRACKET {
        addSymbol(symtable, $2, S_GLOBAL, T_INT, 0, $4, K_GLOBALARRAY);
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_INT; $$->ival = $4;
        $$->varKind = K_GLOBALARRAY; $$->strNeedsFreed = 1;
    }
;

functions:
    /* empty */ { $$ = NULL; }
  | function functions {
        $1->next = $2;
        $$ = $1;
    }
;

function:
    KWFUNCTION ID LPAREN { currentOffset = 16; } parameters RPAREN LBRACE functionbody RBRACE {
        $$ = newASTNode(AST_FUNCTION);
        $$->strval = $2; $$->strNeedsFreed = 1;
        $$->child[0] = $5;
        $$->child[1] = $8;
    }
;

parameters:
    /* empty */ { $$ = NULL; }
  | paramdecl { $$ = $1; }
  | paramdecl COMMA parameters { $1->next = $3; $$ = $1; }
;

paramdecl:
    KWINT ID {
        addSymbol(symtable, $2, S_LOCAL, T_INT, currentOffset, 0, K_PARAM);
        currentOffset += 8;
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_INT; $$->varKind = K_PARAM; $$->strNeedsFreed = 1;
    }
  | KWSTRING ID {
        addSymbol(symtable, $2, S_LOCAL, T_STRING, currentOffset, 0, K_PARAM);
        currentOffset += 8;
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_STRING; $$->varKind = K_PARAM; $$->strNeedsFreed = 1;
    }
;

functionbody:
    localvars statements {
        ASTNode* block = newASTNode(AST_SBLOCK);
        block->child[0] = $1;
        block->child[1] = $2;
        $$ = block;
    }
;

localvars:
    /* empty */ { $$ = NULL; }
  | KWLOCAL localdecl SEMICOLON localvars { $2->next = $4; $$ = $2; }
;

localdecl:
    KWINT ID {
        currentOffset += 4;
        addSymbol(symtable, $2, S_LOCAL, T_INT, currentOffset, 0, K_LOCAL);
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_INT; $$->varKind = K_LOCAL; $$->strNeedsFreed = 1;
    }
  | KWSTRING ID {
        currentOffset += 8;
        addSymbol(symtable, $2, S_LOCAL, T_STRING, currentOffset, 0, K_LOCAL);
        $$ = newASTNode(AST_VARDECL);
        $$->strval = $2; $$->valType = T_STRING; $$->varKind = K_LOCAL; $$->strNeedsFreed = 1;
    }
;

program:
    KWPROGRAM LBRACE statements RBRACE {
        $$ = $3;
    }
;

statements:
    /* empty */ { $$ = NULL; }
  | statement statements { $1->next = $2; $$ = $1; }
;

statement:
    funcall
  | assignment
  | ifthenelse
  | whileloop
  | dowhile
  | returnstmt
;

dowhile:
    KWDO LBRACE statements RBRACE KWWHILE LPAREN boolexpr RPAREN SEMICOLON {
        $$ = newASTNode(AST_DOWHILE);
        $$->child[0] = $3; // body
        $$->child[1] = $7; // condition
    }
;

assignment:
    ID EQUALS expression SEMICOLON {
        Symbol *sym = findSymbol(symtable, $1);
        if (!sym) { fprintf(stderr, "Undeclared variable '%s'\n", $1); exit(1); }
        $$ = newASTNode(AST_ASSIGNMENT);
        $$->strval = $1; $$->strNeedsFreed = 1;
        $$->child[0] = $3; $$->valType = sym->type; $$->varKind = sym->kind;
    }
  | ID LBRACKET expression RBRACKET EQUALS expression SEMICOLON {
        $$ = newASTNode(AST_ARRAYASSIGN);
        $$->strval = $1; $$->strNeedsFreed = 1;
        $$->child[0] = $3;
        $$->child[1] = $6;
    }
;

funcall:
    KWCALL ID LPAREN arguments RPAREN SEMICOLON {
        $$ = newASTNode(AST_FUNCALL);
        $$->strval = $2; $$->strNeedsFreed = 1;
        $$->child[0] = $4;
    }
;

arguments:
    /* empty */ { $$ = NULL; }
  | argument { $$ = $1; }
  | argument COMMA arguments { $1->next = $3; $$ = $1; }
;

argument:
    expression {
        $$ = newASTNode(AST_ARGUMENT);
        $$->child[0] = $1;
    }
;

expression:
    NUMBER {
        $$ = newASTNode(AST_CONSTANT);
        $$->valType = T_INT;
        $$->ival = $1;
    }
  | ID {
        Symbol *sym = findSymbol(symtable, $1);
        if (!sym) { fprintf(stderr, "Undeclared variable '%s'\n", $1); exit(1); }
        $$ = newASTNode(AST_VARREF);
        $$->strval = $1; $$->strNeedsFreed = 1;
        $$->valType = sym->type; $$->varKind = sym->kind;
        $$->ival = sym->offset;
    }
  | ID LBRACKET expression RBRACKET {
        $$ = newASTNode(AST_ARRAYREF);
        $$->strval = $1; $$->strNeedsFreed = 1;
        $$->child[0] = $3;
    }
  | STRING {
        $$ = newASTNode(AST_CONSTANT);
        $$->valType = T_STRING;
        $$->strval = $1; $$->strNeedsFreed = 1;
    }
  | KWRETURNVAL {
        $$ = newASTNode(AST_CONSTANT);
        $$->valType = T_RETURNVAL;
    }
  | expression ADDOP expression {
        $$ = newASTNode(AST_EXPRESSION);
        $$->ival = $2;
        $$->child[0] = $1;
        $$->child[1] = $3;
    }
  | ADDOP expression {
        $$ = newASTNode(AST_UNARY);
        $$->ival = $1;
        $$->child[0] = $2;
    }
  | LPAREN expression RPAREN { $$ = $2; }
;

ifthenelse:
    KWIF LPAREN boolexpr RPAREN KWTHEN LBRACE statements RBRACE KWELSE LBRACE statements RBRACE {
        $$ = newASTNode(AST_IFELSE);
        $$->child[0] = $3;
        $$->child[1] = $7;
        $$->child[2] = $11;
    }
;

whileloop:
    KWWHILE LPAREN boolexpr RPAREN KWDO LBRACE statements RBRACE {
        $$ = newASTNode(AST_WHILE);
        $$->child[0] = $3;
        $$->child[1] = $7;
    }
;

boolexpr:
    expression RELOP expression {
        $$ = newASTNode(AST_RELEXPR);
        $$->ival = $2;
        $$->child[0] = $1;
        $$->child[1] = $3;
    }
;

returnstmt:
    KWRETURN expression SEMICOLON {
        $$ = newASTNode(AST_ASSIGNMENT);
        $$->strval = strdup("returnvalue");
        $$->strNeedsFreed = 1;
        $$->child[0] = $2;
        $$->valType = T_RETURNVAL;
        $$->varKind = K_GLOBAL;
    }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
    freeAllSymbols(symtable);
    symtable = NULL;
    yylex_destroy();
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    FILE *infile = NULL;
    FILE *outfile = stdout;
    int doAST = 0, doAssembly = 1;
    char *inputFileName = NULL;
    char outputFileName[256] = {0};

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) debug = 1;
        else if (!strcmp(argv[i], "-d")) { doAST = 1; doAssembly = 0; }
        else if (strstr(argv[i], ".j")) inputFileName = argv[i];
        else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (inputFileName) {
        infile = fopen(inputFileName, "r");
        if (!infile) {
            fprintf(stderr, "Error opening file: %s\n", inputFileName);
            return 1;
        }
        yyin = infile;
        size_t len = strlen(inputFileName);
        strncpy(outputFileName, inputFileName, len - 2);
        strcat(outputFileName, ".s");
        if (doAssembly) {
            outfile = fopen(outputFileName, "w");
            if (!outfile) {
                fprintf(stderr, "Error opening output: %s\n", outputFileName);
                return 1;
            }
        }
    } else {
        yyin = stdin;
    }

    symtable = newSymbolTable();
    int stat = yyparse();
    if (infile) fclose(infile);

    if (!stat && astRoot) {
        if (doAST) printASTree(astRoot, 0, stdout);
        else if (doAssembly) {
            genCodeFromASTree(astRoot, 0, outfile);
            deleteScopeLevel(S_LOCAL);
            currentOffset = 16;
        }
    }

    freeAllSymbols(symtable);
    symtable = NULL;
    freeASTree(astRoot);
    yylex_destroy();
    return stat;
}
