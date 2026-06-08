/*****************************************************************************
 *
 * FILE NAME     : cli.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : April 2, 2025
 *
 ******************************************************************************
 *
 *  DESCRIPTION : Command Line Interface
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include "cli.h"
#include <errno.h>
#include "jansson.h"
#include "kmrUtils/clist.h"
#include "kmrUtils/tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
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
struct cli_token_s
{
    cli_context_t  *ctx_p;

    tree_node_t     tnode;

    int             type;
    char           *name_p;     // Keyword or value template
    char           *desc_p;     // Description

    cli_cmd_f       cli_cmd_cb;

    cli_prompt_t   *cprompt_p;
    cli_prompt_t   *pprompt_p;
};

struct cli_prompt_s
{
    cli_context_t  *ctx_p;

    char           *name_p;

    clist_t        *history_p;
    char           *cur_cmd_p;

    cli_token_t    *parent_p;
    tree_t          cmd_tree;
};

struct cli_context_s
{
    cli_prompt_t   *root_prompt_p,
                   *cur_prompt_p;

    char           *cfg_filename_p;
    json_t         *cc_jobj_p;

    bool            enable_telnet_b;
    uint16_t        telnet_port;

    bool            enable_ssh_b;
    uint16_t        ssh_port;
};

enum
{
    CLI_KEY_ENTER,
    CLI_KEY_Q,
    CLI_KEY_TAB,
    CLI_KEY_UP_ARROW,
    CLI_KEY_DOWN_ARROW,

    CLI_NUM_KEYS
};


/*****************************************************************************
   Local Variables
*****************************************************************************/

/*****************************************************************************
   Global Variables
*****************************************************************************/

/*****************************************************************************
   Local Macros
******************************************************************************/
#define is_file_empty(fp)       \
    ({  long  size;             \
        fseek(fp, 0, SEEK_END); \
        size = ftell(fp);       \
        rewind(fp);             \
        size == 0;              \
    })

#define cli_out_clear_screen(ctx_p)  cli_out(ctx_p, "\r\033[H\033[J")
#define cli_out_newline(ctx_p)       cli_out(ctx_p, "\n")

