#pragma once

#include <common/ast.hpp>

#include <parser/scanner.hpp>

#include <utils/pointers/unique_ptr.hpp>

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

    UniquePtr<ast::Module> ParseModule() {
        return MakeUnique<ast::Module>(ParseStatements());
    }

    UniquePtr<ast::TypeHint> TryParseTypeHint() {
        if (scanner->NextTokenLookahead().token == Token::COLON) {
            auto colon = scanner->NextToken();
            auto identifier = scanner->NextTokenAssert(Token::IDENTIFIER);
            return MakeUnique<ast::TypeHint>(std::move(colon),
                                             MakeUnique<ast::IdentifierLiteral>(std::move(identifier)));
        }
        return nullptr;
    }

    UniquePtr<ast::DefaultValue> TryParseDefaultValue() {
        if (scanner->NextTokenLookahead().token == Token::ASSIGN) {
            auto assignment = scanner->NextToken();
            return MakeUnique<ast::DefaultValue>(std::move(assignment), ParseExpression());
        }
        return nullptr;
    }

    UniquePtr<ast::ArgumentDefinitionStatementPart> ParseArgumentDefinitionStatementPart() {
        auto name = scanner->NextTokenAssert(Token::IDENTIFIER);
        auto type_hint = TryParseTypeHint();
        auto default_value = TryParseDefaultValue();
        return MakeUnique<ast::ArgumentDefinitionStatementPart>(MakeUnique<ast::IdentifierLiteral>(std::move(name)),
                                                                std::move(type_hint), std::move(default_value));
    }

    UniquePtr<ast::IStatement> ParseVariableDefinitionStatement() {
        auto let = scanner->NextTokenAssert(Token::LET);
        auto name = scanner->NextTokenAssert(Token::IDENTIFIER);
        auto type_hint = TryParseTypeHint();
        auto default_value = TryParseDefaultValue();
        return MakeUnique<ast::VariableDefinitionStatement>(std::move(let),
                                                            MakeUnique<ast::IdentifierLiteral>(std::move(name)),
                                                            std::move(type_hint), std::move(default_value));
    }

    UniquePtr<ast::IStatement> ParseReturnStatement() {
        auto ret = scanner->NextTokenAssert(Token::RETURN);
        if (scanner->IsEOL() || scanner->NextTokenLookahead().token == Token::SEMICOLON) {
            return MakeUnique<ast::ReturnStatement>(std::move(ret), nullptr);
        }
        return MakeUnique<ast::ReturnStatement>(std::move(ret), ParseExpression());
    }

    UniquePtr<ast::IStatement> ParseBreakStatement() {
        auto brk = scanner->NextTokenAssert(Token::BREAK);
        if (scanner->IsEOL() || scanner->NextTokenLookahead().token == Token::SEMICOLON) {
            return MakeUnique<ast::BreakStatement>(std::move(brk), nullptr);
        }
        return MakeUnique<ast::BreakStatement>(std::move(brk), ParseExpression());
    }

    UniquePtr<ast::IStatement> ParseContinueStatement() {
        return MakeUnique<ast::ContinueStatement>(TokenInstance(scanner->NextTokenAssert(Token::CONTINUE)));
    }

    UniquePtr<ast::IStatement> ParseIfElseStatement() {
        auto if_token = scanner->NextTokenAssert(Token::IF);
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        auto body = ParseStatement();
        UniquePtr<ast::ElseStatementPart> else_branch;
        if (scanner->NextTokenLookahead().token == Token::ELSE) {
            auto else_token = scanner->NextToken();
            auto else_body = ParseStatement();
            else_branch = MakeUnique<ast::ElseStatementPart>(std::move(else_token), std::move(else_body));
        }
        return MakeUnique<ast::IfElseStatement>(std::move(if_token), std::move(left_par), std::move(expr),
                                                std::move(right_par), std::move(body), std::move(else_branch));
    }

    UniquePtr<ast::IStatement> ParseWhileStatement() {
        auto while_token = scanner->NextTokenAssert(Token::WHILE);
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeUnique<ast::WhileStatement>(std::move(while_token), std::move(left_par), std::move(expr),
                                               std::move(right_par), ParseBlockStatement());
    }

    UniquePtr<ast::IStatement> ParseExpressionStatement() {
        return MakeUnique<ast::ExpressionStatement>(ParseExpression());
    }

    UniquePtr<ast::IStatement> ParseStatement() {
        switch (scanner->NextTokenLookahead().token) {
            case Token::FN:
                return ParseFunctionDefinitionStatement();

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

    std::vector<UniquePtr<ast::IStatement>> ParseStatements() {
        std::vector<UniquePtr<ast::IStatement>> statements;

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

    UniquePtr<ast::IStatement> ParseBlockStatement() {
        auto left_brace = scanner->NextTokenAssert(Token::LEFT_BRACE);
        auto statements = ParseStatements();
        auto right_brace = scanner->NextTokenAssert(Token::RIGHT_BRACE);
        return MakeUnique<ast::BlockStatement>(std::move(left_brace), std::move(statements), std::move(right_brace));
    }

    UniquePtr<ast::FunctionDefinitionStatement> ParseFunctionDefinitionStatement() {
        std::vector<UniquePtr<ast::ArgumentDefinitionStatementPart>> args;

        auto fn = scanner->NextTokenAssert(Token::FN);
        UniquePtr<ast::IdentifierLiteral> name;
        name = MakeUnique<ast::IdentifierLiteral>(TokenInstance(scanner->NextTokenAssert(Token::IDENTIFIER)));
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        TokenInstance right_par;
        bool default_value_required = false;
        while (true) {
            if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                right_par = scanner->NextToken();
                break;
            }

            auto arg = ParseArgumentDefinitionStatementPart();
            arg->index = args.size();

            if (arg->default_value) {
                default_value_required = true;
            } else if (default_value_required) {
                throw std::runtime_error("Expected default value");
            }

            args.emplace_back(std::move(arg));

            if (scanner->NextTokenLookahead().token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                    throw std::runtime_error("Expected arg, found par");
                }
            }
        }

        auto type_hint = TryParseTypeHint();

        return MakeUnique<ast::FunctionDefinitionStatement>(std::move(fn), std::move(name), std::move(left_par),
                                                            std::move(args), std::move(right_par),
                                                            std::move(type_hint), ParseBlockStatement());
    }

    UniquePtr<ast::FunctionDefinitionExpression> ParseFunctionDefinitionExpression() {
        std::vector<UniquePtr<ast::ArgumentDefinitionStatementPart>> args;

        auto fn = scanner->NextTokenAssert(Token::FN);
        UniquePtr<ast::IdentifierLiteral> name;
        if (scanner->NextTokenLookahead().token == Token::IDENTIFIER) {
            name = MakeUnique<ast::IdentifierLiteral>(TokenInstance(scanner->NextToken()));
        }
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        TokenInstance right_par;
        bool default_value_required = false;
        while (true) {
            if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                right_par = scanner->NextToken();
                break;
            }

            auto arg = ParseArgumentDefinitionStatementPart();
            arg->index = args.size();

            if (arg->default_value) {
                default_value_required = true;
            } else if (default_value_required) {
                throw std::runtime_error("Expected default value");
            }

            args.emplace_back(std::move(arg));

            if (scanner->NextTokenLookahead().token == Token::COMMA) {
                scanner->NextToken();
                if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                    throw std::runtime_error("Expected arg, found par");
                }
            }
        }

        auto type_hint = TryParseTypeHint();

        return MakeUnique<ast::FunctionDefinitionExpression>(std::move(fn), std::move(name), std::move(left_par),
                                                             std::move(args), std::move(right_par),
                                                             std::move(type_hint), ParseBlockStatement());
    }

    UniquePtr<ast::IExpression> ParseParenthesizedExpression() {
        auto left_par = scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        auto right_par = scanner->NextTokenAssert(Token::RIGHT_PAR);
        return MakeUnique<ast::ParenthesizedExpression>(std::move(left_par), std::move(expr), std::move(right_par));
    }

    UniquePtr<ast::IExpression> ParseBasicExpression() {
        auto mark = scanner->Mark();
        auto token = scanner->NextToken();
        switch (token.token) {
            case Token::LEFT_PAR:
                mark.Apply();
                return ParseParenthesizedExpression();

            case Token::IDENTIFIER:
                return MakeUnique<ast::LiteralExpression>(MakeUnique<ast::IdentifierLiteral>(std::move(token)));

            case Token::NUMBER:
                return MakeUnique<ast::LiteralExpression>(MakeUnique<ast::NumberLiteral>(std::move(token)));

            case Token::STRING:
                return MakeUnique<ast::LiteralExpression>(MakeUnique<ast::StringLiteral>(std::move(token)));

            case Token::THE_NULL:
                return MakeUnique<ast::LiteralExpression>(MakeUnique<ast::NullLiteral>(std::move(token)));

            case Token::THE_TRUE:
            case Token::THE_FALSE:
                return MakeUnique<ast::LiteralExpression>(MakeUnique<ast::BoolLiteral>(std::move(token)));

            default:
                throw std::runtime_error("Unexpected token at [" + std::to_string(token.row) + ":" + std::to_string(token.column) + "]");
        }
    }

    std::vector<UniquePtr<ast::IExpression>> ParseCallOrSubscriptArguments(Token close) {
        // TODO pass commas as tokens to ast
        std::vector<UniquePtr<ast::IExpression>> arguments;
        while (true) {
            if (scanner->NextTokenLookahead().token == close) {
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

    UniquePtr<ast::IExpression> ParsePostfixExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD_ADD, Token::SUB_SUB };
        auto expr = ParseBasicExpression();
        while (true) {
            auto mark = scanner->Mark();
            if (auto& token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                expr = MakeUnique<ast::PostfixExpression>(std::move(expr), TokenInstance(token));
            } else if (token.token == Token::LEFT_PAR) {
                auto args = ParseCallOrSubscriptArguments(Token::RIGHT_PAR);
                expr = MakeUnique<ast::FunctionCallExpression>(std::move(expr), TokenInstance(token), std::move(args),
                                                               TokenInstance(
                                                                   scanner->NextTokenAssert(Token::RIGHT_PAR)));
            } else if (token.token == Token::LEFT_BRACKET) {
                auto args = ParseCallOrSubscriptArguments(Token::RIGHT_BRACKET);
                expr = MakeUnique<ast::SubscriptExpression>(std::move(expr), TokenInstance(token), std::move(args),
                                                            TokenInstance(
                                                                scanner->NextTokenAssert(Token::RIGHT_BRACKET)));
            } else if (token.token == Token::DOT) {
                expr = MakeUnique<ast::MemberAccessExpression>(std::move(expr), TokenInstance(token),
                                                               MakeUnique<ast::IdentifierLiteral>(TokenInstance(
                                                                   scanner->NextTokenAssert(Token::IDENTIFIER))));
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    UniquePtr<ast::IExpression> ParsePrefixExpression() {
        static std::unordered_set<Token> tokens_set { Token::ADD, Token::SUB, Token::ADD_ADD, Token::SUB_SUB };
        std::stack<TokenInstance> operators;
        while (true) {
            auto mark = scanner->Mark();
            if (auto& token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                operators.emplace(TokenInstance(token));
            } else {
                mark.Apply();
                auto expr = ParsePostfixExpression();
                while (!operators.empty()) {
                    expr = MakeUnique<ast::PrefixExpression>(std::move(operators.top()), std::move(expr));
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

    UniquePtr<ast::IExpression> ParseMultiplicativeExpression() { return ParseBinaryExpression<&Parser::ParsePrefixExpression, false, Associativity::LEFT, Token::MUL, Token::DIV, Token::REMAINDER>(); }
    UniquePtr<ast::IExpression> ParseAdditiveExpression() { return ParseBinaryExpression<&Parser::ParseMultiplicativeExpression, false, Associativity::LEFT, Token::ADD, Token::SUB>(); }
    UniquePtr<ast::IExpression> ParseInfixCallExpression() { return ParseBinaryExpression<&Parser::ParseAdditiveExpression, false, Associativity::LEFT, Token::IDENTIFIER>(); }
    UniquePtr<ast::IExpression> ParseComparisonExpression() { return ParseBinaryExpression<&Parser::ParseInfixCallExpression, false, Associativity::LEFT, Token::LESS_EQUALS, Token::GREATER_EQUALS, Token::LESS, Token::GREATER>(); }
    UniquePtr<ast::IExpression> ParseEqualityExpression() { return ParseBinaryExpression<&Parser::ParseComparisonExpression, false, Associativity::LEFT, Token::EQUALS, Token::NOT_EQUALS>(); }
    UniquePtr<ast::IExpression> ParseConjunctionExpression() { return ParseBinaryExpression<&Parser::ParseEqualityExpression, true, Associativity::LEFT, Token::AND>(); }
    UniquePtr<ast::IExpression> ParseDisjunctionExpression() { return ParseBinaryExpression<&Parser::ParseConjunctionExpression, true, Associativity::LEFT, Token::OR>(); }
    UniquePtr<ast::IExpression> ParseAssignmentExpression() { return ParseBinaryExpression<&Parser::ParseDisjunctionExpression, false, Associativity::RIGHT, Token::ASSIGN, Token::ASSIGN_ADD, Token::ASSIGN_SUB, Token::ASSIGN_MUL, Token::ASSIGN_DIV, Token::ASSIGN_REMAINDER>(); }

    UniquePtr<ast::IExpression> ParseExpression() {
        if (scanner->NextTokenLookahead().token == Token::FN) {
            return ParseFunctionDefinitionExpression();
        } else if (scanner->NextTokenLookahead().token == Token::OP) {
            // TODO return ParseOperatorDefinitionExpression();
        } else if (scanner->NextTokenLookahead().token == Token::CLASS) {
            // TODO return ParseClassDefinitionExpression();
        }
        return ParseAssignmentExpression();
    }

    using ParseExpressionFunctionPointer = UniquePtr<ast::IExpression> (Parser::*)();

    template<ParseExpressionFunctionPointer next, bool allow_newline_before_op, Associativity associativity, Token ...tokens>
    UniquePtr<ast::IExpression> ParseBinaryExpression() {
        static std::unordered_set<Token> tokens_set { tokens... };
        UniquePtr<ast::IExpression> expr = (this->*next)();
        while (true) {
            if constexpr (!allow_newline_before_op) {
                if (scanner->IsEOL()) {
                    break;
                }
            }
            auto mark = scanner->Mark();
            if (auto& token = scanner->NextToken(); tokens_set.find(token.token) != tokens_set.end()) {
                if constexpr (associativity == Associativity::LEFT) {
                    expr = MakeUnique<ast::BinaryExpression>(std::move(expr), TokenInstance(token), (this->*next)());
                } else {
                    expr = MakeUnique<ast::BinaryExpression>(std::move(expr), TokenInstance(token),
                                                             ParseBinaryExpression<next, allow_newline_before_op, associativity, tokens...>());
                }
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    static UniquePtr<Parser> New(UniquePtr<Scanner>&& scanner) {
        return UniquePtr<Parser>(new Parser(std::move(scanner)));
    }

private:
    explicit Parser(UniquePtr<Scanner>&& scanner)
        : scanner(std::move(scanner))
    {

    }

private:
    UniquePtr<Scanner> scanner;
};

}