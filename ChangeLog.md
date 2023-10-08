ChangeLog
=========

History of user-visible changes.  See the GIT log for the details of all
changes.  Detailed changes <= v1.3.1, see `doc/ChangeLog.old`.

[v1.4.0][UNRELEASED] - Unreleased
---------------------------------

* The dhcp-project has been adopted by the libnet project at GitHub,
  <https://github.com/libnet/dhcp-probe>
* License clarification, Princeton BSD-style


[v1.3.1][] - January 18, 2021
-----------------------------

* When the interface link type is not Ethernet, we used to
  treat that as a permanent error, and exited.  We now
  treat it as a transient error, just logging it and skipping
  the rest of the current probe cycle.
 
  This was prompted by a change (bug ?) that appeared in libpcap
  sometime after version 0.9.8. The behavior is present by libpcap
  version 1.1.1.  It remains (at least) through version 1.4.0.  On
  occassion, `pcap_datalink()` returns `DLT_NULL` instead of
  `DLT_EN10MB` when pointed to an Ethernet interface.

* The new `do_not_lookup_enet_and_ip_addresses` configuration file
  statement may be used to specify that the program should not try to
  look up the network interface's Ethernet address (and IP address if
  necessary).

  This may allow the program to operate on network interfaces and platforms
  where those lookup operations would not work.  It may also permit
  operation on a network interface which has no assigned IP address.

  Specifying `do_not_lookup_enet_and_ip_addresses` requires that the
  `ether_src` and `chaddr` statements also appear in the configuration
  file.

* Previously if a response frame arrived with `ether_type ==
  ETHERTYPE_VLAN` (802.1Q tagging), we ignored the frame.  We did not
  expect to receive such frames because we expect to operate on an
  untagged network interface.  (Possibly a logical network interface for
  which the OS has already stripped the incoming 802.1Q tags.)

  Now we do not ignore the frame if its `VLAN ID == 0`.  802.1Q says
  such frames should be treated like untagged frames.  Some Ethernet
  drivers may deliver such frames even to an "untagged" logical network
  interface.  They might choose to do so, for example, when the incoming
  frame contained a non-zero 802.1p priority.  To avoid discarding the
  priority when delivering the frame to an untagged logical network
  interface, the driver might choose to retain the 802.1Q header,
  changing the VLAN ID to 0.

* Previously if a request to quit arrived while we were waiting for packets in 
  response to a probe packet, the program could postpone quitting forever
  in environments in which no DHCP servers responded to the probe packet.

* Previously the `response_wait_time` was not respected on those
  platforms where the read timeout used by pcap is not supported.  After
  sending a probe packet, we might wait less than `response_wait_time`.
  We might see only the first response packet.  Now we try to wait the
  entire `response_wait_time`.

* The messages sent by `dhcp_probe_notify2` via the `THROTTLE_PAGE_CMD`
  are slightly terser to reduce the likelihood they will be split or truncated
  by SMS providers which split/truncate messages at fewer than 160 characters.

* If you apply the new `libnet_cq_destroy()` patch to libnet (or run a
  version newer than libnet 1.2rc3), this fixes a crash when we handle
  `SIGHUP`.


[v1.3.0][] - March 9, 2009
--------------------------

* The "received unexpected response ..." message passed to syslog now
  always ends with a period.  If you parse this message, you may need to
  update your code.

* New optional `alert_program_name2` configuration file statement.  This
  works like the older `alert_program_name`, but calls an alert program
  using a different syntax, intended to be extensible.  (The existing
  `alert_program_name` called an alert program using positional
  arguments, so was not extensible without breaking existing alert
  programs.  That limited our ability to enhance `dhcp_probe` to pass
  additional information to the alert program.  The new
  `alert_program_name2` calls an alert program using options.)
  
  The older optional `alert_program_name` statement continues to be
  supported and unchanged.  If you are using that, you need not change.
  However, you may wish to upgrade to the new `alert_program_name2`; if
  you do so, you will need to replace or revise the alert program so it
  supports the new call syntax.  See "New `dhcp_probe_notify2` program"
  item below.

  You may not specify both an `alert_program`_name and an
  `alert_program_name2` in the same configuration file.

* New `dhcp_probe_notify2` program.

  This program is like the older `dhcp_probe`_notify program (which
  continues to be supported), but supports the new syntax expected by
  the new `alert_program_name2` configuration file statement.

  If you are currently using the supplied `dhcp_probe_notify` alert
  program, and you have not modified it (other than to update the
  definitions at the top), you should be able to
  switch to the new `dhcp_probe_notify2` easily; you will need to
  replace the `alert_program_name` statement in your configuration
  file with the `alert_program_name2` statement, specify
  alert program `dhcp_probe_notify2` instead of `dhcp_probe_notify2`,
  and customized the definitions at the top of `dhcp_probe_notify2`
  to match the definitions you customized in `dhcp_probe_notify`.

  `dhcp_probe`_notify2 also accepts a new option so it can report a 
  'yiaddr' value when the new "Lease Networks of Concern" feature is 
   triggered.

