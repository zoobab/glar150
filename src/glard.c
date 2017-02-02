/*  =========================================================================
    glard - Glar150 daemon

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Glar150 Project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    glard - Glar150 daemon
@discuss
    Runs a single instance of a Glar150 node. The node can be a robot or
    a console depending on the command line arguments.

    glard [options] ...
      --help / -h            this help
      --verbose / -v         verbose test output
      --interface / -i       use this interface
      --console / -c         remote control console
@end
*/

#include "glar150.h"

int
main (int argc, char *argv [])
{
    puts ("glard v1.0.1 -- GL-AR150 demo'n");

    //  Defaults
    bool verbose = false;
    bool console = false;
    bool ipv6 = false;
    char *iface = "wlan0";
    char *node_name = NULL;

    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("glard [options] ...");
            puts ("  --help / -h            this help");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --interface / -i       use this interface");
            puts ("  --console / -c         remote control console");
            puts ("  --name / -n MY_NAME    my node name (default: random)");
            puts ("  --ipv6 / -6            connect over IPv6");
            return 0;
        }
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--console")
        ||  streq (argv [argn], "-c"))
            console = true;
        else
        if (streq (argv [argn], "--name")
        ||  streq (argv [argn], "-n")) {
            if (argn + 1 >= argc) {
                zsys_error ("argument required for --name\n");
                return -1;
            }
            node_name = argv [++argn];
            }
        else
        if (streq (argv [argn], "--interface")
        ||  streq (argv [argn], "-i"))
            iface = argv [++argn];
        else
        if (streq (argv [argn], "--ipv6")
        ||  streq (argv [argn], "-6"))
            ipv6 = true;
        else {
            zsys_error ("unknown option: %s\n", argv [argn]);
            return -1;
        }
    }
    zsys_set_ipv6(ipv6);
    glar_node_t *node = glar_node_new (iface, console, node_name);
    glar_node_set_verbose (node, verbose);
    glar_node_execute (node);
    glar_node_destroy (&node);
    return 0;
}
