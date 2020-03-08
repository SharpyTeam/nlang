grammar nlang;

file: statements eof;

// utility
newline: '\n';
eof: EOF;
assign_operator: '=' | '+=' | '-=' | '*=' | '/=' | '%=';
equality_operator: '==' | '!=';
comparison_operator: '<=' | '>=' | '<' | '>';
identifier: IDENTIFIER;
string: STRING_LITERAL;
number: NUMBER_SIGN? NUMBER ('.' NUMBER)?;
literal: string | number | 'null' | 'true' | 'false';
additive_operator: '+' | '-';
multiplicative_operator: '*' | '/' | '%';
unary_prefix: '+' | '-' | '++' | '--';
unary_postfix: '++' | '--' | call_postfix | indexed_access_postfix | NL* member_access_postfix;
call_postfix: '(' NL* function_call_arguments NL* ')';
indexed_access_postfix: '[' indexed_access_arguments ']';
indexed_access_arguments: expression NL* (NL* ',' NL* expression)*;
member_access_postfix: '.' NL* identifier;
type_hint: ':' identifier;
argument: identifier type_hint?;
function_def_arguments: (argument NL* (NL* ',' NL* argument)*)?  (NL* ',' NL* argument '=' expression)*;
function_call_arguments: (expression NL* (NL* ',' NL* expression)*)?;

// expressions
expression: assign_expression | function_expression;
assign_expression: disjunction_expression (assign_operator disjunction_expression)*;
disjunction_expression: conjunction_expression (NL* 'or' NL* conjunction_expression)*;
conjunction_expression: equality_expression (NL* 'and' NL* equality_expression)*;
equality_expression: comparison_expression (equality_operator NL* comparison_expression)*;
comparison_expression: function_as_operator_expression (comparison_operator NL* function_as_operator_expression)*;
function_as_operator_expression: range_expression (identifier NL* range_expression)*;
range_expression: additive_expression ('..' | 'downto' | 'until' NL* additive_expression ('step' NL* additive_expression)?)?;
additive_expression: multiplicative_expression (additive_operator NL* multiplicative_expression)*;
multiplicative_expression: prefix_unary_expression (multiplicative_operator NL* prefix_unary_expression)*;
prefix_unary_expression: unary_prefix* postfix_unary_expression;
postfix_unary_expression: basic_expression unary_postfix*;
basic_expression: parenthesized_expression | identifier | literal;
parenthesized_expression: '(' NL* expression NL* ')';
function_expression: 'fn' NL* identifier? NL* '(' NL* function_def_arguments NL* ')' NL* type_hint? NL* (block_statement | ('=' NL* expression));

// statements
statements: (statement (statement_delimiter* statement)* statement_delimiter?)?;
statement_delimiter: (';' | NL+);
expression_statement: expression;
break_statement: 'break' expression?;
return_statement: 'return' expression?;

statement: expression_statement | var_def_statement | return_statement | break_statement | 'continue' | block_statement | while_statement | if_statement;

block_statement: '{' NL* statements NL* '}';

var_def_statement: 'let' argument ('=' expression)?;

while_statement: 'while' NL* '(' expression ')' NL* block_statement;
if_statement: 'if' NL* '(' expression ')' NL* block_statement (NL* 'else' NL* 'if' NL* '(' expression ')' NL* block_statement)* (NL* 'else' NL* block_statement)?;

WS: [ \t]+ -> skip;

IDENTIFIER: [_a-zA-Z][_a-zA-Z0-9]*;
NUMBER: [0-9]+;
NUMBER_SIGN: [+-];
STRING_LITERAL: ('"' STRING_LITERAL_CHAR* '"') | ('\'' STRING_LITERAL_CHAR* '\'');
STRING_LITERAL_CHAR: ~["\\\r\n] | '\\\''| '\\"' | '\\?' | '\\\\' | '\\a'| '\\b' | '\\f'| '\\n' | '\\r' | '\\t' | '\\v';
NL: '\n';
