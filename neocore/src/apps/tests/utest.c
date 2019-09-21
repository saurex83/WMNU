#include "utest_suite.h"
#include "utest_framepart.h"

int main()
{
	nwDebugerInit();

// Вызов функций тестирования
	run_utest_framepart();



// Добавляем результаты тестов
	umsg_summary();
	while(0);
	return 0;
}
