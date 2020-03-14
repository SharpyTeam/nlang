#pragma once

#include "scanner.hpp"

#include <utils/holder.hpp>
#include <common/ast.hpp>

#include <memory>
#include <vector>
#include <stack>
#include <utility>
#include <string>
#include <unordered_set>
#include <stdexcept>

namespace nlang {

class Parser {
public:
    Holder<TypeHint> TryParseTypeHint() {
        if (scanner->NextTokenLookahead()->token == Token::COLON) {
            auto colon = scanner->NextToken();
            auto identifier = scanner->NextTokenAssert(Token::IDENTIFIER);
            return MakeHolder<TypeHint>(std::move(*colon), MakeHolder<IdentifierLiteral>(std::move(*identifier)));
        }
        return nullptr;
    }

    Holder<DefaultValue> TryParseDefaultValue() {
        if (scanner->NextTokenLookahead()->token == Token::ASSIGN) {
            auto assignment = scanner->NextToken();
            return MakeHolder<DefaultValue>(std::move(*assignment), ParseExpression());
        }
        return nullptr;
    }

    Holder<ArgumentDefinitionStatementPart> ParseArgumentDefinitionStatementPart() {
        auto name = scanner->NextTokenAssert(Token::IDENTIFIER);
        auto type_hint = TryParseTypeHint();
        auto default_value = TryParseDefaultValue();
        return MakeHolder<ArgumentDefinitionStatementPart>(MakeHolder<IdentifierLiteral>(std::move(*name)), std::move(type_hint), std::move(default_value));
    }

    Holder<IStatement> ParseVariableDefinitionStatement() {
        auto let = scanner->NextTokenAssert(Token::LET);
        auto name = scanner->NextTokenAssert(Token::IDENTIFIER);
        auto type_hint = TryParseTypeHint();
        auto default_value = TryParseDefaultValue();
        return MakeHolder<VariableDefinitionStatement>(std::move(*let), MakeHolder<IdentifierLiteral>(std::move(*name)), std::move(type_hint), std::move(default_value));
    }

    Holder<IStatement> ParseReturnStatement() {
        auto ret = scanner->NextTokenAssert(Token::RETURN);
        if (scanner->IsEOL() || scanner->NextTokenLookahead()->token == Token::SEMICOLON) {
            return MakeHolder<ReturnStatement>(std::move(*ret), nullptr);
        }
        return MakeHolder<ReturnStatement>(std::move(*ret), ParseExpression());
    }

    Holder<IStatement> ParseBreakStatement() {
        auto brk = scanner->NextTokenAssert(Token::BREAK);
        if (scanner->IsEOL() || scanner->NextTokenLookahead()->token == Token::SEMICOLON) {
            return MakeHolder<BreakStatement>(std::move(*brk), nullptr);
        }
        return MakeHolder<BreakStatement>(std::move(*brk), ParseExpression());
    }

    Holder<IStatement> ParseContinueStatement() {
        return MakeHolder<ContinueStatement>(std::move(*scanner->NextTokenAssert(Token::CONTINUE)));
    }

    Holder<IStatement> ParseIfElseStatement() {
        auto if_token = scanner->NextTokenAssert(Token::IF);
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        auto body = ParseStatement();
        Holder<ElseStatementPart> else_branch;
        if (scanner->NextTokenLookahead()->token == Token::ELSE) {
            auto else_token = scanner->NextToken();
            auto else_body = ParseStatement();
            else_branch = MakeHolder<ElseStatementPart>(std::move(*else_token), std::move(else_body));
        }
        return MakeHolder<IfElseStatement>(std::move(*if_token), std::move(*left_par), std::move(expr), std::move(*right_par), std::move(body), std::move(else_branch));
    }

    Holder<IStatement> ParseWhileStatement() {
        auto while_token = scanner->NextTokenAssert(Token::WHILE);
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeHolder<WhileStatement>(std::move(*while_token), std::move(*left_par), std::move(expr), std::move(*right_par), ParseBlockStatement());
    }

    Holder<IStatement> ParseExpressionStatement() {
        return MakeHolder<ExpressionStatement>(ParseExpression());
    }

    Holder<IStatement> ParseStatement() {
        switch (scanner->NextTokenLookahead()->token) {
            case Token::LET:
                return ParseVariableDefinitionStatement();

            case Token::RETURN:
                return ParseReturnStatement();

            case Token::CONTINUE:
                return ParseContinueStatement();

            case Token::BREAK:
                return ParseBreakStatement();

            case Token::IF:
                return ParseIfElseStatement();

            case Token::WHILE:
                return ParseWhileStatement();

            case Token::LEFT_BRACE:
                return ParseBlockStatement();

            default:
                return ParseExpressionStatement();
        }
    }

