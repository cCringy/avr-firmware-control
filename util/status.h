#ifndef STATUS_H_
#define STATUS_H_

#include <stdint.h>

typedef enum:uint8_t{
  STATUS_OK = 0,
  STATUS_ERR_TIMEOUT,
  STATUS_ERR_STATE,
  STATUS_ERR_PARAM,
  STATUS_ERR_CHECKSUM,
  STATUS_ERR_BUSY,
  STATUS_ERR_UNSUPPORTED
}status_t;

#endif /* STATUS_H_ */
