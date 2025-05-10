#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

Symbol *symtable = NULL;

Symbol *newSymbolTable() {
    return NULL;
}

Symbol *addSymbol(Symbol *table, char *name, Scope scope, DataType type, int offset, int size, SymbolKind kind) {
    Symbol *sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->scope = scope;
    sym->type = type;
    sym->offset = offset;
    sym->size = size;
    sym->kind = kind;
    sym->next = table;
    symtable = sym;
    return sym;
}

Symbol *findSymbol(Symbol *table, char *name) {
    while (table) {
        if (strcmp(table->name, name) == 0)
            return table;
        table = table->next;
    }
    return NULL;
}

void deleteScopeLevel(Scope s) {
    Symbol *prev = NULL;
    Symbol *curr = symtable;

    while (curr) {
        if (curr->scope == s) {
            Symbol *temp = curr;
            if (prev) {
                prev->next = curr->next;
            } else {
                symtable = curr->next;
            }
            curr = curr->next;
            if (temp->name) free(temp->name);
            free(temp);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

void freeAllSymbols(Symbol *table) {
    while (table) {
        Symbol *next = table->next;
        if (table->name) free(table->name);
        free(table);
        table = next;
    }
}