#define cli_log(level, fmt_p, ...)                                                                                      \
    do                                                                                                                  \
    {                                                                                                                   \
        logger_log_line(cli_logger(), __FILE__, __LINE__, __func__, NULL, level, "CLI", NULL, fmt_p, ##__VA_ARGS__);    \
        logger_flush(cli_logger());                                                                                     \
    }                                                                                                                   \
    while (0)

#define cli_tree_find_token(t, depth, key_p, member_match_fn)                   \
    ({                                                                          \
        tree_node_t *__tnp = tree_find_node(t, depth, key_p, member_match_fn);  \
        __tnp ? tree_get(__tnp, cli_token_t, tnode) : NULL;                     \
    })


/*****************************************************************************
   Local Function Prototypes
*****************************************************************************/
static logger_t* cli_logger (void);
static void log_stub (const char *line_p);

static cli_prompt_t* cli_prompt_new (cli_context_t  *ctx_p,
                                     cli_token_t    *parent_p,
                                     const char     *name_p);
static cli_prompt_t* cli_prompt_del (cli_prompt_t  *prompt_p);

static void cli_token_del (tree_node_t  *tnode_p);

static bool cli_match_value_token (tree_node_t *tnode_p, void *key_p);
static bool cli_match_token       (tree_node_t *tnode_p, void *key_p);

static void cli_cmd_add_to_history (cli_prompt_t  *prompt_p,
                                    const char    *cmd_p);

static void cli_cmd_parse (cli_context_t  *ctx_p,
                           const char     *cmd_p);

static void cli_cmd_help_q (cli_context_t  *ctx_p,
                            const char     *in_cmd_p);

static void cli_out_help_q (tree_t  *cmd_tree_p);

static void cli_cmd_help_tab (cli_context_t  *ctx_p,
                              char          **in_cmd_pp,
                              size_t         *in_cmd_len_p);

static void cli_out_prompt (cli_prompt_t  *prompt_p,
                            int            cmd_argc,
                            char          *cmd_args[]);

static void cli_start_telnet_server (cli_context_t  *ctx_p);
static void cli_stop_telnet_server  (cli_context_t  *ctx_p);

static void cli_start_ssh_server (cli_context_t  *ctx_p);
static void cli_stop_ssh_server  (cli_context_t  *ctx_p);

static void cli_start (cli_context_t  *ctx_p);

static int cli_get_input (cli_context_t  *ctx_p,
                          char          **cmd_pp,
                          size_t         *cmd_len_p);

static int  cli_out (cli_context_t  *ctx_p, const char *fmt_p, ...);
static char cli_in  (cli_context_t  *ctx_p);


/*****************************************************************************
   Local Functions
*****************************************************************************/
static logger_t* cli_logger (void)
{
    static logger_t  *L_logger_p = NULL;

    if (!L_logger_p)
    {
        L_logger_p = logger_create();
        if (L_logger_p)
        {
            logger_set_filename(L_logger_p, "/tmp/cli.log");
            logger_set_log_fn(L_logger_p, log_stub);
        }
    }

    return L_logger_p;
}


/*****************************************************************************
 *
 *  NAME        : log_stub
 *
 *  DESCRIPTION : Stub function for logging.
 *
 *  PARAMS      : line_p - Line to be logged
 *
 *  RETURNS     : Nothing.
 *
 * NOTES        : We cannot log to stdout because CLI is running in the
 *                foreground and reading input from stdin.
 *                But, we are logging to a file.
 *
 *****************************************************************************/
static void log_stub (const char *line_p) { (void)line_p; }

/*****************************************************************************
 *
 *  NAME        : cli_prompt_new
 *
 *  DESCRIPTION : Create a new CLI prompt.
 *
 *  PARAMS      : ctx_p    - CLI context
 *                parent_p - Parent token
 *                name_p   - Prompt name
 *
 *  RETURNS     : New CLI prompt or NULL on failure
 *
 *****************************************************************************/
static cli_prompt_t* cli_prompt_new (cli_context_t  *ctx_p,
                                     cli_token_t    *parent_p,
                                     const char     *name_p)
{
    cli_prompt_t  *prompt_p;

    prompt_p = calloc(1, sizeof(cli_prompt_t));
    if (!prompt_p)
        return NULL;

    prompt_p->ctx_p = ctx_p;
    prompt_p->parent_p = parent_p;

    tree_init(prompt_p->cmd_tree, NULL);

    prompt_p->name_p = strdup(name_p);
    if (!prompt_p->name_p)
        goto FATAL;

    prompt_p->history_p = clist_new();
    if (!prompt_p->history_p)
        goto FATAL;

    return prompt_p;

FATAL:
    return cli_prompt_del(prompt_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_prompt_del
 *
 *  DESCRIPTION : Delete a CLI prompt.
 *
 *  PARAMS      : prompt_p - Prompt to delete
 *
 *  RETURNS     : NULL
 *
 *****************************************************************************/
static cli_prompt_t* cli_prompt_del (cli_prompt_t  *prompt_p)
{
    if (prompt_p)
    {
        if (prompt_p->parent_p)
            prompt_p->parent_p->cprompt_p = NULL;

        tree_del(&prompt_p->cmd_tree, cli_token_del);

        if (prompt_p->name_p)
            free(prompt_p->name_p);

        clist_del(prompt_p->history_p);

        free(prompt_p);
    }

    return NULL;
}

/*****************************************************************************
 *
 *  NAME        : cli_token_del
 *
 *  DESCRIPTION : Delete a CLI token.
 *
 *  PARAMS      : tnode_p - Tree node of the Token to delete
 *
 *  RETURNS     : NULL
 *
 *****************************************************************************/
static void cli_token_del (tree_node_t  *tnode_p)
{
    cli_token_t  *token_p = tree_get(tnode_p, cli_token_t, tnode);

    if (token_p)
    {
        if (token_p->cprompt_p)
            cli_prompt_del(token_p->cprompt_p);

        if (token_p->name_p)
            free(token_p->name_p);

        if (token_p->desc_p)
            free(token_p->desc_p);

        free(token_p);
    }
}

/*****************************************************************************
 *
 *  NAME        : cli_match_token
 *
 *  DESCRIPTION : Match CLI token with the given name.
 *
 *  PARAMS      : tnode_p - Tree node containing the CLI token to match
 *                key_p   - CLI token name to match
 *
 *  RETURNS     : true if equal
 *                false otherwise
 *
 *****************************************************************************/
static bool cli_match_token (tree_node_t *tnode_p, void *key_p)
{
    cli_token_t  *token_p = tree_get(tnode_p, cli_token_t, tnode);
    char         *name_p = key_p;

    return (0 == strncmp(token_p->name_p, name_p, strlen(name_p)));
}

/*****************************************************************************
 *
 *  NAME        : cli_match_value_token
 *
 *  DESCRIPTION : Match CLI token with the given name.
 *
 *  PARAMS      : member_p - CLI token
 *                key_p    - NULL
 *
 *  RETURNS     : true if member_p is a value token
 *                false otherwise
 *
 *****************************************************************************/
static bool cli_match_value_token (tree_node_t *tnode_p, void *key_p)
{
    cli_token_t  *token_p = tree_get(tnode_p, cli_token_t, tnode);

    (void)key_p;

    return (CLI_TOKEN_TYPE_VALUE == token_p->type);
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_add_to_history
 *
 *  DESCRIPTION : Add command to history of the prompt.
 *
 *  PARAMS      : prompt_p - CLI prompt
 *                cmd_p    - Command string to add to history
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_cmd_add_to_history (cli_prompt_t  *prompt_p,
                                    const char    *cmd_p)
{
    if (   prompt_p
        && cmd_p
       )
    {
        char *hist_cmd_p = strdup(cmd_p);
        if (!hist_cmd_p)
        {
            cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
            return;
        }

        if (!clist_push_back(prompt_p->history_p, hist_cmd_p))
        {
            cli_log(LOG_LEVEL_HIGH, "Failed to add command to history\n");
            free(hist_cmd_p);
            return;
        }
    }
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_parse
 *
 *  DESCRIPTION : Parse the given command and call registered callback.
 *
 *  PARAMS      : ctx_p    - CLI context
 *                in_cmd_p - Command string
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_cmd_parse (cli_context_t  *ctx_p,
                           const char     *in_cmd_p)
{
    char         *cmd_p = NULL;
    char         *cmdtok_p,
                 *rem_cmd_p = NULL;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;
    int           cmd_argc = 0;
    char         *cmd_args[CLI_CMD_MAX_NUM_TOKENS];

    if (!in_cmd_p)
        goto RETURN;

    cmd_p = strdup(in_cmd_p);
    if (!cmd_p)
    {
        cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
        return;
    }

    cmdtok_p = strtok_r(cmd_p, " ", &rem_cmd_p);
    while (cmdtok_p)
    {
        cmd_args[cmd_argc] = cmdtok_p;
        cmd_argc++;

        token_p = cli_tree_find_token(cmd_tree_p, 1, cmdtok_p, cli_match_token);
        if (!token_p)
            token_p = cli_tree_find_token(cmd_tree_p, 1, NULL, cli_match_value_token);

        if (!token_p)
        {
            cli_out(ctx_p, "\nInvalid command\n\n");
            goto RETURN;
        }

        cmd_tree_p = &token_p->tnode.sub_tree;

        cmdtok_p = strtok_r(NULL, " ", &rem_cmd_p);
    }

    if (!token_p)
        goto RETURN;

    if (token_p->cprompt_p)
    {
        ctx_p->cur_prompt_p = token_p->cprompt_p;

        cli_out_prompt(ctx_p->cur_prompt_p, 0, NULL);
        goto RETURN;
    }

    if (!tree_node_is_leaf(token_p->tnode))
    {
        cli_out(ctx_p, "\nIncomplete command\n\n");
        goto RETURN;
    }

    if (token_p->cli_cmd_cb)
        token_p->cli_cmd_cb(ctx_p, cmd_argc, cmd_args);

    cli_cmd_add_to_history(ctx_p->cur_prompt_p, in_cmd_p);

RETURN:
    cli_out_prompt(ctx_p->cur_prompt_p, 0, NULL);

    if (cmd_p)
        free(cmd_p);

    return;
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_help_q
 *
 *  DESCRIPTION : Responds to the ask for available options for next token
 *
 *  PARAMS      : ctx_p    - CLI context
 *                in_cmd_p - Command string
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_cmd_help_q (cli_context_t  *ctx_p,
                            const char     *in_cmd_p)
{
    char         *cmd_p = NULL;
    char         *cmdtok_p,
                 *rem_cmd_p = NULL;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;
    int           cmd_argc = 0;
    char         *cmd_args[CLI_CMD_MAX_NUM_TOKENS];

    cli_out_newline(ctx_p);

    if (!in_cmd_p)
    {
        cli_out_help_q(cmd_tree_p);
        goto RETURN;
    }

    cmd_p = strdup(in_cmd_p);
    if (!cmd_p)
    {
        cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
        return;
    }

    cmdtok_p = strtok_r(cmd_p, " ", &rem_cmd_p);
    while (cmdtok_p)
    {
        cmd_args[cmd_argc] = cmdtok_p;
        cmd_argc++;
 
        token_p = cli_tree_find_token(cmd_tree_p, 1, cmdtok_p, cli_match_token);
        if (!token_p)
            token_p = cli_tree_find_token(cmd_tree_p, 1, NULL, cli_match_value_token);

        if (!token_p)
        {
            cli_out(ctx_p, "Invalid command\n");
            goto RETURN;
        }

        cmd_tree_p = &token_p->tnode.sub_tree;

        cmdtok_p = strtok_r(NULL, " ", &rem_cmd_p);
    }

    if (list_empty(cmd_tree_p->nodes))
        cli_out(ctx_p, "\t<ENTER>\n");
    else
        cli_out_help_q(cmd_tree_p);

RETURN:
    cli_out_newline(ctx_p);
    cli_out_prompt(ctx_p->cur_prompt_p, cmd_argc, cmd_args);

    if (cmd_p)
        free(cmd_p);

    return;
}

/*****************************************************************************
 *
 *  NAME        : cli_out_help_q
 *
 *  DESCRIPTION : Prints next tokens and their description.
 *
 *  PARAMS      : cmd_tree_p - Options for next token.
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_out_help_q (tree_t  *cmd_tree_p)
{
    cli_token_t   *token_p;
    int            max_len = 0,
                   namelen;

    tree_foreach_member(*cmd_tree_p, cli_token_t, tnode, token_p)
    {
        namelen = strlen(token_p->name_p);

        if (namelen > max_len)
            max_len = namelen;
    }

    tree_foreach_member(*cmd_tree_p, cli_token_t, tnode, token_p)
        cli_out(token_p->ctx_p, "\t%-*s%s\n", namelen+4, token_p->name_p, token_p->desc_p);
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_help_tab
 *
 *  DESCRIPTION : Auto-complete next token
 *
 *  PARAMS      : ctx_p        - CLI context
 *                in_cmd_pp    - Command string
 *                in_cmd_len_p - Length of the command string
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_cmd_help_tab (cli_context_t  *ctx_p,
                              char          **in_cmd_pp,
                              size_t         *in_cmd_len_p)
{
    char         *in_cmd_p = *in_cmd_pp;
    char         *cmd_p = NULL;
    char         *cmdtok_p,
                 *rem_cmd_p = NULL;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;
    int           cmd_argc = 0;
    char         *cmd_args[CLI_CMD_MAX_NUM_TOKENS];

    if (!in_cmd_p)
    {
        token_p = tree_first_member(*cmd_tree_p, cli_token_t, tnode);
        if (token_p)
        {
            size_t  name_len = strlen(token_p->name_p);

            cli_out(ctx_p, "%s ", token_p->name_p);
            
            *in_cmd_pp = malloc(name_len + 1 + 1);
            if (*in_cmd_pp)
            {
                sprintf(*in_cmd_pp, "%s ", token_p->name_p);
                *in_cmd_len_p = name_len + 1;
            }
            else
            {
                cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
            }
        }

        return;
    }

    cmd_p = strdup(in_cmd_p);
    if (!cmd_p)
    {
        cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
        return;
    }

    cmdtok_p = strtok_r(cmd_p, " ", &rem_cmd_p);
    while (cmdtok_p)
    {
        cmd_args[cmd_argc] = cmdtok_p;
        cmd_argc++;
 
        token_p = cli_tree_find_token(cmd_tree_p, 1, cmdtok_p, cli_match_token);
        if (!token_p)
        {
            cli_out(ctx_p, "Invalid command\n");
            goto RETURN;
        }

        cmd_tree_p = &token_p->tnode.sub_tree;

        cmdtok_p = strtok_r(NULL, " ", &rem_cmd_p);
    }

    if (token_p)
    {
        size_t   last_token_len = strlen(cmd_args[cmd_argc-1]),
                 name_len = strlen(token_p->name_p);

        in_cmd_p = realloc(*in_cmd_pp, *in_cmd_len_p + name_len - last_token_len + 1 + 1);
        if (in_cmd_p)
        {
            *in_cmd_pp = in_cmd_p;
            (*in_cmd_len_p) += sprintf(*in_cmd_pp + *in_cmd_len_p, "%s ", token_p->name_p + last_token_len);
        }
        else
        {
            cli_log(LOG_LEVEL_HIGH, "Alloc error: %s\n", strerror(errno));
            goto RETURN;
        }

        cmd_args[cmd_argc-1] = token_p->name_p;
        cli_out_prompt(ctx_p->cur_prompt_p, cmd_argc, cmd_args);
    }

RETURN:
    if (cmd_p)
        free(cmd_p);

    return;
}

/*****************************************************************************
 *
 *  NAME        : cli_out_prompt
 *
 *  DESCRIPTION : Print the given prompt.
 *                Optionally print the commands as well.
 *
 *  PARAMS      : prompt_p - The prompt to be printed
 *                cmd_argc - Number of command tokens
 *                cmd_args - Commmand tokens
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_out_prompt (cli_prompt_t  *prompt_p,
                            int            cmd_argc,
                            char          *cmd_args[])
{
    int  ii;

    cli_out(prompt_p->ctx_p, "\r%s ", prompt_p->name_p);

    for (ii = 0; ii < cmd_argc; ii++)
        cli_out(prompt_p->ctx_p, "%s ", cmd_args[ii]);
}

static void cli_start_telnet_server (cli_context_t  *ctx_p)
{
}

static void cli_stop_telnet_server (cli_context_t  *ctx_p)
{
}

static void cli_start_ssh_server (cli_context_t  *ctx_p)
{
}

static void cli_stop_ssh_server (cli_context_t  *ctx_p)
{
}

static void cli_start (cli_context_t  *ctx_p)
{
    char   *cmd_p = NULL;
    size_t  cmd_len = 0;

    cli_out_clear_screen(ctx_p);
    cli_out_prompt(ctx_p->root_prompt_p, 0, NULL);

    while (true)
    {
        switch (cli_get_input(ctx_p, &cmd_p, &cmd_len))
        {
            case CLI_KEY_ENTER:
            {
                if (cmd_p)
                    cli_log(LOG_LEVEL_LOW, "Received command: %s\n", cmd_p);

                cli_cmd_parse(ctx_p, cmd_p);

                free(cmd_p);
                cmd_p = NULL;
                cmd_len = 0;
            }
            break;

            case CLI_KEY_Q:
                cli_cmd_help_q(ctx_p, cmd_p);
                break;

            case CLI_KEY_TAB:
                cli_cmd_help_tab(ctx_p, &cmd_p, &cmd_len);
                break;

            case CLI_KEY_UP_ARROW:
            case CLI_KEY_DOWN_ARROW:
                break;

            default:
                break;
        }
    }
}

static int cli_get_input (cli_context_t  *ctx_p,
                          char          **cmd_pp,
                          size_t         *cmd_len_p)
{
    int     last_key = CLI_KEY_ENTER;

    char    cmd_buf[129];
    int     ch,
            ii;

    memset(cmd_buf, '\0', sizeof(cmd_buf));
    ii = 0;

    while (true)
    {
        ch = cli_in(ctx_p);

        if (ch == '\n')
        {
            cli_out(ctx_p, "%c", ch);
            break;
        }
        else if (ch == '?')
        {
            last_key = CLI_KEY_Q;
            cli_out(ctx_p, "%c\n", ch);
            break;
        }
        else if (ch == '\t')
        {
            last_key = CLI_KEY_TAB;
            break;
        }
        else if (ch == 27) // Escape character
        {
            ch = getchar();
            if (ch == '[')
            {
                ch = getchar();
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
                )
        {
            if (ii < (sizeof(cmd_buf) - 1))
                cmd_buf[ii++] = ch;
            else
            {
                *cmd_pp = realloc(*cmd_pp, (*cmd_len_p) + sizeof(cmd_buf));
                strcpy(*cmd_pp + (*cmd_len_p), cmd_buf);
                (*cmd_len_p) += (sizeof(cmd_buf) - 1);

                memset(cmd_buf, '\0', sizeof(cmd_buf) - 1);
                ii = 0;
                cmd_buf[ii++] = ch;
            }

            cli_out(ctx_p, "%c", ch);
        }
    }

    if (ii > 0)
    {
        *cmd_pp = realloc(*cmd_pp, (*cmd_len_p) + ii + 1);
        strcpy(*cmd_pp + (*cmd_len_p), cmd_buf);
        (*cmd_len_p) += ii;
    }

    return last_key;
}

/*****************************************************************************
 *
 *  NAME        : cli_out
 *
 *  DESCRIPTION : CLI printf
 *
 *  PARAMS      : ctx_p - CLI context
 *                As same as printf
 *
 *  RETURNS     : As same as printf
 *
 *****************************************************************************/
static int cli_out (cli_context_t *ctx_p, const char *fmt_p, ...)
{
    int      nchars;
    va_list  args;

    va_start(args, fmt_p);
    nchars = vprintf(fmt_p, args);
    va_end(args);

    fflush(stdout);

    return nchars;
}

/*****************************************************************************
 *
 *  NAME        : cli_in
 *
 *  DESCRIPTION : Reads a character from CLI input
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : The character read
 *
 *****************************************************************************/
static char cli_in (cli_context_t  *ctx_p)
{
    char  ch;

    while (true)
    {
        if (read(STDIN_FILENO, &ch, 1) == 1)
            break;
    }

    return ch;
}


/*****************************************************************************
   Global Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_context_new
 *
 *  DESCRIPTION : Create a new CLI context
 *
 *  PARAMS      : cfg_filename_p      - Configuration file name
 *                enable_telnet_b     - true to enable telnet
 *                telnet_port         - 0 to use 23
 *                enable_ssh_b        - true to enable ssh
 *                ssh_port            - 0 to use 22
 *
 *  RETURNS     : CLI context
 *
 *****************************************************************************/
cli_context_t* cli_context_new (const char  *cfg_filename_p,
                                bool         enable_telnet_b,
                                uint16_t     telnet_port,
                                bool         enable_ssh_b,
                                uint16_t     ssh_port)
{
    cli_context_t  *ctx_p;

    char           *save_name_p = NULL,
                   *save_filename_p = NULL;
    json_t         *cfg_jobj_p = NULL;
    FILE           *cfg_fl = NULL;
    logger_t       *logger_p = NULL;

    if (!cfg_filename_p)
    {
        return NULL;
    }

    save_filename_p = strdup(cfg_filename_p);
    if (!save_filename_p)
        goto FATAL;

    /* If the file didn't exist, its okay.
     * It only means, this is the first time this program is being run.
     * But if it exists and we are unable to open it, then it is an error.
     */
    cfg_fl = fopen(cfg_filename_p, "r");
    if (   !cfg_fl
        &&  errno != ENOENT
       )
    {
        goto FATAL;
    }

    /* If the file is empty, it is as good as it didn't exist. */
    if (   cfg_fl
        && is_file_empty(cfg_fl)
       )
    {
        fclose(cfg_fl);
        cfg_fl = NULL;
    }

    if (cfg_fl)
    {
        cfg_jobj_p = json_loadf(cfg_fl, JSON_DECODE_ANY, NULL);
        if (!cfg_jobj_p)
            goto FATAL;
    }

    ctx_p = calloc(1, sizeof(*ctx_p));
    if (!ctx_p)
        goto FATAL;

    ctx_p->cfg_filename_p = save_filename_p;
    ctx_p->cc_jobj_p = cfg_jobj_p;

    ctx_p->enable_telnet_b = enable_telnet_b;
    ctx_p->telnet_port =   telnet_port == 0
                         ? 23
                         : telnet_port;

    ctx_p->enable_ssh_b = enable_ssh_b;
    ctx_p->ssh_port =   ssh_port == 0
                         ? 22
                         : ssh_port;

    if (cfg_fl)
        fclose(cfg_fl);

    return ctx_p;

FATAL:
    if (save_filename_p)  free(save_filename_p);
    if (cfg_fl)           fclose(cfg_fl);
    if (cfg_jobj_p)       json_decref(cfg_jobj_p);

    return NULL;
}

/*****************************************************************************
 *
 *  NAME        : cli_context_set_root_prompt
 *
 *  DESCRIPTION : Set the root CLI prompt
 *
 *  PARAMS      : ctx_p     - CLI context
 *                name_p    - Prompt name
 *
 *  RETURNS     : CLI prompt that was added
 *
 *****************************************************************************/
cli_prompt_t* cli_context_set_root_prompt (cli_context_t  *ctx_p,
                                           const char     *name_p)
{
    if (   !ctx_p
        || !name_p
       )
    {
        return NULL;
    }

    ctx_p->root_prompt_p = cli_prompt_new(ctx_p, NULL, name_p);
    ctx_p->cur_prompt_p = ctx_p->root_prompt_p;

    return ctx_p->root_prompt_p;
}

/*****************************************************************************
 *
 *  NAME        : cli_prompt_add_token
 *
 *  DESCRIPTION : Add a CLI token
 *
 *  PARAMS      : ctx_p     - CLI context
 *                parent_p  - Parent CLI prompt
 *                name_p    - Keyword or value name
 *                desc_p    - Description of this value or keyword
 *                type      - KEYWORD or VALUE
 *                cmd_valid - Validation function
 *
 *  RETURNS     : CLI token that was added
 *
 *****************************************************************************/
cli_token_t* cli_prompt_add_token (cli_context_t  *ctx_p,
                                   cli_prompt_t   *parent_p,
                                   const char     *name_p,
                                   const char     *desc_p,
                                   int             type,
                                   cli_cmd_f       cb)
{
    cli_token_t  *token_p = NULL;

    char         *save_name_p = NULL,
                 *save_desc_p = NULL;

    if (   !ctx_p
        || !parent_p
        || !name_p
        || !desc_p
        ||  type < 0
        ||  type >= CLI_NUM_TOKEN_TYPES
       )
    {
        return NULL;
    }

    save_name_p = strdup(name_p);
    save_desc_p = strdup(desc_p);
    if (   !save_name_p
        || !save_desc_p
       )
    {
        goto FATAL;
    }

    token_p = calloc(1, sizeof(*token_p));
    if (!token_p)
        goto FATAL;

    token_p->ctx_p = ctx_p;

    token_p->pprompt_p = parent_p;

    tree_node_init(token_p->tnode);
    tree_add_node(parent_p->cmd_tree, token_p->tnode);

    token_p->type = type;
    token_p->cli_cmd_cb = cb;
    token_p->name_p = save_name_p;
    token_p->desc_p = save_desc_p;

    return token_p;

FATAL:
    if (save_name_p)  free(save_name_p);
    if (save_desc_p)  free(save_desc_p);
    if (token_p)      free(token_p);

    return NULL;
}

/*****************************************************************************
 *
 *  NAME        : cli_token_add_token
 *
 *  DESCRIPTION : Add a CLI token
 *
 *  PARAMS      : ctx_p     - CLI context
 *                parent_p  - Parent CLI token
 *                name_p    - Keyword or value name
 *                desc_p    - Description of this value or keyword
 *                type      - KEYWORD or VALUE
 *                cmd_valid - Validation function
 *
 *  RETURNS     : CLI token that was added
 *
 *****************************************************************************/
cli_token_t* cli_token_add_token (cli_context_t  *ctx_p,
                                  cli_token_t    *parent_p,
                                  const char     *name_p,
                                  const char     *desc_p,
                                  int             type,
                                  cli_cmd_f       cb)
{
    cli_token_t  *token_p = NULL;

    char         *save_name_p = NULL,
                 *save_desc_p = NULL;

    if (   !ctx_p
        || !parent_p
        || !name_p
        || !desc_p
        ||  type < 0
        ||  type >= CLI_NUM_TOKEN_TYPES
       )
    {
        return NULL;
    }

    /* If a node extends to a prompt, then it cannot have children */
    if (parent_p->cprompt_p)
        return NULL;

    save_name_p = strdup(name_p);
    save_desc_p = strdup(desc_p);
    if (   !save_name_p
        || !save_desc_p
       )
    {
        goto FATAL;
    }

    token_p = calloc(1, sizeof(*token_p));
    if (!token_p)
        goto FATAL;

    token_p->ctx_p = ctx_p;

    tree_node_init(token_p->tnode);
    tree_add_node(parent_p->tnode.sub_tree, token_p->tnode);

    token_p->type = type;
    token_p->cli_cmd_cb = cb;
    token_p->name_p = save_name_p;
    token_p->desc_p = save_desc_p;

    return token_p;

FATAL:
    if (save_name_p)  free(save_name_p);
    if (save_desc_p)  free(save_desc_p);
    if (token_p)      free(token_p);

    return NULL;
}

/*****************************************************************************
 *
 *  NAME        : cli_token_set_prompt
 *
 *  DESCRIPTION : Set a CLI prompt
 *
 *  PARAMS      : ctx_p     - CLI context
 *                parent_p  - Parent CLI token
 *                name_p    - Prompt name
 *
 *  RETURNS     : CLI prompt that was added
 *
 *****************************************************************************/
cli_prompt_t* cli_token_set_prompt (cli_context_t  *ctx_p,
                                    cli_token_t    *parent_p,
                                    const char     *name_p)
{
    if (   !ctx_p
        || !parent_p
       )
    {
        return NULL;
    }

    /* A node having children cannot be extended to a prompt */
    if (!tree_node_is_leaf(parent_p->tnode))
        return NULL;

    if (parent_p->cprompt_p)
        cli_prompt_del(parent_p->cprompt_p);

    parent_p->cprompt_p = cli_prompt_new(ctx_p, parent_p, name_p);

    return parent_p->cprompt_p;
}

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
        struct termios  orig_termios,
                        raw_termios;

        cli_log(LOG_LEVEL_HIGH, "Starting CLI in interactive mode\n");

        // Get the original terminal settings
        tcgetattr(STDIN_FILENO, &orig_termios);
        raw_termios = orig_termios;

        // Set the terminal to raw mode
        raw_termios.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);

        cli_start(ctx_p);

        // Restore the original terminal settings before exiting
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
    else
    {
    }
}


/*****************************************************************************
   Test Functions
*****************************************************************************/

