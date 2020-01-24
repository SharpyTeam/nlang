//
// Created by selya on 16.11.2019.
//

#ifndef NLANG_PARSER_HPP
#define NLANG_PARSER_HPP

#include "scanner.hpp"
#include <ast.hpp>

#include <functional>
#include <memory>
#include <utility>
#include <map>
#include <unordered_set>

namespace nlang {

class Parser {
public:
    std::shared_ptr<Expression> ParseExpression() {
        if (scanner->NextTokenLookahead().token == Token::FN) {
            return ParseFunctionDefExpression();
        }
        return ParseAssignExpression();
    }

    std::shared_ptr<Statement> ParseStatement() {
        switch (scanner->NextTokenLookahead().token) {
            case Token::LET:
                return ParseVarDefStatement();

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

    std::shared_ptr<FileNode> ParseFile() {
        auto statements = ParseStatements();
        scanner->NextTokenAssert(Token::THE_EOF);
        return std::make_shared<FileNode>(statements);
    }

    static std::shared_ptr<Parser> Create(const std::shared_ptr<Scanner> &scanner) {
        return std::shared_ptr<Parser>(new Parser(scanner));
    }

private:
    std::shared_ptr<Scanner> scanner;


    explicit Parser(const std::shared_ptr<Scanner> &scanner)
        : scanner(scanner)
    {
        scanner->ResetIgnore();
        scanner->SetIgnore(Token::SPACE);
        scanner->SetIgnore(Token::COMMENT);
        scanner->SetIgnore(Token::NEWLINE);
    }

    bool NextIsStatementBreak() {
        if (auto token = scanner->NextTokenLookahead(); token.token == Token::SEMICOLON || token.token == Token::THE_EOF) {
            return true;
        }
        bool ignoring_newline = scanner->IsIgnoring(Token::NEWLINE);
        scanner->SetIgnore(Token::NEWLINE, false);
        bool has_newline = scanner->NextTokenLookahead().token == Token::NEWLINE;
        scanner->SetIgnore(Token::NEWLINE, ignoring_newline);
        return has_newline;
    }

    bool TryEatStatementBreak() {
        if (!NextIsStatementBreak()) {
            return false;
        }
        if (scanner->TrySkipToken(Token::SEMICOLON) || scanner->NextTokenLookahead().token == Token::THE_EOF) {
            return true;
        }
        bool ignoring_newline = scanner->IsIgnoring(Token::NEWLINE);
        scanner->SetIgnore(Token::NEWLINE, false);
        while (scanner->TrySkipToken(Token::NEWLINE)) {

        }
        scanner->SetIgnore(Token::NEWLINE, ignoring_newline);
        return true;
    }

    std::vector<std::shared_ptr<Statement>> ParseStatements() {
        std::vector<std::shared_ptr<Statement>> statements;

        while (true) {
            if (auto token = scanner->NextTokenLookahead(); token.token == Token::RIGHT_BRACE || token.token == Token::THE_EOF) {
                break;
            }
            statements.emplace_back(ParseStatement());
            if (!TryEatStatementBreak()) {
                break;
            }
        }

        return statements;
    }

    std::shared_ptr<Statement> ParseExpressionStatement() {
        return std::make_shared<ExpressionStatement>(ParseExpression());
    }

    std::shared_ptr<Statement> ParseBlockStatement() {
        scanner->NextTokenAssert(Token::LEFT_BRACE);
        auto r = std::make_shared<BlockStatement>(ParseStatements());
        scanner->NextTokenAssert(Token::RIGHT_BRACE);
        return r;
    }

    std::shared_ptr<Statement> ParseVarDefStatement() {
        scanner->NextTokenAssert(Token::LET);
        const std::string identifier = scanner->NextTokenAssert(Token::IDENTIFIER).source;
        if (scanner->TrySkipToken(Token::ASSIGN)) {
            return std::make_shared<VarDefStatement>(identifier, ParseExpression());
        }
        return std::make_shared<VarDefStatement>(identifier);
    }

    std::shared_ptr<Statement> ParseReturnStatement() {
        scanner->NextTokenAssert(Token::RETURN);
        if (NextIsStatementBreak()) {
            return std::make_shared<ReturnStatement>();
        }
        return std::make_shared<ReturnStatement>(ParseExpression());
    }

    std::shared_ptr<Statement> ParseBreakStatement() {
        scanner->NextTokenAssert(Token::BREAK);
        return std::make_shared<BreakStatement>();
    }

    std::shared_ptr<Statement> ParseContinueStatement() {
        scanner->NextTokenAssert(Token::CONTINUE);
        return std::make_shared<ContinueStatement>();
    }

    std::shared_ptr<Statement> ParseIfStatement() {
        std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Statement>>> ifs;
        std::shared_ptr<Statement> else_statement;
        while (true) {
            scanner->NextTokenAssert(Token::IF);
            scanner->NextTokenAssert(Token::LEFT_PAR);
            auto expr = ParseExpression();
            scanner->NextTokenAssert(Token::RIGHT_PAR);
            auto body = ParseBlockStatement();
            ifs.emplace_back(expr, body);
            if (!scanner->TrySkipToken(Token::ELSE)) {
                break;
            }
            if (scanner->NextTokenLookahead().token != Token::IF) {
                else_statement = ParseBlockStatement();
                break;
            }
        }
        return std::make_shared<IfStatement>(ifs, else_statement);
    }

    std::shared_ptr<Statement> ParseWhileStatement() {
        scanner->NextTokenAssert(Token::WHILE);
        scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        scanner->NextTokenAssert(Token::RIGHT_PAR);
        return std::make_shared<WhileStatement>(expr, ParseBlockStatement());
    }

#define BINARY(name, next, ...)                                                                 \
    std::shared_ptr<Expression> name() {                                                        \
        static std::unordered_set<Token> tokens { __VA_ARGS__ };                                \
        auto expr = next();                                                                     \
        while (true) {                                                                          \
            auto mark = scanner->Mark();                                                        \
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {  \
                expr = std::make_shared<BinaryExpression>(token.token, expr, next());           \
            } else {                                                                            \
                mark.Apply();                                                                   \
                break;                                                                          \
            }                                                                                   \
        }                                                                                       \
        return expr;                                                                            \
    }

    std::shared_ptr<Expression> ParseParenthesizedExpression() {
        scanner->NextTokenAssert(Token::LEFT_PAR);
        auto expr = ParseExpression();
        scanner->NextTokenAssert(Token::RIGHT_PAR);
        return expr;
    }

    std::shared_ptr<Expression> ParseBasicExpression() {
        auto mark = scanner->Mark();
        auto token = scanner->NextToken();
        switch (token.token) {
            case Token::LEFT_PAR:
                mark.Apply();
                return ParseParenthesizedExpression();

            case Token::IDENTIFIER:
                return std::make_shared<IdentifierExpression>(token.source);

            case Token::NUMBER:
                return std::make_shared<NumberLiteralExpression>(std::stod(token.source));

            case Token::STRING:
                return std::make_shared<StringLiteralExpression>(token.source);

            case Token::THE_NULL:
            case Token::THE_TRUE:
            case Token::THE_FALSE:
                return std::make_shared<LiteralExpression>(token.token);

            default:
                throw std::runtime_error("Unexpected token \"" + token.source + "\" at [" + std::to_string(token.row) + ":" + std::to_string(token.column) + "]");
        }
    }

    std::shared_ptr<Expression> ParsePostfixUnaryExpression() {
        static std::unordered_set<Token> tokens { Token::ADD_ADD, Token::SUB_SUB };
        auto expr = ParseBasicExpression();
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {
                expr = std::make_shared<PostfixExpression>(token.token, expr);
            } else if (token.token == Token::LEFT_PAR) {
                std::vector<std::shared_ptr<Expression>> arguments;
                while (true) {
                    if (scanner->TrySkipToken(Token::RIGHT_PAR)) {
                        break;
                    }
                    arguments.emplace_back(ParseExpression());
                    if (scanner->TrySkipToken(Token::COMMA)) {
                        if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                            throw std::runtime_error("Expected arg, found par");
                        }
                    }
                }
                expr = std::make_shared<FunctionCallExpression>(expr, arguments);
            } else {
                mark.Apply();
                break;
            }
        }
        return expr;
    }

