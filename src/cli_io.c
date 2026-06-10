/*****************************************************************************
 *
 * FILE NAME     : cli_io.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 ******************************************************************************
 *
 *  DESCRIPTION : Command Line Interface Input / Output
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include "cli_context.h"
#include "cli_io.h"
#include "cli_log.h"
#include "cli_prompt.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


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
static char cli_read_char (void);


/*****************************************************************************
   Local Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_read_char
 *
 *  DESCRIPTION : Read a single character from the input stream
 *
 *  PARAMS      : void
 *
 *  RETURNS     : char
 *
 *****************************************************************************/
static char cli_read_char (void)
{
    char ch;

    while (read(STDIN_FILENO, &ch, 1) != 1);

    return ch;
}


/*****************************************************************************
   Global Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_line_editor_init
 *
 *  DESCRIPTION : Initialize the line editor
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : true if line editor initialized successfully
 *                false otherwise
 *
 *****************************************************************************/
bool cli_line_editor_init (cli_context_t  *ctx_p)
{
    ctx_p->line_editor.cmd_p = string_new(NULL);
    if (!ctx_p->line_editor.cmd_p)
    {
        cli_log(LOG_LEVEL_HIGH, "string_new failed\n");
        return false;
    }

    if (!string_reserve(ctx_p->line_editor.cmd_p, 256))
    {
        cli_log(LOG_LEVEL_HIGH, "string_reserve failed\n");
        return false;
    }

    ctx_p->line_editor.pos = 0;

    return true;
}

/*****************************************************************************
 *
 *  NAME        : cli_line_editor_deinit
 *
 *  DESCRIPTION : Deinitialize the line editor
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_line_editor_deinit (cli_context_t  *ctx_p)
{
    ctx_p->line_editor.cmd_p = string_delete(ctx_p->line_editor.cmd_p);
    ctx_p->line_editor.pos = 0;
}

/*****************************************************************************
 *
 *  NAME        : cli_terminal_init
 *
 *  DESCRIPTION : Setup terminal settings
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_terminal_init (cli_context_t  *ctx_p)
{
    struct termios  raw_termios;

    // Get the original terminal settings
    tcgetattr(STDIN_FILENO, &ctx_p->line_editor.orig_termios);
    raw_termios = ctx_p->line_editor.orig_termios;

    // Set the terminal to raw mode
    raw_termios.c_lflag &= ~(ICANON | ECHO);  // Disable line buffering and echo
    raw_termios.c_cc[VMIN] = 1;               // Read at least 1 byte
    raw_termios.c_cc[VTIME] = 0;              // No timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);
}

/*****************************************************************************
 *
 *  NAME        : cli_terminal_deinit
 *
 *  DESCRIPTION : Restore terminal settings
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_terminal_deinit (cli_context_t  *ctx_p)
{
    // Restore the original terminal settings before exiting
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &ctx_p->line_editor.orig_termios);
}

/*****************************************************************************
 *
 *  NAME        : cli_clear_line_editor
 *
 *  DESCRIPTION : Clear the line editor buffer
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_clear_line_editor (cli_context_t  *ctx_p)
{
    string_clear(ctx_p->line_editor.cmd_p);
    ctx_p->line_editor.pos = 0;
}

/*****************************************************************************
 *
 *  NAME        : cli_in
 *
 *  DESCRIPTION : Read an event
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : cli_key_e
 *
 *****************************************************************************/
