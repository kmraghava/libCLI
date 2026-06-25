/*****************************************************************************
 *
 * FILE NAME     : cli_vvalidator.c
 * MODULE        : libCLI
 * AUTHOR        : KM Raghava
 * CREATION DATE : June 11, 2026
 *
 ******************************************************************************
 *
 *  NAME        : cli_value_is_int
 *                cli_value_is_hex
 *                cli_value_is_real
 *                cli_value_is_string
 *                cli_value_is_bool
 *                cli_value_is_ip4addr
 *                cli_value_is_ip4addr_plen
 *                cli_value_is_ip6addr
 *                cli_value_is_ip6addr_plen
 *                cli_value_is_fqdn
 *                cli_value_is_macaddr
 *
 *  DESCRIPTION : Value validators
 *
 *  PARAMS      : ctx_p      - CLI context
 *                value_type - Type of the value to validate
 *                value_p    - User entered value to match
 *
 *  RETURNS     : true if the value is valid for the given type
 *                false otherwise
 *
 *****************************************************************************/

/*****************************************************************************
   Include Files
*****************************************************************************/
#include <arpa/inet.h>
#include "cli_context.h"
#include "cli_vvalidator.h"
#include <errno.h>
#include "kmrUtils/str.h"


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

/*****************************************************************************
   Local Functions
*****************************************************************************/

/*****************************************************************************
   Global Functions
*****************************************************************************/
bool cli_value_is_int (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    char  *endptr;

    errno = 0;
    strtoll(string_cstr(value_p), &endptr, 10);

    return (errno == 0 && *endptr == '\0');
}

bool cli_value_is_hex (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    char  *endptr;

    errno = 0;
    strtoll(string_cstr(value_p), &endptr, 16);
    
    return (errno == 0 && *endptr == '\0');
}

bool cli_value_is_real (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    char  *endptr;

    errno = 0;
    strtold(string_cstr(value_p), &endptr);
    
    return (errno == 0 && *endptr == '\0');
}

bool cli_value_is_string (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    return (string_length(value_p) > 0);
}

bool cli_value_is_bool (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    return (   0 == string_compare(value_p, true_string(),  -1, true)
            || 0 == string_compare(value_p, false_string(), -1, true)
           );
}

bool cli_value_is_ip4addr (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    struct in_addr  addr;

    return inet_pton(AF_INET, string_cstr(value_p), &addr) == 1;
}

bool cli_value_is_ip4addr_plen (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    string_t  **parts_pp = string_split(value_p, "/");
    if (!parts_pp)
        return false;

    bool valid_b = false;

    if (string_array_count(parts_pp) == 2)
    {
        valid_b = (   cli_value_is_ip4addr (ctx_p, value_type, parts_pp[0])
                   && cli_value_is_int     (ctx_p, value_type, parts_pp[1])
                  );

        if (valid_b)
        {
            int plen = atoi(string_cstr(parts_pp[1]));
            valid_b = (plen >= 0 && plen <= 32);
        }
    }

    string_array_del(parts_pp);

    return valid_b;
}

bool cli_value_is_ip6addr (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    struct in6_addr  addr;

    return inet_pton(AF_INET6, string_cstr(value_p), &addr) == 1;
}

bool cli_value_is_ip6addr_plen (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    string_t  **parts_pp = string_split(value_p, "/");
    if (!parts_pp)
        return false;

    bool valid_b = false;

    if (string_array_count(parts_pp) == 2)
    {
        valid_b = (   cli_value_is_ip6addr (ctx_p, value_type, parts_pp[0])
                   && cli_value_is_int     (ctx_p, value_type, parts_pp[1])
                  );

        if (valid_b)
        {
            int plen = atoi(string_cstr(parts_pp[1]));
            valid_b = (plen >= 0 && plen <= 128);
        }
    }

    string_array_del(parts_pp);

    return valid_b;
}

bool cli_value_is_fqdn (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    static const char  *allowed_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.$-_+!*'()";

    // Basic validation for FQDN (simplified)
    return (   string_contains(value_p, ".")
            && string_span(value_p, allowed_chars) == string_length(value_p)
           );
}

bool cli_value_is_macaddr (cli_context_t  *ctx_p, uint64_t  value_type, string_t  *value_p)
{
    UNUSED_PARAMETER(ctx_p);
    UNUSED_PARAMETER(value_type);

    if (string_length(value_p) == 0)
        return false;

    // Basic validation for MAC address in the format XX:XX:XX:XX:XX:XX
    string_t  **parts_pp = string_split(value_p, ":");
    if (!parts_pp)
        return false;

    bool valid_b = false;

    if (string_array_count(parts_pp) == 6)
    {
        valid_b = true;

        for (int i = 0; i < 6; i++)
        {
            if (   string_length(parts_pp[i]) == 0
                || string_length(parts_pp[i])  > 2
               )
            {
                valid_b = false;
                break;
            }

            char *endptr;
            
            errno = 0;
            strtoul(string_cstr(parts_pp[i]), &endptr, 16);
            
            if (errno != 0 || *endptr != '\0')
            {
                valid_b = false;
                break;
            }
        }
    }

    string_array_del(parts_pp);

    return valid_b;
}


/*****************************************************************************
   Test Functions
*****************************************************************************/
