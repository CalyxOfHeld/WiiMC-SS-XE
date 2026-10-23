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
#include <strings.h>
#include <dirent.h>

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

/* a DOL there and sound: its header read, its entry point in the Wii's
 * memory (not an empty or half-copied file) */
static bool dolThere(const char* path)
{
	FILE* f = openDol(path);
	if(!f) return false;
	u8 head[0x100];
	bool ok = fread(head, 1, sizeof(head), f) == sizeof(head);
	fclose(f);
	u32 entry = (u32)head[0xE0] << 24 | (u32)head[0xE1] << 16 | (u32)head[0xE2] << 8 | head[0xE3];
	return ok && (entry & 0xF0000000) == 0x80000000;
}

/* an app in apps/ whose meta.xml names it WiiFin (a folder of another
 * name), its boot.dol into out ("sd:/apps/NAME/boot.dol") */
static bool scanApps(const char* dev, const char* mounted, char* out, size_t outLen)
{
	char dir[64];
	snprintf(dir, sizeof(dir), "%s/apps/", mounted);
	DIR* d = opendir(dir);
	if(!d) return false;
	bool found = false;
	struct dirent* e;
	while(!found && (e = readdir(d)) != NULL)
	{
		if(e->d_name[0] == '.') continue;
		char meta[320], buf[1024];
		snprintf(meta, sizeof(meta), "%s%s/meta.xml", dir, e->d_name);
		FILE* f = fopen(meta, "rb");
		if(!f) continue;
		size_t n = fread(buf, 1, sizeof(buf) - 1, f);
		fclose(f);
		buf[n] = 0;
		char* a = strstr(buf, "<name>");
		char* b = a ? strstr(a, "</name>") : NULL;
		if(!a || !b) continue;
		*b = 0;
		bool wiifin = false;
		for(char* p = a + 6; *p && !wiifin; p++)
			wiifin = strncasecmp(p, "wiifin", 6) == 0;
		if(!wiifin) continue;
		snprintf(out, outLen, "%s/apps/%s/boot.dol", dev, e->d_name);
		found = dolThere(out);
	}
	closedir(d);
	return found;
}

/* WiiFin's DOL, as WiiFin names its devices (sd:, usb:) */
static bool findWiiFin(char* out, size_t outLen)
{
	const char* usual[3] = { wiifinPath, "sd:/apps/WiiFin/boot.dol", "usb:/apps/WiiFin/boot.dol" };
	for(int i = 0; i < 3; i++)
		if(usual[i][0] && dolThere(usual[i])) { snprintf(out, outLen, "%s", usual[i]); return true; }
	if(scanApps("sd:", "sd1:", out, outLen)) return true;
	for(int m = 1; m < 6; m++)
	{
		char mounted[8];
		snprintf(mounted, sizeof(mounted), "usb%d:", m);
		if(scanApps("usb:", mounted, out, outLen)) return true;
	}
	return false;
}

bool WiiFinFound()
{
	char p[256];
	return dol != NULL || findWiiFin(p, sizeof(p));
}

bool WiiFinLoad()
{
	if(dol) return true;
	char p[256];
	return findWiiFin(p, sizeof(p)) && loadFrom(p);
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
