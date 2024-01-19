/*
 *	HITEST.C - Test Routines for HIMEM.SYS
 *
 *	Copyright (c) 1988, Microsoft Corporation
 */

#include	"xmm.h"

int	quiet = 0;

#define	MAXHANDLES	(32*2)
#define	BUFLEN		1024

struct	HandleInfo {
	short		Handle;
	short		Length;
	short		Filler;
	unsigned long	RealAddress;
};

struct HandleInfo	HandleTable[MAXHANDLES];

struct XMM_Move MoveS;

short	BufferLow[BUFLEN];
short	BufferExt[BUFLEN];
short	BufferHi[BUFLEN];

main(argc, argv)
char *argv[];
{
	long		XMM_Ret;
	int		Request_Amount;
	int		Handle, hi;
	long		Alloc_Ret;
	short		TotalAllocated = 0;
	short		Handles;
	short		MemPerHandle;

	if ( argc > 1 && argv[1][0] == 'q' )
		quiet = 1;

	if ( XMM_Installed() ) {
		printf("Extended Memory Manager Found\n\n");
		XMM_Ret = XMM_Version();
		printf("XMM Version %4X, Internal Version %4x\n\n",
			(short)XMM_Ret, (short)(XMM_Ret >> 16));
	} else {
		printf("No Extended Memory Manager Present\n\n");
		Exit_Test();
	}
	Request_Amount = 0;
	do { 
		XMM_Ret = XMM_RequestHMA(Request_Amount);
		if ( XMM_Ret == 0 )
			break;
		Request_Amount++;
	} while ( Request_Amount );
	if ( XMM_Ret == 0 ) {
		printf("Got HMA with Request of %d Kb\n", Request_Amount);
		if ( XMM_RequestHMA(Request_Amount) == 0 ) {
			printf("Unexpected success requesting HMA again\n");
		}
		XMM_Ret = XMM_ReleaseHMA();
		if ( XMM_Ret )
			Error("Could not release HMA", XMM_Ret);
		else
			printq("Released HMA\n");
	} else {
		Error("Could not Get HMA", XMM_Ret);
	}
	EnableA20();
	EnableA20();
	DisableA20();
	EnableA20();
	DisableA20();
	EnableA20();
	DisableA20();
	DisableA20();

	/* Soak up all extended memory and handles */
	hi = 0;
	while ( hi < MAXHANDLES ) {
		long	FreeExtended;
		FreeExtended = XMM_QueryLargestFree();
		if ( FreeExtended < 0 ) {
			if ( XMSERROR(FreeExtended) == (char)0xA0 )
				FreeExtended = 0L;
			else {
				Error("Query Free Extended Failed", FreeExtended);
				break;
			}
		}
		if ( hi < MAXHANDLES -1  && FreeExtended > 1 )
			FreeExtended /= 2;
		if ( !quiet )
			printf("Attempt to Allocate %d Kb\n", FreeExtended);
		Alloc_Ret = XMM_AllocateExtended(FreeExtended);
		if ( Alloc_Ret < 0 ) {
			if ( XMSERROR(Alloc_Ret) == (char)0xA1 )
				break;
			Error("Allocate Failed", Alloc_Ret);
		} else {
			TotalAllocated += FreeExtended;
			HandleTable[hi].Handle = (short)Alloc_Ret;
		}
		hi++;
	}
	Handles = hi;
	printf("%d Kb Free Found and Allocated \n", TotalAllocated);
	printf("%d handle(s) allocated\n", Handles);
	printq("Freeing first handle\n");
	XMM_Ret = XMM_FreeExtended(HandleTable[0].Handle);
	if ( XMM_Ret )
		Error("Free Extended Memory Block", XMM_Ret);
	printq("Reallocate first handle with 0 length\n");
	Alloc_Ret = XMM_AllocateExtended(0);
	if ( Alloc_Ret < 0 )
		Error("Zero Length Allocate failed", Alloc_Ret);
	else
		HandleTable[0].Handle = (short)Alloc_Ret;
	hi = 0;
	while ( hi < Handles ) {
		if ( HandleTable[hi].Handle == 0 )
			break;
		if ( !quiet )
			printf("Freeing Handle #%d (%x)\n", hi,
				HandleTable[hi].Handle);
		XMM_Ret = XMM_FreeExtended(HandleTable[hi].Handle);
		if ( XMM_Ret )
			Error("Free Extended Memory Block", XMM_Ret);
		hi++;
	}

	if ( Handles == 0 )
		Exit_Test();
	MemPerHandle = TotalAllocated / Handles + 1;
	if ( (long)BUFLEN < MemPerHandle * 512L )
		MemPerHandle = BUFLEN/512;
	if ( !quiet )
		printf("Allocating in %d Kb chunks\n", MemPerHandle);
	hi = 0;
	while ( hi < Handles ) {
		XMM_Ret = XMM_AllocateExtended(MemPerHandle);
		if ( XMM_Ret < 0 ) {
			if ( XMSERROR(XMM_Ret) != 0xA0 )
				Error("Allocate", XMM_Ret);
			break;
		}
		HandleTable[hi].Handle = (short)XMM_Ret;
		HandleTable[hi].Length = MemPerHandle;
		XMM_Ret = XMM_LockExtended((short)XMM_Ret);
		if ( XMM_Ret < 0 ) {
			printf("Handle %4x: ", HandleTable[hi].Handle);
			Error("Lock Failed", XMM_Ret);
		}
		if ( !quiet )
			printf("Handle %4x: Real Address %lx\n", HandleTable[hi].Handle,XMM_Ret);
		HandleTable[hi].RealAddress = XMM_Ret;
		XMM_Ret = XMM_FreeExtended(HandleTable[hi].Handle);
		if ( XMM_Ret >= 0 ) {
			printf("Unexpected success freeing locked Handle\n");
			break;
		} else if ( XMSERROR(XMM_Ret) != (char)0xAB ) {
			Error("Freeing Locked Handle", XMM_Ret);
		}
		Fill(BufferExt, HandleTable[hi].Handle ^ 0xAAAA, BUFLEN);
		MoveS.Length = MemPerHandle*1024L;
		MoveS.SourceHandle = 0;
		(char far *)MoveS.SourceOffset = (char far *)BufferExt;
		MoveS.DestHandle = HandleTable[hi].Handle;
		MoveS.DestOffset = 0L;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMM_Ret < 0 )
			Error("Move to Extended Memory Failed", XMM_Ret);
		hi++;
	}
	Handles = hi;		/* How many we got this time */

	if ( hi == 0 )
		Exit_Test();

	while ( --hi >= 0 ) {
		if ( !quiet )
			printf("Checking Handle %x\n", HandleTable[hi].Handle);
		Fill(BufferLow, HandleTable[hi].Handle ^ 0xAAAA, BUFLEN);
		MoveS.Length = MemPerHandle*1024L;
		MoveS.SourceHandle = HandleTable[hi].Handle;
		MoveS.SourceOffset = 0L;
		MoveS.DestHandle = 0;
		(char far *)MoveS.DestOffset = (char far *)BufferHi;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMM_Ret < 0 )
			Error("Move from Extended Memory Failed", XMM_Ret);
		if ( Cmp(BufferLow, BufferHi, BUFLEN) )
			printf("Comparison Failed, Handle %x\n",
				HandleTable[hi].Handle);
		XMM_Ret = XMM_UnlockExtended(HandleTable[hi].Handle);
		if ( XMM_Ret < 0 )
			Error("Unlock of Extended Memory Failed", XMM_Ret);
	}

	hi = 0;
	while ( hi < Handles ) {
		Fill(BufferLow, HandleTable[hi].Handle ^ 0xAAAA, BUFLEN);
		MoveS.Length = MemPerHandle*1024L;
		MoveS.SourceHandle = HandleTable[hi].Handle;
		MoveS.SourceOffset = 0L;
		MoveS.DestHandle = 0;
		(char far *)MoveS.DestOffset = (char far *)BufferHi;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMM_Ret < 0 )
			Error("Move from Unlocked Extended Memory Failed", XMM_Ret);
		if ( Cmp(BufferLow, BufferHi, MemPerHandle*512) )
			printf("Comparison Failed, Unlocked Handle %x\n",
				HandleTable[hi].Handle);

			/* Check weird cases */
		MoveS.Length = 1;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMSERROR(XMM_Ret) != (char)0xA7 )
			Error("Move Extended with Odd Length", XMM_Ret);

		MoveS.Length = MemPerHandle*1024L + 2;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMSERROR(XMM_Ret) != (char)0xA7 )
			Error("Move Extended with Length too long", XMM_Ret);

		MoveS.Length = MemPerHandle*1024L -2;
		MoveS.SourceOffset = 4L;
		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMM_Ret >= 0 )
			Error("Move Extended Base+Length too large", XMM_Ret);

		XMM_Ret = XMM_FreeExtended(HandleTable[hi].Handle);
		if ( XMM_Ret < 0 )
			Error("Free of Extended Memory Failed", XMM_Ret);

		XMM_Ret = XMM_MoveExtended(&MoveS);
		if ( XMSERROR(XMM_Ret) != (char)0xA3 )
			Error("Move Extended from Invalid Handle", XMM_Ret);
		hi++;
	}

	TestWrap();

	Exit_Test();
}

