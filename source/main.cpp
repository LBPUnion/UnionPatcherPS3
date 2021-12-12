  /////////////////////
 ////   IMPORTS   ////
/////////////////////

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

// psl1ght libraries
#include <io/pad.h>
#include <ppu-lv2.h>
#include <lv2/sysfs.h>
#include <sysutil/msg.h>
#include <sysutil/sysutil.h>

// local dependencies - rsxutil does basic boilerplate stuff that I don't want to worry about, helpers is the same thing
#include "rsxutil.h"
#include "helpers.cpp"
#include "constants.cpp"

  /////////////////////
 ////  THE CODE   ////
/////////////////////

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
	// Run program_exit_callback() at program exit (why does the sample do this all the way up here??)
	atexit(program_exit_callback);

	// Register the exit callback
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0,sysutil_exit_callback,NULL);

	// Constant messages so I can figure out where tf I am while I'm """"debugging"""" (you don't get breakpoints in RPCS3)
	printf("UnionPatcher UI starting...\n");

	// Create instance of msgType (dialogType) we'll use this later
	msgType dialogType;

	// Stuff for reading directories that I completely forgot, thanks IRISMAN source code for the example
	int fd;
	sysFSDirent dir;
	size_t read;

	int e_fd;
	sysFSDirent e_dir;
	size_t e_read;

	// String for storing all of the names of different installations found on internal storage and displaying them in the UI
	std::string installationsFriendlyNames = "";

	// Integer for tracking number of installations so I can reflect that in the UI (lol) and actually check to see if I find ANY installations
	int installationsCount = 0;

	// String to give notification of broken installations (if more than 0 broken installs are detected)
	std::string brokenInstalls = "";
	int brokenInstallsCount = 0;

	// Initialize 1mb buffer in system memory for RSX (i think)
 	void *host_addr = memalign(1024*1024,HOST_SIZE);

	// Initialize the RSX (rsxutil.cpp)
	init_screen(host_addr,HOST_SIZE);

	// Initialize controller (with up to 1 player) (rsxutil.cpp)
	ioPadInit(1);


	// Let's start doing some fun UI things like showing a quick menu explaining the usage of this utility
	//
	// Screen 1

	// Set dialogue type
	dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO);

	// Open dialogue with message
	msgDialogOpen2(dialogType, "UnionPatcherPS3 - a tool brought to you by LoganTGT & LBP Union\n\nLBP Union's UnionPatcherPS3 will assist you in preparing your installation(s) of LittleBigPlanet 1, 2, or 3 to connect to Project: Lighthouse and other compatible custom servers. Before continuing, please ensure you have a copy of LittleBigPlanet installed to your internal storage, and all avaiable updates have been installed.\n\nDo you want to continue?", dialog_handler,NULL,NULL);

	// Wait for the dialog_action (value of last pressed button) to change, we're basically resetting the controller button state and waiting for any key
	dialog_action = 0;
	while(!dialog_action)		
		do_flip(); // ALWAYS DO FLIP or the app doesn't keep track of when events are getting fired and you're screwed

	// We just got out of that while loop, so lets see what button the user pressed and act on it. Return 0 if O/exit was pressed else continue execution
	if(dialog_action == 2)
	{
		return 0;
	}

	// Abort message box once we get our answer
	msgDialogAbort();

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
		dialogType = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO);
		std::string message = "UnionPatcher found the following installations on your internal storage:\n\n" + installationsFriendlyNames + "\n" + brokenInstalls + "\nIf this seems correct, select Yes to continue.\n\nDo you want to continue?";
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

	// We just got out of that while loop, so lets see what button the user pressed and act on it. Return 0 if O/exit was pressed else continue execution
	if(dialog_action == 2)
	{
		return 0;
	}

	// Abort message box once we get our answer
	msgDialogAbort();

    printf("UnionPatcher finished!\n");

	// Exit the program (goodbye!)
    return 0;
}

// God Bless whoever wrote the sample that I started with