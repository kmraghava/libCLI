/*****************************************************************************
 *
 * FILE NAME     : cli_vvalidator.h
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 11, 2026
 *
 *****************************************************************************
 *
 *  DESCRIPTION : Command Line Interface Value Validator
 *
 *****************************************************************************/

#ifndef __CLI_VVALIDATOR_H
#define __CLI_VVALIDATOR_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "cli_context.h"
#include "kmrUtils/str.h"
#include <stdbool.h>


/*****************************************************************************
 * Global Defines
 *****************************************************************************/

/*****************************************************************************
 * Global Constants
 *****************************************************************************/

/*****************************************************************************
 * Global Types
 *****************************************************************************/

/*****************************************************************************
 * Global Variables
 *****************************************************************************/
 
/*****************************************************************************
 * Inline functions
 *****************************************************************************/
 
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
extern bool cli_value_is_int          (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_hex          (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_real         (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_string       (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_bool         (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_ip4addr      (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_ip4addr_plen (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_ip6addr      (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_ip6addr_plen (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_fqdn         (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);
extern bool cli_value_is_macaddr      (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_VVALIDATOR_H */
