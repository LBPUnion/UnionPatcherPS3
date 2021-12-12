// C++ standard library
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <sstream>
#include <map>

// There's a bug in the standard library that breaks std::to_string() LMAO here's my fix
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

// local thing - rsxutil does basic boilerplate stuff that I don't want to worry about, helpers is the same thing
#include "rsxutil.h"
#include "helpers.cpp"
#include "constants.cpp"

// Store the value of the button pressed so we can do stuff with it in code
static vs32 dialog_action = 0;

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

int main(int argc,char *argv[])
{
	printf("UnionPatcher UI starting...\n");

	// Create instance of msgType (dialogType) we'll use this later
	msgType dialogType;

	// Stuff for reading directories that I completely forgot, thanks IRISMAN source code
	int fd;
	sysFSDirent dir;
	size_t read;

	int e_fd;
	sysFSDirent e_dir;
	size_t e_read;

	// String for storing all of the names of different installations found on internal storage
	std::string installationsFriendlyNames = "";

	// Integer for tracking number of installations so I can reflect that in the UI (lol) and actually check to see if I find ANY installations
	int installationsCount = 0;

	// Store string to give notification of broken installations
	std::string brokenInstalls = "";
	int brokenInstallsCount = 0;

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
							// Bool to check whether or not EBOOT.BIN exists
							bool ebootExists = false;

							// Inside path to check EBOOT
							std::string path = "/dev_hdd0/game/" + patch::to_string(dir.d_name) + "/USRDIR/";
							sysFsOpendir(path.c_str(), &e_fd);
        					if(e_fd >= 0)
        					{
								while(!sysFsReaddir(e_fd, &e_dir, &e_read) && e_read)
								{
									if(patch::to_string(e_dir.d_name) == patch::to_string("EBOOT.BIN"))
									{
										ebootExists = true;
									}
									do_flip();
								}
								sysFsClosedir(e_fd);
							}

							if(ebootExists)
							{
								printf(" Yes!\n");
								installationsCount++;
								installationsFriendlyNames += gameIDRealNames.at(patch::to_string(dir.d_name)) + "\n";
							}
							else
							{
								printf(" Directory name matched but no EBOOT.BIN is present in USRDIR\n");
								brokenInstallsCount++;
							}
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

		// Check for broken installs
		if(brokenInstallsCount > 0)
		{
			brokenInstalls = "We also found " + patch::to_string(brokenInstallsCount) + " LBP install directories without EBOOT.BIN files. These may be corrupted.";
		}

	// Prompt the user depending on whether or not any LBP installations were found on internal storage
	if(installationsCount > 0)
	{
		printf("Matching LBP installation found! :)\n");
		dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
		std::string message = "UnionPatcher found the following installations on your internal storage:\n\n" + installationsFriendlyNames + "\n" + brokenInstalls + "\n\nDo you want to continue?";
		msgDialogOpen2(dialogType, message.c_str(), dialog_handler,NULL,NULL);
	}
	else
	{
		printf("No matching LBP installation found :(\n");
		dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
		msgDialogOpen2(dialogType, ("UnionPatcher could not find any valid installations of LittleBigPlanet to patch. Ensure you have booted your copy of LittleBigPlanet 1, 2, or 3 at least once to copy all neccessary game files to your internal storage. If you have completed this step, and you're still getting this error, let us know on GitHub;\n\nhttps://www.github.com/lbpunion/unionpatcherps3/issues\n\n"+ brokenInstalls).c_str(), dialog_handler,NULL,NULL);
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