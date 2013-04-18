#ifndef CONSOLE_T_H_
#define CONSOLE_T_H_

#include <stddef.h>
#include <stdint.h>

namespace console
{

typedef void (* callback_t) (const char *command);

void set_callback (callback_t handler);
void endl ();
inline void show_prompt ();

char *str_argument (uint8_t position, const char *def = NULL);
int32_t int_argument (uint8_t position);

void tick ();

}

#endif /* CONSOLE_T_H_ */
