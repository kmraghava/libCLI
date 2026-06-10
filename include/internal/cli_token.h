/*****************************************************************************
 *
 * FILE NAME     : cli_token.h
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 *****************************************************************************
 *
 *  DESCRIPTION : Command Line Interface token
 *
 *****************************************************************************/

#ifndef __CLI_TOKEN_H
#define __CLI_TOKEN_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "cli.h"
#include "kmrUtils/str.h"
#include "kmrUtils/tree.h"


/*****************************************************************************
 * Global Defines
 *****************************************************************************/

/*****************************************************************************
 * Global Constants
 *****************************************************************************/

/*****************************************************************************
 * Global Types
 *****************************************************************************/
struct cli_token_s
{
    cli_context_t    *ctx_p;

    tree_node_t       tnode;

    string_t         *name_p;     // Keyword or value template
    string_t         *desc_p;     // Description

    uint64_t          type;
    cli_validator_f   validator_cb;

    cli_prompt_t     *cprompt_p;
    cli_prompt_t     *pprompt_p;
};


/*****************************************************************************
 * Global Variables
 *****************************************************************************/
 
/*****************************************************************************
 * Inline functions
 *****************************************************************************/
 
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
extern cli_token_t* cli_token_add_token (cli_context_t    *ctx_p,
                                         cli_token_t      *parent_p,
                                         const char       *name_p,
                                         const char       *desc_p,
                                         uint64_t          type,
                                         cli_validator_f   cb);

extern cli_prompt_t* cli_token_set_prompt (cli_context_t  *ctx_p,
                                           cli_token_t    *parent_p,
                                           const char     *name_p);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_TOKEN_H */
