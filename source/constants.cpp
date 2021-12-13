// C++ standard library
#include <stdlib.h>
#include <string>
#include <map>


// Map of all LBP Game IDs to title names. This is really important for us to scan for installations, check that our folder matches a real LBP title ID, and reflect it in the UI
const std::map<std::string, std::string> gameIDRealNames = 
{
	// LBP1
	{"BCUS98148", "LittleBigPlanet™ [US] [Disc] [BCUS98148]"},
	{"BCES00141", "LittleBigPlanet™ [EU] [Disc] [BCES00141]"},
	{"NPUA80472", "LittleBigPlanet™ [US] [PSN] [NPUA80472]"},
	{"NPEA00241", "LittleBigPlanet™ [EU] [PSN] [NPEA00241]"},
	{"BCUS98208", "LittleBigPlanet™ Game of the Year Edition [US] [Disc] [BCUS98208]"},
	{"BCES00611", "LittleBigPlanet™ Game of the Year Edition [EU] [Disc] [BCES00611]"},

	// LBP2
	{"BCUS98245", "LittleBigPlanet™ 2 [US] [Disc] [BCUS98245]"},
	{"BCES01086", "LittleBigPlanet™ 2 [EU] [Disc] [BCES01086]"},
	{"BCES00850", "LittleBigPlanet™ 2 [EU] [Disc] [BCES00850]"},
	{"BCAS20113", "LittleBigPlanet™ 2 [AS] [Disc] [BCAS20113]"},
	{"NPUA80662", "LittleBigPlanet™ 2 [US] [PSN] [NPUA80662]"},
	{"NPEA00324", "LittleBigPlanet™ 2 [EU] [PSN] [NPEA00324]"},
	{"BCUS98372", "LittleBigPlanet™ 2: Special Edition [US] [Disc] [BCUS98372]"},
	{"BCES01693", "LittleBigPlanet™ 2 Extras Edition [EU] [Disc] [BCES01693]"},
	{"BCES01694", "LittleBigPlanet™ 2 Extras Edition [EU] [Disc] [BCES01694]"},

	// LBP3
	{"BCUS98362", "LittleBigPlanet™ 3 [US] [Disc] [BCUS98362]"},
	{"BCES02068", "LittleBigPlanet™ 3 [EU] [Disc] [BCES02068]"},
	{"BCES01663", "LittleBigPlanet™ 3 [EU] [Disc] [BCES01663]"},
	{"NPUA81116", "LittleBigPlanet™ 3 [US] [PSN] [NPUA81116]"}
};