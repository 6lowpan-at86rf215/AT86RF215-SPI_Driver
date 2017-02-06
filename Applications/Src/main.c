/*
 * AT86RF215 test code with spi driver
 *
 * Copyright (c) 2017  Cisco, Inc.
 * Copyright (c) 2017  <binyao@cisco.com>
 *
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <poll.h>
#include "spi.h"
#include "pal.h"
#include "gpio.h"
#include "tal.h"
#include "app_common.h"
#include "tal_internal.h"

/* === MACROS ============================================================== */
#define MAX_BUF 64

/* Default modulation */
#define MOD_SUB1        OQPSK
#define MOD_2_4G        LEG_OQPSK


spi_t at86rf215_spi={
	.name="/dev/spidev1.0",
	.mode=0,
	.bits=8,
	.speed=25000000,
	.delay=0,
	.fd=-1

};

gpio_t at86rf215_gpio_irq={
	//.name="/gpio/pin25",
	.pin=25,
	.fd=-1,
	.direction=IN,
	.edge=RISING,
	.value=low
};

gpio_t at86rf215_gpio_rest={
	//.name="/gpio/pin24",
	.pin=24,
	.fd=-1,
	.direction=OUT,
	.edge=NONE,
	.value=low
};

At86rf215_Dev_t at86rf215_dev={
	.spi=&at86rf215_spi,
	.gpio_irq=&at86rf215_gpio_irq,
	.gpio_rest=&at86rf215_gpio_rest
};

/* === GLOBALS ============================================================= */

app_state_t app_state[NUM_TRX];
bool sleep_enabled[NUM_TRX];
#ifdef MULTI_TRX_SUPPORT
modulation_t current_mod[NUM_TRX];
#endif  /* #ifdef MULTI_TRX_SUPPORT */
trx_id_t current_trx_id = (trx_id_t)0;


static void clean(void);
/* === PROTOTYPES ========================================================== */

static void app_task(int);
static void app_init(void);

#ifdef MULTI_TRX_SUPPORT
static void switch_tx_band(trx_id_t id);
static void handle_menu(void);
static char *get_preset_text(uint8_t preset);
#endif



int main(int argc, char *argv[]){
	int nfds = 2;
	struct pollfd fdset[2];
	char buf[MAX_BUF];
	int ret;
	atexit(clean);
	/* Initialize the TAL layer */	
    if (tal_init() != MAC_SUCCESS){
		perror("Initialize the TAL layer failure");
		return -1;
	}
	app_init();
	print_chat_menu();

	while (1) {
		memset((void*)fdset, 0, sizeof(fdset));
		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = at86rf215_gpio_irq.fd;
		fdset[1].events = POLLPRI;

		ret = poll(fdset, nfds, -1);	  
		
		if (ret < 0) {
			perror("\npoll() failed!\n");
			break;
		}
		if (fdset[1].revents & POLLPRI) {
			//lseek(fdset[1].fd, 0, SEEK_SET);
			//len = read(fdset[1].fd, buf, MAX_BUF);
			//printf("\npoll() GPIO %d interrupt occurred\n", gpio);
			trx_irq_handler_cb();
			tal_task();
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			//printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
			app_task(buf[0]);
		}
		fflush(stdout);

	}
}


/**
 * @brief Initializes the application
 */
static void app_init(void)
{
    for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        app_state[i] = APP_IDLE;
        sleep_enabled[i] = false;
    }

    /* Configure the TAL PIBs; e.g. set short address */
    uint16_t pan_id = OWN_PAN_ID;
    tal_pib_set_all(macPANId, (pib_value_t *)&pan_id);
    uint16_t addr = OWN_SHORT_ADDR;
    tal_pib_set_all(macShortAddress, (pib_value_t *)&addr);

#ifdef MULTI_TRX_SUPPORT
    /* Configure PHY for sub-1GHz and 2.4GHz */
    set_mod(RF09, MOD_SUB1);
    set_mod(RF24, MOD_2_4G);
#endif

    /* Init tx frame info structure value that do not change during program execution */
    init_tx_frame();

    /* Switch receiver(s) on */
    for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        tal_rx_enable(i, PHY_RX_ON);
    }
}

/**
 * @brief Application task
 */
