#include "main.h"
#include "SocketCAN.h"
#include "gui.h"
#include "backgroundTasks.h"

// Функция для создания задачи
InterThreadTaskStruct *create_task(uint16_t CMD,
								   uint16_t LEN,
								   uint8_t *data)
{
	InterThreadTaskStruct *task = g_new(InterThreadTaskStruct, 1);
	task->CMD = CMD;
	task->LEN = LEN;
	task->data = (uint8_t *)g_memdup2(data, LEN);
	return task;
}
// Функция для освобождения задачи
gpointer free_task(InterThreadTaskStruct *task)
{
	if (task)
	{
		user_free(task->data);
		user_free(task);
	}
}
void user_free(gpointer mem)
{
	if (mem != NULL)
	{
		g_free(mem);
		mem = NULL;
	}
}

gchar *firmware_file_name = NULL;

AppThreadStruct CANRx = {NULL, NULL};
AppThreadStruct CANTx = {NULL, NULL};
AppThreadStruct Background = {NULL, NULL};

GAsyncQueue *CANReceiveMSGQueue = NULL;
int main(int argc, char **argv)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");

	CANRx.Queue = g_async_queue_new();
	CANTx.Queue = g_async_queue_new();
	Background.Queue = g_async_queue_new();
	CANReceiveMSGQueue = g_async_queue_new();

	CANRx.Thread = g_thread_new("ReceiveSocketCAN", socketCANReceiveThread, NULL);
	CANTx.Thread = g_thread_new("TransmitSocketCAN", socketCANTransmitThread, NULL);
	Background.Thread = g_thread_new("BackgroundThread", backgroundTasksThread, NULL);

	GtkApplication *app;
	int status;

	app = gtk_application_new("Andrew.InvertorCTRL", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_app), NULL);

	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	g_async_queue_push(CANRx.Queue, create_task(RECEIVE_TASK_EXIT, 0, NULL));
	g_async_queue_push(CANTx.Queue, create_task(TRANSMIT_TASK_EXIT, 0, NULL));
	g_async_queue_push(Background.Queue, create_task(TASK_EXIT, 0, NULL));

	g_thread_join(CANRx.Thread);
	g_thread_join(CANTx.Thread);
	g_thread_join(Background.Thread);

	g_async_queue_unref(CANRx.Queue);
	g_async_queue_unref(CANTx.Queue);
	g_async_queue_unref(Background.Queue);
	g_async_queue_unref(CANReceiveMSGQueue);
	
	g_free(firmware_file_name);

	return status;
}