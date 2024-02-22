#pragma once
#include <tchar.h>

// макросы параметров коммандной строки
#define PARM_IN			L"-in:"
#define PARM_OUT		L"-out:"
#define PARM_LOG		L"-log:"
#define PARM_MAX_SIZE 300
#define PARM_OUT_DEFAULT_EXT L".asm"
#define PARM_LOG_DEFAULT_EXT L".log"


namespace Parm		// считывание параметров коммандной строки
{
	struct PARM
	{
		wchar_t in[PARM_MAX_SIZE];		// параметр in
		wchar_t out[PARM_MAX_SIZE];		// параметр out
		wchar_t log[PARM_MAX_SIZE];		// параметр log
	};

	PARM getparm(int argc, wchar_t* argv[]);	// получить параметры
};