* New "Lease Networks of Concern" feature.  

  This is intended to add more text to the messages logged (and via 
  alerts) when a rogue DHCP server is distributing IP addresses that 
  fall into network ranges of special concern -- for example, *your* 
  networks' IP ranges. 

  Most rogue DHCP servers distribute IP addresses associated with
  a private network, or NAK legitimate clients.  Rogue DHCP servers
  that distribute your own network's addresses may be of special
  concern.

  To activate this new feature, add `lease_network_of_concern`
  statements to the configuration file to specify network ranges.

  When `dhcp_probe` detects a response from a rogue DHCP server, if the
  response's yiaddr field is non-zero and falls within any of the "Lease
  Networks of Concern", that fact will be reported.  Specifically, the
  "received unexpected response ..." message sent to syslog will be
  extended to add additional text reporting the value of the yiaddr
  field.  And if you have specified an `alert_program_name2` in the
  configuration file, that alert program will be called with the new `-y
  yiaddr` option.  (If you are still using the older
  `alert_program_name`, the alerts will not contain this additional
  information, as the old `alert_program_name` doesn't support this
  change.)

* New "`legal_server_ethersrc`" test.

  The program traditionally compares the IP source address of response packets
  to the values specified by `legal_server` statements in the configuration
  file.  Any response with an IP source address that doesn't appear as a
  legal_server is treated as a rogue server.

  The new `legal_server_ethersrc` configuration statement allows you to
  also check the Ethernet source address of response packets.  If you do
  not specify any legal_`server_ethersrc` statements, the program
  continues to behave as it has in the past.  If you specify one or more
  `legal_server_ethersrc` statements, the program will check the
  Ethernet source address of response packets to verify they appear
  among those listed.  Any response from an unlisted Ethernet source
  address is treated as a rogue server.  This test is done in addition
  to the `legal_server` test.  So if you specify by `legal_server` and
  `legal_server_ethersrc` statements, a response's IP source and
  Ethernet source addresses are both checked; if either is missing from
  those you specified, the response is treated as a rogue.

  This new test is considered experimental in version 1.3.0,
  as it has received only limited testing.


[v1.2.2][] - October 15, 2008
-----------------------------

* The default name of the pid file has changed from `/etc/dhcp_probe.pid`
  to `/var/run/dhcp_probe.pid`.  You can still override the default by
  specifying the `-p pidfile` command line option.

* New `-T` option to enable the 'socket receive timeout' feature.
  This may work around a problem on some platforms where the program
  waits forever for responses after sending a probe packet.
  This new feature does not work on all platforms.

* When exiting, the program will more consistently log an "exiting" 
  message.


[v1.2.1][] - February 28, 2008
------------------------------

* No user-visible changes.


[v1.2.0][] - March 14, 2007
---------------------------

* Upgraded autoconf and automake, may build on newer systems.

* Allow you to add 802.1Q VLAN ID to outgoing frames.


[v1.1.0][] - November 10, 2004
------------------------------

* Upgraded from the libnet 1.0.x API to the libnet 1.1.x API.  We no
  longer support the libnet 1.0.x API.  To compile this version, you
  must first upgrade libnet to 1.1.2.1.

  Additionally, you must modify libnet to add a `libnet_cq_end_loop()`
  function, as described in our INSTALL document.


[v1.0.7][] - November 2, 2004
-----------------------------

* No user-visible changes.


[v1.0.6][] - August 24, 2004
----------------------------

* Upgraded autoconf and automake, may build on newer systems.


[v1.0.5][] - August 24, 2004
----------------------------

* We make some effort to ignore BootP/DHCP responses we happen to capture 
  that aren't actually in response to our probe (we check htype, hlen, 
  and chaddr fields).


[v1.0.4][] - May 6, 2002
------------------------

* We should be able to compile under gcc 3.0.x now.


v1.0.3 - February 15, 2001
--------------------------

* No user-visible changes.


v1.0.2 - January 27, 2001
-------------------------

* Cleanup of some of the configuration code.  You may now specify
  where libnet and libpcap libraries and includes are located.


v1.0.1 - August 17, 2000
------------------------

Release curated by by Irwin Tillman.

* Added alert_program_name feature, so you may specify an external
  program to run each time a response is received from an unexpected server.


v1.0.0 - June 27, 2000
----------------------

* Initial version.

[UNRELEASED]: https://github.com/libnet/dhcp-probe/compare/v1.3.1...HEAD
[v1.0.7]:     https://github.com/libnet/dhcp-probe/compare/v1.0.6...v1.0.7
[v1.0.6]:     https://github.com/libnet/dhcp-probe/compare/v1.0.5...v1.0.6
[v1.0.5]:     https://github.com/libnet/dhcp-probe/compare/v1.0.4...v1.0.5
[v1.0.4]:     https://github.com/libnet/dhcp-probe/compare/v1.0.3...v1.0.4
