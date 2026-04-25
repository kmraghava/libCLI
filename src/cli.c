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
#include "kmrUtils/ctree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

    ctree_node_t   *tnode_p;

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

    cli_token_t    *parent_p;
    ctree_t        *cmd_tree_p;
};

struct cli_context_s
{
    cli_prompt_t  *root_prompt_p,
                  *cur_prompt_p;

    char          *cfg_filename_p;
    json_t        *cc_jobj_p;

    bool           enable_telnet_b;
    uint16_t       telnet_port;

    bool           enable_ssh_b;
    uint16_t       ssh_port;
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


/*****************************************************************************
   Local Function Prototypes
*****************************************************************************/
static cli_prompt_t* cli_prompt_new (cli_context_t  *ctx_p,
                                     cli_token_t    *parent_p,
                                     const char     *name_p);
static cli_prompt_t* cli_prompt_del (cli_prompt_t  *prompt_p);

static bool cli_match_value_token (void *member_p, void *key_p);
static bool cli_match_token       (void *member_p, void *key_p);

static void cli_cmd_parse (cli_context_t  *ctx_p,
                           const char     *cmd_p);

static void cli_out_help_q (ctree_t  *cmd_tree_p);

static void cli_out_prompt (cli_prompt_t  *prompt_p,
                            int            cmd_argc,
                            char          *cmd_args[]);

static void cli_start_telnet_server (cli_context_t  *ctx_p);

static void cli_start_ssh_server (cli_context_t  *ctx_p);


/*****************************************************************************
   Local Functions
*****************************************************************************/
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

    prompt_p->cmd_tree_p = ctree_new(NULL);
    if (!prompt_p->cmd_tree_p)
        goto FATAL;

    prompt_p->name_p = strdup(name_p);
    if (!prompt_p->name_p)
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
        if (prompt_p->cmd_tree_p)
            ctree_del(prompt_p->cmd_tree_p);

        if (prompt_p->name_p)
            free(prompt_p->name_p);

        free(prompt_p);
    }

    return NULL;
}

/*****************************************************************************
 *
 *  NAME        : cli_match_token
 *
 *  DESCRIPTION : Match CLI token with the given name.
 *
 *  PARAMS      : member_p - CLI token
 *                key_p    - CLI token name to match
 *
 *  RETURNS     : true if equal
 *                false otherwise
 *
 *****************************************************************************/
