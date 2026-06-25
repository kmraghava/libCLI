
#include "cli.h"


extern void cisco_ios_build_cli_show            (cli_context_t *ctx_p, cli_prompt_t *priv_exec_p);
extern void cisco_ios_build_cli_config_terminal (cli_context_t *ctx_p, cli_prompt_t *priv_exec_p);


void cisco_ios_build_cli_user_exec (cli_context_t *ctx_p, cli_prompt_t *user_exec_p)
{
    /*
     * Router> enable
     */
    cli_token_t *enable_p =
        cli_prompt_add_token(
            ctx_p,
            user_exec_p,
            "enable",
            "Turn on privileged commands",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    /*
     * Privileged EXEC mode
     *
     * Router#
     */
    cli_prompt_t *priv_exec_p =
        cli_token_set_prompt(
            ctx_p,
            enable_p,
            "Router#");

    cisco_ios_build_cli_show(ctx_p, priv_exec_p);
    cisco_ios_build_cli_config_terminal(ctx_p, priv_exec_p);
}

void cisco_ios_build_cli (cli_context_t *ctx_p)
{
    /*
     * User EXEC mode
     *
     * Router>
     */
    cli_prompt_t *user_exec_p =
        cli_context_set_root_prompt(ctx_p, "Router>");

    cisco_ios_build_cli_user_exec(ctx_p, user_exec_p);
}

int main (void)
{
    cli_context_t *ctx_p;

    ctx_p = cli_context_new(
                "/tmp/ios_cfg.json",
                false,
                0,
                false,
                0);

    cisco_ios_build_cli(ctx_p);

    cli_start(ctx_p);

    return 0;
}