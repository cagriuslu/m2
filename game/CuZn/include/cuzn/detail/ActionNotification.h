#pragma once
#include <cuzn/Detail.h>

// These functions should be called before the card is discarded, because the number of cards is used to determine
// first/last action.

std::string GenerateBuildNotification(Industry, City);
std::string GenerateNetworkNotification(City connection1City1, City connection1City2, City connection2City1, City connection2City2);
std::string GenerateSellNotification(Industry, City);
std::string GenerateLoanNotification();
std::string GenerateDevelopNotification(Industry industry1, Industry industry2);
std::string GenerateScoutNotification();
std::string GeneratePassNotification();
