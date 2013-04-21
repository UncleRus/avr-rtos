#ifndef SDAC_H_
#define SDAC_H_

#include <stdint.h>
#include <avr/io.h>

namespace sdac
{

void init ();
void set (uint16_t value);

}

#endif /* SDAC_H_ */
