====================
 ntobjx: NT objects
====================

About
-----
This utility is meant to become an open source clone and eventually to surpass
the functionality of WinObj_. I have dabbled with the NT object manager
namespace before and there are some other corners of Windows systems which are
usually hard to explore using default Win32-APIs.

This utility aims to shed some light into these dark corners of Windows systems.

Download
--------
A code-signed version of the utility can be found in the `download section`_.
Please be sure to verify the signature using either ``signtool`` or ``sigcheck``
from live.sysinternals.com_ (both from Microsoft).

State of affairs and compatibility
----------------------------------
The utility already duplicates the functionality of WinObj_ and surpasses it.
It allows you to export a text or XML representation (both using Unicode) of
the object manager namespace.

The utility should work on Windows 2000 and newer. Windows NT 4.0 and older are
explicitly **unsupported** (although the source can probably be adapted to run
on those).

Build your own
--------------
I still prefer Visual Studio 2005 and therefore the projects and solution which
I committed to the repository are of that format. However, the ``premake4.lua``
should allow you to generate projects and solution for a newer Visual Studio at
will. Potential caveat: you may need to use my own `premake4 flavor`_, as I have
not tested the ``premake4.lua`` against the stock version. However, you can find
pointers to that end at the top of ``premake4.lua``.

I noticed that newer versions of the Windows SDK contain a more complete
``winternl.h`` file, such that some of the ``typedef`` declarations in
``ntnative.h`` may be duplicates and the compiler may choke. If you find any
such cases, please file a ticket to point them out to me.

Creating the solutions
~~~~~~~~~~~~~~~~~~~~~~
Using the correct `premake4 flavor`_ the creation of the solutions and projects
is as easy as::

    premake4 vs2005

For example to create all solutions and projects for VS 2005 through VS 2015 in
parallel::

    for %i in (vs2005 vs2008 vs2010 vs2012 vs2013 vs2015) do @premake4 %i

And don't worry, the names of the solutions and projects contain the version of
Visual Studio and therefore won't clash.

Personally I currently use VS 2005 to _build_ release versions, but VS 2015 to
develop the utility.

License
-------
The GUI utility itself is released under Microsoft Public License (MS-PL_), the
license under which the Windows Template Library (WTL_) comes. That is owed to
the fact that I am not a lawyer and therefore cannot judge how compatible that
license is with other licenses. For example the MS-PL is *not* compatible with
the GPL for all I know. Meaning that linking the WTL into a program under GPL
would not be possible.

However, I am dual-licensing all of my source files under the MIT license *and*
MS-PL. The resulting binary is released under the terms of the MS-PL, but the
individual source files are released under both licenses. The WTL source files
remain licensed the way they are offered on the official website (i.e. MS-PL).

This is meant to allow my code to be reused under a permissive license, while
at the same time complying with the requirements of the MS-PL.

The texts for both licenses can be found in ``LICENSE.txt`` and ``wtl/MS-PL.txt``
respectively.

.. _download section: https://bitbucket.org/assarbad/ntobjx/downloads
.. _live.sysinternals.com: https://live.sysinternals.com/sigcheck.exe
.. _premake4 flavor: https://bitbucket.org/windirstat/premake-stable
.. _WinObj: https://technet.microsoft.com/en-us/sysinternals/winobj.aspx
.. _MS-PL: https://opensource.org/licenses/MS-PL
.. _WTL: https://sourceforge.net/projects/wtl/
