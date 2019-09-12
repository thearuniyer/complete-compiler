#ifndef __DEMO_H__
#define __DEMO_H__

#include <string>
#include "lexer.h"
#include "compiler.h"
#include "ir_debug.h"

class Parser {
  private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
    ValueNode* get_node(Token tok);
    void appendNode(StatementNode *body, StatementNode *node);
    StatementNode* parse_program();
    void parse_var_section();
    std::vector<Token> parse_id_list();
    StatementNode* parse_body();
    StatementNode* parse_stmt_list();
    StatementNode* parse_stmt();
    StatementNode* parse_assign_stmt();
    Token parse_primary();
    ArithmeticOperatorType parse_op();
    StatementNode* parse_print_stmt();
    StatementNode* parse_while_stmt();
    StatementNode* parse_if_stmt();
    StatementNode* parse_condition();
    ConditionalOperatorType parse_relop();
    StatementNode* parse_switch_stmt();
    StatementNode* parse_for_stmt();
    StatementNode* parse_case_list(ValueNode* switchValue, StatementNode* noopSwitch);
    StatementNode* parse_case(ValueNode* switchValue, StatementNode* noopSwitch);
    StatementNode* parse_default_case(StatementNode *noopSwitch);

    struct StatementNode * parse_generate_intermediate_representation();

  public:
    StatementNode* ParseInput();
};

#endif
