/*****************************************************************************
 *
 * FILE NAME     : cli_builder.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 ******************************************************************************
 *
 *  DESCRIPTION : Command Line Interface builder
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include "cli_context.h"
#include "cli_io.h"
#include "cli_log.h"
#include "cli_prompt.h"
#include "cli_token.h"
#include <errno.h>


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

static void cli_token_del (tree_node_t  *tnode_p);


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

    tree_init(prompt_p->cmd_tree, NULL);

    prompt_p->name_p = string_new(name_p);
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
        string_t      *cmd_p;
        clist_node_t  *nd_p;

        if (prompt_p->parent_p)
            prompt_p->parent_p->cprompt_p = NULL;

        tree_del(&prompt_p->cmd_tree, cli_token_del);

        if (prompt_p->name_p)
            string_delete(prompt_p->name_p);

        clist_foreach_member(prompt_p->history_p, nd_p, cmd_p)
            string_delete(cmd_p);
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
            string_delete(token_p->name_p);

        if (token_p->desc_p)
            string_delete(token_p->desc_p);

        free(token_p);
    }
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

    string_t       *save_filename_p = NULL;
    json_t         *cfg_jobj_p = NULL;
    FILE           *cfg_fl = NULL;

    if (!cfg_filename_p)
        return NULL;

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

    save_filename_p = string_new(cfg_filename_p);
    if (!save_filename_p)
        goto FATAL;

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
    if (save_filename_p)  string_delete(save_filename_p);
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
cli_token_t* cli_prompt_add_token (cli_context_t   *ctx_p,
                                   cli_prompt_t     *parent_p,
                                   const char       *name_p,
                                   const char       *desc_p,
                                   uint64_t          type,
                                   cli_validator_f   cb)
{
    cli_token_t  *token_p = NULL;

    string_t     *save_name_p = NULL,
                 *save_desc_p = NULL;

    if (   !ctx_p
        || !parent_p
        || !name_p
        || !desc_p
       )
    {
        return NULL;
    }

    save_name_p = string_new(name_p);
    save_desc_p = string_new(desc_p);
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
    token_p->validator_cb = cb;
    token_p->name_p = save_name_p;
    token_p->desc_p = save_desc_p;

    return token_p;

FATAL:
    if (save_name_p)  string_delete(save_name_p);
    if (save_desc_p)  string_delete(save_desc_p);
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
cli_token_t* cli_token_add_token (cli_context_t    *ctx_p,
                                  cli_token_t      *parent_p,
                                  const char       *name_p,
                                  const char       *desc_p,
                                  uint64_t          type,
                                  cli_validator_f   cb)
{
    cli_token_t  *token_p = NULL;

    string_t     *save_name_p = NULL,
                 *save_desc_p = NULL;

    if (   !ctx_p
        || !parent_p
        || !name_p
        || !desc_p
       )
    {
        return NULL;
    }

    /* If a node extends to a prompt, then it cannot have children */
    if (parent_p->cprompt_p)
        return NULL;

    save_name_p = string_new(name_p);
    save_desc_p = string_new(desc_p);
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
    token_p->validator_cb = cb;
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
   Test Functions
*****************************************************************************/
