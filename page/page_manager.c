
#include <common.h>
#include <string.h>
#include <page_manager.h>

extern void MainPageRegister();


static PPageAction g_ptPages = NULL;


void PageRegister(PPageAction ptPageAction){

	ptPageAction->ptNext = g_ptPages;
	g_ptPages = ptPageAction;
}

PPageAction Page(char *name){

	PPageAction ptTmp = g_ptPages;

	while (ptTmp)
	{
		if(strcmp(ptTmp->name, name) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

void PageSystemRegister(void){

	MainPageRegister();
}



