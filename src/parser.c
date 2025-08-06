//
// Created by Matevž Kovačič on 18. 7. 25.
//

#include <stdlib.h>
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/token_stream.h"
#include "../include/error_utils.h"

#define SUCCESS true
#define FAILURE false

#define PR_OK(ret_node) (ParseResult){ .status = PS_OK, .node = ret_node }
#define PR_ERR_NULL (ParseResult){ .status = PS_ERROR, .node = NULL }
#define PR_NO_MATCH (ParseResult){ .status = PS_NO_MATCH, .node = NULL }
#define PR_EOF (ParseResult){ .status = PS_EOF, .node = NULL }

TokenStream* local_ts;

const char* fileName;

static bool parsingSuccessfull = true;

// funkcija vrne true | false glede na to ali je naslednji token == TOKEN_EOF (peekToken)
static bool isEOFNext();

void parse(const TokenStream* inputTokenStream, Options* opts, const char* inputFileName) {
    fileName = inputFileName;
    local_ts = (TokenStream*) inputTokenStream;

    const ParseResult parseRes = parse_program();

    if (parseRes.status == PS_OK && parsingSuccessfull)
    {
        printf("Parsing successfull\n");
        printAST(parseRes.node, 0);
    } else
    {
        printf("Parsing failed\n");
    }

    freeAST(parseRes.node);
}

ParseResult parse_program() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* root = newASTNode(AST_ROOT, NULL);
    if (!root) { return PR_ERR_NULL; }

    const ParseResult parsedDefinitions = parse_definitions(false);

    if (parsedDefinitions.status != PS_OK)
    {
        freeAST(root);
        return PR_ERR_NULL;
    }

    if (parsedDefinitions.node->childCount < 1)
    {
        printf("definition required");

        freeAST(root);
        return PR_ERR_NULL;
    }

    appendASTNode(root, parsedDefinitions.node);

    return PR_OK(root);
}

ParseResult parse_definitions(const bool expectNonDefinitionTokensToFollow) {
    if (isEOFNext()) { return PR_NO_MATCH; }

    ASTNode* definitionsNode = newASTNode(AST_DEF_LIST, NULL);
    if (!definitionsNode) { return PR_ERR_NULL; }

    while (true) {
        const ParseResult res = parse_individual_definition(false);
        if (res.status == PS_OK) {
            appendASTNode(definitionsNode, res.node);
        } else if (res.status == PS_NO_MATCH) {
            break;
        } else {
            freeAST(definitionsNode);
            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }
    }

    return PR_OK(definitionsNode);
}

ParseResult parse_individual_definition(const bool expectNonDefinitionTokensToFollow) {
    if (isEOFNext()) { return PR_NO_MATCH; }

    if (checkToken(local_ts, TOKEN_KEYWORD_FUN))
    {
        const ParseResult funDef = parse_fun_def();
        if (funDef.status != PS_OK)
        {
            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }

        return PR_OK(funDef.node);
    }

    if (checkToken(local_ts, TOKEN_KEYWORD_VAR))
    {
        const ParseResult varDef = parse_var_def();
        if (varDef.status != PS_OK)
        {
            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }

        return PR_OK(varDef.node);
    }

    if (expectNonDefinitionTokensToFollow)
    {
        return PR_NO_MATCH;
    }

    printSyntaxError(fileName, "incorrect definition", currentToken(local_ts)); // ujame npr. ime = ime = ime,
    return PR_ERR_NULL;
}

