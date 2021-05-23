#include "UI.h"
#include <string.h>

int UIInit(UI *ui) {
	memset(ui, 0, sizeof(UI));
	return 0;
}

void UIDeinit(UI *ui) {
}
