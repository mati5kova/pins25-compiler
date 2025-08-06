//
// Created by Matevž Kovačič on 18. 7. 25.
//

#ifndef PARSER_H
#define PARSER_H

#include "./token_stream.h"
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

ParseResult parse_program();

ParseResult parse_definitions(bool expectNonDefinitionTokensToFollow);

// `expectNonDefinitionTokensToFollow` je namenjen za let-in-end da vrne PS_NO_MATCH ne pa error
ParseResult parse_individual_definition(bool expectNonDefinitionTokensToFollow);

ParseResult parse_fun_def();

ParseResult parse_var_def();

ParseResult parse_parameters();

ParseResult parse_individual_parameter();

ParseResult parse_identifiers();

ParseResult parse_statements();

ParseResult parse_individual_statement();

ParseResult parse_if_statement();

ParseResult parse_while_statement();

ParseResult parse_let_in_end();

ParseResult parse_expression(int precedence);

ParseResult parse_initializers();

ParseResult parse_individual_initializer(bool isFirstInitializer);

ParseResult parse_constant();

ParseResult parse_arguments();

ParseResult parse_individual_argument();

ASTNode* parse(const TokenStream* inputTokenStream, Options* opts, const char* inputFileName);;

// funkcija vrne precedence za nek operator
int getPrecedence(TokenType type, bool isPrefix);

// vrne true | false glede na to ali je bil parsing uspesen
bool passedSyntaxAnalysis();

#endif //PARSER_H
