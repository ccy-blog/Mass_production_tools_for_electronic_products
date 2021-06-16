
#include <page_manager.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ui.h>

#define X_GAP 5
#define Y_GAP 5

static Button g_tButtons[ITEMCFG_MAX_NUM];
static int g_tButtonCnt;


static int MainPageOnPressed(struct Button *ptButton, PDispBuff ptDisBuff, PInputEvent ptInputEvent){

	unsigned int dwColor = BUTTON_DEFAULT_COLOR;
	char name[100];
	char status[100];
	char *strButton;
	char *command_status[3] = {"err", "ok", "percent"};
	int command_status_index = 0;
	char command[1000];
	PItemCfg ptItemCfg;

	strButton = ptButton->name;
	/* 1. 对于触摸屏事件 */
	if(ptInputEvent->iType == INPUT_TYPE_TOUCH){

		/* 1.1 分辨能否被点击 */
		if(GetItemCfgByName(ptButton->name)->bCanBeTouched == 0){

			return -1;
		}

		/* 1.2 修改颜色 */
		
		ptButton->status = !ptButton->status;
		if(ptButton->status){

			dwColor = BUTTON_PRESSED_COLOR;
			command_status_index = 1;
		}
		
	}else if(ptInputEvent->iType == INPUT_TYPE_NET){

		/* 2. 对于网络事件 */

		/* 根据传入的字符串修改颜色 : wifi ok, wifi err, burn 70 */
		sscanf(ptInputEvent->str, "%s %s", name, status);
		if (strcmp(status, "OK") == 0 || strcmp(status, "ok") == 0){
			
			dwColor = BUTTON_PRESSED_COLOR;
			command_status_index = 1;

		}else if (strcmp(status, "err") == 0){

			dwColor = BUTTON_DEFAULT_COLOR;
			command_status_index = 0;

		}else if (status[0] >= '0' && status[1] <= '9'){

			dwColor = BUTTON_PERCENT_COLOR;
			strButton = status;
			command_status_index = 2;

		}else{

			return -1;
		}
	}else{

		return -1;
	}

	/* 绘制底色 */
	DrawRegion(&ptButton->tRegion, dwColor);

	/* 居中写文字 */
	DrawTextInRegionCentral(strButton, &ptButton->tRegion, BUTTON_TEXT_COLOR);

	/* flush to lcd/web */
	FlushDisplayRegion(&ptButton->tRegion, ptDisBuff);

	/* 执行command */
	ptItemCfg = GetItemCfgByName(ptButton->name);
	if(ptItemCfg->command[0] != '\0'){

		sprintf(command, "%s %s", ptItemCfg->command, command_status[command_status_index]);
		system(command);
	}

	return 0;
}

static int GetFontSizeForAllButton(void){

	int i;
	int len;
	int max_len = -1;
	int max_index = 0;
	RegionCartesian tRegionCar;
	float k, kx, ky;

	/* 1. 找出name最长的Button */
	for (i = 0; i < g_tButtonCnt; i++)
	{
		len = strlen(g_tButtons[i].name);
		if(len > max_len){

			max_len = len;
			max_index = i;
		}
	}

	/* 2. 以font_size = 100 , 算出它的外框 */
	SetFontSize(100);
	GetStringRegionCar(g_tButtons[max_index].name, &tRegionCar);

	/* 3. 把文字的外框缩放为 Button 的外框 */
	kx = (float)g_tButtons[max_index].tRegion.iWidth / tRegionCar.iWidth;
	ky = (float)g_tButtons[max_index].tRegion.iHeigh / tRegionCar.iHeigh;
	k = (kx < ky ? kx : ky);

	/* 4. 反算出font_size,    只取0.80，避免文字过于接近边界   */
	return k * 100 * 0.8;
}

