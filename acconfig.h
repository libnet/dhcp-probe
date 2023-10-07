/* acconfig.h

   Descriptive text for the C preprocessor macros that
   the locally-defined Autoconf macros can define.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */



/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */

@BOTTOM@

/* Define the following if ether_aton() is declared */
#undef  HAVE_ETHER_ATON_PROTO /* <netinet/ether.h> */

/* Define the following if ether_ntoa() is declared */
#undef  HAVE_ETHER_NTOA_PROTO /* <netinet/ether.h> */

/* Define the following if the inet_aton() function prototype is in <arpa/inet.h> */
#undef  HAVE_INET_ATON_PROTO    /* <arpa/inet.h> */

/* Define if struct sockaddr{} has sa_len member */
#undef  STRUCT_SOCKADDR_HAS_SA_LEN

