#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "astree.h"
#include "symtable.h"

int strlabel = 0;
int argcount = 0;

static int getUniqueLabelID() {
    static int lid = 100;
    return lid++;
}

ASTNode* newASTNode(ASTNodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = type;
    node->valType = T_INT;
    node->varKind = K_GLOBAL;
    node->ival = 0;
    node->strval = NULL;
    node->strNeedsFreed = 0;
    node->next = NULL;
    for (int i = 0; i < ASTNUMCHILDREN; i++) node->child[i] = NULL;
    return node;
}

void freeASTree(ASTNode* node) {
    if (!node) return;
    for (int i = 0; i < ASTNUMCHILDREN; i++) freeASTree(node->child[i]);
    freeASTree(node->next);
    if (node->strNeedsFreed && node->strval) free(node->strval);
    free(node);
}

void printASTree(ASTNode* node, int level, FILE *out) {
    if (!node) return;
    for (int i = 0; i < level * 3; i++) fprintf(out, " ");
    switch (node->type) {
        case AST_PROGRAM:
            fprintf(out, "Program:\n");
            break;
        case AST_SBLOCK:
            fprintf(out, "SBlock:\n");
            break;
        case AST_VARDECL:
            fprintf(out, "VarDecl: %s (%s)\n", node->strval,
                    node->valType == T_INT ? "int" : "string");
            break;
        case AST_FUNCTION:
            fprintf(out, "Function: %s\n", node->strval);
            break;
        case AST_ASSIGNMENT:
            fprintf(out, "Assign to %s\n", node->strval);
            break;
        case AST_ARRAYASSIGN:
            fprintf(out, "Array Assign: %s[index] = value\n", node->strval);
            break;
        case AST_ARRAYREF:
            fprintf(out, "Array Reference: %s[index]\n", node->strval);
            break;
        case AST_VARREF:
            fprintf(out, "VarRef: %s\n", node->strval);
            break;
        case AST_CONSTANT:
            if (node->valType == T_INT) fprintf(out, "Const INT: %d\n", node->ival);
            else if (node->valType == T_STRING) fprintf(out, "Const STR: %s\n", node->strval);
            else fprintf(out, "Const: RETURNVAL\n");
            break;
        case AST_EXPRESSION:
            fprintf(out, "Expr: %c\n", node->ival);
            break;
        case AST_UNARY:
            fprintf(out, "Unary Expr: %c\n", node->ival);
            break;
        case AST_FUNCALL:
            fprintf(out, "FuncCall: %s\n", node->strval);
            break;
        case AST_ARGUMENT:
            fprintf(out, "Argument:\n");
            break;
        case AST_WHILE:
            fprintf(out, "While Loop:\n");
            break;
        case AST_DOWHILE:
            fprintf(out, "Do-While Loop:\n");
            break;
        case AST_IFELSE:
            fprintf(out, "If-Else:\n");
            break;
        case AST_RELEXPR:
            fprintf(out, "RelExpr: %c\n", node->ival);
            break;
        default:
            fprintf(out, "Unknown node\n");
    }
    for (int i = 0; i < ASTNUMCHILDREN; i++) printASTree(node->child[i], level + 1, out);
    printASTree(node->next, level, out);
}

