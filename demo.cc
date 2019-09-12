#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include "demo.h"

using namespace std;

map<string, ValueNode*> nodeMap;

// General Functions

void Parser::syntax_error(){
  cout << "Syntax Error" << '\n';
    exit(1);
}

Token Parser::expect(TokenType expected_type){
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

Token Parser::peek(){
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

ValueNode* Parser:: get_node(Token tok){
    if(nodeMap.find(tok.lexeme) != nodeMap.end()){
        return nodeMap[tok.lexeme];
    }
    else{
        ValueNode* numNode = new ValueNode();
        numNode->value = atoi((tok.lexeme).c_str());
        return numNode;
    }
}

void Parser::appendNode(StatementNode *body, StatementNode *node){
    StatementNode *it = body;
    if(it == NULL){
        syntax_error();
    }
    while(it->next != NULL){
        it = it->next;
    }

    it->next = node;
}

// Parsing Begins

StatementNode* Parser::parse_program(){
    //program -> var_section body
    parse_var_section();
    StatementNode *programNode = parse_body();
    return programNode;
}

void Parser:: parse_var_section(){
// var_section -> id list SEMICOLON
    vector<Token> idList = parse_id_list();
    vector<Token>::iterator it;
    for(it = idList.begin(); it != idList.end(); it++){
        ValueNode* vn = new ValueNode;
        vn->name = it->lexeme;
        vn->value = 0;
        nodeMap[it->lexeme] = vn;
    }
    expect(SEMICOLON);
}

vector<Token> Parser::parse_id_list(){
    // id_list -> ID COMMA id_list | ID
    vector<Token> ids;
    ids.push_back(expect(ID));
    Token t = peek();
    if(t.token_type == COMMA){
        Token t = lexer.GetToken();
        vector<Token> ids1 = parse_id_list();
        for (std::vector<Token>::iterator it = ids1.begin(); it != ids1.end(); ++it) {
            ids.push_back(*it);
        }
        return ids;
    }
    else if(t.token_type == SEMICOLON){
        return ids;
    }
    else{
        syntax_error();
    }
}

StatementNode* Parser::parse_body(){
// body -> LBRACE stmt_list RBRACE
    expect(LBRACE);
    StatementNode *s = parse_stmt_list();
    expect(RBRACE);

    return s;
}

StatementNode* Parser::parse_stmt_list(){
    // stmt_list -> stmt stmt_list | stmt
    Token t1 = peek();
    StatementNode *s = parse_stmt();

    Token t = peek();
    if (t.token_type == WHILE ||
        t.token_type == ID ||
        t.token_type == PRINT ||
        t.token_type == IF ||
        t.token_type == FOR ||
        t.token_type == SWITCH)
    {
        StatementNode *sl = parse_stmt_list();
        if(t1.token_type == FOR){
            s->next->next->next = sl;
        }
        else if(t1.token_type == WHILE || t1.token_type == IF){
            s->next->next = sl;
        }
        else if(t1.token_type == SWITCH){
            appendNode(s, sl);
        }
        else{
            s->next = sl;
        }
        return s;
    }
    else if (t.token_type == RBRACE)
    {
        return s;
    }
    else
    {
        syntax_error();
    }
}

StatementNode* Parser::parse_stmt(){
// stmt -> assign_stmt | print_stmt | while_stmt | if_stmt | switch_stmt
    Token t = peek();
    if(t.token_type == FOR){
        return parse_for_stmt();
    }
    if(t.token_type == ID){
        return parse_assign_stmt();
    }
    else if(t.token_type == PRINT){
        return parse_print_stmt();
    }
    else if(t.token_type == WHILE){
       return parse_while_stmt();
    }
    else if(t.token_type == IF){
        return parse_if_stmt();
    }
    else if(t.token_type == SWITCH){
        return parse_switch_stmt();
    }
    else{
        syntax_error();
    }
}

StatementNode* Parser::parse_assign_stmt(){
    // assign_stmt -> ID EQUAL primary SEMICOLON | ID EQUAL expr SEMICOLON
    StatementNode *s = new StatementNode;
    s->type = ASSIGN_STMT;
    s->assign_stmt = new AssignmentStatement;

    Token first = expect(ID);
    s->assign_stmt->left_hand_side = get_node(first);
    expect(EQUAL);

    Token op1 = parse_primary();
    s->assign_stmt->operand1 = get_node(op1);
    Token t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV){
        ArithmeticOperatorType op = parse_op();
        s->assign_stmt->op = op;
        Token op2 = parse_primary();
        s->assign_stmt->operand2 = get_node(op2);
    }
    else{
        s->assign_stmt->op = OPERATOR_NONE;
        s->assign_stmt->operand2 = NULL;
    }
    expect(SEMICOLON);
    return s;
}

Token Parser::parse_primary(){
    // primary -> ID | NUM
    Token t = lexer.GetToken();
    if(t.token_type == ID || t.token_type == NUM){
        return t;
    }
    else{
      lexer.UngetToken(t);
        syntax_error();
    }
}

ArithmeticOperatorType Parser:: parse_op(){
    // op -> PLUS | MINUS | MULT | DIV
    ArithmeticOperatorType op;
    Token t = lexer.GetToken();
    if(t.token_type == PLUS){
        op = OPERATOR_PLUS;
    }
    else if(t.token_type == MINUS){
        op = OPERATOR_MINUS;
    }
    else if(t.token_type == MULT){
        op = OPERATOR_MULT;
    }
    else if(t.token_type == DIV){
        op = OPERATOR_DIV;
    }
    else{
      lexer.UngetToken(t);
        syntax_error();
    }
    return op;
}

StatementNode* Parser::parse_print_stmt(){
// print_stmt -> print ID SEMICOLON
    StatementNode *s = new StatementNode;
    s->type = PRINT_STMT;
    s->print_stmt = new PrintStatement;

    expect(PRINT);
    Token t = expect(ID);
    expect(SEMICOLON);
    s->print_stmt->id = get_node(t);

    return s;
}

StatementNode* Parser::parse_while_stmt(){
   // while_stmt -> WHILE condition LBRACE stmt list RBRACE
    expect(WHILE);
    StatementNode *s = parse_condition();
    StatementNode *w = parse_body();

    s->if_stmt->true_branch = w;

    StatementNode *gt = new StatementNode;
    gt->type = GOTO_STMT;
    gt->goto_stmt = new GotoStatement;
    gt->goto_stmt->target = s;
    appendNode(w, gt);

    StatementNode *nw = new StatementNode;
    nw->type = NOOP_STMT;
    s->if_stmt->false_branch = nw;
    s->next = nw;
    gt->next = nw;

    return s;
}

StatementNode* Parser::parse_if_stmt(){
  // if_stmt -> IF condition body
    expect(IF);
    StatementNode *s = parse_condition();
    StatementNode *b = parse_body();
    s->if_stmt->true_branch = b;

    StatementNode* noop = new StatementNode;
    noop->type = NOOP_STMT;

    s->if_stmt->false_branch = noop;
    appendNode(b, noop);
    s->next = noop;

    return s;
}

StatementNode* Parser::parse_condition(){
    // condition -> primary relop primary
    StatementNode *con = new StatementNode;
    con->type = IF_STMT;
    con->if_stmt = new IfStatement;

    Token op1 = parse_primary();
    con->if_stmt->condition_operand1 = get_node(op1);
    ConditionalOperatorType relop = parse_relop();
    con->if_stmt->condition_op = relop;
    Token op2 = parse_primary();
    con->if_stmt->condition_operand2 = get_node(op2);

    return con;
}

ConditionalOperatorType Parser::parse_relop(){
// relop -> GREATER | LESS | NOTEQUAL
    ConditionalOperatorType cop;

    Token t = lexer.GetToken();
    if(t.token_type == GREATER){
        cop = CONDITION_GREATER;
    }
    else if(t.token_type == LESS){
        cop = CONDITION_LESS;
    }
    else if(t.token_type == NOTEQUAL){
        cop = CONDITION_NOTEQUAL;
    }
    else{
      lexer.UngetToken(t);
        syntax_error();
    }

    return cop;
}

StatementNode* Parser::parse_switch_stmt(){
// switch_stmt -> SWITCH ID LBRACE case_list RBRACE
// switch_stmt -> SWITCH ID LBRACE case_list default_case RBRACE

    expect(SWITCH);
    Token var = expect(ID);
    expect(LBRACE);
    ValueNode* switchValue = get_node(var);

    StatementNode* noopSwitch = new StatementNode;
    noopSwitch->type = NOOP_STMT;

    StatementNode *switchNode = parse_case_list(switchValue, noopSwitch);

    Token t = peek();
    if(t.token_type == DEFAULT){
        StatementNode *defNode = parse_default_case(noopSwitch);
        appendNode(switchNode, defNode);
    }
    else{
        StatementNode *defNode = new StatementNode();
        defNode->type = NOOP_STMT;
        defNode->next = noopSwitch;
        appendNode(switchNode, defNode);
    }
    expect(RBRACE);

    return switchNode;
}

StatementNode* Parser::parse_for_stmt(){
// for_stmt -> FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body

    expect(FOR);
    expect(LPAREN);
    StatementNode *a = parse_assign_stmt();
    StatementNode *s = parse_condition();

    a->next = s;
    StatementNode *noopFor = new StatementNode;
    noopFor->type = NOOP_STMT;
    s->next = noopFor;

    expect(SEMICOLON);
    StatementNode *s1 = parse_assign_stmt();
    expect(RPAREN);

    StatementNode *forBody = parse_body();
    appendNode(forBody, s1);

    StatementNode *gt = new StatementNode;
    gt->type = GOTO_STMT;
    gt->goto_stmt = new GotoStatement;
    gt->goto_stmt->target = s;
    appendNode(forBody, gt);

    s->if_stmt->true_branch = forBody;
    s->if_stmt->false_branch = noopFor;
    s->next = noopFor;
    gt->next = noopFor;

    return a;
}

StatementNode* Parser::parse_case_list(ValueNode* switchValue, StatementNode* noopSwitch){
// case_list -> case case_list | case

    StatementNode* c = parse_case(switchValue, noopSwitch);
    Token t = peek();
    if(t.token_type == CASE){
        StatementNode *cl = parse_case_list(switchValue, noopSwitch);
        c->next->next = cl;
    }
    return c;
}

StatementNode* Parser::parse_case(ValueNode* switchValue, StatementNode* noopSwitch){
// case -> CASE NUM COLON body

    expect(CASE);
    Token case_num = expect(NUM);
    expect(COLON);

    StatementNode *caseIf = new StatementNode;
    caseIf->type = IF_STMT;
    caseIf->if_stmt = new IfStatement;
    caseIf->if_stmt->condition_op = CONDITION_NOTEQUAL;
    caseIf->if_stmt->condition_operand1 = switchValue;
    caseIf->if_stmt->condition_operand2 = get_node(case_num);

    StatementNode* case_body = parse_body();
    caseIf->if_stmt->false_branch = case_body;

    StatementNode *noopTrue = new StatementNode();
    noopTrue->type = NOOP_STMT;
    caseIf->if_stmt->true_branch = noopTrue;
    caseIf->next = noopTrue;


    StatementNode *gt = new StatementNode;
    gt->type = GOTO_STMT;
    gt->goto_stmt = new GotoStatement;
    gt->goto_stmt->target = noopSwitch;
    gt->next = noopSwitch;
    appendNode(case_body, gt);

    return caseIf;
}

StatementNode* Parser::parse_default_case(StatementNode *noopSwitch){
// default_case -> DEFAULT COLON body

    expect(DEFAULT);
    expect(COLON);
    StatementNode *b = parse_body();
    appendNode(b, noopSwitch);
    return b;
}

StatementNode* Parser::ParseInput(){
    return parse_program();
    // for(std::map<string, ValueNode*>::iterator it = nodeMap.begin(); it != nodeMap.end(); it++)
    //   {std::cout << it->first << "->" << it->second << '\n';}
}

struct StatementNode * parse_generate_intermediate_representation(){
    Parser p;
    return p.ParseInput();
}
