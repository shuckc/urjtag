#include <jim.h>
#include <stdint.h>
#include <stdlib.h>

void intel_28f800b3_init(jim_bus_device_t *d)
{
}

void intel_28f800b3_free(jim_bus_device_t *d)
{
}

void intel_28f800b3_access(jim_bus_device_t *d,
    uint32_t address, uint32_t data, uint32_t control)
{
}

jim_bus_device_t intel_28f800b3 =
{
    16, /* width [bits] */
    0x800, /* size [bytes] */
    NULL,  /* state */
    intel_28f800b3_init,  /* init() */
    intel_28f800b3_access,  /* access() */
    intel_28f800b3_free  /* free() */
};

