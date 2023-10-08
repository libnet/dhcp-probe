dhcp-probe
==========

dhcp-probe is intended to be used by network administrators to locate
unknown BootP and DHCP servers on a directly-attached network.

dhcp-probe is intended for use by a network administrator.  Before
running the program on any network other than one for which you are
responsible, contact that network's administrator to learn if it is
acceptable for you to run this software on that network.  Running this
software on a network where you don't have permission to do so may
violate that network's acceptable use policy.

For help, see the [dhcp_probe(8)][] and [dhcp_probe.cf(5)][] man pages.


Build & Install
---------------

The program depends on libpcap and libnet (>1.3), patched earlier
versions of libnet, compatible with dhcp-probe, are available in
most Linux distributions.  On Debian/Ubuntu derived systems:

    sudo apt install libpcap-dev libnet1-dev

The dhcp-probe configure script and Makefile supports de facto standard
settings and environment variables such as `--prefix=PATH` and `DESTDIR=`
for the install process.  For example, to install dhcp-probe to `/usr`,
instead of the default `/usr/local`, and redirect install to a package
directory in `/tmp`:

    ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var
    make
    make DESTDIR=/tmp/dhcp-probe-1.4.0 install-strip


Building from GIT
-----------------

If you want to contribute, or simply just try out the latest but
unreleased features, then you need to know a few things about the
[GNU build system][buildsystem]:

- `configure.ac` and a per-directory `Makefile.am` are key files
- `configure` and `Makefile.in` are generated from `autogen.sh`
- `Makefile` is generated by `configure` script

To build from GIT you first need to clone the repository and run the
`autogen.sh` script.  This requires `automake` and `autoconf` to be
installed on your system (not required when you build from released
tarballs):

    git clone https://github.com/libnet/dhcp-probe.git
    cd dhcp-probe/
    ./autogen.sh
    ./configure && make

GIT sources are a moving target and are not recommended for production
systems, unless you know what you are doing!


Contributing
------------

The basic functionality has been tested thoroughly over the years, but
that does not mean the program is bug free.  Please report bugs, feature
requests, patches and pull requests at [GitHub][repo].


Origin & References
-------------------

This software was created by the Network Systems Group at Princeton
University's Office of Information Technology, <networking at princeton
dot edu>.  With Irwin Tillman as the principal author.  The project is
now maintained by the [Libnet project](https://github.com/libnet/).

`dhcp_probe` was intended for our use at Princeton University, so was
written to operate on the platform(s) we use, with the features we need.
As others have asked for the software, we've made it generally available, 
however, we do not have the resources available to port it to additional 
platforms or add features.

The product is free, however, it includes code that was published in
other programs; those parts are subject to the copyright and license
restrictions listed in these files:

 - `src/report.c`
 - `src/daemonize.c`
 - `src/get_myeaddr.c`
 - `src/get_myipaddr.c`

This software is provided "as is" and any express or implied warranties,
including, but not limited to, the implied warranties of merchantability
and fitness for a particular purpose are disclaimed.  In no event shall
the authors or Princeton University be liable for any direct, indirect,
incidental, special exemplary, or consequential damages (including, but
not limited to, procurement of substitute goods or services; loss of
use, data, or profits; or business interruption) however caused and on
any theory of liability, whether in contract, strict liability, or tort
(including negligence or otherwise) arising in any way out of the use of
this software, even if advised of the possibility of such damage.

[repo]:             https://github.com/libnet/dhcp-probe/
[buildsystem]:      https://airs.com/ian/configure/
[dhcp_probe(8)]:    https://manpages.debian.org/unstable/dhcp-probe/dhcp_probe.8.en.html
[dhcp_probe.cf(5)]: https://manpages.debian.org/unstable/dhcp-probe/dhcp_probe.cf.5.en.html