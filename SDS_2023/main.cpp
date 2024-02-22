#include "stdafx.h"
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{

	setlocale(0, "ru");
	Log::LOG log;

	try
	{
		Parm::PARM parm = Parm::getparm(argc, argv);                            
		log = Log::getstream(parm.log);											
		Log::writeLog(log);														
		Log::writeParm(log, parm);												
		In::IN in = In::getin(parm.in, log.stream);								
		Log::writeIn(log.stream, in);											


		in.words = In::getWordsTable(log.stream, in.text, in.code, in.size);	
		Lexer::LEX tables;														
		bool lex_ok = Lexer::analyze(tables, in, log, parm);					
		LT::writeLexTable(log.stream, tables.lextable);							
		IT::writeIdTable(log.stream, tables.idtable);							
		LT::writeLexemsOnLines(log.stream, tables.lextable);					
		if (!lex_ok)
		{
			Log::writeLine(log.stream, LEXERROR, "");
			Log::writeLine(&std::cout, LEXERROR, STOP, "");						// ошибка лексического анализа
			return 0;
		}
		else
			Log::writeLine(&std::cout, LEXGOOD, "");							// успешная запись в протокол


		MFST_TRACE_START(log.stream);
		MFST::Mfst mfst(tables, GRB::getGreibach());							// выполнить СИНТАКСИЧЕСКИЙ анализ
		bool synt_ok = mfst.start(log);
		mfst.savededucation();
		mfst.printrules(log);													// вывести дерево разбора
		if (!synt_ok)
		{
			Log::writeLine(log.stream, SYNTERROR, "");
			Log::writeLine(&std::cout, SYNTERROR, STOP, "");					// ошибка синтаксического анализа
			return 0;
		}
		else
			Log::writeLine(&std::cout, SYNTGOOD, "");							// успешная запись в протокол


		bool sem_ok = Semantic::semanticsCheck(tables, log);					// выполнить СЕМАНТИЧЕСКИЙ анализ
		if (!sem_ok)
		{
			Log::writeLine(log.stream, SEMERROR, "");
			Log::writeLine(&std::cout, SEMERROR, STOP, "");						// ошибка семантического анализа
			return 0;
		}
		else
			Log::writeLine(&std::cout, SEMGOOD, "");							// успешная запись в протокол


		bool polish_ok = Polish::PolishNotation(tables, log);					// выполнить преобразование выражений в ПОЛИЗ
		if (!polish_ok)
		{
			Log::writeLine(log.stream, POLISHERROR, "");
			Log::writeLine(&std::cout, POLISHERROR, STOP, "");					// ошибка преобразования выражений
			return 0;
		}
		else Log::writeLine(&std::cout, POLISHGOOD, "");						// успешная запись в протокол



		Log::writeLine(log.stream, MESSAGE, "");								// запись в протокол после преобразования выражений
		LT::writeLexTable(log.stream, tables.lextable);							// записать таблицу лексем
		IT::writeIdTable(log.stream, tables.idtable);							// записать таблицу идентификаторов
		LT::writeLexemsOnLines(log.stream, tables.lextable);					// записать соответствие лексем исходному коду


		Gener::CodeGeneration(tables, parm, log);								// выполнить генерацию кода
		Log::writeLine(log.stream, ALLGOOD, "");								// итог работы программы
		Log::Close(log);													    // закрыть протокол

	}


	catch (Error::ERROR e)
	{
		Log::writeError(log.stream, e);											// обработка исключений
	}
}


