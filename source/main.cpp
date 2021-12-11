// C++ standard library
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <sstream>
#include <map>

// There's a bug in the standard library that breaks std::to_string LMAO here's my fix
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

// psl1ght stuff
#include <io/pad.h>
#include <ppu-lv2.h>
#include <lv2/sysfs.h>
#include <sysutil/msg.h>
#include <sysutil/sysutil.h>

// local thing - rsxutil does basic boilerplate stuff that I don't want to worry about
#include "rsxutil.h"

// Map of all LBP Game IDs to title names. This is really important for us to scan for installations and reflect it in the UI
const std::map<std::string, std::string> gameIDRealNames = 
{
	// LBP1
	{"BCUS98148", "LittleBigPlanet™ [US] [Disc]"},
	{"BCES00141", "LittleBigPlanet™ [EU] [Disc]"},
	{"NPUA80472", "LittleBigPlanet™ [US] [PSN]"},
	{"NPEA00241", "LittleBigPlanet™ [EU] [PSN]"},
	{"BCUS98208", "LittleBigPlanet™ Game of the Year Edition [US] [Disc]"},
	{"BCES00611", "LittleBigPlanet™ Game of the Year Edition [EU] [Disc]"},

	// LBP2
	{"BCUS98245", "LittleBigPlanet™ 2 [US] [Disc]"},
	{"BCES01086", "LittleBigPlanet™ 2 [EU] [Disc]"},
	{"BCES00850", "LittleBigPlanet™ 2 [EU] [Disc]"},
	{"BCAS20113", "LittleBigPlanet™ 2 [AS] [Disc]"},
	{"NPUA80662", "LittleBigPlanet™ 2 [US] [PSN]"},
	{"NPEA00324", "LittleBigPlanet™ 2 [EU] [PSN]"},
	{"BCUS98372", "LittleBigPlanet™ 2: Special Edition [US] [Disc]"},
	{"BCES01693", "LittleBigPlanet™ 2 Extras Edition [EU] [Disc]"},
	{"BCES01694", "LittleBigPlanet™ 2 Extras Edition [EU] [Disc]"},

	// LBP3
	{"BCUS98362", "LittleBigPlanet™ 3 [US] [Disc]"},
	{"BCES02068", "LittleBigPlanet™ 3 [EU] [Disc]"},
	{"BCES01663", "LittleBigPlanet™ 3 [EU] [Disc]"},
	{"NPUA81116", "LittleBigPlanet™ 3 [US] [PSN]"}
};

// Store the value of the button pressed so we can do stuff with it in code
static vs32 dialog_action = 0;

extern "C" {
// dialog_handler uses the button pressed to set the value of the dialog_action integer
static void dialog_handler(msgButton button,void *usrData)
{
	switch(button)
	{
		case MSG_DIALOG_BTN_OK:
			dialog_action = 1;
			break;
		case MSG_DIALOG_BTN_NO:
		case MSG_DIALOG_BTN_ESCAPE:
			dialog_action = 2;
			break;
		case MSG_DIALOG_BTN_NONE:
			dialog_action = -1;
			break;
		default:
			break;
	}
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
}

// Check for new events (checks pending callback functions is my best way of trying to understand this)
static void do_flip()
{
	sysUtilCheckCallback();
	flip();
}

// Hash char arrays so I can compare them. This is an absolutely disgusting hack but this is my first ever writing stuff in C++
// and this one guy who thought this up had lots of upvotes on Stackoverflow.
constexpr unsigned int hash(const char *s, int off = 0)
{                        
    return !s[off] ? 0 : (hash(s, off+1)*33) ^ s[off];                           
}

int main(int argc,char *argv[])
{
	printf("UnionPatcher UI starting...\n");

	// Create instance of msgType (dialogType) we'll use this later
	msgType dialogType;

	// Stuff for reading directories that I completely forgot, thanks IRISMAN source code
	int fd;
	sysFSDirent dir;
	size_t read;

	// String for storing all of the names of different installations found on internal storage
	std::string installationsFriendlyNames = "";

	// Integer for tracking number of installations so I can reflect that in the UI (lol) and actually check to see if I find ANY installations
	int installationsCount;

	// Initialize 1mb buffer in system memory for RSX (i think)
 	void *host_addr = memalign(1024*1024,HOST_SIZE);

	// Initialize the RSX (rsxutil.cpp)
	init_screen(host_addr,HOST_SIZE);

	// Initialize controllers (with up to 1 player)
	ioPadInit(1);

	// Run program_exit_callback() at program exit (why does the sample do this all the way up here??)
	atexit(program_exit_callback);

	// Register the exit callback
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0,sysutil_exit_callback,NULL);

	// Read /dev_hdd0/game/ and find game installations
	printf("Checking /dev_hdd0/game/ for LittleBigPlanet Installations...\n");

	// Opendir opens the directory for... Reading
	sysFsOpendir("/dev_hdd0/game/", &fd);
        if(fd >= 0)
        {
			// We recursively read the directory to see what's inside
            while(!sysFsReaddir(fd, &dir, &read) && read)
			{
				// Report our findings to console
				printf("Found directory /dev_hdd0/game/");
				printf(dir.d_name);	
				printf(" - checking to see if it's an LBP installation...");

						if(gameIDRealNames.count(patch::to_string(dir.d_name)) > 0)
						{
							printf(" Yes!\n");
							installationsCount++;
							installationsFriendlyNames += gameIDRealNames.at(patch::to_string(dir.d_name)) + "\n";
						}
						else
						{
							printf(" No!\n");
						}


				// keep checking for events
				do_flip();
			}
			// Close game directory
            sysFsClosedir(fd);
        }

	// Prompt the user depending on whether or not any LBP installations were found on internal storage
	if(installationsCount > 0)
	{
		printf("Matching LBP installation found! :)\n");
		printf(installationsFriendlyNames.c_str());
		dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
		std::string message = "UnionPatcher found the following installations on your internal storage:\n\n" + installationsFriendlyNames + "\nDo you want to continue?";
		msgDialogOpen2(dialogType, message.c_str(), dialog_handler,NULL,NULL);
	}
	else
	{
		printf("No matching LBP installation found :(\n");
		dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
		msgDialogOpen2(dialogType,"UnionPatcher could not find any valid installations of LittleBigPlanet to patch. Ensure you have booted your copy of LittleBigPlanet 1, 2, or 3 at least once to copy all neccessary game files to your internal storage.\n\nIf you have completed this step, and you're still getting this error, let us know on GitHub;\n\nhttps://www.github.com/lbpunion/unionpatcherps3/issues",dialog_handler,NULL,NULL);
	}
	
	// Wait for the dialog_action (value of last pressed button) to change, we're basically resetting the controller button state and waiting for any key
	dialog_action = 0;
	while(!dialog_action)
		// ALWAYS DO FLIP or the app doesn't keep track of when events are getting fired and you're screwed
		do_flip();

	// Abort message box once we get our answer
	msgDialogAbort();
	
    printf("UnionPatcher finished!\n");

	// Exit the program (goodbye!)
    return 0;
}

// God Bless whoever wrote the sample that I started with