    std::shared_ptr<Expression> ParsePrefixUnaryExpression() {
        static std::unordered_set<Token> tokens { Token::ADD, Token::SUB, Token::ADD_ADD, Token::SUB_SUB };
        std::stack<Token> operators;
        while (true) {
            auto mark = scanner->Mark();
            if (auto token = scanner->NextToken(); tokens.find(token.token) != tokens.end()) {
                operators.push(token.token);
            } else {
                mark.Apply();
                auto expr = ParsePostfixUnaryExpression();
                while (!operators.empty()) {
                    expr = std::make_shared<UnaryExpression>(operators.top(), expr);
                    operators.pop();
                }
                return expr;
            }
        }
    }

    BINARY(ParseMultiplicativeExpression, ParsePrefixUnaryExpression, Token::MUL, Token::DIV, Token::REMAINDER)
    BINARY(ParseAdditiveExpression, ParseMultiplicativeExpression, Token::ADD, Token::SUB)

    std::shared_ptr<Expression> ParseRangeExpression() {
        // TODO maybe add range syntax later
        return ParseAdditiveExpression();
    }

    BINARY(ParseFunctionAsOperatorExpression, ParseRangeExpression, Token::IDENTIFIER)
    BINARY(ParseComparisonExpression, ParseFunctionAsOperatorExpression, Token::LESS_EQUALS, Token::GREATER_EQUALS, Token::LESS, Token::GREATER)
    BINARY(ParseEqualityExpression, ParseComparisonExpression, Token::EQUALS, Token::NOT_EQUALS)
    BINARY(ParseConjunctionExpression, ParseEqualityExpression, Token::AND)
    BINARY(ParseDisjunctionExpression, ParseConjunctionExpression, Token::OR)
    BINARY(ParseAssignExpression, ParseDisjunctionExpression, Token::ASSIGN, Token::ASSIGN_ADD, Token::ASSIGN_SUB, Token::ASSIGN_MUL, Token::ASSIGN_DIV, Token::ASSIGN_REMAINDER)

#undef BINARY

    std::shared_ptr<Expression> ParseFunctionDefExpression() {
        std::string name;
        std::vector<std::string> args_list;
        std::vector<std::shared_ptr<Statement>> body;
        scanner->NextTokenAssert(Token::FN);
        if (scanner->NextTokenLookahead().token == Token::IDENTIFIER) {
            name = scanner->NextToken().source;
        }
        scanner->NextTokenAssert(Token::LEFT_PAR);
        while (true) {
            if (scanner->TrySkipToken(Token::RIGHT_PAR)) {
                break;
            }
            args_list.emplace_back(scanner->NextTokenAssert(Token::IDENTIFIER).source);
            if (scanner->TrySkipToken(Token::COMMA)) {
                if (scanner->NextTokenLookahead().token == Token::RIGHT_PAR) {
                    throw std::runtime_error("Expected arg, found par");
                }
            }
        }

        return std::make_shared<FunctionDefExpression>(name, args_list, ParseBlockStatement());
    }
};

}

#endif //NLANG_PARSER_HPP