ParseResult parse_fun_def() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* funNode = newASTNode(AST_DEF_FUN, peekToken(local_ts)); // pricakovano je peekek token identifer
    if (!funNode) { return PR_ERR_NULL; }

    if (!checkToken(local_ts, TOKEN_IDENTIFIER))
    {
        printSyntaxError(fileName, "incorrect function declaration", peekToken(local_ts));
        // TODO verbose: missing identifier after `var`

        parsingSuccessfull = false;
        freeAST(funNode);
        return PR_ERR_NULL;
    }

    if (!checkToken(local_ts, TOKEN_SYMBOL_LEFT_PAREN))
    {
        printSyntaxError(fileName, "incorrect function declaration", peekToken(local_ts));
        // TODO verbose: missing ( after `var identifer`

        parsingSuccessfull = false;
        freeAST(funNode);
        return PR_ERR_NULL;
    }

    const ParseResult optionalParameters = parse_parameters();
    if (optionalParameters.status != PS_OK)
    {
        freeAST(funNode);
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }
    appendASTNode(funNode, optionalParameters.node);

    if (!checkToken(local_ts, TOKEN_SYMBOL_RIGHT_PAREN))
    {
        printSyntaxError(fileName, "incorrect function declaration", peekToken(local_ts));
        // TODO verbose: missing ) after `var identifer(parameters`

        parsingSuccessfull = false;
        freeAST(funNode);
        return PR_ERR_NULL;
    }

    if (checkToken(local_ts, TOKEN_SYMBOL_ASSIGN))
    {
        const ParseResult statements = parse_statements();
        if (statements.status != PS_OK)
        {
            // TODO --verbose: expected statements after `var identifier(parameters) = `

            parsingSuccessfull = false;
            freeAST(funNode);
            return PR_ERR_NULL;
        }
        appendASTNode(funNode, statements.node);
    }

    return PR_OK(funNode);
}

ParseResult parse_var_def() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* varNode = newASTNode(AST_DEF_VAR, peekToken(local_ts)); // peeked je pricakovan da bo identifier
    if (!varNode) { return PR_ERR_NULL; }

    if (!checkToken(local_ts, TOKEN_IDENTIFIER))
    {
        printSyntaxError(fileName, "incorrect variable declaration", peekToken(local_ts));
        //TODO --verbose: missing identifier after `var`

        parsingSuccessfull = false;
        freeAST(varNode);
        return PR_ERR_NULL;
    }

    if (checkToken(local_ts, TOKEN_SYMBOL_ASSIGN))
    {
        const ParseResult initializersList = parse_initializers();
        if (initializersList.status != PS_OK)
        {
            // TODO --help:

            parsingSuccessfull = false;
            freeAST(varNode);
            return PR_ERR_NULL;
        }
        appendASTNode(varNode, initializersList.node);
    }

    return PR_OK(varNode);
}

// parametri so optional
ParseResult parse_parameters() {
    if (isEOFNext()) { return PR_EOF; }

    // params list node ki ga vracamo
    ASTNode* paramsList = newASTNode(AST_PARAM_LIST, NULL);
    if (!paramsList) { return PR_ERR_NULL; }

    const ParseResult firstParam = parse_individual_parameter();

    // ni parametrov, `parameter` vrne PR_NO_MATCH
    if (firstParam.status == PS_ERROR)
    {
        printSyntaxError(fileName, "incorrect function declaration", peekToken(local_ts));
        // TODO verbose: incorrect parameters

        freeAST(paramsList);
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }
    if (firstParam.status == PS_NO_MATCH)
    {
        return PR_OK(paramsList);
    }

    // else: appendamo param node
    appendASTNode(paramsList, firstParam.node);

    // dokler prepoznamo vejico
    while (checkToken(local_ts, TOKEN_SYMBOL_COMMA))
    {
        const ParseResult nextParameter = parse_individual_parameter();

        // za vejico v parametrih funkcije ni bilo identifier-ja
        if (nextParameter.status != PS_OK)
        {
            printSyntaxError(fileName, "incorrect function declaration", prevCheckedToken(local_ts));
            // TODO verbose: expected parameter(identifier) after \",\" in function parameters

            parsingSuccessfull = false;
            freeAST(paramsList);
            return PR_ERR_NULL;
        }

        appendASTNode(paramsList, nextParameter.node);
    }

    return PR_OK(paramsList);
}

ParseResult parse_individual_parameter() {
    if (isEOFNext()) { return PR_EOF; }

    if (checkToken(local_ts, TOKEN_IDENTIFIER))
    {
        ASTNode* id = newASTNode(AST_IDENT, prevCheckedToken(local_ts));
        if (!id) { return PR_ERR_NULL; }

        return PR_OK(id);
    }

    // peek ne pa check ker se ")" consuma drugje
    if (peekToken(local_ts)->type == TOKEN_SYMBOL_RIGHT_PAREN)
    {
        return PR_NO_MATCH;
    }

    return PR_ERR_NULL;
}

