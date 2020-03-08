#pragma once

#include "scanner.hpp"

#include <utils/holder.hpp>
#include <common/ast.hpp>

#include <memory>
#include <vector>
#include <stdexcept>

namespace nlang {

class Parser {
public:

    Holder<IStatement> ParseVariableDefinitionStatement() {
        scanner->NextTokenAssert(Token::LET);
        auto identifier = MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text);
        Holder<IdentifierExpression> type_hint;
        Holder<IExpression> default_value;

        if (scanner->NextTokenLookahead().token == Token::COLON) {
            scanner->NextToken();
            type_hint = MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text);
        }
        if (scanner->NextTokenLookahead().token == Token::ASSIGN) {
            scanner->NextToken();
            default_value = ParseExpression();
        }
        return MakeHolder<VariableDefinitionStatement>(std::move(identifier), std::move(default_value), std::move(type_hint));
    }

    Holder<IStatement> ParseReturnStatement() {
        scanner->NextTokenAssert(Token::RETURN);
        if (scanner->IsEOL() || scanner->NextTokenLookahead().token == Token::COLON) {
            return MakeHolder<ReturnStatement>();
        }
        return MakeHolder<ReturnStatement>(ParseExpression());
    }

    Holder<IStatement> ParseBreakStatement() {
        scanner->NextTokenAssert(Token::BREAK);
        return MakeHolder<BreakStatement>();
    }

    Holder<IStatement> ParseContinueStatement() {
        scanner->NextTokenAssert(Token::CONTINUE);
        return MakeHolder<ContinueStatement>();
    }

    Holder<IStatement> ParseIfStatement() {
        std::vector<std::pair<Holder<IExpression>, Holder<IStatement>>> ifs;
        while (true) {
            scanner->NextTokenAssert(Token::IF);
            scanner->NextTokenAssert(Token::LEFT_PAR);
            auto expr = ParseExpression();
            scanner->NextTokenAssert(Token::RIGHT_PAR);
            ifs.emplace_back(std::move(expr), ParseBlockStatement());
            if (scanner->NextTokenLookahead().token != Token::ELSE) {
                break;
            } else {
                scanner->NextToken();
            }
            if (scanner->NextTokenLookahead().token != Token::IF) {
                ifs.emplace_back(nullptr, ParseBlockStatement());
                break;
            }
        }
        return MakeHolder<BranchStatement>(std::move(ifs));
    }

    Holder<IStatement> ParseWhileStatement() {
        scanner->NextTokenAssert(Token::WHILE);
        scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeHolder<WhileStatement>(std::move(expr), ParseBlockStatement());
    }

    Holder<IStatement> ParseExpressionStatement() {
        return MakeHolder<ExpressionStatement>(ParseExpression());
    }

    Holder<IStatement> ParseStatement() {
        switch (scanner->NextTokenLookahead().token) {
            case Token::LET:
                return ParseVariableDefinitionStatement();

            case Token::RETURN:
                return ParseReturnStatement();

            case Token::CONTINUE:
                return ParseContinueStatement();

            case Token::BREAK:
                return ParseBreakStatement();

            case Token::IF:
                return ParseIfStatement();

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
            if (scanner->IsEOF() || scanner->NextTokenLookahead().token == Token::RIGHT_BRACE) {
                break;
            }
            statements.emplace_back(ParseStatement());
            if (!scanner->IsEOF() && scanner->NextTokenLookahead().token == Token::SEMICOLON) {
                scanner->NextToken();
            }
        }

        return statements;
    }

    Holder<IStatement> ParseBlockStatement() {
        scanner->NextTokenAssert(Token::LEFT_BRACE);
        auto r = MakeHolder<BlockStatement>(ParseStatements());
        scanner->NextTokenAssert(Token::RIGHT_BRACE);
        return r;
    }

    Holder<FunctionDefinitionExpression> ParseFunctionDefinitionExpression() {
        Holder<IdentifierExpression> name;
        std::vector<Holder<VariableDefinitionStatement>> args;

        scanner->NextTokenAssert(Token::FN);
        if (scanner->NextTokenLookahead().token == Token::IDENTIFIER) {
            name = MakeHolder<IdentifierExpression>(scanner->NextToken().text);
        }
        scanner->NextTokenAssert(Token::LEFT_PAR);
        bool default_value_required = false;
        while (true) {
            if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                scanner->NextToken();
                break;
            }
            auto arg_name = MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text);
            Holder<IdentifierExpression> type_hint;
            Holder<IExpression> default_value;

            if (scanner->NextTokenLookahead().token == Token::COLON) {
                scanner->NextToken();
                type_hint = MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text);
            }
            if (scanner->NextTokenLookahead().token == Token::ASSIGN) {
                scanner->NextToken();
                default_value = ParseExpression();
                default_value_required = true;
            } else if (default_value_required) {
                throw std::runtime_error("Expected default value");
            }

            args.emplace_back(MakeHolder<VariableDefinitionStatement>(
                std::move(arg_name), std::move(default_value), std::move(type_hint)));

            if (scanner->NextTokenLookahead().token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                    throw std::runtime_error("Expected arg, found par");
                }
            }
        }

        Holder<IdentifierExpression> type_hint;
        if (scanner->NextTokenLookahead().token == Token::COLON) {
            scanner->NextToken();
            type_hint = MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text);
        }

        return MakeHolder<FunctionDefinitionExpression>(std::move(name), ParseBlockStatement(), std::move(args), std::move(type_hint));
    }

    Holder<IExpression> ParseParenthesizedExpression() {
        scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeHolder<ParenthesizedExpression>(std::move(expr));
    }

    Holder<IExpression> ParseBasicExpression() {
        auto mark = scanner->Mark();
        auto token = scanner->NextToken();
        switch (token.token) {
            case Token::LEFT_PAR:
                mark.Apply();
                return ParseParenthesizedExpression();

            case Token::IDENTIFIER:
                return MakeHolder<IdentifierExpression>(token.text);

            case Token::NUMBER:
                return MakeHolder<NumberExpression>(std::stod(token.text));

            case Token::STRING:
                return MakeHolder<StringExpression>(token.text);

            case Token::THE_NULL:
                return MakeHolder<NullExpression>();

            case Token::THE_TRUE:
            case Token::THE_FALSE:
                return MakeHolder<BoolExpression>(token.token == Token::THE_TRUE);

            default:
                throw std::runtime_error("Unexpected token \"" + token.text + "\" at [" + std::to_string(token.row) + ":" + std::to_string(token.column) + "]");
        }
    }

    std::vector<Holder<IExpression>> ParseCallOrSubscriptArguments() {
        std::vector<Holder<IExpression>> arguments;
        Token open = scanner->NextToken().token;
        if (open != Token::LEFT_PAR && open != Token::LEFT_BRACKET) {
            throw std::runtime_error("TODO");
        }
        Token close = open == Token::LEFT_PAR ? Token::RIGHT_PAR : Token::RIGHT_BRACKET;
        while (true) {
            if (const TokenInstance& token = scanner->NextTokenLookahead(); token.token == close) {
                scanner->NextToken();
                break;
            }
            arguments.emplace_back(ParseExpression());
            if (scanner->NextTokenLookahead().token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead().token == close) {
                    throw std::runtime_error("Expected arg, found close");
                }
            }
        }
        return arguments;
    }

    Holder<IExpression> ParsePostfixUnaryExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD_ADD, Token::SUB_SUB };
        auto expr = ParseBasicExpression();
        while (true) {
            auto mark = scanner->Mark();
            if (const TokenInstance& token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                expr = MakeHolder<PostfixExpression>(token.token, std::move(expr));
            } else if (token.token == Token::LEFT_PAR) {
                mark.Apply();
                expr = MakeHolder<FunctionCallExpression>(std::move(expr), ParseCallOrSubscriptArguments());
            } else if (token.token == Token::LEFT_BRACKET) {
                mark.Apply();
                expr = MakeHolder<SubscriptExpression>(std::move(expr), ParseCallOrSubscriptArguments());
            } else if (token.token == Token::DOT) {
                expr = MakeHolder<MemberAccessExpression>(std::move(expr), MakeHolder<IdentifierExpression>(scanner->NextTokenAssert(Token::IDENTIFIER).text));
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    Holder<IExpression> ParsePrefixExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD, Token::SUB, Token::ADD_ADD, Token::SUB_SUB };
        std::stack<Token> operators;
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                operators.push(token.token);
            } else {
                mark.Apply();
                auto expr = ParsePostfixUnaryExpression();
                while (!operators.empty()) {
                    expr = MakeHolder<PrefixExpression>(operators.top(), std::move(expr));
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
        if (scanner->NextTokenLookahead().token == Token::FN) {
            return ParseFunctionDefinitionExpression();
        } else if (scanner->NextTokenLookahead().token == Token::OP) {
            // TODO return ParseOperatorDefinitionExpression();
        } else if (scanner->NextTokenLookahead().token == Token::CLASS) {
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
            if (const TokenInstance& token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                if constexpr (associativity == Associativity::LEFT) {
                    expr = MakeHolder<BinaryExpression>(token.token, std::move(expr), (this->*next)());
                } else {
                    expr = MakeHolder<BinaryExpression>(token.token, std::move(expr), ParseBinaryExpression<next, allow_newline_before_op, associativity, tokens...>());
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