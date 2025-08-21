//
// Created by Matevž Kovačič on 18. 7. 25.
//

#ifndef PARSER_H
#define PARSER_H

#include "./ast.h"

// PS...parse status
typedef enum {
    PS_OK,
    PS_NO_MATCH,
    PS_ERROR,
    PS_EOF
} ParseStatus;

typedef struct {
    ParseStatus  status;
    ASTNode*     node; // samo ce je node.status == OK
} ParseResult;

ParseResult parse_program(void);

ParseResult parse_definitions(void);

// `expectNonDefinitionTokensToFollow` je namenjen za let-in-end da vrne PS_NO_MATCH ne pa error
ParseResult parse_individual_definition(bool expectNonDefinitionTokensToFollow);

ParseResult parse_fun_def(void);

ParseResult parse_var_def(void);

ParseResult parse_parameters(void);

ParseResult parse_individual_parameter(void);

ParseResult parse_identifiers(void);

ParseResult parse_statements(void);

ParseResult parse_individual_statement(void);

ParseResult parse_if_statement(void);

ParseResult parse_while_statement(void);

ParseResult parse_let_in_end(void);

ParseResult parse_expression(int precedence);

ParseResult parse_initializers(void);

ParseResult parse_individual_initializer(bool isFirstInitializer);

ParseResult parse_constant(void);

ASTNode* parse(CompilerData* compDataIn);

// funkcija vrne precedence za nek operator
int getPrecedence(TokenType type, bool isPrefix);

// vrne true | false glede na to ali je bil parsing uspesen
bool passedSyntaxAnalysis(void);

bool possibleRecovery(int n, const TokenType types[]);

#endif //PARSER_H
