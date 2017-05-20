/*
 * Copyright (C) 2016 Kees Bakker
 *               2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_sam0_common
 * @{
 *
 * @file
 * @brief           Wrapper include file for including the specific SAM0 vendor
 *                  header
 *
 * @author          Kees Bakker <kees@sodaq.com>
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef SAM0_H
#define SAM0_H

#ifdef __cplusplus
extern "C" {
#endif

#if   defined(CPU_FAM_SUB_SAMD21)
  #include "samd21/include/samd21.h"
#elif defined(CPU_FAM_SUB_SAML21)
  #include "saml21/include/saml21.h"
#elif defined(CPU_FAM_SUB_SAMR21)
  #include "samr21/include/samr21.h"
#else
  #error "Unsupported SAM0 variant."
#endif

#ifdef __cplusplus
}
#endif

#endif /* SAM0_H */
/** @} */
