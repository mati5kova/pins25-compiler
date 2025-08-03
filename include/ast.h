//
// Created by Matevž Kovačič on 27. 7. 25.
//

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "./lexer.h"

typedef enum {
    AST_ROOT,           // the root of the whole program

    AST_DEF_LIST,       // definitions
    AST_DEF_FUN,        // a “fun” definition
    AST_DEF_VAR,        // a “var” definition

    // lists
    AST_INITS_LIST,
    AST_PARAM_LIST,     // comma‑separated function parameters
    AST_ARG_LIST,       // comma‑separated call arguments
    AST_STMT_LIST,      // comma‑separated statements

    // statements
    AST_STMT_EXPR,      // an expression statement (including assignment)
    AST_STMT_IF,        // if‑then (no else)
    AST_STMT_IF_ELSE,   // if‑then‑else
    AST_STMT_WHILE,     // while‑do‑end
    AST_STMT_LET,       // let … in … end

    // expressions (Pratt‑parsed)
    AST_EXPR_BINARY,    // binary operator (lhs, rhs)
    AST_EXPR_PREFIX,    // prefix operator (op, rhs)
    AST_EXPR_POSTFIX,   // postfix operator (lhs, op)
    AST_EXPR_CALL,      // function call (callee, args…)

    // leaves
    AST_IDENT,          // identifier reference
    AST_CONST_INT,      // integer literal
    AST_CONST_CHAR,     // char literal
    AST_CONST_STRING,   // string literal

    AST_TEMP
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType      type;
    Token*           token;           // samo  za liste drevesa
    size_t           childCount;
    size_t           maxChildCount;
    struct ASTNode** children;
} ASTNode;

ASTNode* newASTNode(ASTNodeType type, Token* token);

bool appendASTNode(ASTNode* parent, ASTNode* child);

void freeAST(ASTNode* root);

// ASTNodeType -> string representation
const char* ASTNodeType_toString(ASTNodeType type);

// print `count` presledkov
void printIndent(int count);

// recursive ast printer
void printAST(const ASTNode* node, int indent);

#endif //AST_H
