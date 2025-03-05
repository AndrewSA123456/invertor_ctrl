#ifndef __GUI_H
#define __GUI_H

// #warning "g_idle_add(update_label, update_data);\
// gboolean update_label(gpointer data)\
// {\
// return G_SOURCE_REMOVE;\
// }"

#include "globals.h"
/////////////////////////////////////////////////////////////////////
//---------------------Главные функции приложения
/////////////////////////////////////////////////////////////////////
// Функция: активация приложения
gpointer activate_app(GtkApplication *app, gpointer user_data);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: использовать styles.css
gpointer use_css();
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: закрытие приложения
gpointer delete_event_handler(GtkWidget *widget, gpointer data);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Создать окно с органами управления инвертором
GtkWidget *create_ctrl_window_vbox(
	GtkWidget *ctrl_window_stack_switcher_page);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Создать окно с бутлоадером инвертора
GtkWidget *create_bootloader_vbox(
	GtkWidget *bootloader_stack_switcher_page);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: Создать окно с настройками приложения
GtkWidget *create_settings_vbox(
	GtkWidget *settings_stack_switcher_page);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: открыть окно выбора прошивки
gpointer open_firmware_chooser_window(
	GtkWidget *widget,
	gpointer data);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: активация шифрования
gpointer is_bootloader_encryption_activate(
	GtkWidget *widget,
	gpointer data);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
/////////////////////////////////////////////////////////////////////
// Функция: загрузить прошивку в инвертор
gpointer upload_inverter_firmware(
	GtkWidget *widget,
	gpointer data);
//-----DEFINES
//-----STRUCTS
//-----EXTERN VARIABLES
#endif //__GUI_H