
#include "cli.h"


void cisco_ios_build_cli_show (cli_context_t *ctx_p, cli_prompt_t *priv_exec_p)
{
    /*
     * Router# show
     *
     * Children of show can be added later.
     */
    cli_token_t *show_p =
         cli_prompt_add_token(
             ctx_p,
             priv_exec_p,
             "show",
             "Show running system information",
             CLI_TOKEN_TYPE_KEYWORD,
             NULL);

}