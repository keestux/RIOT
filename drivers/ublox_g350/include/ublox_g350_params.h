/*
 * Copyright (C) 2022 IJzerbout IT
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ublox_g350
 *
 * @{
 * @file
 * @brief       Default configuration
 *
 * @author      Kees Bakker <kees@ijzerbout.nl>
 */

#ifndef UBLOX_G350_PARAMS_H
#define UBLOX_G350_PARAMS_H

#include "board.h"
#include "ublox_g350.h"
#include "ublox_g350_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters
 * @{
 */
#ifndef UBLOX_G350_PARAM_PARAM1
#define UBLOX_G350_PARAM_PARAM1
#endif

#ifndef UBLOX_G350_PARAMS
#define UBLOX_G350_PARAMS
#endif
/**@}*/

/**
 * @brief   Configuration struct
 */
static const ublox_g350_params_t ublox_g350_params[] =
{
    UBLOX_G350_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* UBLOX_G350_PARAMS_H */
/** @} */
