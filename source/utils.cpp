// C++ standard library
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <sstream>
#include <map>
#include <sys/stat.h>

// psl1ght stuff
#include <io/pad.h>
#include <ppu-lv2.h>
#include <lv2/sysfs.h>
#include <sysutil/msg.h>
#include <sysutil/sysutil.h>

// local thing - rsxutil does basic boilerplate stuff that I don't want to worry about
#include "rsxutil.h"

// Check for new events (checks pending callback functions is my best way of trying to understand this)
static void do_flip()
{
	sysUtilCheckCallback();
	flip();
}

static void program_exit_callback()
{
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

static void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status)
	{
		case SYSUTIL_EXIT_GAME:
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}

int CopyFile(const char *source, const char *dest){
	s32 src = -1;
	s32 dst = -1;
	sysFSStat stats;

	char buffer[0x1000];
	u64  i;
	s32  ret;

	ret = sysFsOpen(source, SYS_O_RDONLY, &src, NULL, 0);
	if (ret)
		goto out;

	ret = sysFsOpen(dest, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &dst, NULL, 0);
	if (ret)
		goto out;

	sysFsChmod(dest, S_IFMT | 0777);

	sysFsFstat(src, &stats);

	for (i = 0; i < stats.st_size;) {
		u64 pos, read, written;

		sysFsLseek(src, i, 0, &pos);
		sysFsLseek(dst, i, 0, &pos);

		ret = sysFsRead(src, buffer, sizeof(buffer), &read);
		if (ret || !read)
			break;

		ret = sysFsWrite(dst, buffer, read, &written);
		if (ret || !written)
			break;

		i += written;
	}

out:
	if (src >= 0) sysFsClose(src);
	if (dst >= 0) sysFsClose(dst);

	return ret;
}