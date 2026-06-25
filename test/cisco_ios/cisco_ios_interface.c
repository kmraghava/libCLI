
#include "cli.h"
#include <errno.h>
#include "kmrUtils/str.h"


static bool interface_validator(cli_context_t *ctx_p,
                                uint64_t value_type,
                                string_t *value_p)
{
    (void)ctx_p;
    (void)value_type;

    if (string_length(value_p) == 0)
        return false;

    char  *endptr;

    errno = 0;
    long result = strtol(string_cstr(value_p), &endptr, 10);

    if (errno != 0 || *endptr != '\0')
        return false;

    return (result >= 0 && result <= 255);
}

static void cisco_ios_build_cli_if_ipv4_address (cli_context_t *ctx_p, cli_token_t *ip_p)
{
    /*************************************************************************
     * Router(config-if)# ip address
     *************************************************************************/

    cli_token_t *address_p = cli_token_add_token(
                    ctx_p,
                    ip_p,
                    "address",
                    "Set the IP address of an interface",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    /*
     * Router(config-if)# ip address dhcp
     */
    cli_token_t *dhcp_p = cli_token_add_token(
            ctx_p,
            address_p,
            "dhcp",
            "Acquire address via DHCP",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    cli_token_t *client_id_p = cli_token_add_token(
                        ctx_p,
                        dhcp_p,
                        "client-id",
                        "Specify DHCP client identifier",
                        CLI_TOKEN_TYPE_KEYWORD,
                        NULL);

    cli_token_add_token(
            ctx_p,
            client_id_p,
            "<client-id>",
            "DHCP client identifier",
            CLI_TOKEN_TYPE_VALUE_STRING,
            NULL);

    cli_token_t *hostname_p = cli_token_add_token(
                        ctx_p,
                        dhcp_p,
                        "hostname",
                        "Specify DHCP hostname",
                        CLI_TOKEN_TYPE_KEYWORD,
                        NULL);

    cli_token_add_token(
            ctx_p,
            hostname_p,
            "<hostname>",
            "Hostname sent to DHCP server",
            CLI_TOKEN_TYPE_VALUE_STRING,
            NULL);

    cli_token_add_token(
        ctx_p,
        dhcp_p,
        "setroute",
        "Install default route learned from DHCP",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    /*
     * Router(config-if)# ip address negotiated
     */
    cli_token_t *negotiated_p = cli_token_add_token(
            ctx_p,
            address_p,
            "negotiated",
            "IP address negotiated dynamically",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    /*
     * Router(config-if)# ip address A.B.C.D
     */
    cli_token_t *addr_p = cli_token_add_token(
                    ctx_p,
                    address_p,
                    "<ipv4-address>",
                    "IPv4 address",
                    CLI_TOKEN_TYPE_VALUE_IP4ADDR,
                    NULL);

    /*
     * Router(config-if)# ip address A.B.C.D M.M.M.M
     */
    cli_token_t *mask_p = cli_token_add_token(
                    ctx_p,
                    addr_p,
                    "<netmask>",
                    "Subnet mask",
                    CLI_TOKEN_TYPE_VALUE_IP4ADDR,
                    NULL);

    /*
     * Router(config-if)# ip address A.B.C.D M.M.M.M secondary
     */
    cli_token_t *secondary_p = cli_token_add_token(
            ctx_p,
            mask_p,
            "secondary",
            "Make this a secondary address",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);
}

static void cisco_ios_build_cli_if_ipv4 (cli_context_t *ctx_p, cli_prompt_t *if_prompt_p)
{
    /*************************************************************************
     * Router(config-if)# ip
     *************************************************************************/

    cli_token_t *ip_p = cli_prompt_add_token(
                    ctx_p,
                    if_prompt_p,
                    "ip",
                    "Interface Internet Protocol configuration commands",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cisco_ios_build_cli_if_ipv4_address(ctx_p, ip_p);
}

static void cisco_ios_build_cli_if_ipv6_address (cli_context_t *ctx_p, cli_token_t *ipv6_p)
{
    cli_token_t *ipv6_addr_p = cli_token_add_token(
                        ctx_p,
                        ipv6_p,
                        "address",
                        "Configure IPv6 address",
                        CLI_TOKEN_TYPE_KEYWORD,
                        NULL);

    cli_token_t *addr_p = cli_token_add_token(
                ctx_p,
                ipv6_addr_p,
                "<ipv6-address/prefix-length>",
                "IPv6 address and prefix length",
                CLI_TOKEN_TYPE_VALUE_IP6ADDR_PLEN,
                NULL);

    cli_token_add_token(
        ctx_p,
        addr_p,
        "eui-64",
        "Append interface identifier using EUI-64",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        addr_p,
        "link-local",
        "Link-local address",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        addr_p,
        "anycast",
        "Anycast address",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        ipv6_addr_p,
        "autoconfig",
        "Configure address using SLAAC",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        ipv6_addr_p,
        "dhcp",
        "Configure address using DHCPv6",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_t *prefix_p = cli_token_add_token(
                ctx_p,
                ipv6_addr_p,
                "prefix",
                "Use delegated prefix",
                CLI_TOKEN_TYPE_KEYWORD,
                NULL);

    cli_token_t *prefix_name_p = cli_token_add_token(
                    ctx_p,
                    prefix_p,
                    "<prefix-name>",
                    "Delegated prefix pool name",
                    CLI_TOKEN_TYPE_VALUE_STRING,
                    NULL);

    cli_token_add_token(
        ctx_p,
        prefix_name_p,
        "eui-64",
        "Generate interface identifier using EUI-64",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        prefix_name_p,
        "no-autoconfig",
        "Do not advertise for autoconfiguration",
        CLI_TOKEN_TYPE_KEYWORD,
        NULL);

    cli_token_add_token(
        ctx_p,
        prefix_name_p,
        "<interface-id>",
        "IPv6 interface identifier",
        CLI_TOKEN_TYPE_VALUE_STRING,
        NULL);
}

static void cisco_ios_build_cli_if_ipv6_dhcp(cli_context_t *ctx_p, cli_token_t *ipv6_p)
{
    cli_token_t *dhcp_if_p;
    cli_token_t *server_p;

    dhcp_if_p = cli_token_add_token(
                    ctx_p,
                    ipv6_p,
                    "dhcp",
                    "DHCPv6 configuration",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    server_p = cli_token_add_token(
                    ctx_p,
                    dhcp_if_p,
                    "server",
                    "Attach DHCPv6 server pool",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cli_token_add_token(
            ctx_p,
            server_p,
            "<pool-name>",
            "DHCPv6 pool name",
            CLI_TOKEN_TYPE_VALUE_STRING,
            NULL);
}

static void cisco_ios_build_cli_if_ipv6 (cli_context_t *ctx_p, cli_prompt_t *if_prompt_p)
{
    /*************************************************************************
     * Router(config-if)# ipv6
     *************************************************************************/

    cli_token_t *ipv6_p = cli_prompt_add_token(
                    ctx_p,
                    if_prompt_p,
                    "ipv6",
                    "IPv6 configuration",
                    CLI_TOKEN_TYPE_KEYWORD,
                    NULL);

    cisco_ios_build_cli_if_ipv6_address(ctx_p, ipv6_p);
    cisco_ios_build_cli_if_ipv6_dhcp(ctx_p, ipv6_p);
}

void cisco_ios_build_cli_interface (cli_context_t *ctx_p, cli_prompt_t *config_prompt_p)
{
    /*
     * Router(config)# interface
     */
    cli_token_t *interface_p =
        cli_prompt_add_token(
            ctx_p,
            config_prompt_p,
            "interface",
            "Select an interface to configure",
            CLI_TOKEN_TYPE_KEYWORD,
            NULL);

    /*
     * Router(config)# interface ethernet
     */
    cli_token_t *ethernet_p =
        cli_token_add_token(
            ctx_p,
            interface_p,
            "ethernet",
            "Ethernet interface",
            CLI_TOKEN_TYPE_KEYWORD,
                        NULL);

    /*
     * Router(config)# interface ethernet <0-255>
     */
    cli_token_t *if_num_p =
        cli_token_add_token(
            ctx_p,
            ethernet_p,
            "<0-255>",
            "Ethernet interface number",
                        CLI_TOKEN_TYPE_VALUE_OTHER,
                        interface_validator);

    /*
     * Router(config-if)#
     */
    cli_prompt_t *if_prompt_p =
        cli_token_set_prompt(
            ctx_p,
            if_num_p,
            "Router(config-if)#");

    cisco_ios_build_cli_if_ipv4(ctx_p, if_prompt_p);
    cisco_ios_build_cli_if_ipv6(ctx_p, if_prompt_p);

}