#ifndef MTWI_H_
#define MTWI_H_

#include <stdint.h>

#define MTWI_ERR_TIMEOUT 1
#define MTWI_ERR_BUS_FAIL 2
#define MTWI_ERR_BUSY 3
#define MTWI_ERR_BUFFER_OVERFLOW 4
#define MTWI_ERR_NOT_ACKED 5
#define MTWI_ERR_INVALID_COMMAND 6

#include "setup.h"

#ifndef MTWI_TIMEOUT
#	define MTWI_TIMEOUT 500
#endif

#ifndef MTWI_RX_BUFFER_SIZE
#	define MTWI_RX_BUFFER_SIZE 16
#endif


namespace mtwi
{

namespace utils
{
	uint8_t hex2nibble (char hex);
}

volatile static uint8_t error = 0;
const uint8_t *exec (const char *cmd);
const uint8_t *exec_p (const char *progmem_cmd);
bool exists (uint8_t addr);

}

#endif /* MTWI_H_ */
