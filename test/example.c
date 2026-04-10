
#include "cli.h"
#include <stddef.h>


int main (int argc, char *argv[])
{
    cli_context_t  *ctx_p = cli_context_new ("/tmp/example_cfg.json", false, 0, false, 0);
    {
        cli_prompt_t  *root_prompt_p = cli_context_set_root_prompt(ctx_p, "router>");
        {
            cli_node_t  *en_nd_p = cli_prompt_add_node(ctx_p, root_prompt_p, "en", "Enable", CLI_NODE_TYPE_KEYWORD, NULL);
            {
                cli_prompt_t  *sh_prompt_p = cli_node_add_prompt(ctx_p, en_nd_p, "router#");
                {
                    cli_node_t  *cfg_nd_p = cli_prompt_add_node(ctx_p, sh_prompt_p, "configure", "Configure this router", CLI_NODE_TYPE_KEYWORD, NULL);
                    {
                        cli_prompt_t  *cfg_prompt_p = cli_node_add_prompt(ctx_p, cfg_nd_p, "router(config)#");
                        {
                            cli_node_t  *if_nd_p = cli_prompt_add_node(ctx_p, cfg_prompt_p, "interface", "Configure interface", CLI_NODE_TYPE_KEYWORD, NULL);
                            {
                                cli_node_t  *if_eth_nd_p = cli_node_add_node(ctx_p, if_nd_p, "ethernet", "Configure ethernet interface", CLI_NODE_TYPE_KEYWORD, NULL);
                                {
                                    cli_node_t  *if_n_nd_p = cli_node_add_node(ctx_p, if_eth_nd_p, "<0-5>", "Ethernet interface number", CLI_NODE_TYPE_VALUE, NULL);
                                    {
                                        cli_prompt_t  *cfg_if_prompt = cli_node_add_prompt(ctx_p, if_n_nd_p, "router(config-if)#");
                                        {
                                            cli_node_t  *ipv6_nd_p = cli_prompt_add_node(ctx_p, cfg_if_prompt, "ipv6", "Configure IPv6 parameters", CLI_NODE_TYPE_KEYWORD, NULL);
                                            {
                                                cli_node_t  *ipv6_addr_nd_p = cli_node_add_node(ctx_p, ipv6_nd_p, "address", "Configure IPv6 address", CLI_NODE_TYPE_KEYWORD, NULL);
                                                {
                                                    cli_node_t  *ipv6_addr_cfg_meth_nd_p;

                                                    ipv6_addr_cfg_meth_nd_p = cli_node_add_node(ctx_p, ipv6_addr_nd_p, "auto", "Enable SLAAC", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    {
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "advertise", "Include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "no-advertise", "Don't include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    }
                                                    ipv6_addr_cfg_meth_nd_p = cli_node_add_node(ctx_p, ipv6_addr_nd_p, "dhcp", "Obtain address from DHCPv6 server", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    {
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "force", "Force mode configuration of address using DHCPv6", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "advertise", "Include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "no-advertise", "Don't include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    }
                                                    ipv6_addr_cfg_meth_nd_p = cli_node_add_node(ctx_p, ipv6_addr_nd_p, "delegated", "Obtain address from prefixes delegated via an upstream interface", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    {
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "advertise", "Include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "no-advertise", "Don't include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    }
                                                    ipv6_addr_cfg_meth_nd_p = cli_node_add_node(ctx_p, ipv6_addr_nd_p, "<prefix/prefix-length>", "IPv6 address/prefix-length. Eg: 2001:db8:f101::1/64", CLI_NODE_TYPE_VALUE, NULL);
                                                    {
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "advertise", "Include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        cli_node_add_node(ctx_p, ipv6_addr_cfg_meth_nd_p, "no-advertise", "Don't include this address in router advertisements", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    }
                                                }

                                                cli_node_t  *ipv6_dhcp_nd_p = cli_node_add_node(ctx_p, ipv6_nd_p, "dhcp", "Configure DHCP parameters", CLI_NODE_TYPE_KEYWORD, NULL);
                                                {
                                                    cli_node_t *ipv6_dhcli_nd_p = cli_node_add_node(ctx_p, ipv6_dhcp_nd_p, "client", "Configure DHCP client parameters", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    {
                                                        cli_node_t  *ipv6_pdlen_nd_p;

                                                        cli_node_add_node(ctx_p, ipv6_dhcli_nd_p, "request-prefix-delegation", "Request prefix delegation", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        ipv6_pdlen_nd_p = cli_node_add_node(ctx_p, ipv6_dhcli_nd_p, "delegated-prefix-length", "Delegated Prefix length to request in DHCPv6 PD", CLI_NODE_TYPE_KEYWORD, NULL);
                                                        {
                                                            cli_node_add_node(ctx_p, ipv6_pdlen_nd_p, "<1-64>", "Prefix length", CLI_NODE_TYPE_VALUE, NULL);
                                                        }
                                                        cli_node_add_node(ctx_p, ipv6_dhcli_nd_p, "request-other-info", "Request other information such as Name Servers, NTP Servers", CLI_NODE_TYPE_KEYWORD, NULL);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    cli_run(ctx_p);
    
    return 0;
}

