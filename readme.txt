XMS Distribution Diskette
=========================

This diskette contains the following files:

\HIMEM.SYS	    - The latest version of Microsoft's XMS Driver version
			2.06.

SPEC directory:

\SPEC\XMS.TXT	    - A machine-readable version of the XMS Spec
\SPEC\HIMEM.TXT     - User documentation for the HIMEM.SYS driver

CLIB directory :

\CLIB\XMM.LIB	    - A library of functions which allow C programs to
			 access XMS functions
\CLIB\XMMLIB.DOC    - Documentation for the C library
\CLIB\XMM.H	    - A C header file for use with the C library functions
\CLIB\XMM.ASM	    - Source listing for XMM.LIB
\CLIB\CMACROS.INC   - Included by XMM.ASM


OEMSRC Directory :

\OEMSRC\HIMEM.ASM   - Source listing for HIMEM version 2.06
\OEMSRC\XM286.ASM   - Included by HIMEM.ASM
\OEMSRC\XM386.ASM   - Included by HIMEM.ASM
\OEMSRC\HIMEM	    - The MAKE file for HIMEM

TESTS directory :

Make sure that HIMEM.SYS is loaded before running these test programs.

\TESTS\HITEST.COM   - A simple XMS driver testing program
\TESTS\HITEST.ASM   - Source to Hitest.asm
\TESTS\HITEST       - Make file for Hitest.com
\TESTS\TEST.EXE     - A more advanced XMS test program
\TESTS\TEST.C       - Source to test.exe
\TESTS\TEST         - Make file for test.exe
\TESTS\XMSTIME.EXE  - An ExtBlockMove Timing test program

Make sure that HIMEM.SYS is loaded before running these test programs.
To make these files, you need to use Microsoft's MASM 5.1, as well
as C 5.1 and Microsoft's MAKE.
