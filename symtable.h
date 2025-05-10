#ifndef SYMTABLE_H
#define SYMTABLE_H

typedef enum {
    T_INT,
    T_STRING,
    T_RETURNVAL
} DataType;

typedef enum {
    K_GLOBAL,
    K_PARAM,
    K_LOCAL,
    K_GLOBALARRAY
} SymbolKind;

typedef enum {
    S_GLOBAL,
    S_LOCAL
} Scope;

typedef struct symbol_s {
    char *name;
    Scope scope;
    DataType type;
    int offset;
    int size;  // For arrays: number of elements
    SymbolKind kind;
    struct symbol_s *next;
} Symbol;

extern Symbol *symtable;

Symbol *newSymbolTable();
Symbol *addSymbol(Symbol *table, char *name, Scope scope, DataType type, int offset, int size, SymbolKind kind);
Symbol *findSymbol(Symbol *table, char *name);
void deleteScopeLevel(Scope s);
void freeAllSymbols(Symbol *table);

#endif
