/*****************************************************************************
 *
 * FILE NAME     : cli_parser.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 9, 2026
 *
 ******************************************************************************
 *
 *  DESCRIPTION : Command Line Interface parser
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include "cli_context.h"
#include "cli_log.h"
#include "cli_parser.h"
#include "cli_prompt.h"
#include "cli_token.h"


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
#define cli_tree_find_token(t, depth, key_p, member_match_fn)                   \
    ({                                                                          \
        tree_node_t *__tnp = tree_find_node(t, depth, key_p, member_match_fn);  \
        __tnp ? tree_get(__tnp, cli_token_t, tnode) : NULL;                     \
    })


/*****************************************************************************
   Local Function Prototypes
*****************************************************************************/
static bool cli_match_value_token (tree_node_t *tnode_p, void *key_p);
static bool cli_match_token       (tree_node_t *tnode_p, void *key_p);

static void cli_cmd_add_to_history (cli_prompt_t  *prompt_p,
                                    string_t      *cmd_p);

static void cli_print_help_q (tree_t  *cmd_tree_p);


/*****************************************************************************
   Local Functions
*****************************************************************************/
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
    string_t     *name_p = key_p;

    return (   CLI_TOKEN_TYPE_KEYWORD == token_p->type
            && 0 == string_compare(token_p->name_p, name_p, string_length(name_p), true)
           );
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
                                    string_t      *cmd_p)
{
    if (   prompt_p
        && cmd_p
       )
    {
        string_t *hist_cmd_p = string_clone(cmd_p);
        if (!hist_cmd_p)
        {
            cli_log(LOG_LEVEL_HIGH, "string_clone failed\n");
            return;
        }

        clist_node_t *node_p = clist_push_back(prompt_p->history_p, hist_cmd_p);
        if (node_p)
            prompt_p->cur_cmd_p = node_p;
        else
        {
            cli_log(LOG_LEVEL_HIGH, "Failed to add command to history\n");
            string_delete(hist_cmd_p);
        }
    }
}

