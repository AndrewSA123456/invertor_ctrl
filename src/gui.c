#include "gui.h"
///////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------Глобальные переменные
///////////////////////////////////////////////////////////////////////////////////////////////
GtkWidget *main_window_app = NULL;
///////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------Главные функции приложения
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: активация приложения
gpointer activate_app(GtkApplication *app, gpointer user_data)
{
	main_window_app = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(main_window_app), "Отладочный стенд");
	// gtk_window_maximize(GTK_WINDOW(main_window_app));
	gtk_window_set_position(GTK_WINDOW(main_window_app), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(main_window_app), 10);
	gtk_widget_set_name(main_window_app, "main_window_app");
	g_signal_connect(G_OBJECT(main_window_app), "destroy", G_CALLBACK(delete_event_handler), app);

	GtkWidget *window_app_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(window_app_vbox), 0);
	gtk_container_add(GTK_CONTAINER(main_window_app), window_app_vbox);

	GtkWidget *stack_switcher_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_halign(stack_switcher_vbox, GTK_ALIGN_CENTER);
	gtk_box_pack_start(GTK_BOX(window_app_vbox), stack_switcher_vbox, FALSE, FALSE, 10);

	GtkWidget *stack_switcher = gtk_stack_switcher_new();
	gtk_box_pack_start(GTK_BOX(stack_switcher_vbox), stack_switcher, TRUE, TRUE, 0);

	GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_name(separator, "separator");
	gtk_box_pack_start(GTK_BOX(window_app_vbox), separator, FALSE, FALSE, 0);

	GtkWidget *stack_switcher_stack = gtk_stack_new();
	gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(stack_switcher), GTK_STACK(stack_switcher_stack));
	gtk_box_pack_start(GTK_BOX(window_app_vbox), stack_switcher_stack, TRUE, TRUE, 0);

	GtkWidget *ctrl_window_stack_switcher_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_titled(GTK_STACK(stack_switcher_stack),
						 ctrl_window_stack_switcher_page,
						 "page1",
						 "Управление инвертором");
	GtkWidget *ctrl_window_vbox = create_ctrl_window_vbox(ctrl_window_stack_switcher_page);
	gtk_box_pack_start(GTK_BOX(ctrl_window_stack_switcher_page), ctrl_window_vbox, TRUE, TRUE, 0);

	GtkWidget *bootloader_stack_switcher_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_titled(GTK_STACK(stack_switcher_stack),
						 bootloader_stack_switcher_page,
						 "page2",
						 "Перепрошивка инвертора");
	GtkWidget *bootloader_vbox = create_bootloader_vbox(bootloader_stack_switcher_page);
	gtk_box_pack_start(GTK_BOX(bootloader_stack_switcher_page), bootloader_vbox, TRUE, TRUE, 0);

	GtkWidget *settings_stack_switcher_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_titled(GTK_STACK(stack_switcher_stack),
						 settings_stack_switcher_page,
						 "page3",
						 "Настройки");
	GtkWidget *settings_vbox = create_settings_vbox(settings_stack_switcher_page);
	gtk_box_pack_start(GTK_BOX(settings_stack_switcher_page), settings_vbox, TRUE, TRUE, 0);

	gtk_widget_show_all(main_window_app);
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: закрытие приложения
gpointer delete_event_handler(GtkWidget *widget, gpointer data)
{
	g_application_quit(G_APPLICATION(data));
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: использовать styles.css
gpointer use_css()
{
	GtkCssProvider *provider;
	GdkDisplay *display;
	GdkScreen *screen;

	provider = gtk_css_provider_new();
	display = gdk_display_get_default();
	screen = gdk_display_get_default_screen(display);
	gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	GError *error = 0;
	gtk_css_provider_load_from_file(provider, g_file_new_for_path("others/styles.css"), &error);
	g_object_unref(provider);
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: Создать окно с органами управления инвертором
GtkWidget *create_ctrl_window_vbox(GtkWidget *ctrl_window_stack_switcher_page)
{
	GtkWidget *local_ctrl_window_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	return local_ctrl_window_vbox;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: Создать окно с бутлоадером инвертора
GtkWidget *create_bootloader_vbox(GtkWidget *bootloader_stack_switcher_page)
{
	GtkWidget *local_bootloader_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	GtkWidget *bootloader_frame_1 = gtk_frame_new(NULL);
	gtk_widget_set_name(bootloader_frame_1, "frame");
	gtk_box_pack_start(GTK_BOX(local_bootloader_vbox), bootloader_frame_1, FALSE, FALSE, 10);

	GtkWidget *bootloader_vbox_1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(bootloader_frame_1), bootloader_vbox_1);

	GtkWidget *bootloader_bbox_1 = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_set_border_width(GTK_CONTAINER(bootloader_bbox_1), 5);
	gtk_box_set_spacing(GTK_BOX(bootloader_bbox_1), 40);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bootloader_bbox_1), GTK_BUTTONBOX_CENTER);
	gtk_box_pack_start(GTK_BOX(bootloader_vbox_1), bootloader_bbox_1, FALSE, FALSE, 10);

	GtkWidget *bootloader_label_1 = gtk_label_new("Выберите файл с прошивкой:");
	gtk_container_add(GTK_CONTAINER(bootloader_bbox_1), bootloader_label_1);

	GtkWidget *bootloader_entry_1 = gtk_entry_new();
	gtk_widget_set_sensitive(bootloader_entry_1, FALSE);
	gtk_entry_set_max_width_chars(GTK_ENTRY(bootloader_entry_1), 100);
	gtk_container_add(GTK_CONTAINER(bootloader_bbox_1), bootloader_entry_1);

	GtkWidget *bootloader_button_browse = gtk_button_new_with_label("Обзор");
	g_signal_connect(bootloader_button_browse, "clicked", G_CALLBACK(open_firmware_chooser_window), bootloader_entry_1);
	gtk_container_add(GTK_CONTAINER(bootloader_bbox_1), bootloader_button_browse);

	GtkWidget *bootloader_bbox_2 = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_set_border_width(GTK_CONTAINER(bootloader_bbox_2), 5);
	gtk_box_set_spacing(GTK_BOX(bootloader_bbox_2), 40);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bootloader_bbox_2), GTK_BUTTONBOX_CENTER);
	gtk_box_pack_start(GTK_BOX(bootloader_vbox_1), bootloader_bbox_2, FALSE, FALSE, 10);

	GtkWidget *bootloader_button_encryption = gtk_check_button_new_with_label("Шифрование");
	g_signal_connect(bootloader_button_encryption, "toggled", G_CALLBACK(is_bootloader_encryption_activate), NULL);
	gtk_container_add(GTK_CONTAINER(bootloader_bbox_2), bootloader_button_encryption);

	GtkWidget *bootloader_upload_start = gtk_toggle_button_new_with_label("Загрузить прошивку");
	g_signal_connect(bootloader_upload_start, "toggled", G_CALLBACK(upload_inverter_firmware), NULL);
	gtk_container_add(GTK_CONTAINER(bootloader_bbox_2), bootloader_upload_start);

	return local_bootloader_vbox;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: Создать окно с настройками приложения
