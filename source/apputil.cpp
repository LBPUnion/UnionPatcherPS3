// C++ standard library
#include <stdlib.h>
#include <stdio.h>

// psl1ght stuff
#include <lv2/sysfs.h>
#include <sysutil/sysutil.h>
#include <sys/stat.h>

// load RSXUtil, some basic boilerplate code for initializing the RSX and stuff that I don't want to worry about
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

static int CopyFile(const char *source, const char *dest){
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

static void utf16_to_utf8(const uint16_t *src, uint8_t *dst)
{
    int i;
    for (i = 0; src[i]; i++)
    {
        if ((src[i] & 0xFF80) == 0)
        {
            *(dst++) = src[i] & 0xFF;
        }
        else if((src[i] & 0xF800) == 0)
        {
            *(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
            *(dst++) = (src[i] & 0x3F) | 0x80;
        }
        else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00)
        {
            *(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
            *(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
            *(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
            *(dst++) = (src[i + 1] & 0x3F) | 0x80;
            i += 1;
        }
        else
        {
            *(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
            *(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
            *(dst++) = (src[i] & 0x3F) | 0x80;
        }
    }

    *dst = '\0';
}

static void utf8_to_utf16(const uint8_t *src, uint16_t *dst)
{
    int i;
    for (i = 0; src[i];)
    {
        if ((src[i] & 0xE0) == 0xE0)
        {
            *(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
            i += 3;
        }
        else if ((src[i] & 0xC0) == 0xC0)
        {
            *(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
            i += 2;
        }
        else
        {
            *(dst++) = src[i];
            i += 1;
        }
    }

    *dst = '\0';
}