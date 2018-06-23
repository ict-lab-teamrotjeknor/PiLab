#include "pilab-popup.h"
#include "pilab-string.h"
#include "pilab-log.h"

static const char *glade_file_paths[] = {
	SYSCONFDIR "/pilab/popup.glade",
};

/*
 * Conjure up a new popup, with the possibility to set a default callback.
 *
 * Returns a pointer to a new popup, or NULL otherwise.
 */

struct t_popup *popup_create_custom(
	struct t_pilab_config *config,
	t_popup_on_button_clicked *callback_on_button_clicked)
{
	struct t_popup *popup;

	if (!config)
		return NULL;

	popup = malloc(sizeof(*popup));
	if (callback_on_button_clicked)
		popup->callback_on_button_clicked = callback_on_button_clicked;
	else
		popup->callback_on_button_clicked =
			&popup_on_button_clicked_default_cb;

	popup->config = config;

	if (popup_try_open_glade_file(popup, 0) < 1)
		free(popup);

	return (popup) ? popup : NULL;
}

/*
 * Conjure up a new popup.
 *
 * Returns a pointer to a new popup, or NULL otherwise.
 */

struct t_popup *popup_create(struct t_pilab_config *config)
{
	return popup_create_custom(config, NULL);
}

/*
 * Initialise the popup.
 *
 * This function is expected to parse argc and argv from the main() function,
 * but it is possible to pass NULL, if argv is not available.
 *
 * NOTE: This needs to be called before any other popup functions.
 */

void popup_init(struct t_popup *popup, int *argc, char ***argv)
{
	if (!popup || !argc || !argv)
		return;

	gtk_init(argc, argv);

	popup->builder = gtk_builder_new();

	gtk_builder_add_from_file(popup->builder, popup->glade_file, NULL);

	popup->window = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "window_main"));
	popup->entry_classroom = GTK_WIDGET(gtk_builder_get_object(
		popup->builder, "classroom_entry_field"));
	popup->entry_email = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "email_entry_field"));
	popup->entry_password = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "password_entry_field"));
	popup->entry_address = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "address_entry_field"));
	popup->entry_port = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "port_entry_field"));
	popup->button = GTK_WIDGET(gtk_builder_get_object(
		popup->builder, "config_register_button"));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(popup->window), 1);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(popup->window), 1);
	gtk_window_set_type_hint(GTK_WINDOW(popup->window),
				 GDK_WINDOW_TYPE_HINT_DIALOG);
}

/*
* Try to open glade file.
*
* Returns:
* -1: if glade file could not be found.
*  0: glade file could not be opened.
*  1: if glade file found.
*/

int popup_try_open_glade_file(struct t_popup *popup, int index)
{
	FILE *file;

	if (!popup)
		return -1;

	file = fopen(glade_file_paths[index], "r");
	if (file) {
		popup->glade_file = glade_file_paths[index];
		fclose(file);
		return 1;
	}

	return 0;
}

/*
 * Show the popup.
 */

void popup_show(struct t_popup *popup)
{
	if (!popup)
		return;

	gtk_builder_connect_signals(popup->builder, NULL);
	g_signal_connect(G_OBJECT(popup->window), "destroy",
			 G_CALLBACK(popup_create_exiting_program_dialog), NULL);
	/* when you press the button */
	g_signal_connect(G_OBJECT(popup->button), "clicked",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry_classroom), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry_address), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry_email), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry_password), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry_port), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);

	g_object_unref(popup->builder);
	gtk_widget_show(popup->window);
	gtk_main();
}

/*
 * Create an exit warning/error.
 */

void popup_create_exiting_program_dialog()
{
	GtkWidget *dialog;
	GtkDialogFlags flags;

	flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK, NULL);
	gtk_message_dialog_set_markup(
		GTK_MESSAGE_DIALOG(dialog),
		"Thanks! We've saved your settings in /etc/pilab/config!");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
	gtk_main_quit();
}

/*
 * Create an invalid fields dialog.
 */

int popup_create_invalid_fields_dialog(struct t_popup *popup, const char *msg)
{
	GtkDialogFlags flags;

	if (!popup || !msg || (string_strcmp(msg, "") == 0))
		return 0;

	flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	popup->dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK, NULL);
	gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(popup->dialog), msg);
	gtk_dialog_run(GTK_DIALOG(popup->dialog));

	return 1;
}

