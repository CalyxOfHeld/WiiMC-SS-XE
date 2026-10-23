/* wiifin.cpp: WiiFin started in WiiMC-SS XE's place.  As WiiFin starts
 * this one (its Reload): the DOL read whole first, then, the system shut
 * down and the interrupts off, moved to MEM2 (out of the way of WiiFin's
 * sections), a small routine copied there too, which puts each section
 * in place and jumps to its entry with WiiFin's path as argv[0]. */
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "wiifin.h"

char wiifinPath[256] = {0};
bool LaunchWiiFin = false;

extern "C" void wiifinTrampoline(const void* dol, const void* arg);
extern "C" char wiifinTrampolineEnd[];

/* the argument block, WiiFin's path: high in MEM2, out of reach of its
 * first allocations; the DOL and the routine below it */
static char* const ARG_AREA   = (char*)0x93300000;
static u8*   const STAGE_AREA = (u8*)0x92000000;
static const long  STAGE_MAX  = 0x1200000;        /* 18 MB: up to 0x93200000 */
static u8*   const CODE_AREA  = (u8*)0x93280000;

static u8*  dol = NULL;
static long dolSize = 0;
static char argPath[256];   /* argv[0], as WiiFin names its devices */

/* WiiMC mounts sd1:, usb1:...; WiiFin's paths say sd:, usb: */
static FILE* openDol(const char* path)
{
	FILE* f = fopen(path, "rb");
	if(f) return f;
	char p[300];
	if(strncmp(path, "sd:", 3) == 0)
	{
		snprintf(p, sizeof(p), "sd1:%s", path + 3);
		return fopen(p, "rb");
	}
	if(strncmp(path, "usb:", 4) == 0)
	{
		for(int m = 1; m < 6 && !f; m++)
		{
			snprintf(p, sizeof(p), "usb%d:%s", m, path + 4);
			f = fopen(p, "rb");
		}
	}
	return f;
}

/* the heap full: the DOL in MEM2's free part, taken off the arena as
 * WiiMC's own MEM2 areas are */
static u8* mem2Buffer(long size)
{
	u32 lo = ((u32)SYS_GetArena2Lo() + 31) & ~31;
	u32 hi = ((u32)SYS_GetArena2Hi() - (u32)size) & ~31;
	if(hi < lo || hi + size > (u32)SYS_GetArena2Hi()) return NULL;
	SYS_SetArena2Hi((void*)hi);
	return (u8*)hi;
}

static bool loadFrom(const char* path)
{
	FILE* f = openDol(path);
	if(!f) return false;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(size < 0x100 || size > STAGE_MAX) { fclose(f); return false; }
	u8* buf = (u8*)memalign(32, (size + 31) & ~31);
	bool heap = buf != NULL;
	if(!buf) buf = mem2Buffer((size + 31) & ~31);
	if(!buf) { fclose(f); return false; }
	bool read = fread(buf, 1, size, f) == (size_t)size;
	fclose(f);
	u32 entry = *(u32*)(buf + 0xE0);
	if(!read || (entry & 0xF0000000) != 0x80000000)
	{
		if(heap) free(buf);
		return false;
	}
	dol = buf;
	dolSize = size;
	snprintf(argPath, sizeof(argPath), "%s", path);
	return true;
}

bool WiiFinLoad()
{
	if(dol) return true;
	if(wiifinPath[0] && loadFrom(wiifinPath)) return true;
	return loadFrom("sd:/apps/WiiFin/boot.dol") || loadFrom("usb:/apps/WiiFin/boot.dol");
}

void WiiFinGo()
{
	if(!dol) return;
	const size_t len = (size_t)(wiifinTrampolineEnd - (char*)wiifinTrampoline);
	const size_t argLen = strlen(argPath) + 1;
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	u32 level;
	_CPU_ISR_Disable(level);
	(void)level;
	/* nothing runs any more but this: the DOL to MEM2 first (it may lie
	 * where the block and the routine go), then those */
	memmove(STAGE_AREA, dol, dolSize);
	DCFlushRange(STAGE_AREA, (dolSize + 31) & ~31);
	memcpy(ARG_AREA + 64, argPath, argLen);
	u32* arg = (u32*)ARG_AREA;
	arg[0] = 0x5f617267;   /* "_arg" */
	arg[1] = (u32)(ARG_AREA + 64);
	arg[2] = (u32)argLen;
	arg[3] = 0;
	arg[4] = 0;
	arg[5] = 0;
	DCFlushRange(ARG_AREA, 64 + argLen);
	memcpy(CODE_AREA, (const void*)wiifinTrampoline, len);
	DCFlushRange(CODE_AREA, (len + 31) & ~31);
	ICInvalidateRange(CODE_AREA, (len + 31) & ~31);
	((void (*)(const void*, const void*))(void*)CODE_AREA)(STAGE_AREA, arg);
}
