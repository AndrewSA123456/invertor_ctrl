#ifndef __BACKGROUNDTASKS_H
#define __BACKGROUNDTASKS_H
#include "globals.h"

/////////////////////////////////////////////////////////////////////
// Поток:
gpointer backgroundTasksThread(gpointer);
/////////////////////////////////////////////////////////////////////
// Поток: принимает сообщения по CAN
gpointer socketCANReceiveThread(gpointer data);
/////////////////////////////////////////////////////////////////////
// Поток: передает сообщения по CAN
gpointer socketCANTransmitThread(gpointer data);



#endif //__BACKGROUNDTASKS_H