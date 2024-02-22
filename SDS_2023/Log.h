#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "Parm.h"
#include "Error.h"
#include "In.h"
#include "LT.h"
#include "IT.h"
#include <iomanip>
#include <iostream>

namespace Log
{
	struct LOG	// структура для запсии логов в выходной файл
	{
		wchar_t logfile[PARM_MAX_SIZE];		// путь к файлу
		std::ofstream* stream;				// поток
		LOG() : stream(nullptr)				// копирование
		{
			wcscpy(logfile, L"");
		}
	};

	LOG getstream(wchar_t logfile[]);							// получить поток
	void writeLog(LOG& log);									// написать в протокол
	void writeLine(std::ostream* stream, char* c, ...);			// вывести  строку в поток
	void writeParm(LOG& log, Parm::PARM& parm);					// вывести параметры в протокол
	void writeIn(std::ostream* stream, In::IN& in);				// получить поток
	void writeWords(std::ostream* stream, In::InWord* words);	// вывести слова в поток
	void writeError(std::ostream* stream, Error::ERROR error);	// вывести ошибки в поток
	void Close(LOG& log);										// закрыть поток
}