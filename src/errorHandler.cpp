#include "errorHandler.h"

void terminate(byte err_code)
{
    const char err_msg[3] = {'E', err_code, '\0'};
    remoteBroadcast(err_msg);
    delay(100);

    cli();
    srReset();

    screen.home();
    screen.print(F(TT_ERROR));
    screen.print(err_code);

    while (1);
}

void warning(byte warn_code)
{
    const char warn_msg[3] = {'W', warn_code, '\0'};
    remoteBroadcast(warn_msg);

    // ЧЕК прямо как в автомобиле
    screen.home();
    screen.print(F("Check: "));
    screen.print(warn_code);
}