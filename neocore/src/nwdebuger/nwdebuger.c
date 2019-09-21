
// Настройка выводов SIG и порта uart для отладки
void nwDebugerInit(void)
{

}

__attribute__((weak)) void STACK_FAILURE(char* msg) {while(1);}