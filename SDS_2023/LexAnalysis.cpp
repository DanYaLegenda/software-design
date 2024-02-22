#include "pch.h"
#include "LexAnalysis.h"
#include "Graphs.h"
#include<stack>
#include<string>

int DecimicalNotation(std::string input, int scaleofnot)	// перевод 16-ного токена в 10-ное число
{
	std::string num = input.substr(1, input.size());
	const char* c = num.c_str();
	int res = strtol(c, NULL, 16);
	return res;
}


namespace Lexer
{
	Graph graphs[N_GRAPHS] =	// соответствие лексем и токенов
	{
		{ LEX_SEPARATORS, FST::FST(GRAPH_SEPARATORS)) },
		{ LEX_LITERAL, FST::FST(GRAPH_INT_LITERAL) },
		{ LEX_LITERAL, FST::FST(GRAPH_STRING_LITERAL) },
		{ LEX_LITERAL, FST::FST(GRAPH_OCT) },
		{ LEX_LITERAL_NUMO, FST::FST(GRAPH_V_LITERAL) },
		{ LEX_NEW, FST::FST(GRAPH_NEW) },
		{ LEX_MAIN, FST::FST(GRAPH_MAIN) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_NUMBER) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_STRING) },
		{ LEX_FUNCTION, FST::FST(GRAPH_FUNCTION) },
		{ LEX_SCRIPT, FST::FST(GRAPH_SCRIPT) },
		{ LEX_RETURN, FST::FST(GRAPH_RETURN) },
		{ LEX_WRITE, FST::FST(GRAPH_WRITE) },
		{ LEX_NEWLINE, FST::FST(GRAPH_NEWLINE) },
		{ LEX_CONDITION, FST::FST(GRAPH_CONDITION) },
		{ LEX_CYCLE, FST::FST(GRAPH_CYCLE) },
		{ LEX_ISFALSE, FST::FST(GRAPH_ISFALSE) },
		{ LEX_ISTRUE, FST::FST(GRAPH_ISTRUE) },
		{ LEX_MOD, FST::FST(GRAPH_MOD) },
		{ LEX_ID, FST::FST(GRAPH_ID) }
	};

	char* getScopeName(IT::IdTable idtable, char* prevword) // имя текущей области видимости (если находимся внутри блока)
	{
		char* a = new char[5];
		a[0] = 'm';
		a[1] = 'a';
		a[2] = 'i';
		a[3] = 'n';
		a[4] = '\0';
		if (strcmp(prevword, MAIN) == 0)
			return a;
		for (int i = idtable.size - 1; i >= 0; i--)
			if (idtable.table[i].idtype == IT::IDTYPE::F)
				return idtable.table[i].id;
		return nullptr;		// не найдено имя функции
	}

	int getLiteralIndex(IT::IdTable ittable, char* value, IT::IDDATATYPE type)	// получаем индекс повторного литерала(по значению)
	{
		for (int i = 0; i < ittable.size; i++)
		{
			if (ittable.table[i].idtype == IT::IDTYPE::L && ittable.table[i].iddatatype == type)
			{
				switch (type)
				{
				case IT::IDDATATYPE::NUM:		// преобразовать токен целочесленного литерала в десятичное число
					if (ittable.table[i].value.vint == atoi(value))
						return i;
					break;
				case IT::IDDATATYPE::STR:
					char buf[STR_MAXSIZE];
					for (unsigned j = 1; j < strlen(value) - 1; j++)	// строковый литерал без кавычек
						buf[j - 1] = value[j];
					buf[strlen(value) - 2] = IN_CODE_NULL;
					if (strcmp(ittable.table[i].value.vstr.str, buf) == 0)
						return i;
					break;
				}
			}
		}
		return NULLIDX_TI;
	}

	IT::IDDATATYPE getType(char* curword, char* idtype)		// какой тип токена
	{
		if (!strcmp(TYPE_SCRIPT, idtype))
			return IT::IDDATATYPE::PROC; // скрипты
		if (!strcmp(TYPE_STRING, idtype))
			return IT::IDDATATYPE::STR;  // строковый идентификатор
		if (!strcmp(TYPE_NUMBER, idtype))
			return IT::IDDATATYPE::NUM;	 // числовой  идентификатор
		if (isdigit(*curword) || *curword == LEX_MINUS)
			return IT::IDDATATYPE::NUM;	 // целочисленный литерал
		else if (*curword == IN_CODE_QUOTE)
			return IT::IDDATATYPE::STR;	 // строковый литерал

		return IT::IDDATATYPE::UNDEF;	 // неопределенный тип; индикатор ошибки
	}

	bool isOctLiteral(char* ch)
	{
		if (ch[0] == '8' && ch[1] == 'x')
			return true;
		return false;
	}

	// из 8 в 10
	char* for_10(char* s, Log::LOG& log, int curline)
	{

		char* tss = new char[std::strlen(s)];

		for (int i = 2; i < strlen(s) + 1; i++)
			tss[i - 2] = s[i];
		tss[std::strlen(s) - 1] = '\0';

		for (int i = 0; i < strlen(tss); i++)
			if (tss[i] < 48 || tss[i] > 55)
				Log::writeError(log.stream, Error::GetError(204, curline, 0));

		int num = atoi(tss);

		int digit = 0;
		int res = 0;
		int power = 0;
		while (num != 0)
		{
			digit = num % 10;
			res += digit * pow(8, power);
			power++;
			num /= 10;
		}
		int temp = res;
		power = 0;
		while (temp != 0)
		{
			temp /= 10;
			power++;
		}

		char* ch = new char[power + 1];
		int t3 = power;
		for (int i = 0; i < power; i++)
		{
			int t2 = res / pow(10, (t3 - 1));
			ch[i] = t2 + 48;
			res = res % (int)pow(10, (t3 - 1));
			t3--;
		}
		ch[power] = '\0';
		return ch;

	}


	int getIndexInLT(LT::LexTable& lextable, int itTableIndex)		
	{
		if (itTableIndex == NULLIDX_TI)		
			return lextable.size;
		for (int i = 0; i < lextable.size; i++)
			if (itTableIndex == lextable.table[i].idxTI)
				return i;
		return NULLIDX_TI;
	}

	bool isLiteral(char* id)	// проверка, ялвяется ли токен литералом
	{
		if (isdigit(*id) || *id == IN_CODE_QUOTE || *id == LEX_MINUS)
			return true;
		return false;
	}

	IT::STDFNC getStandFunction(char* id)
	{
		if (!strcmp(STRCOPY, id))
			return IT::STDFNC::F_STRCOPY;
		if (!strcmp(LENGHT, id))
			return IT::STDFNC::F_LENGHT;
		if (!strcmp(ATOII, id))
			return IT::STDFNC::F_ATOII;
		return IT::STDFNC::F_NOT_STD;
	}

	char* getNextLiteralName()		// сгенерировать следующее имя литерала
	{
		static int literalNumber = 1;
		char* buf = new char[SCOPED_ID_MAXSIZE], num[3];
		strcpy_s(buf, MAXSIZE_ID, "LTRL");
		_itoa_s(literalNumber++, num, 10);
		strcat(buf, num);
		return buf;
	}

	IT::Entry* getEntry(		
		Lexer::LEX& tables,		// ТЛ + ТИ
		char lex,				
		char* id,				
		char* idtype,			// предыдущая (тип)
		bool isParam,			// признак параметра функции
		bool isFunc,			// признак функции
		Log::LOG log,			
		int line,				// строка в исходном тексте
		bool& lex_ok)			// флаг ошибки(по ссылке)
	{

		IT::IDDATATYPE type = getType(id, idtype);	// тип данных
		int index = IT::isId(tables.idtable, id);	// индекс существующего идентификатора или -1
		if (lex == LEX_LITERAL)
			index = getLiteralIndex(tables.idtable, id, type);
		if (index != NULLIDX_TI)
			return nullptr;		// уже существует

		IT::Entry* itentry = new IT::Entry;
		itentry->iddatatype = type;		// сохраняем тип данных

		itentry->idxfirstLE = getIndexInLT(tables.lextable, index);		// индекс первой строки в таблице лексем 

		if (lex == LEX_LITERAL)		// литерал
		{
			bool int_ok = IT::SetValue(itentry, id);
			if (!int_ok)
			{
				Log::writeError(log.stream, Error::GetError(313, line, 0));		// превышен максимальный размер числового литерала
				lex_ok = false;
			}
			if (itentry->iddatatype == IT::IDDATATYPE::STR && itentry->value.vstr.len == 0)
			{
				Log::writeError(log.stream, Error::GetError(310, line, 0));		// пустой строковый литерал
				lex_ok = false;
			}
			strcpy_s(itentry->id, getNextLiteralName());
			itentry->idtype = IT::IDTYPE::L;
		}
		else	// если идентификатор
		{
			switch (type)
			{
			case IT::IDDATATYPE::STR:	// если строковый, то копируем литерал
				strcpy_s(itentry->value.vstr.str, "");
				itentry->value.vstr.len = STR_DEFAULT;
				break;
			case IT::IDDATATYPE::NUM:	// если целочисленный, то записываем
				itentry->value.vint = NUM_DEFAULT;
				break;
			case IT::IDDATATYPE::OCTSTR:
				itentry->value.vint = NUM_DEFAULT;
				break;
			}

			if (isFunc)
			{
				switch (getStandFunction(id))
				{
				case IT::STDFNC::F_STRCOPY:
				{
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = STRCOPY_TYPE;
					itentry->value.params.count = STRCOPY_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[STRCOPY_PARAMS_CNT];
					for (int k = 0; k < STRCOPY_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::STRCOPY_PARAMS[k];
					break;
				}
				case IT::STDFNC::F_LENGHT:
				{
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = LENGHT_TYPE;
					itentry->value.params.count = LENGHT_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[LENGHT_PARAMS_CNT];
					for (int k = 0; k < LENGHT_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::LENGHT_PARAMS[k];
					break;
				}
				case IT::STDFNC::F_ATOII:
				{
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = ATOII_TYPE;
					itentry->value.params.count = ATOII_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[ATOII_PARAMS_CNT];
					for (int k = 0; k < ATOII_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::ATOII_PARAMS[k];
					break;
				}
				case IT::STDFNC::F_NOT_STD:
					itentry->idtype = IT::IDTYPE::F;
					break;
				}
			}
			else if (isParam)
				itentry->idtype = IT::IDTYPE::P;
			else
				itentry->idtype = IT::IDTYPE::V;

			strncpy_s(itentry->id, id, SCOPED_ID_MAXSIZE);
		}

		int i = tables.lextable.size;	 // индекс в ТЛ текущего идентификатора
		if (i > 1 && itentry->idtype == IT::IDTYPE::V && tables.lextable.table[i - 2].lexema != LEX_NEW)
		{
			Log::writeError(log.stream, Error::GetError(304, line, 0));		// в объявлении отсутствует ключевое слово new
			lex_ok = false;
		}
		if (i > 1 && itentry->idtype == IT::IDTYPE::F && tables.lextable.table[i - 1].lexema != LEX_FUNCTION)
		{
			Log::writeError(log.stream, Error::GetError(303, line, 0));		// в объявлении не указан тип функции
			lex_ok = false;
		}
		if (itentry->iddatatype == IT::IDDATATYPE::UNDEF)
		{
			Log::writeError(log.stream, Error::GetError(300, line, 0));		// невозможно определелить тип
			lex_ok = false;
		}

		return itentry;
	}

	bool analyze(LEX& tables, In::IN& in, Log::LOG& log, Parm::PARM& parm)		// анализ действий при встрече соответствующих лексем
	{
		static bool lex_ok = true;
		tables.lextable = LT::Create(MAXSIZE_LT);
		tables.idtable = IT::Create(MAXSIZE_TI);

		bool isParam = false, isFunc = false;
		int enterPoint = NULL;
		char curword[STR_MAXSIZE], nextword[STR_MAXSIZE];	// один символ из исходного кода
		int curline;
		std::stack <char*> scopes;			// область видимости

		for (int i = 0; i < In::InWord::size; i++)
		{
			strcpy_s(curword, in.words[i].word);
			if (i < In::InWord::size - 1)
				strcpy_s(nextword, in.words[i + 1].word);
			curline = in.words[i].line;		// берем исходную строку из структуры
			isFunc = false;
			int idxTI = NULLIDX_TI;

			for (int j = 0; j < N_GRAPHS; j++)
			{
				FST::FST fst(curword, graphs[j].graph);
				if (FST::execute(fst))
				{
					char lexema = graphs[j].lexema;		// одна лексема из таблицы
					switch (lexema)
					{
					case LEX_MAIN:
						enterPoint++;
						break;
					case LEX_SEPARATORS:
					{
						switch (*curword)
						{
						case LEX_LEFTHESIS:		// открывающая скобка - параметры  функции
						{
							isParam = true;
							if (*nextword == LEX_RIGHTTHESIS || ISTYPE(nextword))
							{
								char* functionname = new char[MAXSIZE_ID];
								char* scopename = getScopeName(tables.idtable, in.words[i - 1].word);
								if (scopename == nullptr)
									break;
								strcpy_s(functionname, MAXSIZE_ID, scopename);
								scopes.push(functionname);
							}
							break;
						}
						case LEX_RIGHTTHESIS:	// закрывающая скобка
						{
							isParam = false;
							if (*in.words[i - 1].word == LEX_LEFTHESIS || (i > 2 && (tables.lextable.table[tables.lextable.size - 2].lexema == LEX_ID)))
								scopes.pop();
							break;

						}
						case LEX_LEFTBRACE:		// начало области видимости
						{
							if (i > 0 && *in.words[i - 1].word == LEX_ISFALSE || *in.words[i - 1].word == LEX_ISTRUE || *in.words[i - 1].word == LEX_CYCLE)
								break;
							char* functionname = new char[MAXSIZE_ID];
							char* scopename = getScopeName(tables.idtable, in.words[i - 1].word);
							if (scopename == nullptr)  break;		// получаем имя этой области
							strcpy_s(functionname, MAXSIZE_ID, scopename);
							scopes.push(functionname);
							break;
						}
						case LEX_BRACELET:		// конец области видимости
						{
							if (*in.words[i + 1].word == LEX_ID_TYPE || *in.words[i + 1].word == LEX_SCRIPT || *in.words[i + 1].word == LEX_MAIN)
								if (!scopes.empty())
									scopes.pop();
							break;
						}
						}
						lexema = *curword;
						break;
					}
					case LEX_ID:
					case LEX_LITERAL:
					{
						char id[STR_MAXSIZE] = "";
						idxTI = NULLDX_TI;		// индекс идентификатора в ТИ
						if (*nextword == LEX_LEFTHESIS)
							isFunc = true;					// идентификатор функции
						char* idtype = (isFunc && i > 1) ?	// тип идентификатора
							in.words[i - 2].word : in.words[i - 1].word;		// пропускаем ключевое слово function
						if (!isFunc && !scopes.empty())
							strncpy_s(id, scopes.top(), MAXSIZE_ID);
						strncat(id, curword, MAXSIZE_ID);
						if (isLiteral(curword))
							if (!isOctLiteral(curword))
								strcpy_s(id, curword);	
							else
								strcpy_s(id, for_10(curword, log, curline));

						IT::Entry* itentry = getEntry(tables, lexema, id, idtype, isParam, isFunc, log, curline, lex_ok);
						if (itentry != nullptr)		
						{
							if (isFunc)		// если функция - сохранить список параметров
							{
								if (getStandFunction(id) == IT::STDFNC::F_NOT_STD)	
								{
									itentry->value.params.count = NULL;
									itentry->value.params.types = new IT::IDDATATYPE[MAX_PARAMS_COUNT];
									for (int k = i; in.words[k].word[0] != LEX_RIGHTTHESIS; k++)
									{
										if (k == In::InWord::size || in.words[k].word[0] == LEX_SEPARATOR)
											break;
										if (ISTYPE(in.words[k].word))
										{
											if (itentry->value.params.count >= MAX_PARAMS_COUNT)
											{
												Log::writeError(log.stream, Error::GetError(306, curline, 0));
												lex_ok = false;
												break;
											}
											itentry->value.params.types[itentry->value.params.count++] = getType(NULL, in.words[k].word);
										}
									}
								}
							}
							IT::Add(tables.idtable, *itentry);
							idxTI = tables.idtable.size - 1;
						}
						else     // повторный идентификатор
						{
							int i = tables.lextable.size - 1;	// попытка переопределить идентификатор
							if (i > 0 && tables.lextable.table[i - 1].lexema == LEX_NEW || tables.lextable.table[i].lexema == LEX_NEW
								|| tables.lextable.table[i - 1].lexema == LEX_FUNCTION || tables.lextable.table[i].lexema == LEX_FUNCTION
								|| tables.lextable.table[i - 1].lexema == LEX_ID_TYPE || tables.lextable.table[i].lexema == LEX_ID_TYPE
								|| tables.lextable.table[i - 1].lexema == LEX_SCRIPT || tables.lextable.table[i].lexema == LEX_SCRIPT
								|| tables.lextable.table[i - 1].lexema == LEX_LITERAL_NUMO)
							{
								Log::writeError(log.stream, Error::GetError(305, curline, 0));
								lex_ok = false;
							}
							idxTI = IT::isId(tables.idtable, id);	
							if (lexema == LEX_LITERAL || lexema == LEX_LITERAL_NUMO)
								idxTI = getLiteralIndex(tables.idtable, curword, getType(id, in.words[i - 1].word)); 
						}
					}
					break;
					}

					LT::Entry* ltentry = new LT::Entry(lexema, curline, idxTI);
					LT::Add(tables.lextable, *ltentry);			
					break;
				}
				else if (j == N_GRAPHS - 1)		 
				{
					Log::writeError(log.stream, Error::GetError(201, curline, 0));
					lex_ok = false;
				}
			}
		}

		if (enterPoint == NULL)		// не найдена точка входа
		{
			Log::writeError(log.stream, Error::GetError(301));
			lex_ok = false;
		}
		if (enterPoint > 1)		//больше одной точки входа
		{
			Log::writeError(log.stream, Error::GetError(302));
			lex_ok = false;
		}
		return lex_ok;
	}
};