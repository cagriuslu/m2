#include "Ui.h"
#include <string.h>

int UiInit(Ui *ui) {
	memset(ui, 0, sizeof(Ui));
	return 0;
}

void UiDeinit(Ui *ui) {
	(void)ui;
}