static void app_task(int input)
{
    if (input != -1)
    {
        switch (input)
        {
#ifdef MULTI_TRX_SUPPORT
            case SUB1_CHAR:
                switch_tx_band(RF09);
                break;

            case TWO_G_CHAR:
                switch_tx_band(RF24);
                break;

            case MENU_CHAR:
                handle_menu();
                break;
#endif
            default:
                if (input != 0xFF)
                {
                    get_chat_input(input);
                }
                break;
        }
    }
#ifdef MULTI_TRX_SUPPORT
#   ifdef BUTTON_0
    if (pal_button_read(BUTTON_0) == BUTTON_PRESSED)
    {
        switch_tx_band(RF09);
    }
#   endif
#   ifdef BUTTON_1
    if (pal_button_read(BUTTON_1) == BUTTON_PRESSED)
    {
        switch_tx_band(RF24);
    }
#   endif
#endif
}

#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Switch frequency band for transmission
 */
static void handle_menu(void)
{
    retval_t status;
    phy_t phy;
    tal_pib_get(current_trx_id, phySetting, (uint8_t *)&phy);

    /* To avoid any side effects switch receiver off during PHY change */
    for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        if (!sleep_enabled[i])
        {
            tal_rx_enable(i, PHY_TRX_OFF);
        }
    }

	printf("\nActive transmitter: %s, ", get_trx_id_text(current_trx_id));
    printf("\nModulation configuration, current selection: ");
    if (sleep_enabled[current_trx_id])
    {
        printf("off\n\n");
    }
    else
    {
        printf ("%s\n\n", get_mod_text(current_mod[current_trx_id]));
    }
    if (sleep_enabled[current_trx_id])
    {
        printf("->");
    }
    else
    {
        printf("  ");
    }
    printf(" (0)  Off\n");
    /* Print modulation presets */
    for (uint8_t i = 1; i <= PRESET_TEXT_COUNT; i++)
    {
        if ((!sleep_enabled[current_trx_id]) && (current_mod[current_trx_id] == (modulation_t)(i - 1)))
        {
            printf("->");
        }
        else
        {
            printf("  ");
        }
        printf(" (%"PRIu8")  %s\n", i, get_preset_text(i));
    }

    printf("\nEnter new modulation: ");
    fflush(stdout);

    /* Wait for input */
    //char input = sio_getchar();
	char input = '0';
    if (input == '0')
    {
        if (tal_trx_sleep(current_trx_id, SLEEP_MODE_1) == MAC_SUCCESS)
        {
            sleep_enabled[current_trx_id] = true;
        }
    }
    else if ((input >= '1') && (input <= '5'))
    {
        if (sleep_enabled[current_trx_id])
        {
            tal_trx_wakeup(current_trx_id);
            sleep_enabled[current_trx_id] = false;
        }
        status = set_mod(current_trx_id, (modulation_t)(input - '1'));
        printf("\nNew modulation set: %s\n", get_retval_text(status));
		input_len = 0;
		chat_pay_ptr = tx_frm_pay_ptr;
		memset(tx_frm_pay_ptr,0,MAX_INPUT_LENGTH);
    }
	else if (input == SUB1_CHAR)
	{
		switch_tx_band(RF09);
	}
	else if (input == TWO_G_CHAR)
	{
		switch_tx_band(RF24);
	}

    /* Switch all receivers on again */
    for (trx_id_t i = (trx_id_t)0; i < NUM_TRX; i++)
    {
        if (!sleep_enabled[i])
        {
            tal_rx_enable(i, PHY_RX_ON);
        }
    }

    print_chat_menu();
}
#endif


/**
 * @brief Get text for PAL type
 *
 * @param pal_type PAL type
 * @return Text describing the PAL type
 */
