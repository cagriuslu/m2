#pragma once
#include <cuzn/Detail.h>

// These functions should be called before the card is discarded, because the number of cards is used to determine
// first/last action.

std::string build_notification(Industry industry, City city);
std::string network_notification(City connection1_city1, City connection1_city2, City connection2_city1, City connection2_city2);
std::string sell_notification(Industry industry, City city);
std::string loan_notification();
std::string develop_notification(Industry industry1, Industry industry2);
std::string scout_notification();
std::string pass_notification();
