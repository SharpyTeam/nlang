grammar nlang;

file: statements eof;

// utility
newline: '\n';
eof: '\0';
assign_operator: '=' | '+=' | '-=' | '*=' | '/=' | '%=';
equality_operator: '==' | '!=';
comparison_operator: '<=' | '>=' | '<' | '>';
identifier: ; // TODO add identifier regex here
//number: ; // TODO
//string: ; // TODO
literal: number | string | 'null' | 'true' | 'false';
additive_operator: '+' | '-';
multiplicative_operator: '*' | '/' | '%';
unary_prefix: '+' | '-' | '++' | '--';
unary_postfix: '++' | '--' | call_postfix | indexed_access_postfix | member_access_postfix;
call_postfix: '(' function_call_arguments ')';
type_hint: ':' identifier;
argument: identifier type_hint?;
function_def_arguments: (argument (',' argument)*)?  (',' argument '=' expression)*;
function_call_arguments: (expression (',' expression)*)?;

// expressions
expression: assign_expression | function_expression;
assign_expression: disjunction_expression (assign_operator disjunction_expression)*;
disjunction_expression: conjunction_expression ('or' conjunction_expression)*;
conjunction_expression: equality_expression ('and' equality_expression)*;
equality_expression: comparison_expression (equality_operator comparison_expression)*;
comparison_expression: function_as_operator_expression (comparison_operator function_as_operator_expression)*;
function_as_operator_expression: range_expression (identifier range_expression)*;
range_expression: additive_expression ('..' | 'downto' | 'until' additive_expression ('step' additive_expression)?)?;
additive_expression: multiplicative_expression (additive_operator multiplicative_expression)*;
multiplicative_expression: prefix_expression (multiplicative_operator prefix_expression)*;
prefix_unary_expression: unary_prefix* postfix_unary_expression;
postfix_unary_expression: basic_expression unary_postfix*;
basic_expression: parenthesized_expression | identifier | literal;
parenthesized_expression: '(' expression ')';
function_expression: 'fn' identifier? '(' function_def_arguments ')'  type_hint? (block_statement | ('=' expression));

// statements
statements: (statement (statement_delimiter statement)* statement_delimiter?)?;
statement_delimiter: (';' | newline+);
expression_statement: expression;
break_statement: 'break' expression?;
return_statement: 'return' expression?;

statement: expression_statement | var_def_statement | return_statement | break_statement | 'continue' | block_statement | while_statement | if_statement;

block_statement: '{' statements '}';

var_def_statement: 'let' argument ('=' expression)?;

while_statement: 'while' '(' expression ')' block_statement;
if_statement: 'if' '(' expression ')' block_statement ('else' 'if' '(' expression ')' block_statement)* ('else' block_statement)?;