ParseResult parse_statements() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* statementListNode = newASTNode(AST_STMT_LIST, NULL);
    if (!statementListNode) { return PR_ERR_NULL; }

    const ParseResult firstStatement = parse_individual_statement();
    if (firstStatement.status != PS_OK)
    {
        printf("incorrect statements as function body\n");

        parsingSuccessfull = false;
        freeAST(statementListNode);
        return PR_ERR_NULL;
    }

    appendASTNode(statementListNode, firstStatement.node);

    while (checkToken(local_ts, TOKEN_SYMBOL_COMMA))
    {
        const ParseResult stmntAfterComma = parse_individual_statement();
        if (stmntAfterComma.status != PS_OK)
        {
            printf("invalid statement after comma in function body\n");

            parsingSuccessfull = false;
            freeAST(statementListNode);
            return PR_ERR_NULL;
        }

        appendASTNode(statementListNode, stmntAfterComma.node);
    }

    return PR_OK(statementListNode);
}

ParseResult parse_individual_statement() {
    if (isEOFNext()) { return PR_EOF; }

    // if statement; else block se pogleda v `parse_if_statement`
    if (checkToken(local_ts, TOKEN_KEYWORD_IF))
    {
        const ParseResult parsedIfStatement = parse_if_statement();
        if (parsedIfStatement.status != PS_OK)
        {
            return PR_ERR_NULL;
        }

        return PR_OK(parsedIfStatement.node);
    }

    if (checkToken(local_ts, TOKEN_KEYWORD_WHILE))
    {
        const ParseResult parsedWhileStatement = parse_while_statement();
        if (parsedWhileStatement.status != PS_OK)
        {
            return PR_ERR_NULL;
        }

        return PR_OK(parsedWhileStatement.node);
    }

    if (checkToken(local_ts, TOKEN_KEYWORD_LET))
    {
        const ParseResult parsedLetStatement = parse_let_in_end();
        if (parsedLetStatement.status != PS_OK)
        {
            return PR_ERR_NULL;
        }

        return PR_OK(parsedLetStatement.node);
    }

    const ParseResult loneExpression = parse_expression(0);
    if (loneExpression.status != PS_OK)
    {
        return PR_ERR_NULL;
    }

    ASTNode* statementNode = newASTNode(AST_STMT_EXPR, peekToken(local_ts));
    if (!statementNode)
    {
        freeAST(loneExpression.node);
        return PR_ERR_NULL;
    }

    appendASTNode(statementNode, loneExpression.node);

    if (checkToken(local_ts, TOKEN_SYMBOL_ASSIGN))
    {
        statementNode->type = AST_EXPR_BINARY;
        statementNode->token = prevCheckedToken(local_ts); // expr "=" expr

        const ParseResult secondExpression = parse_expression(0);
        if (secondExpression.status != PS_OK)
        {
            freeAST(loneExpression.node);
            return PR_ERR_NULL;
        }

        appendASTNode(statementNode, secondExpression.node);
    }

    return PR_OK(statementNode);
}

ParseResult parse_if_statement() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* ifNode = newASTNode(AST_STMT_IF, prevCheckedToken(local_ts));
    if (!ifNode) { return PR_ERR_NULL; }

    const ParseResult ifExpression = parse_expression(0);
    if (ifExpression.status != PS_OK)
    {
        printf("incorrect expression after if\n");
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }

    if (!checkToken(local_ts, TOKEN_KEYWORD_THEN))
    {
        printf("missing `then` keyword in if statement\n");
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }

    const ParseResult ifStatements = parse_statements();
    if (ifStatements.status != PS_OK)
    {
        printf("incorrect statement after if-expression-then\n");
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }

    appendASTNode(ifNode, ifExpression.node);
    appendASTNode(ifNode, ifStatements.node);

    if (checkToken(local_ts, TOKEN_KEYWORD_ELSE))
    {
        ifNode->type = AST_STMT_IF_ELSE;
        const ParseResult elseStatements = parse_statements();
        if (elseStatements.status != PS_OK)
        {
            printf("incorrect statements after else\n");
            parsingSuccessfull = false;
            freeAST(ifNode);
            return PR_ERR_NULL;
        }

        appendASTNode(ifNode, elseStatements.node);
    }

    if (!checkToken(local_ts, TOKEN_KEYWORD_END))
    {
        printf("missing `end` keyword in if statement\n");
        parsingSuccessfull = false;
        freeAST(ifNode);
        return PR_ERR_NULL;
    }

    return PR_OK(ifNode);
}

