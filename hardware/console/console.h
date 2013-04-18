/************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
*************************************************************************/

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
