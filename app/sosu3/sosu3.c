#include <stdio.h>
#include "apilib.h"

#define MAX		10000

void ZuviMain(void)
{
	char *flag, s[8];
	int i, j, k, fh;
	api_initmalloc();
	flag = api_malloc(MAX);
	for (i = 0; i < MAX; i++) {
		flag[i] = 0;
	}
	fh = api_fopenEx("sosu.txt", 0x10);
	if (fh == 0) {
		api_end();
	}
	k = 0;
	for (i = 2; i < MAX; i++) {
		if (flag[i] == 0) {
			/* óªÂ¢Ä¢È¢ÌÅf¾I */
//			sprintf(s, "%d ", i);
//			api_putstr0(s);
			sprintf(s, "%04d ", i);
			api_fwrite(s, 5, fh);
			if ((k % 5) == 4) {
				api_fwrite("\n", 1, fh);
			}
			k++;
			for (j = i * 2; j < MAX; j += i) {
				flag[j] = 1;	/* {ÉÍóðÂ¯é */
			}
		}
	}
	api_fcloseEx(fh);
	api_end();
}
