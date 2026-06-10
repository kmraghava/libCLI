/*****************************************************************************
 *
 * FILE NAME     : cli_context.h
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 *****************************************************************************
 *
 *  DESCRIPTION : Command Line Interface context
 *
 *****************************************************************************/

#ifndef __CLI_CONTEXT_H
#define __CLI_CONTEXT_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "cli.h"
#include "cli_io.h"
#include "jansson.h"
#include "kmrUtils/str.h"
#include <stdbool.h>
#include <stdint.h>


/*****************************************************************************
 * Global Defines
 *****************************************************************************/
#define UNUSED_PARAMETER(param_p)  (void)param_p


/*****************************************************************************
 * Global Constants
 *****************************************************************************/

/*****************************************************************************
 * Global Types
 *****************************************************************************/
struct cli_context_s
{
    cli_prompt_t   *root_prompt_p,
                   *cur_prompt_p;

    line_buffer_t   line_buffer;

    string_t       *cfg_filename_p;
    json_t         *cc_jobj_p;

    bool            enable_telnet_b;
    uint16_t        telnet_port;

    bool            enable_ssh_b;
    uint16_t        ssh_port;
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
extern cli_context_t* cli_context_new (const char  *cfg_filename_p,
                                       bool         enable_telnet_b,
                                       uint16_t     telnet_port,
                                       bool         enable_ssh_b,
                                       uint16_t     ssh_port);

extern cli_prompt_t* cli_context_set_root_prompt (cli_context_t  *ctx_p,
                                                  const char     *name_p);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_CONTEXT_H */
