/*
 * txtgfx.h - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_TXTGFX
#define H_TXTGFX

#ifdef __cplusplus
extern "C" {
#endif

void clearscreen(unsigned char col);
void setvideomode(unsigned char mode);
void showcursor(void);
void hidecursor(void);
void setcolor(unsigned char newcol);
void settextposition(unsigned char ypos, unsigned char xpos);
void outtext(char *str);
void outchar(char c);

#ifdef __cplusplus
}
#endif

#endif
