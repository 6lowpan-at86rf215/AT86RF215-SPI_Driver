/* Prevent double inclusion */
#ifndef PAL_H
#define PAL_H
#include <stdbool.h>
#include <stdint.h>
#include "pal_types.h"
#include "Pal_config.h"
#include "app_config.h"
#include "return_val.h"
#include "pal_timer.h"

#ifdef PAL_USE_SPI_TRX
#   ifdef PAL_SPI_BLOCK_MODE
#		include "spi.h" 
#		include "gpio.h"
#		include "pal_trx_spi_block_mode.h"
#	endif
#endif

/* === Macros =============================================================== */
/**
 * Subtracts two time values
 */
	/**
	 * Adds two time values
	 */
#define ADD_TIME(a, b)                  ((a) + (b))
	
	/**
	 * Subtracts two time values
	 */
#define SUB_TIME(a, b)                  ((a) - (b))
	
	/* Returns the minimum of two values */
#define MIN(a, b)                       (((a) < (b)) ?  (a) : (b))


/* === Types =============================================================== */

/**
 * Timeout type
 */
typedef enum
timeout_type_tag
{
    /** The timeout is relative to the current time. */
    TIMEOUT_RELATIVE,
    /** The timeout is an absolute value. */
    TIMEOUT_ABSOLUTE
} SHORTENUM timeout_type_t;

typedef struct 
At86rf215_Dev_tag{
	spi_t* spi;
	gpio_t* gpio_irq;
	gpio_t* gpio_rest;
}At86rf215_Dev_t;

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * @brief Initialization of PAL
	 *
	 * This function initializes the PAL. The RC Oscillator is calibrated.
	 *
	 * @return MAC_SUCCESS	if PAL initialization is successful, FAILURE otherwise
	 * @ingroup apiPalApi
	 */
	retval_t pal_init(void);

    /**
     * @brief Adds two time values
     *
     * @param a Time value 1
     * @param b Time value 2
     *
     * @return Addition of a and b
     * @ingroup apiPalApi
     */
    static inline uint32_t pal_add_time_us(uint32_t a, uint32_t b)
    {
        return (ADD_TIME(a, b));
    }


	/**
	 * @brief Subtracts two time values
	 *
	 * @param a Time value 1
	 * @param b Time value 2
	 *
	 * @return Difference between a and b
	 * @ingroup apiPalApi
	 */
	static inline uint32_t pal_sub_time_us(uint32_t a, uint32_t b)
	{
		return (SUB_TIME(a, b));
	}
	
    /**
     * @brief Generates blocking delay
     *
     * This functions generates a blocking delay of specified time.
     *
     * @param delay in microseconds
     * @ingroup apiPalApi
     */
    void pal_timer_delay(uint16_t delay);

	
    /**
     * @brief Start regular timer
     *
     * This function starts a regular timer and installs the corresponding
     * callback function handle the timeout event.
     *
     * Make sure to call pal_task() periodically to handle the timer IRQ callback
     * function.
     *
     * @param timer_id Timer identifier
     * @param timer_instance_id Timer instance specifying the actual Trx identifier
     * @param timer_count Timeout in microseconds
     * @param timeout_type @ref TIMEOUT_RELATIVE or @ref TIMEOUT_ABSOLUTE
     * @param timer_cb Callback handler invoked upon timer expiry
     * @param param_cb Argument for the callback handler
     *
     * @return
     *          - @ref PAL_TMR_INVALID_ID  if the timer identifier is undefined,
     *          - @ref MAC_INVALID_PARAMETER if the callback function for this timer
     *                 is NULL,
     *          - @ref PAL_TMR_ALREADY_RUNNING if the timer is already running.
     *          - @ref MAC_SUCCESS if timer is started or
     *          - @ref PAL_TMR_INVALID_TIMEOUT if timeout is not within timeout range.
     * @ingroup apiPalApi
     */
    retval_t pal_timer_start(uint16_t timer_id,
                             timer_instance_id_t timer_instance_id,
                             uint32_t timer_count,
                             timeout_type_t timeout_type,
                             FUNC_PTR(timer_cb),
                             void *param_cb);

	/**
	 * @brief Stops an application timer
	 *
	 * This function stops a running timer with the specified timer_id and
	 * timer_instance_id
	 *
	 * @param timer_id Timer identifier
	 * @param timer_instance_id Timer instance specifying the actual Trx identifier
	 *
	 * @return
	 *			- @ref MAC_SUCCESS if timer stopped successfully,
	 *			- @ref PAL_TMR_NOT_RUNNING if specified timer is not running,
	 *			- @ref PAL_TMR_INVALID_ID if the specified timer id is undefined.
	 * @ingroup apiPalApi
	 */
	retval_t pal_timer_stop(uint16_t timer_id,
							timer_instance_id_t timer_instance_id);
	
	/**
     * @brief Gets current time
     *
     * This function returns the current time.
     *
     * @param[out] current_time Returns current system time
     * @ingroup apiPalApi
     */
    void pal_get_current_time(uint32_t *current_time);

	
	void TRX_RST_HIGH();
	void TRX_RST_LOW();
	gpio_value_t TRX_IRQ_GET();
#ifdef __cplusplus
} /* extern "C" */
#endif



/** Defines if multi device support is not used */
#define pal_dev_write(dev_id, addr, data, length)   pal_trx_write(addr, data, length)
#define pal_dev_read(dev_id, addr, data, length)    pal_trx_read(addr, data, length)
#define pal_dev_reg_write(dev_id, addr, data)       pal_trx_reg_write(addr, data)
#define pal_dev_reg_read(dev_id, addr)              pal_trx_reg_read(addr)
#define pal_dev_bit_write(dev_id, addr, val)        pal_trx_bit_write(addr, val)
#define pal_dev_bit_read(dev_id, addr)              pal_trx_bit_read(addr)
//#define pal_dev_irq_flag_clr(dev_id)                CLEAR_TRX_IRQ()
#define pal_dev_irq_flag_clr(dev_id)
//#define pal_dev_irq_init(dev_id, func_ptr)          pal_trx_irq_init(func_ptr)
#define pal_dev_irq_init(dev_id, func_ptr)
//#define pal_dev_irq_en(dev_id)                      ENABLE_TRX_IRQ()
#define pal_dev_irq_en(dev_id)

#define PAL_DEV_RST_HIGH(dev_id)                    TRX_RST_HIGH()
#define PAL_DEV_RST_LOW(dev_id)                    	TRX_RST_LOW()
#define PAL_DEV_IRQ_GET(dev_id)						TRX_IRQ_GET()


#define ASSERT(expr)


#endif