static bool cli_match_token (void *member_p, void *key_p)
{
    cli_token_t  *token_p = member_p;
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
static bool cli_match_value_token (void *member_p, void *key_p)
{
    cli_token_t  *token_p = member_p;

    (void)key_p;

    return (CLI_TOKEN_TYPE_VALUE == token_p->type);
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
    char         *cmd_p;
    char         *token_p,
                 *rem_cmd_p = NULL;
    ctree_t      *cmd_tree_p = ctx_p->cur_prompt_p->cmd_tree_p;
    cli_token_t  *mtoken_p;
    int           cmd_argc = 0;
    char         *cmd_args[CLI_CMD_MAX_NUM_TOKENS];

    cmd_p = strdup(in_cmd_p);
    if (!cmd_p)
        return;

    token_p = strtok_r(cmd_p, " ", &rem_cmd_p);
    while (token_p)
    {
        cmd_args[cmd_argc] = token_p;
        cmd_argc++;

        mtoken_p = ctree_find(cmd_tree_p, -1, token_p, cli_match_token);
        if (!mtoken_p)
            mtoken_p = ctree_find(cmd_tree_p, -1, NULL, cli_match_value_token);

        if (!mtoken_p)
        {
            cli_out(ctx_p, "Invalid command\n");
            goto RETURN;
        }

        cmd_tree_p = ctree_node_get_subtree(mtoken_p->tnode_p);

        token_p = strtok_r(NULL, " ", &rem_cmd_p);
    }

    if (!mtoken_p)
    {
        cli_out_prompt(ctx_p->cur_prompt_p, 0, NULL);
        goto RETURN;
    }

    if (mtoken_p->cprompt_p)
    {
        ctx_p->cur_prompt_p = mtoken_p->cprompt_p;

        cli_out_prompt(ctx_p->cur_prompt_p, 0, NULL);
        goto RETURN;
    }

    if (mtoken_p->cli_cmd_cb)
        mtoken_p->cli_cmd_cb(ctx_p, cmd_argc, cmd_args);

RETURN:
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
    char         *cmd_p;
    char         *token_p,
                 *rem_cmd_p = NULL;
    ctree_t      *cmd_tree_p = ctx_p->cur_prompt_p->cmd_tree_p;
    cli_token_t  *mtoken_p;
    int          cmd_argc = 0;
    char        *cmd_args[CLI_CMD_MAX_NUM_TOKENS];

    if (0 == strcmp(in_cmd_p, "?"))
    {
        cli_out_help_q(cmd_tree_p);
        cli_out_prompt(ctx_p->cur_prompt_p, 0, NULL);

        return;
    }

    cmd_p = strdup(in_cmd_p);
    if (!cmd_p)
        return;

    token_p = strtok_r(cmd_p, " ", &rem_cmd_p);
    while (token_p)
    {
        cmd_args[cmd_argc] = token_p;
        cmd_argc++;

        if (0 == strcmp(token_p, "?"))
        {
            int  ii;

            if (!cmd_tree_p)
                cli_out(ctx_p, "\n\t<ENTER>\n\n");
            else
                cli_out_help_q(cmd_tree_p);
            cli_out_prompt(ctx_p->cur_prompt_p, cmd_argc - 1, cmd_args);

            goto RETURN;
        }
 
        mtoken_p = ctree_find(cmd_tree_p, -1, token_p, cli_match_token);
        if (!mtoken_p)
            mtoken_p = ctree_find(cmd_tree_p, -1, NULL, cli_match_value_token);


        if (!mtoken_p)
        {
            cli_out(ctx_p, "Invalid command\n");
            goto RETURN;
        }

        cmd_tree_p = ctree_node_get_subtree(mtoken_p->tnode_p);

        token_p = strtok_r(NULL, " ", &rem_cmd_p);
    }

RETURN:
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
static void cli_out_help_q (ctree_t  *cmd_tree_p)
{
    ctree_node_t  *tnode_p;
    cli_token_t   *token_p;
    int            max_len = 0,
                   namelen;

    for (tnode_p = ctree_first_node(cmd_tree_p); tnode_p != NULL; tnode_p = ctree_node_next(tnode_p))
    {
        token_p = ctree_node_member(tnode_p);

        namelen = strlen(token_p->name_p);

        if (namelen > max_len)
            max_len = namelen;
    }

    for (tnode_p = ctree_first_node(cmd_tree_p); tnode_p != NULL; tnode_p = ctree_node_next(tnode_p))
    {
        token_p = ctree_node_member(tnode_p);

        cli_out(token_p->ctx_p, "\t%-*s%s\n", namelen+4, token_p->name_p, token_p->desc_p);
    }
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
    cli_out(prompt_p->ctx_p, "%s ", prompt_p->name_p);

    if (cmd_argc)
    {
        int  ii;

        for (ii = 0; ii < cmd_argc; ii++)
            cli_out(prompt_p->ctx_p, "%s ", cmd_args[ii]);
    }
}

static void cli_start_telnet_server (cli_context_t  *ctx_p)
{
}

static void cli_start_ssh_server (cli_context_t  *ctx_p)
{
}

static void cli_start (cli_context_t  *ctx_p)
{
    cli_out_prompt(ctx_p->root_prompt_p, 0, NULL);
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
 *  NAME        : cli_prompt_add_node
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
cli_token_t* cli_prompt_add_node (cli_context_t  *ctx_p,
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

    token_p->tnode_p = ctree_node_new(parent_p->cmd_tree_p, token_p);
    if (!token_p->tnode_p)
        goto FATAL;

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
 *  NAME        : cli_node_add_node
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
cli_token_t* cli_node_add_node (cli_context_t  *ctx_p,
                                cli_token_t    *parent_p,
                                const char     *name_p,
                                const char     *desc_p,
                                int             type,
                                cli_cmd_f       cb)
{
    cli_token_t  *token_p = NULL;

    char         *save_name_p = NULL,
                 *save_desc_p = NULL;
    ctree_t      *ptree_p;

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

    ptree_p = ctree_node_get_subtree(parent_p->tnode_p);
    if (!ptree_p)
    {
        ptree_p = ctree_new(parent_p->tnode_p);
        if (!ptree_p)
            goto FATAL;
    }

    token_p->tnode_p = ctree_node_new(ptree_p, token_p);
    if (!token_p->tnode_p)
        goto FATAL;

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
 *  NAME        : cli_node_add_prompt
 *
 *  DESCRIPTION : Add a CLI prompt
 *
 *  PARAMS      : ctx_p     - CLI context
 *                parent_p  - Parent CLI token
 *                name_p    - Prompt name
 *
 *  RETURNS     : CLI prompt that was added
 *
 *****************************************************************************/
cli_prompt_t* cli_node_add_prompt (cli_context_t  *ctx_p,
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
    if (!ctree_node_is_leaf(parent_p->tnode_p))
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

    if (ctx_p->enable_telnet_b)
        cli_start_telnet_server(ctx_p);

    if (ctx_p->enable_ssh_b)
        cli_start_ssh_server(ctx_p);

    if (   !ctx_p->enable_telnet_b
        && !ctx_p->enable_ssh_b
       )
    {
        cli_start(ctx_p);
    }
}

/*****************************************************************************
 *
 *  NAME        : cli_out
 *
 *  DESCRIPTION : CLI printf
 *
 *  PARAMS      : As same as printf
 *
 *  RETURNS     : As same as printf
 *
 *****************************************************************************/
int cli_out (cli_context_t *ctx_p, const char *fmt_p, ...)
{
    int      nchars;
    va_list  args;

    va_start(args, fmt_p);
    nchars = vprintf(fmt_p, args);
    va_end(args);

    return nchars;
}


/*****************************************************************************
   Test Functions
*****************************************************************************/

