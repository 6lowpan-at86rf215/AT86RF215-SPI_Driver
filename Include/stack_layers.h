/**
 * @file stack_layers.h
 *
 * @brief Stack configuration parameters
 *
 * $Id: stack_layers.h 37447 2015-05-29 12:00:17Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2015, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef STACK_LAYERS_H
#define STACK_LAYERS_H

/* Highest stack layer definitions up to MAC
 *
 * Do NOT change order here! The numbering needs to start with the lowest layer
 * increasing to the higher layers.
 */
#define PAL                                 (1)
#define TAL                                 (2)
#define RTB                                 (3)
#define MAC                                 (4)
/* Highest stack layer definitions above MAC */
#define RF4CE                               (5)

#endif /* STACK_LAYERS_H */
/* EOF */
