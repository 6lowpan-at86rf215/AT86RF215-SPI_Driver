/**
 * @file app_config.h
 *
 * @brief These are application-specific resources which are used
 *        in the application in addition to the underlaying stack.
 *
 * $Id: app_config.h 37444 2015-05-28 13:00:49Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* === Includes ============================================================= */

#include "stack_config.h"

/* === Macros =============================================================== */
/**
 * Define maximum character input length
 */
#define MAX_INPUT_LENGTH    80

/**
 * Enable user to add their custom configuration in 
 * phy_conf.c -> set_user_config()without modifying the default application.
 */
#ifndef USER_CONFIGURATION
#define USER_CONFIGURATION
#endif

/* === Types ================================================================ */

/* Timer ID's used by the Application */
typedef enum
{
    /* App Timers start from APP_FIRST_TIMER_ID */

    /* App timer 1 */
    T_APP        = (APP_FIRST_TIMER_ID),
    T_APP_LED_TX = (APP_FIRST_TIMER_ID + 1),
    T_APP_LED_RX = (APP_FIRST_TIMER_ID + 2)
} app_timer_t;

/* Additional buffers used by the application */
#define NUMBER_OF_LARGE_APP_BUFS    (10)

#define TOTAL_NUMBER_OF_LARGE_BUFS  (NUMBER_OF_LARGE_APP_BUFS + NUMBER_OF_LARGE_STACK_BUFS)

#define TOTAL_NUMBER_OF_BUFS        (TOTAL_NUMBER_OF_LARGE_BUFS)

/*
 * USB transmit buffer size
 */
#define USB_TX_BUF_SIZE             (64)

/*
 * USB receive buffer size
 */
#define USB_RX_BUF_SIZE             (64)

/*
 * USB-specific definitions
 */

/*
 * USB Vendor ID (16-bit number)
 */
#define USB_VID                 0x03EB /* Atmel's USB vendor ID */

/*
 * USB Product ID (16-bit number)
 */
#ifdef RF233_SAM4_BAB_NATIVE_USB
#if (BOARD_TYPE == RF233_SAM4_BAB_NATIVE_USB) && (!defined(USB_PID))
#error "Define an USB PIB in the Makefile"
#endif
#endif

/** CDC IN endpoint address.
 *  Endpoint address (host view) for CDC bulk out transfers from host to device.
 */
#if !defined(USB_EP_CDC_OUT)
#define USB_EP_CDC_OUT          0x01
#endif

/** CDC OUT endpoint address.
 *  Endpoint address (host view) for CDC bulk out transfers from device to host.
 */
#if !defined(USB_EP_CDC_IN)
#define USB_EP_CDC_IN           0x02
#endif

/** CDC IRQ endpoint address.
 *  Endpoint address for CDC interrupt transfers from device to host.
 */
#if !defined(USB_EP_CDC_IRQ)
#define USB_EP_CDC_IRQ          0x03
#endif

/*
 * USB Release number (BCD format, two bytes)
 */
#define USB_RELEASE             { 0x00, 0x01 } /* 01.00 */

/*
 * Maximal number of UTF-16 characters used in any of the strings
 * below.  This is only used for compilers that cannot handle the
 * initialization of flexible array members within structs.
 */
#define USB_STRING_SIZE         10

/*
 * String representation for the USB vendor name.
 */
#define USB_VENDOR_NAME L"ATMEL"

/*
 * String representation for the USB product name.
 */
#define USB_PRODUCT_NAME L"RZUSBSTICK"

/*
 * USB Flushed data size
 */
#define NB_MS_BEFORE_FLUSH           (50)

/*
 * UART transmit buffer size
 */
#define UART_MAX_TX_BUF_LENGTH      (10)

/*
 * UART receive buffer size
 */
#define UART_MAX_RX_BUF_LENGTH      (10)

/* Offset of IEEE address storage location within EEPROM */
#define EE_IEEE_ADDR                (0)

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_CONFIG_H */
/* EOF */