static void GenerateButtons(){

	int n;
	PDispBuff pDispBuff;
	int xres, yres;
	int width, height;
	int n_per_line;
	int col;
	int row, rows;
	int start_x, start_y;
	PButton pButton;
	int i = 0;
	int pre_start_x, pre_start_y;
	int iFontSize;

	/* 算出单个按钮的width/height */
	g_tButtonCnt = n = GetItemCfgCount();

	pDispBuff = GetDisplayBuffer();
	xres = pDispBuff->iXres;
	yres = pDispBuff->iYres;
	width = sqrt(1.0 / 0.618 * xres * yres / n);
	n_per_line = xres / width + 1;
	width = xres / n_per_line;
	height = width * 0.618;
	
	/* 居中显示:  计算每个按钮的region  */
	start_x = (xres - width * n_per_line) / 2;
	rows = n / n_per_line;
	if (rows * n_per_line < n)
	{
		rows++;
	}
	
	start_y = (yres - height * rows) / 2;

	/* 计算每个按钮的region */
	for(row = 0; (row < rows) && (i < n); row++){

		pre_start_y = start_y + row * height;
		pre_start_x = start_x;
		for (col = 0; (col < n_per_line) && (i < n); col++)
		{	
			pButton = &g_tButtons[i];
			pButton->tRegion.iLeftUpX = pre_start_x;
			pButton->tRegion.iLeftUpY = pre_start_y;
			pButton->tRegion.iWidth   = width - X_GAP;
			pButton->tRegion.iHeigh   = height - Y_GAP;
			pre_start_x = pButton->tRegion.iLeftUpX + width;

			/* InitButton */
			InitButton(pButton, GetItemCfgByIndex(i)->name, NULL, NULL, MainPageOnPressed);
			
			i++;
		}
	}

	iFontSize = GetFontSizeForAllButton();

	/* OnDraw */
	for (i = 0; i < n; i++)
	{			
		g_tButtons[i].iFontSize = iFontSize;
		g_tButtons[i].OnDraw(&g_tButtons[i], pDispBuff);
	}
}

static int isTouchPointInRegion(int iX, int iY, PRegion ptRegion){

	if (iX < ptRegion->iLeftUpX || iX > ptRegion->iLeftUpX + ptRegion->iWidth)
	{
		return 0;
	}

	if (iY < ptRegion->iLeftUpY || iY > ptRegion->iLeftUpY + ptRegion->iHeigh)
	{
		return 0;
	}

	return 1;
}

static PButton GetButtonByInputEvent(PInputEvent ptInputEvent){

	int i;
	char name[100];

	if(ptInputEvent->iType == INPUT_TYPE_TOUCH){

		for (i = 0; i < g_tButtonCnt; i++)
		{
			if (isTouchPointInRegion(ptInputEvent->iX, ptInputEvent->iY, &g_tButtons[i].tRegion))
			{
				return &g_tButtons[i];
			}
		}

	}else if(ptInputEvent->iType == INPUT_TYPE_NET){

		for(i = 0; i < g_tButtonCnt; i++){

			sscanf(ptInputEvent->str, "%s", name);
			if (strcmp(name, g_tButtons[i].name) == 0)
			{
				return &g_tButtons[i];
			}
		}

	}

	return NULL;
}

static void MainPageRun(void *pParams){

	int error;
	InputEvent tInputEvent;
	PButton ptButton;
	PDispBuff ptDispBuff = GetDisplayBuffer();

	/* 读取配置文件 */	
	error = ParseConfigFile();
	if (error)
	{
		return ;
	}

	/* 根据配置文件生成按钮、界面 */
	GenerateButtons();
	

	while (1)
	{
		/* 读取输入事件 */
		error = GetInputEvent(&tInputEvent);
		if(error){
			
			continue;
		}

		/* 根据输入事件找到按钮 */
		ptButton = GetButtonByInputEvent(&tInputEvent);
		if(!ptButton){
			
			continue;
		}

		/* 调用按钮的OnPressed函数 */
		ptButton->OnPressed(ptButton, ptDispBuff, &tInputEvent);
	}

}

static PageAction g_tMainPage = {

	.name = "main",
	.Run  = MainPageRun,
};


void MainPageRegister(void){

	PageRegister(&g_tMainPage);
}