    std::vector<Holder<IStatement>> ParseStatements() {
        std::vector<Holder<IStatement>> statements;

        while (true) {
            if (scanner->IsEOF() || scanner->NextTokenLookahead()->token == Token::RIGHT_BRACE) {
                break;
            }
            statements.emplace_back(ParseStatement());
            if (!scanner->IsEOF() && scanner->NextTokenLookahead()->token == Token::SEMICOLON) {
                scanner->NextToken();
            }
        }

        return statements;
    }

    Holder<IStatement> ParseBlockStatement() {
        auto left_brace = scanner->NextTokenAssert(Token::LEFT_BRACE);
        auto statements = ParseStatements();
        auto right_brace = scanner->NextTokenAssert(Token::RIGHT_BRACE);
        return MakeHolder<BlockStatement>(std::move(*left_brace), std::move(statements), std::move(*right_brace));
    }

    Holder<FunctionDefinitionExpression> ParseFunctionDefinitionExpression() {
        std::vector<Holder<ArgumentDefinitionStatementPart>> args;

        auto fn = scanner->NextTokenAssert(Token::FN);
        Holder<IdentifierLiteral> name;
        if (scanner->NextTokenLookahead()->token == Token::IDENTIFIER) {
            name = MakeHolder<IdentifierLiteral>(std::move(*scanner->NextToken()));
        }
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        TokenInstance right_par;
        bool default_value_required = false;
        while (true) {
            if (scanner->NextTokenLookahead()->token == Token::RIGHT_PAR) {
                right_par = std::move(*scanner->NextToken());
                break;
            }

            auto arg = ParseArgumentDefinitionStatementPart();

            if (arg->default_value) {
                default_value_required = true;
            } else if (default_value_required) {
                throw std::runtime_error("Expected default value");
            }

            args.emplace_back(std::move(arg));

            if (scanner->NextTokenLookahead()->token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead()->token == Token::RIGHT_PAR) {
                    throw std::runtime_error("Expected arg, found par");
                }
            }
        }

        auto type_hint = TryParseTypeHint();

        return MakeHolder<FunctionDefinitionExpression>(std::move(*fn), std::move(name), std::move(*left_par), std::move(args), std::move(right_par),
            std::move(type_hint), ParseBlockStatement());
    }

    Holder<IExpression> ParseParenthesizedExpression() {
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeHolder<ParenthesizedExpression>(std::move(*left_par), std::move(expr), std::move(*right_par));
    }

    Holder<IExpression> ParseBasicExpression() {
        auto mark = scanner->Mark();
        auto token = scanner->NextToken();
        switch (token->token) {
            case Token::LEFT_PAR:
                mark.Apply();
                return ParseParenthesizedExpression();

            case Token::IDENTIFIER:
                return MakeHolder<LiteralExpression>(MakeHolder<IdentifierLiteral>(std::move(*token)));

            case Token::NUMBER:
                return MakeHolder<LiteralExpression>(MakeHolder<NumberLiteral>(std::move(*token)));

            case Token::STRING:
                return MakeHolder<LiteralExpression>(MakeHolder<StringLiteral>(std::move(*token)));

            case Token::THE_NULL:
                return MakeHolder<LiteralExpression>(MakeHolder<NullLiteral>(std::move(*token)));

            case Token::THE_TRUE:
            case Token::THE_FALSE:
                return MakeHolder<LiteralExpression>(MakeHolder<BoolLiteral>(std::move(*token)));

            default:
                throw std::runtime_error("Unexpected token \"" + token->text + "\" at [" + std::to_string(token->row) + ":" + std::to_string(token->column) + "]");
        }
    }