ParseResult parse_while_statement() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* whileStatementNode = newASTNode(AST_STMT_WHILE, prevCheckedToken(local_ts));
    if (!whileStatementNode) { return PR_ERR_NULL; }

    const ParseResult whileExpression = parse_expression(0);
    if (whileExpression.status != PS_OK)
    {
        printf("incorrect expression after while\n");
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }

    appendASTNode(whileStatementNode, whileExpression.node);

    if (!checkToken(local_ts, TOKEN_KEYWORD_DO))
    {
        printf("missing `do` keyword in while\n");
        parsingSuccessfull = false;
        freeAST(whileStatementNode);
        return PR_ERR_NULL;
    }

    const ParseResult whileStatements = parse_statements();
    if (whileStatements.status != PS_OK)
    {
        printf("incorrect statements after while-do\n");
        parsingSuccessfull = false;
        freeAST(whileStatementNode);
        return PR_ERR_NULL;
    }

    appendASTNode(whileStatementNode, whileStatements.node);

    if (!checkToken(local_ts, TOKEN_KEYWORD_END))
    {
        printf("missing `end` keyword in while\n");
        parsingSuccessfull = false;
        freeAST(whileStatementNode);
        return PR_ERR_NULL;
    }

    return PR_OK(whileStatementNode);
}

ParseResult parse_let_in_end() {
    if (isEOFNext()) { return PR_EOF; }

    ASTNode* letInEndNode = newASTNode(AST_STMT_LET, NULL);
    if (!letInEndNode) { return PR_ERR_NULL; }

    ASTNode* definitionsNode = newASTNode(AST_DEF_LIST, NULL);
    if (!definitionsNode)
    {
        freeAST(letInEndNode);
        return PR_ERR_NULL;
    }

    while (true) {
        const ParseResult res = parse_individual_definition(true);
        if (res.status == PS_OK) {
            appendASTNode(definitionsNode, res.node);
        } else if (res.status == PS_NO_MATCH) {
            break;
        } else {
            freeAST(letInEndNode);
            freeAST(definitionsNode);
            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }
    }

    appendASTNode(letInEndNode, definitionsNode);

    if (!checkToken(local_ts, TOKEN_KEYWORD_IN))
    {
        printf("missing `in` keyword in let (definition)+ in statements end\n");

        parsingSuccessfull = false;
        freeAST(letInEndNode);
        return PR_ERR_NULL;
    }

    const ParseResult statements = parse_statements();
    if (statements.status != PS_OK)
    {
        printf("incorrect statements after let (definition)+ in\n");

        parsingSuccessfull = false;
        freeAST(letInEndNode);
        return PR_ERR_NULL;
    }

    appendASTNode(letInEndNode, statements.node);

    if (!checkToken(local_ts, TOKEN_KEYWORD_END))
    {
        printf("missing `end` keyword in let (definition)+ in statements end\n");

        parsingSuccessfull = false;
        freeAST(letInEndNode);
        return PR_ERR_NULL;
    }

    return PR_OK(letInEndNode);
}

