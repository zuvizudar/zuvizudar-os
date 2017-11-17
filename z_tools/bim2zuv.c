#include <stdio.h>
#include <stdlib.h>

int getnum(char *p);
int get32(unsigned char *p);
void put32(char *p, int i);

#define MAXSIZ     4 * 1024 * 1024

int main(int argc, char **argv)
{
    char *fbuf = malloc(MAXSIZ);
    int heap_siz, mmarea, fsiz, dsize, dofs, stksiz, wrksiz, entry, bsssiz, heap_adr, i;
    FILE *fp;
    static char sign[4] = "Zuvi";

    /* パラメータの取得 */
    if (argc < 4) {
        puts("usage>bim2hrb appname.bim appname.hrb heap-size [mmarea-size]");
        return 1;
    }
    heap_siz = getnum(argv[3]);
    mmarea = 0;
    if (argc >= 5) {
        mmarea = getnum(argv[4]);
    }

    /* ファイル読み込み */
    fp = fopen(argv[1], "rb");
    if (fp == 0) {
err_bim:
        puts("bim file read error");
        return 1;
    }
    fsiz = fread(fbuf, 1, MAXSIZ, fp);
    fclose(fp);
    if (fsiz >= MAXSIZ || fsiz < 0) {
        goto err_bim;
    }

    /* ヘッダ確認 */
    if (get32(&fbuf[4]) != 0x24) { /* ファイル中の.textスタートアドレス */
err_form:
        puts("bim file format error");
        return 1;
    }
    if (get32(&fbuf[8]) != 0x24) { /* メモリロード時の.textスタートアドレス */
        goto err_form;
    }
    dsize  = get32(&fbuf[12]);     /* .dataセクションサイズ */
    dofs   = get32(&fbuf[16]);     /* ファイルのどこに.dataセクションがあるか */
    stksiz = get32(&fbuf[20]);     /* スタックサイズ */
    entry  = get32(&fbuf[24]);     /* エントリポイント */
    bsssiz = get32(&fbuf[28]);     /* bssサイズ */

    /* ヘッダ生成 */
    heap_adr = stksiz + dsize + bsssiz;
    heap_adr = (heap_adr + 0xf) & 0xfffffff0; /* 16バイト単位に切り上げ */
    wrksiz = heap_adr + heap_siz;
    wrksiz = (wrksiz + 0xfff) & 0xfffff000; /* 4KB単位に切り上げ */
    put32(&fbuf[ 0], wrksiz);
    for (i = 0; i < 4; i++) {
        fbuf[4 + i] = sign[i];
    }
    put32(&fbuf[ 8], mmarea);
    put32(&fbuf[12], stksiz);
    put32(&fbuf[16], dsize);
    put32(&fbuf[20], dofs);
    put32(&fbuf[24], 0xe9000000);
    put32(&fbuf[28], entry - 0x20);
    put32(&fbuf[32], heap_adr);

    /* ファイル書き込み */
    fp = fopen(argv[2], "wb");
    if (fp == 0) {
err_hrb:
        puts("hrb file write error");
        return 1;
    }
    i = fwrite(fbuf, 1, fsiz, fp);
    fclose(fp);
    if (fsiz != i) {
        goto err_hrb;
    }
    return 0;
}

int getnum(char *p)
{
    int i = strtol(p, &p, 0);
    unsigned char c = *p;
    if (c >= 'a') {
        c -= 'a' - 'A';
    }
    if (c == 'K') {
        i <<= 10;
    }
    if (c == 'M') {
        i <<= 20;
    }
    if (c == 'G') {
        i <<= 30;
    }
    return i;
}

int get32(unsigned char *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

void put32(char *p, int i)
{
    p[0] =  i        & 0xff;
    p[1] = (i >>  8) & 0xff;
    p[2] = (i >> 16) & 0xff;
    p[3] = (i >> 24) & 0xff;
    return;
}