char *get_pal_type_text(uint8_t pal_type)
{
    char *text;

    switch (pal_type)
    {
#ifdef ATMEGA128RFA1
        case ATMEGA128RFA1:
            text = "ATMEGA128RFA1";
            break;
#endif
#ifdef ATMEGA256RFR2
        case ATMEGA256RFR2:
            text = "ATMEGA256RFR2";
            break;
#endif
#ifdef ATSAM4S16C
        case ATSAM4S16C:
            text = "ATSAM4S16C";
            break;
#endif
#ifdef ATSAM4SD16C
        case ATSAM4SD16C:
            text = "ATSAM4SD16C";
            break;
#endif
#ifdef ATSAM4S16B
        case ATSAM4S16B:
            text = "ATSAM4S16B";
            break;
#endif
#ifdef ATSAM4SD32C
        case ATSAM4SD32C:
            text = "ATSAM4SD32C";
            break;
#endif
#ifdef ATXMEGA256A3
        case ATXMEGA256A3:
            text = "ATXMEGA256A3";
            break;
#endif
#ifdef IMX6SX
				case IMX6SX:
					text = "IMX6SX";
					break;
#endif
        default:
            text = "unknown PAL type";
            break;
    }

    return text;
}

/**
 * @brief Get text for TAL type
 *
 * @param tal_type TAL type
 * @return Text describing the TAL type
 */
char *get_tal_type_text(uint8_t tal_type)
{
    char *text;

    switch (tal_type)
    {
#ifdef ATMEGARFA1
        case ATMEGARFA1:
            text = "ATMEGARFA1";
            break;
#endif
#ifdef ATMEGARFR2
        case ATMEGARFR2:
            text = "ATMEGARFR2";
            break;
#endif
#ifdef AT86RF231
        case AT86RF231:
            text = "AT86RF231";
            break;
#endif
#ifdef AT86RF233
        case AT86RF233:
            text = "AT86RF233";
            break;
#endif
#ifdef AT86RF215
        case AT86RF215:
#   if defined RF215v1
            text = "AT86RF215v1";
#   elif (defined RF215v2)
            text = "AT86RF215v2";
#   elif (defined RF215v3)
            text = "AT86RF215v3";
#   elif (defined RF215Mv1)
            text = "AT86RF215Mv1";
#   elif (defined RF215Mv2)
            text = "AT86RF215Mv2";
#   else
            text = "unknown";
#   endif
            break;
#endif
        default:
            text = "unknown TAL_TYPE";
            break;
    }

    return text;
}

#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Get text for transceiver identifier
 *
 * @param id Transceiver identifier
 * @return Text describing the transceiver
 */
char *get_trx_id_text(trx_id_t id)
{
    char *text;

    if (id == RF09)
    {
        text = "RF09";
    }
    else if (id == RF24)
    {
        text = "RF24";
    }
    else
    {
        text = "unknown trx id";
    }

    return text;
}
#endif

#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Get text for modulation
 *
 * @param   mod Modulation
 *
 * @return  text
 */
