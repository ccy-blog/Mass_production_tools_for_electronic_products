

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include <input_manager.h>


extern void TouchscreenRegister(void);
extern void NetInputRegister(void);


static PInputDevice g_InputDevs = NULL;
static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;


/* start of 实现环形buffer */
#define BUFFER_LEN 32

static int g_iRead = 0;
static int g_iWrite = 0;
static InputEvent g_atInputEvents[BUFFER_LEN];

static int isInputBufferFull(void){

	return (g_iRead == (g_iWrite + 1) % BUFFER_LEN);
}

static int isInputBufferEmpty(void){

	return (g_iRead == g_iWrite);
}

static void PutInputEvenToBuffer(PInputEvent ptInputEvent){

	if(!isInputBufferFull()){

		g_atInputEvents[g_iWrite] = *ptInputEvent;
		g_iWrite = (g_iWrite + 1) % BUFFER_LEN;
	}
}

static int GetInputEvenFromBuffer(PInputEvent ptInputEvent){

	if(!isInputBufferEmpty()){

		*ptInputEvent = g_atInputEvents[g_iRead];
		g_iRead = (g_iRead + 1) % BUFFER_LEN;

		return 1;

	}else{
			
		return 0;
	}
}
/* end of 实现环形buffer */


void RegisterInputDevice(PInputDevice ptInputDev){

	 ptInputDev->ptNext = g_InputDevs;
	 g_InputDevs = ptInputDev;
}

void InputSystemRegister(void){

	/* regiseter touchscreen */
	TouchscreenRegister();

	/* regiseter netinput */
	NetInputRegister();
}

static void *input_recv_thread_func(void *data){

	PInputDevice tInputDev= (PInputDevice)data;
	InputEvent tEvent;
	int ret;
	
	while(1){

		/* 读数据 */
		ret = tInputDev->GetInputEvent(&tEvent);
		if(!ret){

			/* 保存数据 */
			pthread_mutex_lock(&g_tMutex);
			PutInputEvenToBuffer(&tEvent);

			/* 唤醒等待数据的线程 */
			pthread_cond_signal(&g_tConVar);
			pthread_mutex_unlock(&g_tMutex);			
		}
	}
	return NULL;
}



void IntpuDeviceInit(void){

	int ret;
	pthread_t tid;
	PInputDevice ptTmp = g_InputDevs;
	
	while (ptTmp)
	{
			
		ret = ptTmp->DeviceInit();
		if(!ret){
		
			ret = pthread_create(&tid, NULL, input_recv_thread_func, ptTmp);
		}
		ptTmp = ptTmp->ptNext;
	}

}

int GetInputEvent(PInputEvent ptInputEvent){

	InputEvent tEvent;
	int ret;

	/* 无数据则休眠 */
	pthread_mutex_lock(&g_tMutex);
	if(GetInputEvenFromBuffer(&tEvent)){

		ret = 0;

	}else{

		/* 休眠等待 */
		pthread_cond_wait(&g_tConVar, &g_tMutex);
		if(GetInputEvenFromBuffer(&tEvent)){

			ret = 0;

		}else{
		
			ret = -1;
		}
		
	}

	*ptInputEvent = tEvent;
	pthread_mutex_unlock(&g_tMutex);
	return ret;
}



