#ifndef _TAL_H
#define _TAL_H
#include "at86rf215.h"
#include "return_val.h"
#include "at86rf215_conf.h"
#include "gpio.h"

typedef enum trx_id_tag
{
    RF09, /**< Id for sub-1 GHz device */
    RF24, /**< Id for 2.4 GHz device */
    RFBOTH  /**< Id for both device parts */
}trx_id_t;

retval_t tal_init(struct At86rf215_Dev_t*at86rf215_dev);


#endif