GtkWidget *create_settings_vbox(GtkWidget *settings_stack_switcher_page)
{
	GtkWidget *local_settings_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	return local_settings_vbox;
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: открыть окно выбора прошивки
gpointer open_firmware_chooser_window(GtkWidget *widget, gpointer data)
{
	GtkFileChooserDialog *dialog;
	gint res;

	dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Выберите файл",
																 GTK_WINDOW(main_window_app),
																 GTK_FILE_CHOOSER_ACTION_OPEN,
																 "_Отмена", GTK_RESPONSE_CANCEL,
																 "_Выбрать", GTK_RESPONSE_ACCEPT,
																 NULL));
	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if (res == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		gchar *filename = gtk_file_chooser_get_filename(chooser);
		size_t file_name_size = strlen(filename);
		firmware_file_name = g_realloc(firmware_file_name, (file_name_size + 1) * sizeof(gchar));
		g_strlcpy(firmware_file_name, filename, file_name_size + 1);
		gtk_entry_set_text(GTK_ENTRY(data), firmware_file_name);
		gtk_entry_set_width_chars(GTK_ENTRY(data), file_name_size);
		gtk_widget_show_all(main_window_app);
		g_async_queue_push(Background.Queue, create_task(TASK_PARSE_INTEL_HEX, 0, NULL));
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: активация шифрования
gpointer is_bootloader_encryption_activate(GtkWidget *widget, gpointer data)
{
	gboolean bootloader_encryption_activate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if(bootloader_encryption_activate)
	{
		g_async_queue_push(Background.Queue, create_task(TASK_ENCRYPT_FIRMWARE, 0, NULL));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////
// Функция: загрузить прошивку в инвертор
gpointer upload_inverter_firmware(GtkWidget *widget, gpointer data)
{
	gboolean upload_inverter_firmware = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (upload_inverter_firmware)
	{
		g_async_queue_push(Background.Queue, create_task(TASK_TRANSMIT_FIRMWARE, 0, NULL));
	}
}
