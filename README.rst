====================
 ntobjx: NT objects
====================
:Author: Oliver Schneider

About
-----
This utility is meant to become an open source clone and eventually to surpass
the functionality of WinObj_. I have dabbled with the NT object manager
namespace before and there are some more corners of Windows systems which are
usually hard to explore using default Win32-APIs.

This utility aims to shed some light into these corners of Windows systems.

Download
--------
A code-signed version of the tool can be found in the download_ section.

Build your own
--------------
I still prefer Visual Studio 2005 and therefore the projects and solution which
I committed to the repository are of that format. However, the ``premake4.lua``
should allow you to generate projects and solution for a newer Visual Studio at
will. Potential caveat: you may need to use my own ``premake4`` flavor, as I
have not tested the ``premake4.lua`` against the stock version. However, you can
find pointers to that end at the top of ``premake4.lua``.

I noticed that newer versions of the Windows SDK contain a more complete
``winternl.h`` file, such that some of the ``typedef`` declarations in
``ntnative.h`` may be duplicates and the compiler may choke. If you find any
such cases, please file a ticket to point them out to me.

License
-------
The GUI tool itself is released under Microsoft Public License (MsPL), the
license under which the Windows Template Library (WTL) comes. That is owed to
the fact that I am not a lawyer and therefore cannot judge how compatible that
license is with other licenses. For example the MsPL is *not* compatible with
the GPL for all I know. Meaning that linking the WTL into a program under GPL is
not possible.

However, I am dual-licensing all of my source files under the MIT license *and*
MsPL. The resulting binary is released under the terms of the MsPL, but the
individual source files are released under both licenses. The WTL source files
remain licensed the way they are offered on the official website (i.e. MsPL).


.. _download: https://bitbucket.org/assarbad/ntobjx/downloads
.. _WinObj: https://technet.microsoft.com/en-us/sysinternals/winobj.aspx
