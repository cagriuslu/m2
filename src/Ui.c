#include "Ui.h"
#include <string.h>

int Ui_Init(Ui *ui) {
	memset(ui, 0, sizeof(Ui));
	return 0;
}

void Ui_Term(Ui *ui) {
	(void)ui;
}
