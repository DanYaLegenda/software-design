#pragma once

#include "pch.h"
#include "Header.h"
#include <vector>

// использование vector для польской записи
typedef std::vector <LT::Entry> ltvec;
typedef std::vector <int> intvec;

namespace Polish
{
	bool PolishNotation(Lexer::LEX& lex, Log::LOG& log);		// польская запись
	intvec getExprPositions(Lexer::LEX& lex);			// сохранение позиции начала всех выражений в исходном тексте в вектор
	int  fillVector(int lextable_pos, LT::LexTable& lextable, ltvec& v);			// заполнение вектора лексемами в инфиксной нотации
	bool setPolishNotation(IT::IdTable& idtable, Log::LOG& log, int lextable_pos, ltvec& v);	// получить порядок идентификаторов
	void addToTable(LT::LexTable& new_table, IT::IdTable& idtable, ltvec& v);		// добавить вектор в новую таблицу лексем
};