/*
 * Functionality that will be executed on entering and clicking the register
 * button (default callback).
 */

void popup_on_button_clicked_default_cb(GtkButton *button, gpointer data)
{
	struct t_popup *popup;
	char *string_classroom;
	char *string_email;
	char *string_password;
	char *string_address;
	char *string_port;
	char *entry_classroom;
	char *entry_email;
	char *entry_password;
	char *entry_address;
	char *entry_port;
	char *msg;
	char *post_fix_msg;

	if (!button || !data)
		return;

	popup = (struct t_popup *)data;

	string_classroom =
		(char *)gtk_entry_get_text(GTK_ENTRY(popup->entry_classroom));
	string_email =
		(char *)gtk_entry_get_text(GTK_ENTRY(popup->entry_email));
	string_password =
		(char *)gtk_entry_get_text(GTK_ENTRY(popup->entry_password));
	string_address =
		(char *)gtk_entry_get_text(GTK_ENTRY(popup->entry_address));
	string_port = (char *)gtk_entry_get_text(GTK_ENTRY(popup->entry_port));

	post_fix_msg = "";
	msg = "The following field(s) where invalid: ";

	if (string_strcmp(string_classroom, "") == 0)
		post_fix_msg = string_strcat_delimiter_recursive(
			post_fix_msg, ", ", 1, "Classroom");
	if (string_strcmp(string_email, "") == 0)
		post_fix_msg = string_strcat_delimiter_recursive(
			post_fix_msg, ", ", 1, "E-mail");
	if (string_strcmp(string_password, "") == 0)
		post_fix_msg = string_strcat_delimiter_recursive(
			post_fix_msg, ", ", 1, "Password");
	if (string_strcmp(string_address, "") == 0)
		post_fix_msg = string_strcat_delimiter_recursive(
			post_fix_msg, ", ", 1, "Address");
	if (string_strcmp(string_port, "") == 0)
		post_fix_msg = string_strcat_delimiter_recursive(
			post_fix_msg, ", ", 1, "Port");

	msg = (string_strcmp(post_fix_msg, "") == 0) ?
		      "" :
		      string_strcat(msg, post_fix_msg);
	/* create a popup when the fields are invalid */

	if (popup_create_invalid_fields_dialog(popup, msg)) {
		free(post_fix_msg);
		free(msg);
		gtk_widget_destroy(GTK_WIDGET(popup->dialog));
		return;
	} else {
		entry_classroom = string_strcat("classroom ", string_classroom);
		config_append_to_configuration(popup->config, entry_classroom,
					       1);

		entry_email = string_strcat("email ", string_email);
		config_append_to_configuration(popup->config, entry_email, 1);

		entry_password = string_strcat("password ", string_password);
		config_append_to_configuration(popup->config, entry_password,
					       1);

		entry_address = string_strcat("address ", string_address);
		config_append_to_configuration(popup->config, entry_address, 1);

		entry_port = string_strcat("port ", string_port);
		config_append_to_configuration(popup->config, entry_port, 0);

		pilab_log(LOG_DEBUG, "After append");

		/* free the stuff */
		/* free(entry_classroom); */
		/* free(entry_email); */
		/* free(entry_password); */
		/* free(entry_address); */
		/* free(entry_port); */
		popup_destroy(popup);
	}
}

/*
 * Destroy the popup and all the widgets.
 */

void popup_destroy(struct t_popup *popup)
{
	if (!popup)
		return;

	/* gtk_widget_destroy(GTK_WIDGET(popup->button)); */
	/* gtk_widget_destroy(GTK_WIDGET(popup->entry_classroom)); */
	/* gtk_widget_destroy(GTK_WIDGET(popup->entry_email)); */
	/* gtk_widget_destroy(GTK_WIDGET(popup->entry_password)); */
	/* gtk_widget_destroy(GTK_WIDGET(popup->entry_address)); */
	/* gtk_widget_destroy(GTK_WIDGET(popup->entry_port)); */
	/* TODO: Figure out why this crashes, atm I have no idea why */

	/* gtk_widget_destroy(GTK_WIDGET(popup->dialog)); */
	gtk_widget_destroy(GTK_WIDGET(popup->window));
	gtk_main_quit();
}
