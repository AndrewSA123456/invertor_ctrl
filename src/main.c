#include "main.h"
#include "gui.h"
#include "backgroundTasks.h"

// Функция для создания задачи
TaskStruct *create_task(uint32_t CMD,
						uint32_t LEN,
						uint8_t *data)
{
	TaskStruct *task = g_new(TaskStruct, 1);
	task->CMD = CMD;
	task->LEN = LEN;
	task->data = (uint8_t *)g_memdup2(data, LEN);
	return task;
}
// Функция для освобождения задачи
gpointer free_task(TaskStruct *task)
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

AppThreadStruct CANOpenComm = {NULL, NULL};
AppThreadStruct Background = {NULL, NULL};

GAsyncQueue *incomingQueue = NULL;
GAsyncQueue *outgoingQueue = NULL;

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "ru_RU.UTF-8");

	CANOpenComm.Queue = g_async_queue_new();
	Background.Queue = g_async_queue_new();

	incomingQueue = g_async_queue_new();
	outgoingQueue = g_async_queue_new();

	CANOpenComm.Thread = g_thread_new("CANOpenCommThread", CANOpenCommThread, NULL);
	Background.Thread = g_thread_new("BackgroundThread", backgroundTasksThread, NULL);

	GtkApplication *app;
	int status;

	app = gtk_application_new("Andrew.InvertorCTRL", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_app), NULL);

	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	g_async_queue_push(CANOpenComm.Queue, create_task(COMM_TASK_EXIT, 0, NULL));
	g_async_queue_push(Background.Queue, create_task(TASK_EXIT, 0, NULL));

	g_thread_join(CANOpenComm.Thread);
	g_thread_join(Background.Thread);

	g_async_queue_unref(CANOpenComm.Queue);
	g_async_queue_unref(Background.Queue);

	g_async_queue_unref(incomingQueue);
	g_async_queue_unref(outgoingQueue);

	g_free(firmware_file_name);

	return status;
}