ParseResult parse_expression(const int precedence) {
    if (isEOFNext()) { return PR_EOF; }

    Token* token = peekToken(local_ts);

    ASTNode* lhs = NULL;

    // prefix expression
    // expression -> ... | prefix-operator expression | ...
    switch (token->type)
    {
        case TOKEN_SYMBOL_CARET:
        case TOKEN_SYMBOL_ARITHMETIC_PLUS:
        case TOKEN_SYMBOL_ARITHMETIC_MINUS:
        case TOKEN_SYMBOL_LOGICAL_NOT: {
            token = consumeToken(local_ts);
            const ParseResult rhs = parse_expression(getPrecedence(token->type, true));
            if (rhs.status != PS_OK)
            {
                printSyntaxError(fileName, "incorrect form of expression", prevCheckedToken(local_ts));
                // TODO verbose: incorrect form of prefix expression, --help

                parsingSuccessfull = false;
                return PR_ERR_NULL;
            }
            lhs = newASTNode(AST_EXPR_PREFIX, token);
            if (!lhs) { return PR_ERR_NULL; }

            appendASTNode(lhs, rhs.node);

            break;
        }
        case TOKEN_IDENTIFIER: {
            token = consumeToken(local_ts);

            lhs = newASTNode(AST_IDENT, token);
            if (!lhs) return PR_ERR_NULL;

            break;
        }
        case TOKEN_CONSTANT_INT:
        case TOKEN_CONSTANT_CHAR:
        case TOKEN_CONSTANT_STRING: {
            token = consumeToken(local_ts);
            lhs = newASTNode(
                token->type == TOKEN_CONSTANT_INT ? AST_CONST_INT :
                token->type == TOKEN_CONSTANT_CHAR ? AST_CONST_CHAR :
                AST_CONST_STRING, token
            );
            if (!lhs) { return PR_ERR_NULL; }

            break;
        }
        case TOKEN_SYMBOL_LEFT_PAREN: {
            token = consumeToken(local_ts);
            const ParseResult innerExpression = parse_expression(getPrecedence(token->type, true));

            if (innerExpression.status != PS_OK || !checkToken(local_ts, TOKEN_SYMBOL_RIGHT_PAREN))
            {
                printSyntaxError(fileName, "mismatched parentheses", peekToken(local_ts));
                parsingSuccessfull = false;
                return PR_ERR_NULL;
            }

            lhs = innerExpression.node;

            break;
        }
        default:
            parsingSuccessfull = false;
            return PR_ERR_NULL;
    }

    // function call, postfix expression, binary expression
    while (true)
    {
        // spremenjeno po INTCONST() napaki
        if (checkToken(local_ts, TOKEN_SYMBOL_LEFT_PAREN)) {
            // edina dovoljena oblika je IDENTIFIER()
            if (lhs->type != AST_IDENT) {
                printSyntaxError(fileName, "invalid function call", prevCheckedToken(local_ts));
                parsingSuccessfull = false;
                return PR_ERR_NULL;
            }

            // arg-list node ki ga pripnemo function callu kot drugi node
            ASTNode* argsList = newASTNode(AST_ARG_LIST, NULL);
            if (!argsList) { return PR_ERR_NULL; }

            // ce ni takoj zaklepaja parsaj naprej argumente
            if (peekToken(local_ts)->type != TOKEN_SYMBOL_RIGHT_PAREN) {
                // prvi argument
                const ParseResult arg = parse_expression(0);
                if (arg.status != PS_OK) {
                    printSyntaxError(fileName, "incorrect argument in function call", prevCheckedToken(local_ts));
                    parsingSuccessfull = false;
                    return PR_ERR_NULL;
                }
                appendASTNode(argsList, arg.node);

                // naslednji argumenti
                while (checkToken(local_ts, TOKEN_SYMBOL_COMMA)) {
                    const ParseResult more = parse_expression(0);
                    if (more.status != PS_OK) {
                        printSyntaxError(fileName, "incorrect argument after comma", prevCheckedToken(local_ts));
                        parsingSuccessfull = false;
                        return PR_ERR_NULL;
                    }
                    appendASTNode(argsList, more.node);
                }
            }

            // nujen je zaklepaj od function call-a
            if (!checkToken(local_ts, TOKEN_SYMBOL_RIGHT_PAREN)) {
                //TODO --verbose: possibly missing ")" / incorrect arguments
                // testiraj z fun i(ena)= ime = ena(1==1=2)
                printSyntaxError(fileName, "invalid function call", peekToken(local_ts));
                parsingSuccessfull = false;
                return PR_ERR_NULL;
            }

            // function call node
            ASTNode* funcCallNode = newASTNode(AST_EXPR_CALL, NULL); // daj lhs->token ce hoces da ima EXPR_CALL identifier kot token
            if (!funcCallNode) { return PR_ERR_NULL; }

            appendASTNode(funcCallNode, lhs); // identifier
            appendASTNode(funcCallNode, argsList); // arg-list

            lhs = funcCallNode;

            // preskocimo spodnji del
            continue;
        }

        Token* operator = peekToken(local_ts);

        const int opPrecedence = getPrecedence(operator->type, false);
        if (opPrecedence <= precedence) { break; }

        operator = consumeToken(local_ts);

        ASTNode* node = NULL;

        // potfix
        if (operator->type == TOKEN_SYMBOL_CARET)
        {
            node = newASTNode(AST_EXPR_POSTFIX, operator);
            if (!node)
            {
                freeAST(lhs);
                return PR_ERR_NULL;
            }

            appendASTNode(node, lhs);
            lhs = node;
        } else
        {
            // binary
            const ParseResult rhs = parse_expression(opPrecedence);
            if (rhs.status == PS_OK)
            {
                node = newASTNode(AST_EXPR_BINARY, operator);
                if (!node)
                {
                    freeAST(lhs);
                    return PR_ERR_NULL;
                }

                appendASTNode(node, lhs);
                appendASTNode(node, rhs.node);
                lhs = node;
            } else
            {
                printf("incorrect postfix/infix expression\n");

                parsingSuccessfull = false;
                return PR_ERR_NULL;
            }
        }
    }

    return PR_OK(lhs);
}

