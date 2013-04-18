#ifndef MTWI_H_
#define MTWI_H_

#include <stdint.h>

#define MTWI_ERR_TIMEOUT 1
#define MTWI_ERR_BUS_FAIL 2
#define MTWI_ERR_BUSY 3
#define MTWI_ERR_BUFFER_OVERFLOW 4
#define MTWI_ERR_NOT_ACKED 5

namespace mtwi
{

namespace utils
{
	uint8_t hex2nibble (char hex);
}

volatile static uint8_t error = 0;
const char *exec (const char *cmd);
const char *exec_p (const char *progmem_cmd);

}

#endif /* MTWI_H_ */