    std::vector<Holder<IExpression>> ParseCallOrSubscriptArguments(Token close) {
        // TODO pass commas as tokens to ast
        std::vector<Holder<IExpression>> arguments;
        while (true) {
            if (auto token = scanner->NextTokenLookahead(); token->token == close) {
                break;
            }
            arguments.emplace_back(ParseExpression());
            if (scanner->NextTokenLookahead()->token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead()->token == close) {
                    throw std::runtime_error("Expected arg, found close");
                }
            }
        }
        return arguments;
    }

    Holder<IExpression> ParsePostfixExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD_ADD, Token::SUB_SUB };
        auto expr = ParseBasicExpression();
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens_set.find(token->token) != tokens_set.end()) {
                expr = MakeHolder<PostfixExpression>(std::move(expr), std::move(*token));
            } else if (token->token == Token::LEFT_PAR) {
                expr = MakeHolder<FunctionCallExpression>(std::move(expr), std::move(*token), ParseCallOrSubscriptArguments(Token::RIGHT_PAR), std::move(*scanner->NextTokenAssert(Token::RIGHT_PAR)));
            } else if (token->token == Token::LEFT_BRACKET) {
                expr = MakeHolder<SubscriptExpression>(std::move(expr), std::move(*token), ParseCallOrSubscriptArguments(Token::RIGHT_BRACKET), std::move(*scanner->NextTokenAssert(Token::RIGHT_BRACKET)));
            } else if (token->token == Token::DOT) {
                expr = MakeHolder<MemberAccessExpression>(std::move(expr), std::move(*token), MakeHolder<IdentifierLiteral>(std::move(*scanner->NextTokenAssert(Token::IDENTIFIER))));
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    Holder<IExpression> ParsePrefixExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD, Token::SUB, Token::ADD_ADD, Token::SUB_SUB };
        std::stack<TokenInstance> operators;
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens_set.find(token->token) != tokens_set.end()) {
                operators.emplace(std::move(*token));
            } else {
                mark.Apply();
                auto expr = ParsePostfixExpression();
                while (!operators.empty()) {
                    expr = MakeHolder<PrefixExpression>(std::move(operators.top()), std::move(expr));
                    operators.pop();
                }
                return expr;
            }
        }
    }

    enum class Associativity {
        LEFT,
        RIGHT
    };

    Holder<IExpression> ParseMultiplicativeExpression() { return ParseBinaryExpression<&Parser::ParsePrefixExpression, false, Associativity::LEFT, Token::MUL, Token::DIV, Token::REMAINDER>(); }
    Holder<IExpression> ParseAdditiveExpression() { return ParseBinaryExpression<&Parser::ParseMultiplicativeExpression, false, Associativity::LEFT, Token::ADD, Token::SUB>(); }
    Holder<IExpression> ParseInfixCallExpression() { return ParseBinaryExpression<&Parser::ParseAdditiveExpression, false, Associativity::LEFT, Token::IDENTIFIER>(); }
    Holder<IExpression> ParseComparisonExpression() { return ParseBinaryExpression<&Parser::ParseInfixCallExpression, false, Associativity::LEFT, Token::LESS_EQUALS, Token::GREATER_EQUALS, Token::LESS, Token::GREATER>(); }
    Holder<IExpression> ParseEqualityExpression() { return ParseBinaryExpression<&Parser::ParseComparisonExpression, false, Associativity::LEFT, Token::EQUALS, Token::NOT_EQUALS>(); }
    Holder<IExpression> ParseConjunctionExpression() { return ParseBinaryExpression<&Parser::ParseEqualityExpression, true, Associativity::LEFT, Token::AND>(); }
    Holder<IExpression> ParseDisjunctionExpression() { return ParseBinaryExpression<&Parser::ParseConjunctionExpression, true, Associativity::LEFT, Token::OR>(); }
    Holder<IExpression> ParseAssignmentExpression() { return ParseBinaryExpression<&Parser::ParseDisjunctionExpression, false, Associativity::RIGHT, Token::ASSIGN, Token::ASSIGN_ADD, Token::ASSIGN_SUB, Token::ASSIGN_MUL, Token::ASSIGN_DIV, Token::ASSIGN_REMAINDER>(); }

    Holder<IExpression> ParseExpression() {
        if (scanner->NextTokenLookahead()->token == Token::FN) {
            return ParseFunctionDefinitionExpression();
        } else if (scanner->NextTokenLookahead()->token == Token::OP) {
            // TODO return ParseOperatorDefinitionExpression();
        } else if (scanner->NextTokenLookahead()->token == Token::CLASS) {
            // TODO return ParseClassDefinitionExpression();
        }
        return ParseAssignmentExpression();
    }

    using ParseExpressionFunctionPointer = Holder<IExpression> (Parser::*)();

    template<ParseExpressionFunctionPointer next, bool allow_newline_before_op, Associativity associativity, Token ...tokens>
    Holder<IExpression> ParseBinaryExpression() {
        static std::unordered_set<Token> tokens_set { tokens... };
        Holder<IExpression> expr = (this->*next)();
        while (true) {
            if constexpr (!allow_newline_before_op) {
                if (scanner->IsEOL()) {
                    break;
                }
            }
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens_set.find(token->token) != tokens_set.end()) {
                if constexpr (associativity == Associativity::LEFT) {
                    expr = MakeHolder<BinaryExpression>(std::move(expr), std::move(*token), (this->*next)());
                } else {
                    expr = MakeHolder<BinaryExpression>(std::move(expr), std::move(*token), ParseBinaryExpression<next, allow_newline_before_op, associativity, tokens...>());
                }
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    static Holder<Parser> New(Holder<Scanner>&& scanner) {
        return Holder<Parser>(new Parser(std::move(scanner)));
    }

private:
    explicit Parser(Holder<Scanner>&& scanner)
        : scanner(std::move(scanner))
    {

    }

private:
    Holder<Scanner> scanner;
};

}