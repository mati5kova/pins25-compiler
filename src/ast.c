//
// Created by Matevž Kovačič on 27. 7. 25.
//

#include <stdio.h>
#include <stdlib.h>
#include "../include/ast.h"
#include "../include/lexer.h"

#define INITIAL_ASTNODE_CHILD_COUNT 4

ASTNode* newASTNode(const ASTNodeType type, Token* token) {

    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node)
    {
        return NULL;
    }

    node->type = type;
    node->token = token;

    node->children = malloc(INITIAL_ASTNODE_CHILD_COUNT * sizeof(ASTNode*));
    if (!(node->children))
    {
        free(node);
        return NULL;
    }

    node->childCount = 0;
    node->maxChildCount = INITIAL_ASTNODE_CHILD_COUNT;

    return node;
}

bool appendASTNode(ASTNode* parent, ASTNode* child) {

    if (!child) return false;

    if (parent->childCount + 1 > parent->maxChildCount)
    {
        const size_t newMaxChildCount = parent->maxChildCount * 2;
        ASTNode** temp = realloc(parent->children, newMaxChildCount * sizeof(ASTNode*));
        if (!temp)
        {
            return false;
        }
        parent->children = temp;
        parent->maxChildCount = newMaxChildCount;
    }
    parent->children[parent->childCount] = child;
    (parent->childCount)++;
    return true;
}


void freeAST(ASTNode* root) {
    if (!root) return;

    for (size_t i = 0; i < root->childCount; i++)
    {
        freeAST((root->children)[i]);
    }

    free(root->children);
    free(root);
}

void printIndent(const int count) {
    for (int i = 0; i < count; i++) putchar(' ');
}

// recursive AST‐printer
void printAST(const ASTNode* node, const int indent) {
    if (!node) return;

    printIndent(indent);

    // node type
    printf("%s", ASTNodeType_toString(node->type));

    // za liste (int, char, string, ...)
    if (node->token) {
        printf(" (%.*s)",
               node->token->length,
               node->token->start);
    }
    printf("\n");

    // rekurzivno printamo otroke
    for (size_t i = 0; i < node->childCount; i++) {
        printAST(node->children[i], indent + 4);
    }
}

const char* ASTNodeType_toString(const ASTNodeType type) {
    switch (type) {
    case AST_TEMP:          return "TEMP";
    case AST_ROOT:          return "ROOT";
    case AST_DEF_LIST:      return "DEF_LIST";
    case AST_DEF_FUN:       return "DEF_FUN";
    case AST_DEF_VAR:       return "DEF_VAR";
    case AST_INITS_LIST:    return "INITS_LIST";
    case AST_PARAM_LIST:    return "PARAM_LIST";
    case AST_ARG_LIST:      return "ARG_LIST";
    case AST_STMT_LIST:     return "STMT_LIST";
    case AST_STMT_EXPR:     return "STMT_EXPR";
    case AST_STMT_ASSIGN:   return "STMT_ASSIGN";
    case AST_STMT_IF:       return "STMT_IF";
    case AST_STMT_IF_ELSE:  return "STMT_IF_ELSE";
    case AST_STMT_WHILE:    return "STMT_WHILE";
    case AST_STMT_LET:      return "STMT_LET";
    case AST_EXPR_BINARY:   return "EXPR_BINARY";
    case AST_EXPR_PREFIX:   return "EXPR_PREFIX";
    case AST_EXPR_POSTFIX:  return "EXPR_POSTFIX";
    case AST_EXPR_CALL:     return "EXPR_CALL";
    case AST_IDENT:         return "IDENT";
    case AST_CONST_INT:     return "CONST_INT";
    case AST_CONST_CHAR:    return "CONST_CHAR";
    case AST_CONST_STRING:  return "CONST_STRING";
    default:                return "UNKNOWN";
    }
}
