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
enum
{
    CLI_TOKEN_TYPE_KEYWORD,
    CLI_TOKEN_TYPE_VALUE,
    CLI_TOKEN_TYPE_PROMPT,

    CLI_NUM_TOKEN_TYPES
};

typedef struct cli_token_s   cli_token_t;
typedef struct cli_prompt_s  cli_prompt_t;
typedef struct cli_context_s cli_context_t;

typedef bool (*cli_cmd_f) (cli_context_t *ctx_p, string_t **cmd_tokens_pp);


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

extern cli_token_t* cli_prompt_add_token (cli_context_t  *ctx_p,
                                          cli_prompt_t   *prompt_p,
                                          const char     *name_p,
                                          const char     *desc_p,
                                          int             type,
                                          cli_cmd_f       cmd_valid);
                                        
extern cli_token_t* cli_token_add_token (cli_context_t  *ctx_p,
                                         cli_token_t    *parent_p,
                                         const char     *name_p,
                                         const char     *desc_p,
                                         int             type,
                                         cli_cmd_f       cmd_valid);

extern cli_prompt_t* cli_token_set_prompt (cli_context_t  *ctx_p,
                                           cli_token_t    *parent_p,
                                           const char     *name_p);

extern void cli_run (cli_context_t  *ctx_p);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_H */
