#ifndef _FONT_MANAGER_H
#define _FONT_MANAGER_H

#include <common.h>

#ifndef NULL
#define NULL (void *)0
#endif

typedef struct FontBitMap {

	Region tRegion;
	int iCurOriginX;
	int iCurOriginY;
	int iNextOriginX;
	int iNextOriginY;
	unsigned char *pucBuffer;
}FontBitMap, *PFontBitMap;

typedef struct FontOpr {

	char *name;
	int (*FontInit)(char *aFileName);
	int (*SetFontSize)(int iFontSize);
	int (*GetFontBitMap)(unsigned int dwCode, PFontBitMap ptFontBitMap);
	int (*GetStringRegionCar)(char *str, PRegionCartesian ptRegionCar);
	struct FontOpr *ptNext;
}FontOpr, *PFontOpr;

void RegisterFont(PFontOpr ptFontOpr);
void FontSystemRegister(void);
int SelectAndInitFont(char *aFontOprName, char *aFontFileName);
int SetFontSize(int iFontSize);
int GetFontBitMap(unsigned int dwCode, PFontBitMap ptFontBitMap);
int GetStringRegionCar(char *str, PRegionCartesian ptRegionCar);



#endif


