// C++ standard library
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <sstream>
#include <map>


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