/*****************************************************************************
 *
 *  NAME        : cli_print_help_q
 *
 *  DESCRIPTION : Prints next tokens and their description.
 *
 *  PARAMS      : cmd_tree_p - Options for next token.
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
static void cli_print_help_q (tree_t  *cmd_tree_p)
{
    cli_token_t   *token_p;
    long           max_len = 0,
                   namelen;

    tree_foreach_member(*cmd_tree_p, cli_token_t, tnode, token_p)
    {
        namelen = string_length(token_p->name_p);

        if (namelen > max_len)
            max_len = namelen;
    }

    tree_foreach_member(*cmd_tree_p, cli_token_t, tnode, token_p)
        cli_print(token_p->ctx_p, "\t%-*s%s\n", max_len+4, string_cstr(token_p->name_p), string_cstr(token_p->desc_p));
}


/*****************************************************************************
   Global Functions
*****************************************************************************/
/*****************************************************************************
 *
 *  NAME        : cli_cmd_parse
 *
 *  DESCRIPTION : Parse the given command and call registered callback.
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_cmd_parse (cli_context_t  *ctx_p)
{
    string_t     *in_cmd_p = ctx_p->line_editor.cmd_p;
    string_t    **cmd_tokens_pp = NULL;
    int           i_tok = 0;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;

    cmd_tokens_pp = string_split(in_cmd_p, " ");
    if (!cmd_tokens_pp)
    {
        cli_log(LOG_LEVEL_HIGH, "string_split failed\n");
        return;
    }

    while (cmd_tokens_pp[i_tok])
    {
        if (string_length(cmd_tokens_pp[i_tok]) > 0)
        {
            token_p = cli_tree_find_token(cmd_tree_p, 1, cmd_tokens_pp[i_tok], cli_match_token);
            if (!token_p)
                token_p = cli_tree_find_token(cmd_tree_p, 1, NULL, cli_match_value_token);

            if (!token_p)
            {
                cli_print(ctx_p, "\n\tInvalid command\n\n");
                goto RETURN;
            }

            cmd_tree_p = &token_p->tnode.sub_tree;
        }

        i_tok++;
    }

    if (token_p->cprompt_p)
    {
        ctx_p->cur_prompt_p = token_p->cprompt_p;
        goto RETURN;
    }

    if (!tree_node_is_leaf(token_p->tnode))
    {
        cli_print(ctx_p, "\n\tIncomplete command\n\n");
        goto RETURN;
    }

    if (token_p->cli_cmd_cb)
        token_p->cli_cmd_cb(ctx_p, cmd_tokens_pp);

    cli_cmd_add_to_history(ctx_p->cur_prompt_p, in_cmd_p);

RETURN:
    if (cmd_tokens_pp)
        string_array_del(cmd_tokens_pp);

    return;
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_help_q
 *
 *  DESCRIPTION : Responds to the ask for available options for next token
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_cmd_help_q (cli_context_t  *ctx_p)
{
    string_t     *in_cmd_p = ctx_p->line_editor.cmd_p;
    string_t    **cmd_tokens_pp = NULL;
    int           i_tok = 0;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;

    if (string_length(in_cmd_p) == 0)
    {
        cli_print_help_q(cmd_tree_p);
        goto RETURN;
    }

    cmd_tokens_pp = string_split(in_cmd_p, " ");
    if (!cmd_tokens_pp)
    {
        cli_log(LOG_LEVEL_HIGH, "string_split failed\n");
        return;
    }

    while (cmd_tokens_pp[i_tok])
    {
        if (string_length(cmd_tokens_pp[i_tok]) > 0)
        {
            token_p = cli_tree_find_token(cmd_tree_p, 1, cmd_tokens_pp[i_tok], cli_match_token);
            if (!token_p)
                token_p = cli_tree_find_token(cmd_tree_p, 1, NULL, cli_match_value_token);

            if (!token_p)
            {
                cli_print(ctx_p, "\tInvalid command\n");
                goto RETURN;
            }

            cmd_tree_p = &token_p->tnode.sub_tree;
        }

        i_tok++;
    }

    if (list_empty(cmd_tree_p->nodes))
        cli_print(ctx_p, "\t<ENTER>\n");
    else
        cli_print_help_q(cmd_tree_p);

RETURN:
    if (cmd_tokens_pp)
        string_array_del(cmd_tokens_pp);

    return;
}

/*****************************************************************************
 *
 *  NAME        : cli_cmd_help_tab
 *
 *  DESCRIPTION : Auto-complete next token
 *
 *  PARAMS      : ctx_p - CLI context
 *
 *  RETURNS     : void
 *
 *****************************************************************************/
void cli_cmd_help_tab (cli_context_t  *ctx_p)
{
    string_t     *in_cmd_p  = ctx_p->line_editor.cmd_p;
    string_t    **cmd_tokens_pp = NULL;
    int           i_tok = 0;
    tree_t       *cmd_tree_p = &ctx_p->cur_prompt_p->cmd_tree;
    cli_token_t  *token_p = NULL;

    cmd_tokens_pp = string_split(in_cmd_p, " ");
    if (!cmd_tokens_pp)
    {
        cli_log(LOG_LEVEL_HIGH, "string_split failed\n");
        return;
    }

    while (cmd_tokens_pp[i_tok])
    {
        if (string_length(cmd_tokens_pp[i_tok]) > 0)
        {
            token_p = cli_tree_find_token(cmd_tree_p, 1, cmd_tokens_pp[i_tok], cli_match_token);
            if (!token_p)
            {
                cli_print(ctx_p, "\n\n\tInvalid command\n\n");
                goto RETURN;
            }

            cmd_tree_p = &token_p->tnode.sub_tree;
        }

        i_tok++;
    }

    if (token_p && string_last(in_cmd_p) != ' ')
        cli_out(ctx_p, "%s ", string_cstr(token_p->name_p) + string_length(cmd_tokens_pp[i_tok-1]));
    else
    {
        token_p = tree_first_member(*cmd_tree_p, cli_token_t, tnode);
        if (token_p && token_p->type != CLI_TOKEN_TYPE_VALUE)
             cli_out(ctx_p, "%s ", string_cstr(token_p->name_p));
    }

RETURN:
    if (cmd_tokens_pp)
        string_array_del(cmd_tokens_pp);

    return;
}


/*****************************************************************************
   Test Functions
*****************************************************************************/