cli_key_e cli_in (cli_context_t  *ctx_p)
{
    cli_key_e   last_key = CLI_KEY_ENTER;
    char        ch;

    while (true)
    {
        ch = cli_read_char();

        if (ch == '\n')
        {
            cli_print(ctx_p, "\n");
            break;
        }
        else if (ch == '?')
        {
            last_key = CLI_KEY_Q;
            cli_print(ctx_p, "?\n");
            break;
        }
        else if (ch == '\t')
        {
            last_key = CLI_KEY_TAB;
            break;
        }
        else if (ch == 127 || ch == '\b') // Backspace character
        {
            if (ctx_p->line_editor.pos > 0)
            {
                string_remove(ctx_p->line_editor.cmd_p, ctx_p->line_editor.pos - 1, 1);
                ctx_p->line_editor.pos--;
                cli_flush(ctx_p, true);
            }
        }
        else if (ch == 27) // Escape character
        {
            ch = cli_read_char();
            if (ch == '[')
            {
                ch = cli_read_char();
                if (ch == 'A')
                {
                    last_key = CLI_KEY_UP_ARROW;
                    break;
                }
                else if (ch == 'B')
                {
                    last_key = CLI_KEY_DOWN_ARROW;
                    break;
                }
                else if (ch == 'C')
                {
                    if (ctx_p->line_editor.pos < string_length(ctx_p->line_editor.cmd_p))
                    {
                        ctx_p->line_editor.pos++;
                        cli_print(ctx_p, "\x1b[1C");
                    }
                }
                else if (ch == 'D')
                {
                    if (ctx_p->line_editor.pos > 0)
                    {
                        ctx_p->line_editor.pos--;
                        cli_print(ctx_p, "\x1b[1D");
                    }
                }
                else if (ch == '3')
                {
                    ch = cli_read_char();
                    if (ch == '~')
                    {
                        if (ctx_p->line_editor.pos < string_length(ctx_p->line_editor.cmd_p))
                        {
                            string_remove(ctx_p->line_editor.cmd_p, ctx_p->line_editor.pos, 1);
                            cli_flush(ctx_p, true);
                        }
                    }
                }
            }
        }
        else if (   (ch >= 'a' && ch <= 'z')
                 || (ch >= 'A' && ch <= 'Z')
                 || (ch >= '0' && ch <= '9')
                 || ch == '-'
                 || ch == '_'
                 || ch == '.'
                 || ch == ':'
                 || ch == ' '
                 || ch == '"'
                 || ch == '/'
                )
        {
            string_insertc(ctx_p->line_editor.cmd_p, ctx_p->line_editor.pos, ch);
            ctx_p->line_editor.pos++;

            cli_flush(ctx_p, true);
        }
    }

    return last_key;
}

/*****************************************************************************
 *
 *  NAME        : cli_out
 *
 *  DESCRIPTION : Append formatted output to line buffer
 *
 *  PARAMS      : ctx_p - CLI context
 *                ...   - Format string and arguments
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_out (cli_context_t  *ctx_p, const char *fmt_p, ...)
{
    va_list   args;
    char     *str_p;

    va_start(args, fmt_p);
    if (vasprintf(&str_p, fmt_p, args) == -1)
    {
        cli_log(LOG_LEVEL_HIGH, "vasprintf failed with error [%d, %s]\n", errno, strerror(errno));
    }
    else
    {
        string_append(ctx_p->line_editor.cmd_p, str_p);
        free(str_p);
    }
    va_end(args);
}

/*****************************************************************************
 *
 *  NAME        : cli_flush
 *
 *  DESCRIPTION : Flush the line buffer output to the terminal.
 *                Update the cursor position to the end of the line.
 *
 *  PARAMS      : ctx_p             - CLI context
 *                keep_cursor_pos_b - If true, keep the cursor is moved back
 *                                    to current position
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_flush (cli_context_t  *ctx_p, bool keep_cursor_pos_b)
{
    // Clear the current line
    cli_print(ctx_p, "\x1b[2K");

    // Print the prompt and the current line buffer
    cli_print(ctx_p, "\r%s %s", string_cstr(ctx_p->cur_prompt_p->name_p), string_cstr(ctx_p->line_editor.cmd_p));

    //    Update the cursor position to the end of the line
    // Or move the cursor back to the current position
    if (keep_cursor_pos_b)
        cli_print(ctx_p, "\r\x1b[%ldC", string_length(ctx_p->cur_prompt_p->name_p) + 1 + ctx_p->line_editor.pos);
    else if (ctx_p->line_editor.pos < ctx_p->line_editor.cmd_p->length)
        ctx_p->line_editor.pos = ctx_p->line_editor.cmd_p->length;
}

/*****************************************************************************
 *
 *  NAME        : cli_print
 *
 *  DESCRIPTION : Print formatted output to the terminal.
 *
 *  PARAMS      : ctx_p - CLI context
 *                ...   - Format string and arguments
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_print (cli_context_t  *ctx_p, const char  *fmt_p, ...)
{
    va_list   args;

    UNUSED_PARAMETER(ctx_p);

    va_start(args, fmt_p);
    vprintf(fmt_p, args);
    va_end(args);

    fflush(stdout);
}


/*****************************************************************************
   Test Functions
*****************************************************************************/
