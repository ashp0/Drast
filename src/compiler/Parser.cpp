//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Parser.h"

std::unique_ptr<AST> Parser::parse() {
	return compound();
}

std::unique_ptr<AST> Parser::compound() {
	std::unique_ptr<CompoundStatement> compoundStatement = std::make_unique<CompoundStatement>(this->current->line);

	std::unique_ptr<AST> statement = this->statement();
	std::cout << statement->toString() << std::endl;
	return statement;
}

std::unique_ptr<AST> Parser::statement() {
	switch (this->current->type) {
		case TokenType::IMPORT:
			return import();
		case TokenType::INT:
		case TokenType::CHAR:
		case TokenType::VOID:
		case TokenType::BOOL:
		case TokenType::FLOAT:
		case TokenType::STRING:
		case TokenType::IDENTIFIER:
			return this->functionOrVariableDeclaration(std::nullopt);
		default:
			return nullptr;
	}

	return nullptr;
}

std::unique_ptr<AST> Parser::import() {
	std::unique_ptr<Import> import = std::make_unique<Import>(this->current->value, this->current->line);

	this->advance(TokenType::IMPORT);

	if (this->current->type != TokenType::STRING) {
		throw std::runtime_error("Expected string literal\n");
	}

	import->setImportPath(this->current->value);

	this->advance(TokenType::STRING);

	return import;
}

std::unique_ptr<AST> Parser::functionOrVariableDeclaration(std::optional<std::vector<TokenType>> modifiers) {
	std::unique_ptr<AST> type = this->type();

	return type;
}

//std::unique_ptr<AST>
//Parser::functionDeclaration(std::optional<std::vector<TokenType>> modifiers, std::unique_ptr<AST> return_type);
//
//std::vector<std::unique_ptr<AST>> Parser::functionArguments();
//
//std::unique_ptr<AST>
//Parser::variableDeclaration(std::optional<std::vector<TokenType>> modifiers, std::unique_ptr<AST> variable_type);
//
//std::unique_ptr<AST> Parser::expression();
//
//std::unique_ptr<AST> Parser::equality();
//
//std::unique_ptr<AST> Parser::comparison();
//
//std::unique_ptr<AST> Parser::term();
//
//std::unique_ptr<AST> Parser::factor();
//
//std::unique_ptr<AST> Parser::unary();
//
//std::unique_ptr<AST> Parser::primary();

std::unique_ptr<AST> Parser::type() {
	std::unique_ptr<Type> type = std::make_unique<Type>(*this->current, false, false, false, this->current->line);

	switch (this->current->type) {
		case TokenType::INT:
		case TokenType::CHAR:
		case TokenType::VOID:
		case TokenType::BOOL:
		case TokenType::FLOAT:
		case TokenType::STRING:
		case TokenType::IDENTIFIER:
			type->setType(*this->current);
			break;
		default:
			throw std::runtime_error("Parser: Expected type\n");
	}

	advance();

	for (;;) {
		switch (this->current->type) {
			case TokenType::QUESTION:
				type->setIsOptional(true);
				break;
			case TokenType::OPERATOR_MUL:
				type->setIsPointer(true);
				break;
			case TokenType::SQUARE_OPEN:
				this->advance();
				type->setIsArray(true);
				break;
			default:
				goto end;
		}

		this->advance();
	}

	end:
	return type;
}

//std::unique_ptr<AST> Parser::modifiers();

void Parser::advance(TokenType type) {
	if (type != this->current->type) {
		throw std::runtime_error("Syntax error: expected " + tokenTypeAsLiteral(type) + " but got " +
		                         tokenTypeAsLiteral(this->current->type));
	}

	this->index += 1;

	if (this->index >= this->tokens.size()) {
		this->current->type = TokenType::T_EOF;
	} else {
		this->current = std::move(this->tokens.at(this->index));
	}
}

void Parser::advance() {
	this->index += 1;

	if (this->index >= this->tokens.size()) {
		this->current->type = TokenType::T_EOF;
	} else {
		this->current = std::move(this->tokens.at(this->index));
	}
}
