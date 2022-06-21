#include "errorHandler.h"

void terminate(byte err_code)
{
    cli();
    srReset();

    screen.home();
    screen.print(F(TT_ERROR));
    screen.print(err_code);

    while (1);
}