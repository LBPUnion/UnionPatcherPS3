// C++ standard library
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <sstream>
#include <map>

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