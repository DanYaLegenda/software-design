#include "pch.h"
#include "IT.h"
#include "LT.h"
#include "Error.h"
#include "LexAnalysis.h"
#include "SemAnalysis.h"

namespace Semantic
{
	bool Semantic::semanticsCheck(Lexer::LEX& tables, Log::LOG& log)	// проверка семантики
	{
		bool sem_ok = true;		// флаг верной семантики

		for (int i = 0; i < tables.lextable.size; i++)
		{
			switch (tables.lextable.table[i].lexema)
			{
			case LEX_NEW:
			{
				if (tables.lextable.table[i + 1].lexema != LEX_ID_TYPE)		// отсутствует тип идентификатора
				{
					sem_ok = false;
					Log::writeError(log.stream, Error::GetError(303, tables.lextable.table[i].sn, 0));
				}
			}
			case LEX_DIRSLASH:
			{
				int k = i;
				if (tables.lextable.table[i + 1].lexema == LEX_ID)
				{
					for (k; k > 0; k--)
					{
						if (tables.lextable.table[k].lexema == LEX_ID)
						{
							if (tables.idtable.table[tables.lextable.table[k].idxTI].id == tables.idtable.table[tables.lextable.table[i + 1].idxTI].id)
							{
								if (tables.lextable.table[k + 2].lexema == LEX_LITERAL && tables.idtable.table[tables.lextable.table[k + 2].idxTI].value.vint == 0)
								{
									sem_ok = false;			// деление на нуль
									Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[k].sn, 0));
								}
							}
						}
					}
				}
				if (tables.lextable.table[i + 1].lexema == LEX_LITERAL)
				{
					if (tables.idtable.table[tables.lextable.table[i + 1].idxTI].value.vint == 0)
					{
						sem_ok = false;
						Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[k].sn, 0));
					}
				}
				break;
			}
			case LEX_EQUAL:		// выражение
			{
				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)	// левый операнд
				{
					IT::IDDATATYPE lefttype = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					bool ignore = false;

					for (int k = i + 1; tables.lextable.table[k].lexema != LEX_SEPARATOR; k++)
					{
						if (k == tables.lextable.size)
							break;		// синтаксическая ошибка - нет ;
						if (tables.lextable.table[k].idxTI != NULLIDX_TI) // если идентификатор - проверить совпадение типов
						{
							if (!ignore)
							{
								IT::IDDATATYPE righttype = tables.idtable.table[tables.lextable.table[k].idxTI].iddatatype;
								if (lefttype != righttype)	// типы данных в выражении не совпадают
								{
									Log::writeError(log.stream, Error::GetError(314, tables.lextable.table[k].sn, 0));
									sem_ok = false;
									break;
								}
							}

							if (tables.lextable.table[k + 1].lexema == LEX_LEFTHESIS)		// если лексема сразу после идентификатора скобка - это вызов функции
							{
								ignore = true;
								continue;
							}

							if (ignore && tables.lextable.table[k + 1].lexema == LEX_RIGHTTHESIS)		// закрывающая скобка после списка параметров
							{
								ignore = false;
								continue;
							}
						}

						if (lefttype == IT::IDDATATYPE::STR)	// справа только литерал, идентфиикатор или вызов строковой фукнции
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_PLUS || l == LEX_MINUS || l == LEX_STAR)	// выражениу rvalue недопустимо
							{
								Log::writeError(log.stream, Error::GetError(316, tables.lextable.table[k].sn, 0));
								sem_ok = false;
								break;
							}
						}
					}
				}
				break;
			}
			case LEX_ID:	// проверка типа возвращаемого значения  
			{
				IT::Entry e = tables.idtable.table[tables.lextable.table[i].idxTI];

				if (i > 0 && tables.lextable.table[i - 1].lexema == LEX_FUNCTION)
				{
					if (e.idtype == IT::IDTYPE::F)	//функция, не скрипт
					{
						for (int k = i + 1; ; k++)
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_RETURN)
							{
								int next = tables.lextable.table[k + 1].idxTI;	// следующее значение за return
								if (next != NULLIDX_TI)
								{
									if (tables.idtable.table[next].iddatatype != e.iddatatype)
									{					// тип функции и возвращаемого значения не совпадают
										Log::writeError(log.stream, Error::GetError(315, tables.lextable.table[k].sn, 0));
										sem_ok = false;
										break;
									}
								}
								break;	// найден exit
							}

							if (k == tables.lextable.size) break;
						}
					}
				}

				if (tables.lextable.table[i + 1].lexema == LEX_LEFTHESIS && tables.lextable.table[i - 1].lexema != LEX_FUNCTION) // именно вызов
				{
					if (e.idtype == IT::IDTYPE::F || e.idtype == IT::IDTYPE::S)		// точно функция
					{
						int paramscount = NULL;
						for (int j = i + 1; tables.lextable.table[j].lexema != LEX_RIGHTTHESIS; j++)		// проверка передаваемых параметров
						{
							if (tables.lextable.table[j].lexema == LEX_ID || tables.lextable.table[j].lexema == LEX_LITERAL)
							{						// проверка соответствия передаваемых параметров прототипам
								paramscount++;
								if (e.value.params.count == NULL)
									break;
								IT::IDDATATYPE ctype = tables.idtable.table[tables.lextable.table[j].idxTI].iddatatype;
								if (ctype != e.value.params.types[paramscount - 1])
								{					// несовпадение типов передаваемых параметров
									Log::writeError(log.stream, Error::GetError(309, tables.lextable.table[i].sn, 0));
									sem_ok = false;
									break;
								}
							}
							if (j == tables.lextable.size)
								break;
						}
						if (paramscount != e.value.params.count)
						{			// количество передаваемых и принимаемых параметров не совпадает
							Log::writeError(log.stream, Error::GetError(308, tables.lextable.table[i].sn, 0));
							sem_ok = false;
						}
						if (paramscount > 3)
						{					// слишком много параметров в вызове
							Log::writeError(log.stream, Error::GetError(307, tables.lextable.table[i].sn, 0));
							sem_ok = false;
						}
					}
				}
				break;
			}
			case LEX_MORE:	case LEX_LESS: case LEX_EQUALS:   case LEX_NOTEQUALS:
			{
				bool flag = true;			// левый и правый операнд - числовой тип
				if (i > 1 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)
				{
					if (tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype != IT::IDDATATYPE::NUM)
						flag = false;
				}
				if (tables.lextable.table[i + 1].idxTI != NULLIDX_TI)
				{
					if (tables.idtable.table[tables.lextable.table[i + 1].idxTI].iddatatype != IT::IDDATATYPE::NUM)
						flag = false;
				}
				if (!flag)
				{			// строка или неизвестный идентификатор в условии
					Log::writeError(log.stream, Error::GetError(317, tables.lextable.table[i].sn, 0));
					sem_ok = false;
				}
				break;
			}
			}
		}
		// если нигде не словили исключение, то возвращаем флаг успешного семантического анализа
		return sem_ok;
	}
};