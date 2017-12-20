#include "bootpack.h"
#include <stdio.h>
#include <string.h>
extern char t[];
extern CONSOLE *tcons;
void file_readfat(int *fat, unsigned char *img){
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}
FILEINFO *file_search(char *name, FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) { return 0;}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				s[j] -= 0x20;
			} 
			j++;
		}
	}
	for (i = 0; i < max; ) {
		if (finfo->name[0] == 0x00) {
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			return finfo + i; 
		}
next:
		i++;
	}
	return 0;
}
int file_searchfat(int *fat)
{
	int i;
	for (i = 0; i < 2880; i++) {
		if (fat[i] == 0x0000) {
			return i;
		}
	}
	return 0x0fff;
}

int file_savefile(FILEHANDLE *fh, int size, int *fat, char *img){
	int nextno, clustno = fh->clustno, wsize = 0;
	char *buf = fh->buf;

	if (clustno >= 0x0ff8) {
		return 0;
	} else if (clustno != 0) {
		if (fat[clustno] == 0x0000 || fat[clustno] >= 0x0ff8) {
			fat[clustno] = 0x0fff;
			nextno = file_searchfat(fat);
			fat[clustno] = nextno;
		} else {
			nextno = fat[clustno];
		}
	} else {
		nextno = fh->finfo->clustno;
	}
	int i = 0;
	for (; nextno != 0x0fff;) {
		char msg[30];
		sprintf(msg, "%04d ", clustno);
		cons_putstr0(tcons, msg);
		clustno = nextno;
		sprintf(msg, "%04d ", clustno);
		cons_putstr0(tcons, msg);
		i++;
		if (size <= 512) {
			if (size != 0) {
				memcpy(img + clustno * 512, buf, size);
				wsize += size;
			}
			if (fat[clustno] == 0x0000) {
				fat[clustno] = 0x0fff;
			}
			break;
		}
		memcpy(img + clustno * 512, buf, 512);
		size  -= 512;
		buf   += 512;
		wsize += 512;
		if (fat[clustno] == 0x0000 || fat[clustno] >= 0x0ff8) {
			fat[clustno] = 0x0fff;
			nextno = file_searchfat(fat);
			fat[clustno] = nextno;
		} else {
			nextno = fat[clustno];
		}
	}
	fh->clustno = clustno;
	file_time(fh->finfo);
	fh->finfo->size += (wsize - (fh->finfo->size % 512));

	return wsize;
}

int file_skipfile(FILEHANDLE *fh, int size, int *fat, char *img)
{
	int nextno = fh->finfo->clustno;
	int clustno = 0, ssize = 0, rsize;

	for (; ssize < size - 512;) {
		clustno = nextno;
		nextno  = fat[clustno];
		if (nextno >= 0x0ff8) {
			break;
		}
		ssize += 512;
	}
	rsize = size - ssize;
	if (rsize < 512) {
		memcpy(fh->buf, img + nextno * 512, rsize);
	} else if (rsize > 512) {
		rsize = 512;
		fh->finfo->size = ssize + rsize;
	}
	fh->pos     = ssize + rsize;
	fh->bpos    = rsize % 512;
	fh->clustno = clustno;

	return fh->pos;
}

FILEINFO *file_insert(char *name,FILEINFO *finfo, int max, int *fat)
{
	int i, j = 0, clustno;
	char s[13];

	for (i = 0; i < 12; i++) {
		if (i != 8) {
			s[i] = 0x20;
		}
	}
	for (i = 0; j < 12 && name[i] != 0; i++) {
		if (name[i] == '.') {
			j = 9;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				s[j] -= 0x20;
			} 
			j++;
		}
	}

	for (i = 0; i < max; i++) {
		if (finfo[i].name[0] == 0x00 || finfo[i].name[0] == 0xe5) {
			clustno = file_searchfat(fat);
			if (clustno == 0x0fff) {
				break;
			}
			if (fat[clustno] == 0x0000) {
				fat[clustno] = 0x0fff;
			}
			strcpy(finfo[i].name, s);
			strcpy(finfo[i].ext, s + 9);
			finfo[i].type    = 0x20;
			finfo[i].clustno = clustno;
			finfo[i].size    = 0;
			file_time(&finfo[i]);
			return  finfo + i;
		}
	}

	return 0;
}

void file_time(FILEINFO *finfo)
{
	int i;
	char t16[7];
	unsigned short t10[7];

	memcpy(t16, t, 7);
	for (i = 0; i < 7; i++) {
		t10[i] = (t16[i] >> 4) * 10 + (t16[i] & 0x0f);
	}
	finfo->time = (t10[2] << 11)
				+ (t10[1] << 5)
				+ (unsigned short) (t10[0] / 2);
	finfo->date = ((t10[6] * 100 + t10[5] - 1980) << 9)
				+ (t10[4] << 5)
				+ t10[3];

	return;
}
