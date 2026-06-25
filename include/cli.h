/*****************************************************************************
 *
 * FILE NAME     : cli.h
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : April 2, 2026
 *
 *****************************************************************************
 *
 *  DESCRIPTION : Command Line Interface
 *
 *****************************************************************************/

#ifndef __CLI_H
#define __CLI_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "kmrUtils/logger.h"
#include "kmrUtils/str.h"
#include <stdbool.h>
#include <stdint.h>


/*****************************************************************************
 * Global Defines
 *****************************************************************************/

/*****************************************************************************
 * Global Constants
 *****************************************************************************/

/*****************************************************************************
 * Global Types
 *****************************************************************************/
#define CLI_TOKEN_TYPE_KEYWORD                     0
#define CLI_TOKEN_TYPE_VALUE_INT                   0x0001
#define CLI_TOKEN_TYPE_VALUE_HEX                   0x0002
#define CLI_TOKEN_TYPE_VALUE_REAL                  0x0004
#define CLI_TOKEN_TYPE_VALUE_STRING                0x0008
#define CLI_TOKEN_TYPE_VALUE_BOOL                  0x0010
#define CLI_TOKEN_TYPE_VALUE_IP4ADDR               0x0020
#define CLI_TOKEN_TYPE_VALUE_IP4ADDR_PLEN          0x0040
#define CLI_TOKEN_TYPE_VALUE_IP6ADDR               0x0080
#define CLI_TOKEN_TYPE_VALUE_IP6ADDR_PLEN          0x0100
#define CLI_TOKEN_TYPE_VALUE_IPADDR                (CLI_TOKEN_TYPE_VALUE_IP4ADDR      | CLI_TOKEN_TYPE_VALUE_IP6ADDR     )
#define CLI_TOKEN_TYPE_VALUE_IPADDR_PLEN           (CLI_TOKEN_TYPE_VALUE_IP4ADDR_PLEN | CLI_TOKEN_TYPE_VALUE_IP6ADDR_PLEN)
#define CLI_TOKEN_TYPE_VALUE_FQDN                  0x0200
#define CLI_TOKEN_TYPE_VALUE_INET_ADDR             (CLI_TOKEN_TYPE_VALUE_IPADDR      | CLI_TOKEN_TYPE_VALUE_FQDN)
#define CLI_TOKEN_TYPE_VALUE_INET_ADDR_PLEN        (CLI_TOKEN_TYPE_VALUE_IPADDR_PLEN | CLI_TOKEN_TYPE_VALUE_FQDN)
#define CLI_TOKEN_TYPE_VALUE_MACADDR               0x0400

#define CLI_TOKEN_TYPE_VALUE_OTHER                 0x100000000


typedef struct cli_token_s   cli_token_t;
typedef struct cli_prompt_s  cli_prompt_t;
typedef struct cli_context_s cli_context_t;

typedef bool (*cli_validator_f) (cli_context_t *ctx_p, uint64_t value_type, string_t *value_p);


/*****************************************************************************
 * Global Variables
 *****************************************************************************/
 
/*****************************************************************************
 * Inline functions
 *****************************************************************************/
 
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
extern cli_context_t* cli_context_new (const char  *cfg_filename_p,
                                       bool         enable_telnet_b,
                                       uint16_t     telnet_port,
                                       bool         enable_ssh_b,
                                       uint16_t     ssh_port);

extern cli_prompt_t* cli_context_set_root_prompt (cli_context_t  *ctx_p,
                                                  const char     *name_p);

extern cli_token_t* cli_prompt_add_token (cli_context_t    *ctx_p,
                                          cli_prompt_t     *prompt_p,
                                          const char       *name_p,
                                          const char       *desc_p,
                                          uint64_t          type,
                                          cli_validator_f   cb);
                                        
extern cli_token_t* cli_token_add_token (cli_context_t    *ctx_p,
                                         cli_token_t      *parent_p,
                                         const char       *name_p,
                                         const char       *desc_p,
                                         uint64_t          type,
                                         cli_validator_f   cb);

extern cli_prompt_t* cli_token_set_prompt (cli_context_t  *ctx_p,
                                           cli_token_t    *parent_p,
                                           const char     *name_p);

extern void cli_start (cli_context_t  *ctx_p);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_H */
