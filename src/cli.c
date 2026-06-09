/*****************************************************************************
 *
 * FILE NAME     : cli.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : April 2, 2026
 *
 ******************************************************************************
 *
 *  DESCRIPTION : Command Line Interface
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include "cli_context.h"
#include "cli_io.h"
#include "cli_log.h"
#include "cli_parser.h"


/*****************************************************************************
   External Declarations
*****************************************************************************/

/*****************************************************************************
   Local Constants
*****************************************************************************/

/*****************************************************************************
   Local Types
*****************************************************************************/

/*****************************************************************************
   Local Variables
*****************************************************************************/

/*****************************************************************************
   Global Variables
*****************************************************************************/

/*****************************************************************************
   Local Macros
******************************************************************************/

/*****************************************************************************
   Local Function Prototypes
*****************************************************************************/
static void cli_start_telnet_server (cli_context_t  *ctx_p);
static void cli_stop_telnet_server  (cli_context_t  *ctx_p);

static void cli_start_ssh_server (cli_context_t  *ctx_p);
static void cli_stop_ssh_server  (cli_context_t  *ctx_p);

static void cli_start (cli_context_t  *ctx_p);


/*****************************************************************************
   Local Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_start_telnet_server
 *
 *  DESCRIPTION : Start telnet server and wait for an incoming connection
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_start_telnet_server (cli_context_t  *ctx_p)
{
    UNUSED_PARAMETER(ctx_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_stop_telnet_server
 *
 *  DESCRIPTION : Stop telnet server
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_stop_telnet_server (cli_context_t  *ctx_p)
{
    UNUSED_PARAMETER(ctx_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_start_ssh_server
 *
 *  DESCRIPTION : Start ssh server and wait for an incoming connection
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_start_ssh_server (cli_context_t  *ctx_p)
{
    UNUSED_PARAMETER(ctx_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_stop_ssh_server
 *
 *  DESCRIPTION : Stop ssh server
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_stop_ssh_server (cli_context_t  *ctx_p)
{
    UNUSED_PARAMETER(ctx_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_start
 *
 *  DESCRIPTION : Start
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : Never returns
 *
 *****************************************************************************/
static void cli_start (cli_context_t  *ctx_p)
{
    cli_print_clear_screen(ctx_p);
    cli_flush(ctx_p, false);

    while (true)
    {
        switch (cli_in(ctx_p))
        {
            case CLI_KEY_ENTER:
            {
                if (string_length(ctx_p->line_editor.cmd_p) > 0)
                {
                    cli_log(LOG_LEVEL_LOW, "Received command: %s\n", string_cstr(ctx_p->line_editor.cmd_p));

                    cli_cmd_parse(ctx_p);
                    cli_clear_line_editor(ctx_p);
                }
                cli_flush(ctx_p, false);
            }
            break;

            case CLI_KEY_Q:
                cli_print_newline(ctx_p);
                cli_cmd_help_q(ctx_p);
                cli_print_newline(ctx_p);
                cli_flush(ctx_p, false);
                break;

            case CLI_KEY_TAB:
                cli_cmd_help_tab(ctx_p);
                cli_flush(ctx_p, false);
                break;

            case CLI_KEY_UP_ARROW:
            case CLI_KEY_DOWN_ARROW:
                break;

            default:
                break;
        }
    }
}


/*****************************************************************************
   Global Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_run
 *
 *  DESCRIPTION : Run CLI
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_run (cli_context_t  *ctx_p)
{
    if (!ctx_p)
        return;

    if (!cli_logger())
        return;

    if (ctx_p->enable_telnet_b)
        cli_start_telnet_server(ctx_p);

    if (ctx_p->enable_ssh_b)
        cli_start_ssh_server(ctx_p);

    if (   !ctx_p->enable_telnet_b
        && !ctx_p->enable_ssh_b
       )
    {
        if (!cli_line_editor_init(ctx_p))
            return;
        cli_terminal_init(ctx_p);

        cli_start(ctx_p);

        cli_terminal_deinit(ctx_p);
        cli_line_editor_deinit(ctx_p);
    }
    else
    {
    }
}


/*****************************************************************************
   Test Functions
*****************************************************************************/