TestWrap()
{
	long		XMM_Ret;

	printq("Try Moves from Wrap Area\n");
	MoveS.Length = BUFLEN*2;
	MoveS.SourceHandle = 0;
	MoveS.SourceOffset = 0xFFFFF800L;
	XMM_Ret = XMM_MoveExtended(&MoveS);
	if ( XMM_Ret < 0 )
		Error("Move Extended at end of Conventional", XMM_Ret);

	MoveS.Length += 2;
	XMM_Ret = XMM_MoveExtended(&MoveS);
	if ( XMSERROR(XMM_Ret) != (char)0xA7 )
		Error("Move Extended overflowing end of Conventional", XMM_Ret);
}

EnableA20()
{
	long	XMM_Ret;

	printq("Enable A20: ");
	XMM_Ret = XMM_EnableA20();
	if ( XMM_Ret < 0 )
		Error("A20 not enabled", XMM_Ret);
	QueryA20();
}

DisableA20()
{
	long	XMM_Ret;

	printq("Disable A20: ");
	XMM_Ret = XMM_DisableA20();
	if ( XMM_Ret < 0 )
		Error("A20 not disabled", XMM_Ret);
	QueryA20();
}

QueryA20()
{
	long	XMM_Ret;

	printq("Query A20: ");
	XMM_Ret = XMM_QueryA20();
	if ( XMM_Ret == 1 )
		printq("A20 Enabled\n");
	else if ( XMM_Ret == 0 )
		printq("A20 Disabled\n");
	else
		Error("Failed", XMM_Ret);
}

Fill(ptr, pattern, count)
int *ptr;
{
	while ( count-- )
		*ptr++ = pattern++;
}

Cmp(ptr1, ptr2, count)
int *ptr1, *ptr2;
{
	while ( count-- )
		if ( *ptr1++ != *ptr2++ )
			return(1);

	return(0);
}

printq(arg)
{
	if ( !quiet )
		printf(arg);
}

Error(string, code)
char	*string;
long 	code;
{
	if ( code < 0 ) {
		code = XMSERROR(code);
		printf("%s, Error Code %2x\n", string, (unsigned)code);
	} else if ( !quiet )
		printf("%s\n", string);
}

Exit_Test()
{
	printf("\nXMM Test Completed\n");
	exit(0);
}
