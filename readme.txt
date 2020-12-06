This is the source code for ZDoom 1.23 beta 6.

It is based on the Linux DOOM sources that were prepared by B. Krenheimer
and generously released by John Carmack shortly before Christmas, 1997. If
you wish to obtain the original Linux source distribution, you can find it
at ftp://ftp.idsoftware.com/source/doomsrc.zip. Portions of code were also
taken from various other source ports, the majority of them coming from
the BOOM Phase I source released on 27 May 1998 (credit Team TNT and Chi
Hoang). Portions of this code are also from Heretic and Hexen and are
copyrighted by Raven Software. Many changes are, of course, my own, and I've
tried to flag them as such with [RH] comments blocks (although I missed a few
before I started the commenting convention). At this point, the source is
a far cry from what id originally released, featuring a fancy C++ class
system, object serializer, pointer tracking, and multiple game support.

Be warned that Visual C++ can take a *very* long time to link the release
build of this source. The last time I timed it, it took two and a half
minutes. The debug build links much faster.

To compile this source code, you also need to download several other
packages if you don't already have them. These are:

  FMOD
    http://www.fmod.org

    Fill out the form on their download page to get the FMOD SDK. Unless
    you want to use it for profit, it's free, and it's the sound engine
    ZDoom now uses.

  OpenPTC 1.0.18
    http://www.gaffer.org/ptc

  NASM (for the assembly files)
    http://www.web-sites.co.uk/nasm/

    If you don't want to use NASM, you can #define NOASM while compiling
    ZDoom, and it will use C code instead.

The included project file (doom.dsp) is for Visual C++ 6 and makes a few
assumptions about the development environment:

    The FMOD SDK is installed in f:/fmod.
    NASM is installed in d:/nasm.
    PTC is installed in ../openptc.

If you have things set up differently (and you probably will), be sure
to change the project settings to point to the proper directories.


Randy Heit
rheit@iastate.edu