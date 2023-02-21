#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__
#include "type.h"

namespace dawn
{
  struct abstractTransport
  {
    virtual bool write(const void *write_data, const uint32_t data_len) = 0;
    virtual bool read(void *read_data, uint32_t &data_len) = 0;
    virtual bool wait() = 0;
  };
}


#endif