char *get_mod_text(modulation_t mod)
{
    char *text;

    switch (mod)
    {
        case FSK:
            text = "FSK";
            break;

        case OFDM:
            text = "MR-OFDM";
            break;

        case OQPSK:
            text = "MR-OQPSK";
            break;

        case LEG_OQPSK:
            text = "LEG_OQPSK";
            break;

        default:
            text = "unknown modulation";
#ifdef USER_CONFIGURATION
			if (USER_CONFIG == (uint8_t)mod)
			{
				text = "User Config";
			} 
#endif /*USER_CONFIGURATION*/			
            break;
    }

    return text;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */

/**
 * @brief Get text for return value
 *
 * @param ret return value
 * @return Status text
 */
char *get_retval_text(retval_t ret)
{
    char *text;

    switch (ret)
    {
        case MAC_SUCCESS:
            text = "MAC_SUCCESS";
            break;
        case TAL_TRX_ASLEEP:
            text = "TAL_TRX_ASLEEP";
            break;
        case TAL_TRX_AWAKE:
            text = "TAL_TRX_AWAKE";
            break;
        case FAILURE:
            text = "FAILURE";
            break;
        case TAL_BUSY:
            text = "TAL_BUSY";
            break;
        case TAL_FRAME_PENDING:
            text = "TAL_FRAME_PENDING";
            break;
        case MAC_CHANNEL_ACCESS_FAILURE:
            text = "MAC_CHANNEL_ACCESS_FAILURE";
            break;
        case MAC_DISABLE_TRX_FAILURE:
            text = "MAC_DISABLE_TRX_FAILURE";
            break;
        case MAC_FRAME_TOO_LONG:
            text = "MAC_FRAME_TOO_LONG";
            break;
        case MAC_INVALID_PARAMETER:
            text = "MAC_INVALID_PARAMETER";
            break;
        case MAC_NO_ACK:
            text = "MAC_NO_ACK";
            break;
        case MAC_UNSUPPORTED_ATTRIBUTE:
            text = "MAC_UNSUPPORTED_ATTRIBUTE";
            break;
        case MAC_READ_ONLY:
            text = "MAC_READ_ONLY";
            break;
        case PAL_TMR_ALREADY_RUNNING:
            text = "PAL_TMR_ALREADY_RUNNING";
            break;
        case PAL_TMR_NOT_RUNNING:
            text = "PAL_TMR_NOT_RUNNING";
            break;
        case PAL_TMR_INVALID_ID:
            text = "PAL_TMR_INVALID_ID";
            break;
        case PAL_TMR_INVALID_TIMEOUT:
            text = "PAL_TMR_INVALID_TIMEOUT";
            break;

        default:
            text = "unknown retval";
            break;
    }
    return text;
}


#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Switch frequency band for transmission
 */
static void switch_tx_band(trx_id_t id)
{
    //led_id_t tx_led_id, rx_led_id;
    current_trx_id = id;
	input_len = 0;
	chat_pay_ptr = tx_frm_pay_ptr;
	memset(tx_frm_pay_ptr,0,MAX_INPUT_LENGTH);
    printf("\nActive transmitter frequency band: ");

    if (id == RF09)
    {
        printf("sub-1GHz\n> ");
        //tx_led_id = LED_TX_1GHZ;
        //rx_led_id = LED_RX_1GHZ;
    }
    else
    {
        printf("2.4GHz\n> ");
        //tx_led_id = LED_TX_2GHZ;
        //rx_led_id = LED_RX_2GHZ;
    }

    for (uint8_t i = 0; i < 20; i++)
    {
        //pal_led(tx_led_id, LED_TOGGLE);
        //pal_led(rx_led_id, LED_TOGGLE);
        pal_timer_delay(50000);
        pal_timer_delay(50000);
    }
    //pal_led(tx_led_id, LED_OFF);
    //pal_led(rx_led_id, LED_OFF);
}
#endif

#ifdef MULTI_TRX_SUPPORT
/**
 * @brief Get text for PHY preset configuration
 *
 * @param preset Preset number
 * @return Preset description
 */
static char *get_preset_text(uint8_t preset)
{
    char *text;

    switch (preset)
    {
        case 1:
            text = "MR-FSK, 50kbit/s, mod idx 1.0, dw=1, fec=0, CRC32";
            break;

        case 2:
            text = "MR-OFDM, Opt. 1, MCS3, CRC32, interl=0";
            break;

        case 3:
            text = "MR-OQPSK, chip rate 100kchip/s, rate mode 0, CRC32";
            break;

        case 4:
            text = "Legacy OQPSK";
            break;

#ifdef USER_CONFIGURATION
        case 5:
	        text = "User Config";
		    break;
#endif

        default:
            text = "unknown preset";
            break;
    }
    return text;
}
#endif /* #ifdef MULTI_TRX_SUPPORT */


/**
 * @brief User call back function for frame reception
 *
 * @param trx_id   Transceiver identifier
 * @param rx_frame Pointer to received frame structure of type frame_info_t
 *                 or to received frame array
 */
void tal_rx_frame_cb(trx_id_t trx_id, frame_info_t *rx_frame)
{
    chat_handle_incoming_frame(trx_id, rx_frame);
    /* Free buffer of incoming frame */
    bmm_buffer_free(rx_frame->buffer_header);	
}

static void clean(void){
	close(at86rf215_dev.spi->fd);
	close(at86rf215_dev.gpio_irq->fd);
	close(at86rf215_dev.gpio_rest->fd);
}

/**
 * @brief User call back function for frame transmission
 *
 * @param trx_id Transceiver identifier
 * @param status Status of frame transmission attempt
 * @param frame Pointer to frame structure of type frame_info_t
 */
void tal_tx_frame_done_cb(trx_id_t trx_id, retval_t status, frame_info_t *frame)
{
	chat_tx_done_cb(trx_id, status, frame);
}

