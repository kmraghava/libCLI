
#include "cli.h"


extern void cisco_ios_build_cli_interface (cli_context_t *ctx_p, cli_prompt_t *config_prompt_p);


static void cisco_ios_build_cli_ip_dhcp_server (cli_context_t *ctx_p, cli_prompt_t *config_prompt_p)
{
    cli_token_t *ip_p;
    cli_token_t *dhcp_p;

    ip_p = cli_prompt_add_token(
                ctx_p,
                config_prompt_p,
                "ip",
                "IP configuration",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    dhcp_p = cli_token_add_token(
                ctx_p,
                ip_p,
                "dhcp",
                "DHCP configuration",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    cli_token_t *excluded_p;
    cli_token_t *start_p;

    excluded_p = cli_token_add_token(
                    ctx_p,
                    dhcp_p,
                    "excluded-address",
                    "Prevent DHCP assignment",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    start_p = cli_token_add_token(
                    ctx_p,
                    excluded_p,
                    "<ipv4-address>",
                    "Excluded address",
                    CLI_TOKEN_TYPE_VALUE_IP4ADDR,
                    NULL);

    cli_token_add_token(
        ctx_p,
        start_p,
        "<ipv4-address>",
        "Last excluded address",
        CLI_TOKEN_TYPE_VALUE_IP4ADDR,
        NULL);

    cli_token_t *pool_p;
    cli_token_t *pool_name_p;
    cli_prompt_t *dhcp_pool_prompt_p;

    pool_p = cli_token_add_token(
                ctx_p,
                dhcp_p,
                "pool",
                "Create DHCP pool",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    pool_name_p = cli_token_add_token(
                    ctx_p,
                    pool_p,
                    "<pool-name>",
                    "Pool name",
                    CLI_TOKEN_TYPE_VALUE_STRING,
                    NULL);

    dhcp_pool_prompt_p =
        cli_token_set_prompt(
            ctx_p,
            pool_name_p,
            "Router(dhcp-config)#");

    cli_token_t *network_p;
    cli_token_t *net_addr_p;

    network_p = cli_prompt_add_token(
                    ctx_p,
                    dhcp_pool_prompt_p,
                    "network",
                    "Pool network",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    net_addr_p = cli_token_add_token(
                    ctx_p,
                    network_p,
                    "<network>",
                    "Network address",
                    CLI_TOKEN_TYPE_VALUE_IP4ADDR,
                    NULL);

    cli_token_add_token(
            ctx_p,
            net_addr_p,
            "<netmask>",
            "Subnet mask",
            CLI_TOKEN_TYPE_VALUE_IP4ADDR,
            NULL);

    cli_token_t *defrt_p;

    defrt_p = cli_prompt_add_token(
                    ctx_p,
                    dhcp_pool_prompt_p,
                    "default-router",
                    "Default gateway",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cli_token_add_token(
            ctx_p,
            defrt_p,
            "<ipv4-address>",
            "Gateway address",
            CLI_TOKEN_TYPE_VALUE_IP4ADDR,
            NULL);

    cli_token_t *dns_p;
    cli_token_t *dns1_p;

    dns_p = cli_prompt_add_token(
                ctx_p,
                dhcp_pool_prompt_p,
                "dns-server",
                "DNS servers",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    dns1_p = cli_token_add_token(
                ctx_p,
                dns_p,
                "<ipv4-address>",
                "DNS server",
                CLI_TOKEN_TYPE_VALUE_IP4ADDR,
                NULL);

    cli_token_add_token(
            ctx_p,
            dns1_p,
            "<ipv4-address>",
            "Additional DNS server",
            CLI_TOKEN_TYPE_VALUE_IP4ADDR,
            NULL);

    cli_token_t *domain_p = cli_prompt_add_token(
                ctx_p,
                dhcp_pool_prompt_p,
                "domain-name",
                "Domain name",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    cli_token_add_token(
        ctx_p,
        domain_p,
        "<domain>",
        "Domain name",
        CLI_TOKEN_TYPE_VALUE_FQDN,
        NULL);

    cli_token_t *lease_p;
    cli_token_t *days_p;

    lease_p = cli_prompt_add_token(
                    ctx_p,
                    dhcp_pool_prompt_p,
                    "lease",
                    "Lease duration",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    days_p = cli_token_add_token(
                    ctx_p,
                    lease_p,
                    "<days>",
                    "",
                    CLI_TOKEN_TYPE_VALUE_INT,
                    NULL);

    cli_token_add_token(
            ctx_p,
            days_p,
            "<hours>",
            "",
            CLI_TOKEN_TYPE_VALUE_INT,
            NULL);

    cli_token_add_token(
            ctx_p,
            days_p,
            "infinite",
            "",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);
}

static void cisco_ios_build_cli_ipv6_dhcp_server (cli_context_t *ctx_p, cli_prompt_t *config_prompt_p)
{
    cli_token_t  *ipv6_p;
    cli_token_t  *dhcp_p;
    cli_token_t  *pool_p;
    cli_token_t  *pool_name_p;
    cli_prompt_t *dhcpv6_prompt_p;

    ipv6_p = cli_prompt_add_token(
                ctx_p,
                config_prompt_p,
                "ipv6",
                "IPv6 configuration",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    dhcp_p = cli_token_add_token(
                ctx_p,
                ipv6_p,
                "dhcp",
                "DHCPv6 configuration",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    pool_p = cli_token_add_token(
                ctx_p,
                dhcp_p,
                "pool",
                "Create DHCPv6 pool",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    pool_name_p = cli_token_add_token(
                    ctx_p,
                    pool_p,
                    "<pool-name>",
                    "DHCPv6 pool name",
                    CLI_TOKEN_TYPE_VALUE_STRING,
                    NULL);

    dhcpv6_prompt_p = cli_token_set_prompt(
                        ctx_p,
                        pool_name_p,
                        "Router(config-dhcpv6)#");

    cli_token_t *address_p;
    cli_token_t *prefix_p;

    address_p = cli_prompt_add_token(
                    ctx_p,
                    dhcpv6_prompt_p,
                    "address",
                    "Address configuration",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    prefix_p = cli_token_add_token(
                    ctx_p,
                    address_p,
                    "prefix",
                    "Delegated prefix",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cli_token_add_token(
            ctx_p,
            prefix_p,
            "<ipv6-prefix>",
            "IPv6 prefix",
            CLI_TOKEN_TYPE_VALUE_IP6ADDR_PLEN,
            NULL);

    cli_token_t *dns_p;
    cli_token_t *dns1_p;

    dns_p = cli_prompt_add_token(
                ctx_p,
                dhcpv6_prompt_p,
                "dns-server",
                "Configure DNS server",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    dns1_p = cli_token_add_token(
                ctx_p,
                dns_p,
                "<ipv6-address>",
                "DNS server",
                CLI_TOKEN_TYPE_VALUE_IP6ADDR,
                NULL);

    cli_token_add_token(
            ctx_p,
            dns1_p,
            "<ipv6-address>",
            "Additional DNS server",
            CLI_TOKEN_TYPE_VALUE_IP6ADDR,
            NULL);

    cli_token_t *domain_p;

    domain_p = cli_prompt_add_token(
                    ctx_p,
                    dhcpv6_prompt_p,
                    "domain-name",
                    "Domain name",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cli_token_add_token(
            ctx_p,
            domain_p,
            "<domain>",
            "Domain name",
            CLI_TOKEN_TYPE_VALUE_FQDN,
            NULL);

    cli_token_t *pref_p;

    pref_p = cli_prompt_add_token(
                ctx_p,
                dhcpv6_prompt_p,
                "preference",
                "Server preference",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    cli_token_add_token(
            ctx_p,
            pref_p,
            "<0-255>",
            "Preference value",
            CLI_TOKEN_TYPE_VALUE_INT,
            NULL);
}

void cisco_ios_build_cli_config_terminal (cli_context_t *ctx_p, cli_prompt_t *priv_exec_p)
{
    /*
     * Router# configure
     */
    cli_token_t *configure_p =
        cli_prompt_add_token(
            ctx_p,
            priv_exec_p,
            "configure",
            "Enter configuration mode",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    /*
     * Router# configure terminal
     */
    cli_token_t *terminal_p =
        cli_token_add_token(
            ctx_p,
            configure_p,
            "terminal",
            "Configure from terminal",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    /*
     * Global Configuration Mode
     *
     * Router(config)#
     */
    cli_prompt_t *config_prompt_p =
        cli_token_set_prompt(
            ctx_p,
            terminal_p,
            "Router(config)#");

    cisco_ios_build_cli_interface(ctx_p, config_prompt_p);
    cisco_ios_build_cli_ip_dhcp_server(ctx_p, config_prompt_p);
    cisco_ios_build_cli_ipv6_dhcp_server(ctx_p, config_prompt_p);
}