// TODO REWRITE
ParseResult parse_arguments() {
    if (isEOFNext()) { return PR_EOF; }

    // note:
    // oklepaje "(" ")" checka in consuma parse_expression()

    // params list node ki ga vracamo
    ASTNode* argsList = newASTNode(AST_ARG_LIST, NULL);
    if (!argsList) { return PR_ERR_NULL; }

    const ParseResult firstParam = parse_expression(0);

    if (firstParam.status != PS_OK)
    {
        // TODO --verbose: incorrect first argument in function call

        parsingSuccessfull = false;
        freeAST(argsList);
        return PR_ERR_NULL;
    }

    // argumenti niso obvezni, ce dobimo PS_OK in node NULL samo vrnemo prazen AST_ARG_LIST
    if (firstParam.node == NULL)
    {
        return PR_OK(argsList);
    } else
    {
        appendASTNode(argsList, firstParam.node);
    }

    while (checkToken(local_ts, TOKEN_SYMBOL_COMMA))
    {
        const ParseResult res = parse_expression(0);
        if (res.status != PS_OK)
        {
            printf("incorrect arguments after \",\"\n");

            parsingSuccessfull = false;
            freeAST(argsList);
            return PR_ERR_NULL;
        }

        if (res.node == NULL)
        {
            printf("expected expression after \",\" in arguments\n");

            parsingSuccessfull = false;
            freeAST(argsList);
            return PR_ERR_NULL;
        } else
        {
            appendASTNode(argsList, res.node);
        }
    }

    return PR_OK(argsList);
}

ParseResult parse_initializers() {
    // ni EOF checka ker je lahko zadnja vrstica `var ime = ` in je valid
    // prav tako ni checka v parse_individual_initializer()

    // initializer list node ki ga vracamo
    ASTNode* initsList = newASTNode(AST_INITS_LIST, NULL);
    if (!initsList) { return PR_ERR_NULL; }

    // .status == PS_EOF ni mozen (beri zgoraj)
    const ParseResult firstInit = parse_individual_initializer(true);

    if (firstInit.status == PS_OK)
    {
        appendASTNode(initsList, firstInit.node);
    } else
    {
        // TODO
        // error message da to niso dovoljeni initializerji
        printf("incorrect initializers (first initializer)\n");

        parsingSuccessfull = false;
        freeAST(initsList);
        return PR_ERR_NULL;
    }

    while (checkToken(local_ts, TOKEN_SYMBOL_COMMA))
    {
        const ParseResult res = parse_individual_initializer(false);

        if (res.status != PS_OK)
        {
            printSyntaxError(fileName, "invalid initializer(s)", prevCheckedToken(local_ts));

            parsingSuccessfull = false;
            freeAST(initsList);
            return PR_ERR_NULL;
        }

        appendASTNode(initsList, res.node);
    }

    return PR_OK(initsList);
}

ParseResult parse_individual_initializer(const bool isFirstInitializer) {

    const ParseResult left = parse_constant();

    if (left.status == PS_NO_MATCH) {
        if (isFirstInitializer)
        {
            return PR_OK(left.node);
        }

        freeAST(left.node);
        return PR_NO_MATCH;
    }

    if (left.status == PS_ERROR)
    {
        parsingSuccessfull = false;
        return PR_ERR_NULL;
    }

    // opcijsko: INTCONST * const
    if (checkToken(local_ts, TOKEN_SYMBOL_ARITHMETIC_MULTIPLY)) {

        if (!( left.node->type == AST_CONST_INT
            || (left.node->type == AST_EXPR_PREFIX && left.node->children[0]->type == AST_CONST_INT) ))
        {
            //TODO verbose: printf("incorrect initializer; allowed: (INTCONST *)? const\n");
            printSyntaxError(fileName, "invalid initializer", prevCheckedToken(local_ts));

            parsingSuccessfull = false;
            freeAST(left.node);
            return PR_ERR_NULL;
        }

        ASTNode* mul = newASTNode(AST_EXPR_BINARY, prevCheckedToken(local_ts));
        if (!mul) { return PR_ERR_NULL; }

        appendASTNode(mul, left.node);

        const ParseResult right = parse_constant();
        if (right.status != PS_OK) {
            freeAST(mul);
            return PR_ERR_NULL;
        }
        appendASTNode(mul, right.node);
        return PR_OK(mul);
    }

    return PR_OK(left.node);
}

