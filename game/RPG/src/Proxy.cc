#include <m2g/Proxy.h>
#include <rpg/PowerUp.h>
#include <m2/Exception.h>

void m2g::dynamic_assert() {
	for (int i = 0; i < itm::power_up_count; i++) {
		if (itm::power_ups[i].index != i) {
			throw M2FATAL("Dynamic assert");
		}
	}
}
