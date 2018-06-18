#include "pilab-popup.h"
#include "pilab-string.h"

/*
 * Conjure up a new popup.
 *
 * Returns a pointer to a new popup, or NULL otherwise.
 */

struct t_popup *
	popup_create(t_popup_on_button_clicked *callback_on_button_clicked)
{
	struct t_popup *popup;

	popup = malloc(sizeof(*popup));

	if (callback_on_button_clicked)
		popup->callback_on_button_clicked = callback_on_button_clicked;
	else
		popup->callback_on_button_clicked =
			&popup_on_button_clicked_default_cb;

	return (popup) ? popup : NULL;
}

/*
 * Initialise the popup.
 *
 * This function is expected to parser argc and argv from the main() function,
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

	gtk_builder_add_from_file(popup->builder, "popup.glade", NULL);

	popup->window = GTK_WIDGET(
		gtk_builder_get_object(popup->builder, "window_main"));
	popup->button = GTK_WIDGET(gtk_builder_get_object(
		popup->builder, "classroom_register_button"));
	popup->entry = GTK_WIDGET(gtk_builder_get_object(
		popup->builder, "classroom_entry_field"));
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
			 G_CALLBACK(popup_destroy), NULL);
	/* when you press the button */
	g_signal_connect(G_OBJECT(popup->button), "clicked",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);
	/* when you press ENTER */
	g_signal_connect(G_OBJECT(popup->entry), "activate",
			 G_CALLBACK(popup->callback_on_button_clicked), popup);

	g_object_unref(popup->builder);
	gtk_widget_show(popup->window);
	gtk_main();
}

/*
 * Functionality that will be executed on entering and clicking the register
 * button (default callback).
 */

void popup_on_button_clicked_default_cb(GtkButton *button, gpointer data)
{
	struct t_popup *popup;
	char *string;

	if (!button || !data)
		return;

	popup = (struct t_popup *)data;

	string = (char *)gtk_entry_get_text(GTK_ENTRY(popup->entry));

	if (string_strcmp(string, "") == 0) {
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

	gtk_widget_destroy(GTK_WIDGET(popup->button));
	gtk_widget_destroy(GTK_WIDGET(popup->entry));
	gtk_widget_destroy(GTK_WIDGET(popup->window));
	free(popup);
	gtk_main_quit();
}
