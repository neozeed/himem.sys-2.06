/*
 *	XMS Driver C Interface Routine Definitions
 *
 *	Copyright (c) 1988, Microsoft Corporation
 */

unsigned	XMM_Installed();

long	XMM_Version();
long	XMM_RequestHMA();
long	XMM_ReleaseHMA();
long	XMM_GlobalEnableA20();
long	XMM_GlobalDisableA20();
long	XMM_EnableA20();
long	XMM_DisableA20();
long	XMM_QueryA20();
long	XMM_QueryLargestFree();
long	XMM_QueryTotalFree();
long	XMM_AllocateExtended();
long	XMM_FreeExtended();
long	XMM_MoveExtended();
long	XMM_LockExtended();
long	XMM_UnLockExtended();
long	XMM_GetHandleLength();
long	XMM_GetHandleInfo();
long	XMM_ReallocateExtended();
long	XMM_RequestUMB();
long	XMM_ReleaseUMB();

struct	XMM_Move {
	unsigned long	Length;
	unsigned short	SourceHandle;
	unsigned long	SourceOffset;
	unsigned short	DestHandle;
	unsigned long	DestOffset;
};

#define	XMSERROR(x)	(char)((x)>>24)