// Generates x86-64 assembly code from the AST
void genCodeFromASTree(ASTNode* node, int hval, FILE *out) {
    if (!node) return;

    char *instr = NULL;

    switch (node->type) {
        case AST_UNARY:
            genCodeFromASTree(node->child[0], hval, out);
            switch (node->ival) {
                case '-':
                    fprintf(out, "\tnegl %%eax\n");
                    break;
                case '~':
                    fprintf(out, "\tnot %%eax\n");
                    break;
                case '!':
                    fprintf(out, "\tcmpl $0, %%eax\n");
                    fprintf(out, "\tsete %%al\n");
                    fprintf(out, "\tmovzbl %%al, %%eax\n");
                    break;
                default:
                    fprintf(stderr, "Unknown unary operator '%c'\n", node->ival);
                    exit(1);
            }
            break;
    
        case AST_EXPRESSION:
            genCodeFromASTree(node->child[0], hval, out);
            fprintf(out, "\tpushq %%rax\n");
            genCodeFromASTree(node->child[1], hval, out);
            fprintf(out, "\tpopq %%rcx\n");
            switch (node->ival) {
                case '+':
                    fprintf(out, "\taddl %%ecx, %%eax\n");
                    break;
                case '-':
                    fprintf(out, "\tsubl %%eax, %%ecx\n\tmovl %%ecx, %%eax\n");
                    break;
                case '*':
                    fprintf(out, "\timull %%ecx, %%eax\n");
                    break;
                case '/':
                case '%':
                    fprintf(out, "\tmovl %%eax, %%ebx\n");
                    fprintf(out, "\tmovl %%ecx, %%eax\n");
                    fprintf(out, "\tcltd\n");
                    fprintf(out, "\tidivl %%ebx\n");
                    if (node->ival == '%')
                        fprintf(out, "\tmovl %%edx, %%eax\n");
                    break;
                case '&':
                    fprintf(out, "\tandl %%ecx, %%eax\n");
                    break;
                case '|':
                    fprintf(out, "\torl %%ecx, %%eax\n");
                    break;
                case '^':
                    fprintf(out, "\txorl %%ecx, %%eax\n");
                    break;
                default:
                    fprintf(stderr, "Unknown binary op '%c'\n", node->ival);
                    exit(1);
            }
            break;

        case AST_RELEXPR:
            genCodeFromASTree(node->child[0], hval, out);
            fprintf(out, "\tpushq %%rax\n");
            genCodeFromASTree(node->child[1], hval, out);
            fprintf(out, "\tpopq %%rcx\n");
            fprintf(out, "\tcmpl %%eax, %%ecx\n");
            switch (node->ival) {
                case '<': instr = "jl"; break;
                case '>': instr = "jg"; break;
                case '=': instr = "je"; break;
                case '!': instr = "jne"; break;
                case 256: instr = "jle"; break; // <=
                case 257: instr = "jge"; break; // >=
                default: fprintf(stderr, "Invalid RELOP %c\n", node->ival); exit(1);
            }
            fprintf(out, "\t%s LL%d\n", instr, hval);
            break;
            
        case AST_DOWHILE: {
                int bodyLbl = getUniqueLabelID();
                int condLbl = getUniqueLabelID();
                fprintf(out, "LL%d:\n", bodyLbl);       // loop body
                genCodeFromASTree(node->child[0], hval, out);
                fprintf(out, "LL%d:\n", condLbl);       // condition
                genCodeFromASTree(node->child[1], bodyLbl, out);
                break;
            }

        case AST_PROGRAM:
            fprintf(out, ".data\n");
            genCodeFromASTree(node->child[0], hval, out); // globals
            fprintf(out, ".comm returnvalue,4,4\n");
            // Add scanf format string
            fprintf(out, ".section .rodata\n");
            fprintf(out, "scanf_format:\n");
            fprintf(out, "\t.string \"%%d\"\n");
            fprintf(out, ".text\n");
            genCodeFromASTree(node->child[1], hval, out); // functions
            // Emit readInt function
            fprintf(out, "readInt:\n");
            fprintf(out, "\tpushq %%rbp\n");
            fprintf(out, "\tmovq %%rsp, %%rbp\n");
            fprintf(out, "\tsubq $16, %%rsp\n");
            fprintf(out, "\tleaq -4(%%rbp), %%rsi\n");    // address for the integer
            fprintf(out, "\tleaq scanf_format(%%rip), %%rdi\n"); // format string
            fprintf(out, "\txorl %%eax, %%eax\n");        // no vector registers
            fprintf(out, "\tcall scanf@PLT\n");
            fprintf(out, "\tmovl -4(%%rbp), %%eax\n");    // load result into eax
            fprintf(out, "\tleave\n");
            fprintf(out, "\tret\n");
            // Generate main function
            fprintf(out, ".globl main\nmain:\n\tpushq %%rbp\n\tmovq %%rsp, %%rbp\n\tsubq $128, %%rsp\n");
            genCodeFromASTree(node->child[2], hval, out); // main body
            fprintf(out, "\tmovl $0, %%eax\n\tleave\n\tret\n");
            break;

        case AST_VARDECL:
            if (node->varKind == K_GLOBAL)
                fprintf(out, ".comm %s,4,4\n", node->strval);
            else if (node->varKind == K_GLOBALARRAY)
                fprintf(out, ".comm %s,%d,4\n", node->strval, node->ival * 4);
            break;

        case AST_FUNCTION:
            fprintf(out, ".globl %s\n%s:\n", node->strval, node->strval);
            fprintf(out, "\tpushq %%rbp\n\tmovq %%rsp, %%rbp\n\tsubq $128, %%rsp\n");
            // Save callee-saved registers and parameters
            fprintf(out, "\tmovq %%rbx, -8(%%rbp)\n");
            fprintf(out, "\tmovq %%rdi, -16(%%rbp)\n");  // Param 1
            fprintf(out, "\tmovq %%rsi, -24(%%rbp)\n");  // Param 2
            fprintf(out, "\tmovq %%rdx, -32(%%rbp)\n");  // Param 3
            fprintf(out, "\tmovq %%rcx, -40(%%rbp)\n");  // Param 4
            // Generate code for locals and statements
            genCodeFromASTree(node->child[1]->child[0], hval, out); // locals (AST_SBLOCK's child[0])
            genCodeFromASTree(node->child[1]->child[1], hval, out); // statements (AST_SBLOCK's child
            // Epilogue
            fprintf(out, "\tmovq -8(%%rbp), %%rbx\n");
            fprintf(out, "\tleave\n\tret\n");
            break;

        case AST_ASSIGNMENT: {
            if (node->child[0])
                genCodeFromASTree(node->child[0], hval, out);

            Symbol *sym = findSymbol(symtable, node->strval);
            if (!sym) {
                fprintf(stderr, "Unknown variable '%s' in assignment\n", node->strval);
                exit(1);
            }

            if (node->valType == T_RETURNVAL)
                fprintf(out, "\tmovl returnvalue(%%rip), %%eax\n");

            if (sym->scope == S_LOCAL || sym->kind == K_LOCAL || sym->kind == K_PARAM)
                fprintf(out, "\tmovl %%eax, -%d(%%rbp)\n", sym->offset);
            else
                fprintf(out, "\tmovl %%eax, %s(%%rip)\n", node->strval);

            break;
        }

        case AST_ARRAYASSIGN:
            genCodeFromASTree(node->child[1], hval, out); // Generate value to assign
            fprintf(out, "\tpushq %%rax\n");
            genCodeFromASTree(node->child[0], hval, out); // Generate index
            fprintf(out, "\tcltq\n");
            fprintf(out, "\tleaq %s(%%rip), %%rdx\n", node->strval); // Load array base
            fprintf(out, "\tpopq %%rcx\n");
            fprintf(out, "\tmovl %%ecx, (%%rdx,%%rax,4)\n"); // Store value
            break;

        case AST_ARRAYREF:
            genCodeFromASTree(node->child[0], hval, out);
            fprintf(out, "\tcltq\n");
            fprintf(out, "\tleaq %s(%%rip), %%rdx\n", node->strval);
            fprintf(out, "\tmovl (%%rdx,%%rax,4), %%eax\n");
            break;

        case AST_VARREF: {
            Symbol *sym = findSymbol(symtable, node->strval);
            if (!sym) {
                fprintf(stderr, "Unknown variable '%s' in varref\n", node->strval);
                exit(1);
            }
            
            if (sym->type == T_STRING) {
                if (sym->kind == K_PARAM || sym->kind == K_LOCAL)
                    fprintf(out, "\tmovq -%d(%%rbp), %%rax\n", sym->offset);
                else
                    fprintf(out, "\tmovq %s(%%rip), %%rax\n", node->strval);
            } else {
                if (sym->kind == K_PARAM || sym->kind == K_LOCAL)
                    // Use movl for 32-bit integers
                    fprintf(out, "\tmovl -%d(%%rbp), %%eax\n", sym->offset);
                else
                    fprintf(out, "\tmovl %s(%%rip), %%eax\n", node->strval);
            }
            break;
        }

        case AST_CONSTANT:
            if (node->valType == T_STRING) {
                // Load string address into %rax
                fprintf(out, ".section .rodata\nSTR%d:\n\t.string %s\n.text\n", strlabel, node->strval);
                fprintf(out, "\tleaq STR%d(%%rip), %%rax\n", strlabel); // Key fix: Use %%rax
                strlabel++;
                break;
            } else if (node->valType == T_INT) {
                fprintf(out, "\tmovl $%d, %%eax\n", node->ival);
            } else if (node->valType == T_RETURNVAL) {
                fprintf(out, "\tmovl returnvalue(%%rip), %%eax\n");
            }
            break;

        case AST_IFELSE: {
            int l1 = getUniqueLabelID(), l2 = getUniqueLabelID();
            genCodeFromASTree(node->child[0], l1, out);  // condition
            genCodeFromASTree(node->child[2], hval, out); // else block
            fprintf(out, "\tjmp LL%d\n", l2);
            fprintf(out, "LL%d:\n", l1);
            genCodeFromASTree(node->child[1], hval, out); // then block
            fprintf(out, "LL%d:\n", l2);
            break;
        }

        case AST_WHILE: {
            int top = getUniqueLabelID(), cond = getUniqueLabelID();
            fprintf(out, "\tjmp LL%d\n", cond); // Jump to condition check first
            fprintf(out, "LL%d:\n", top); // Loop body label
            genCodeFromASTree(node->child[1], hval, out); // Body statements
            fprintf(out, "LL%d:\n", cond); // Condition label
            genCodeFromASTree(node->child[0], top, out); // Evaluate condition
            break;
        }

        case AST_FUNCALL:
            argcount = 0;
            genCodeFromASTree(node->child[0], hval, out);
            if (strcmp(node->strval, "printf") == 0) {
                fprintf(out, "\txorl %%eax, %%eax\n"); // Set %al to 0
            }
            fprintf(out, "\tcall %s@PLT\n", node->strval);
            if (strcmp(node->strval, "readInt") == 0)
                fprintf(out, "\tmovl %%eax, returnvalue(%%rip)\n");
            break;

        case AST_SBLOCK:  // Handle statement blocks
            genCodeFromASTree(node->child[0], hval, out); // Local declarations
            genCodeFromASTree(node->child[1], hval, out); // Statements
            break;

        case AST_ARGUMENT: {
            genCodeFromASTree(node->child[0], hval, out); // Generate code for the argument expression
            DataType argType = node->child[0]->valType;
            switch (argcount++) {
                case 0:
                    if (argType == T_STRING) {
                        fprintf(out, "\tmovq %%rax, %%rdi\n"); // 64-bit address to %rdi
                    } else {
                        fprintf(out, "\tmovl %%eax, %%edi\n"); // 32-bit value to %edi
                    }
                    break;
                case 1:
                    if (argType == T_STRING) {
                        fprintf(out, "\tmovq %%rax, %%rsi\n");
                    } else {
                        fprintf(out, "\tmovl %%eax, %%esi\n");
                    }
                    break;
                case 2:
                    if (argType == T_STRING) {
                        fprintf(out, "\tmovq %%rax, %%rdx\n");
                    } else {
                        fprintf(out, "\tmovl %%eax, %%edx\n");
                    }
                    break;
                case 3:
                    if (argType == T_STRING) {
                        fprintf(out, "\tmovq %%rax, %%rcx\n");
                    } else {
                        fprintf(out, "\tmovl %%eax, %%ecx\n");
                    }
                    break;
                default:
                    // Handle additional arguments if needed
                    break;
            }
            break;
        }

    }

    genCodeFromASTree(node->next, hval, out);
}
