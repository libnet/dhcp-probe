#!/bin/sh

# PROVIDE: dhcp_probe
# REQUIRE: LOGIN

. /etc/rc.subr

name="dhcp_probe"
rcvar=`set_rcvar`

load_rc_config $name

: ${dhcp_probe__enable="NO"}

command=/usr/local/sbin/dhcp_probe

run_rc_command "$1"
