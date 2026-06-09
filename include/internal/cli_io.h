/*****************************************************************************
 *
 * FILE NAME     : cli_io.h
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 *****************************************************************************
 *
 *  DESCRIPTION : Command Line Interface Input / Output
 *
 *****************************************************************************/

#ifndef __CLI_IO_H
#define __CLI_IO_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
/*****************************************************************************
 * Include Files
 *****************************************************************************/
#include "cli.h"
#include "kmrUtils/str.h"
#include <stdbool.h>
#include <termios.h>


/*****************************************************************************
 * Global Defines
 *****************************************************************************/
#define cli_print_clear_screen(ctx_p)  cli_print(ctx_p, "\r\033[H\033[J")
#define cli_print_newline(ctx_p)       cli_print(ctx_p, "\n")


/*****************************************************************************
 * Global Constants
 *****************************************************************************/

/*****************************************************************************
 * Global Types
 *****************************************************************************/
typedef enum
{
    CLI_KEY_ENTER,
    CLI_KEY_Q,
    CLI_KEY_TAB,
    CLI_KEY_UP_ARROW,
    CLI_KEY_DOWN_ARROW,

    CLI_NUM_KEYS

} cli_key_e;

typedef struct line_editor_s
{
    string_t        *cmd_p;
    long             pos;

    struct termios   orig_termios;

} line_editor_t;


/*****************************************************************************
 * Global Variables
 *****************************************************************************/
 
/*****************************************************************************
 * Inline functions
 *****************************************************************************/
 
/*****************************************************************************
 * Global Function Prototypes
 *****************************************************************************/
extern bool cli_line_editor_init   (cli_context_t  *ctx_p);
extern void cli_line_editor_deinit (cli_context_t  *ctx_p);

extern void cli_terminal_init   (cli_context_t  *ctx_p);
extern void cli_terminal_deinit (cli_context_t  *ctx_p);

extern void cli_clear_line_editor (cli_context_t  *ctx_p);

extern cli_key_e cli_in    (cli_context_t  *ctx_p);

extern void cli_out   (cli_context_t  *ctx_p, const char *fmt_p, ...);
extern void cli_flush (cli_context_t  *ctx_p, bool keep_cursor_pos_b);

extern void cli_print (cli_context_t  *ctx_p, const char *fmt_p, ...);


/*****************************************************************************
   Test Functions
*****************************************************************************/


#if defined(__cplusplus)
}
#endif

#endif /* __CLI_IO_H */