ParseResult parse_constant() {
    if (isEOFNext()) { return PR_EOF; }

    // peek za predznak
    Token* tok = peekToken(local_ts);
    bool isSigned = false;
    Token* signTok   = NULL;

    if (tok->type == TOKEN_SYMBOL_ARITHMETIC_PLUS || tok->type == TOKEN_SYMBOL_ARITHMETIC_MINUS)
    {
        isSigned = true;
        signTok  = consumeToken(local_ts);
        if (isEOFNext()) {
            //TODO verbose: printf("lone sign as constant in invalid\n");
            printSyntaxError(fileName, "invalid constant", prevCheckedToken(local_ts));

            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }
    }

    // peek za dejanski const token
    tok = peekToken(local_ts);
    ASTNode* constNode = NULL;

    switch (tok->type) {
      case TOKEN_CONSTANT_INT:
        consumeToken(local_ts);
        constNode = newASTNode(AST_CONST_INT, tok);
        if (!constNode) { return PR_ERR_NULL; }

        break;

      case TOKEN_CONSTANT_CHAR:
      case TOKEN_CONSTANT_STRING:
        // stringi in chari niso predznaceni
        if (isSigned) {
            printSyntaxError(fileName, "invalid constant", prevCheckedToken(local_ts));
            //TODO verbose; printf("invalid constant. char and string constants cannot be signed\n");

            parsingSuccessfull = false;
            return PR_ERR_NULL;
        }
        tok = consumeToken(local_ts);
        constNode = newASTNode(tok->type == TOKEN_CONSTANT_CHAR ? AST_CONST_CHAR : AST_CONST_STRING, tok);
        if (!constNode) { return PR_ERR_NULL; }

        break;

      default:
        // npr. var i =
        // (brez initializerjev)
        return PR_NO_MATCH;
    }

    // ce je predznak damo vse skupaj v prefix node
    if (isSigned) {
        ASTNode* prefix = newASTNode(AST_EXPR_PREFIX, signTok);
        if (!prefix)
        {
            freeAST(constNode);
            return PR_ERR_NULL;
        }

        appendASTNode(prefix, constNode);
        return PR_OK(prefix);
    }

    return PR_OK(constNode);
}

int getPrecedence(const TokenType type, const bool isPrefix) {
    switch (type) {
    case TOKEN_SYMBOL_CARET:
        return isPrefix ? 6 : 7;
    case TOKEN_SYMBOL_LOGICAL_NOT:
        return 6;
    case TOKEN_SYMBOL_ARITHMETIC_PLUS:
    case TOKEN_SYMBOL_ARITHMETIC_MINUS:
        return isPrefix ? 6 : 4;
    case TOKEN_SYMBOL_ARITHMETIC_MULTIPLY:
    case TOKEN_SYMBOL_ARITHMETIC_DIVIDE:
    case TOKEN_SYMBOL_ARITHMETIC_MOD:
        return 5;
    case TOKEN_SYMBOL_LOGICAL_EQUALS:
    case TOKEN_SYMBOL_LOGICAL_NOT_EQUALS:
    case TOKEN_SYMBOL_LOGICAL_LESS:
    case TOKEN_SYMBOL_LOGICAL_LESS_OR_EQUALS:
    case TOKEN_SYMBOL_LOGICAL_GREATER:
    case TOKEN_SYMBOL_LOGICAL_GREATER_OR_EQUALS:
        return 3;
    case TOKEN_SYMBOL_LOGICAL_AND:
        return 2;
    case TOKEN_SYMBOL_LOGICAL_OR:
        return 1;
    default:
        return 0;
    }
}

static bool isEOFNext() {
    return peekToken(local_ts)->type == TOKEN_EOF;
}
