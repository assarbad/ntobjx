====================
 ntobjx: NT objects
====================

About
-----
This utility is meant to be an open source replacement of WinObj_. I have
dabbled with the NT object manager namespace before and there are some other
corners of Windows systems which are usually hard to explore using default
Win32-APIs.

This utility aims to shed some light into these dark corners of Windows systems.

You can find screenshots in the documentation_.

Kurzer Hinweis: deutsche Version
--------------------------------
Dieses Programm gibt es jetzt auch auf Deutsch. Entsprechend der Einstellungen
eures Benutzerkontos wird das Programm ggf. schon direkt auf Deutsch gestartet.
Ansonsten könnt ihr F11 oder das "View"-Menü zum Umschalten benutzen. Viel Spaß!

Die deutsche `Dokumentation findet sich hier`_.

Das Fälertoifelchen
~~~~~~~~~~~~~~~~~~~

Defekte könnt ihr mir gern auch `auf Deutsch melden`_.

Call to action: translations
----------------------------
Hey there. Yes, you. Want to see this program in your native language? Feel
confident enough to translate any existing translation into your native tongue?
If so, please send a pull request with the translated ``.rc`` file or
alternately `file a ticket`_ here to get in touch.

Download
--------
A code-signed version of the utility can be found in the `download section`_.
Please be sure to verify the signature using either ``signtool`` or ``sigcheck``
from live.sysinternals.com_ (both from Microsoft).

There is also a PGP-signed ``.7z`` archive. The signature is detached and carries
the file extension ``.asc``. These archives contain the debug symbols (``.pdb``)
alongside the executables. The name contains the revision number and the short
changeset ID, so I don't need to remove old builds all the time due to name
clashes.

State of affairs and compatibility
----------------------------------
The utility already duplicates the functionality of WinObj_ and surpasses it.
It allows you to export a text or XML representation (both using Unicode) of
the object manager namespace.

The utility should work on Windows 2000 and newer. Windows NT 4.0 and older are
explicitly **unsupported** (although the source can probably be adapted to run
on those). Please note that not all builds may run with Windows versions prior
to Windows XP SP2/SP3. So while the source code can be targeted at Windows 2000,
e.g. by using the WDK build method or VS2005, the configuration or compiler may
not actually generate compatible code (e.g. VS2017 will only support down to
Windows XP, but not Windows 2000).

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

More details on `how to build`_ can be found in the project Wiki. Please also
note that there is an additional set of solutions in the ``ddkbuild`` subfolder
which allows you to use one of the supported WDKs to generate a much smaller
build of ntobjx than is possible with vanilla VS solutions.

Creating the solutions
~~~~~~~~~~~~~~~~~~~~~~
Using the correct `premake4 flavor`_ the creation of the solutions and projects
is as easy as::

    premake4 vs2005

For example to create all solutions and projects for VS 2005 through VS 2017 in
parallel::

    for %i in (vs2005 vs2008 vs2010 vs2012 vs2013 vs2015 vs2017) do @premake4 %i

And don't worry, the names of the solutions and projects contain the version of
Visual Studio and therefore won't clash.

Personally I currently use VS 2005 to *build* release versions, but VS 2017 to
develop the utility.

Command line version
~~~~~~~~~~~~~~~~~~~~
If you are merely interested in listing the objects, you can pass the parameter
``--cmdline`` to your invocation of ``premake4`` and it will generate an extra
project named ``ntobjx_c`` inside the solution. This project also is a nice
show case for just the stuff provided in ``objmgr.hpp``, so if you want to play
with that or understand it, you needn't sift through lots of GUI related code.

Don't look any further than said command line tool.

Defects
-------
Some people call them "bugs", but "bug" misses the distinction between cause
(the defect), propagation of the defective program state and symptom (the
defective behavior exhibited). I prefer the terminology as proposed by Andreas
Zeller in his book `Why Programs Fail`_ (2nd ed.). Therefore I call them defects.

Now, if you find one of those, please report them here. Feel free to use German,
English or - if absolutely necessary - Russian to describe the erratic behavior.
Please make sure to provide some way to contact you for more feedback. One easy
way is to log into Bitbucket and `file a ticket`_ as logged on user.

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

.. _file a ticket: https://bitbucket.org/assarbad/ntobjx/issues?status=new&status=open
.. _Dokumentation findet sich hier: https://bitbucket.org/assarbad/ntobjx/wiki/help/Deutsch
.. _auf Deutsch melden: https://bitbucket.org/assarbad/ntobjx/issues?status=new&status=open
.. _documentation: https://bitbucket.org/assarbad/ntobjx/wiki/help/English
.. _download section: https://bitbucket.org/assarbad/ntobjx/downloads
.. _live.sysinternals.com: https://live.sysinternals.com/sigcheck.exe
.. _premake4 flavor: https://bitbucket.org/windirstat/premake-stable
.. _WinObj: https://technet.microsoft.com/en-us/sysinternals/winobj.aspx
.. _Why Programs Fail: http://www.whyprogramsfail.com/
.. _MS-PL: https://opensource.org/licenses/MS-PL
.. _WTL: https://sourceforge.net/projects/wtl/
.. _how to build: https://bitbucket.org/assarbad/ntobjx/wiki/build/Home
