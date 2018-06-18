#ifndef _PILAB_POPUP_H
#define _PILAB_POPUP_H
#include <gtk/gtk.h>

typedef void(t_popup_on_button_clicked)(GtkButton *button, gpointer data);

struct t_popup {
	GtkBuilder *builder;
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *entry;
	t_popup_on_button_clicked *callback_on_button_clicked;
};

extern struct t_popup *
	popup_create(t_popup_on_button_clicked *callback_on_button_clicked);
extern void popup_init(struct t_popup *popup, int *argc, char ***argv);
extern void popup_show(struct t_popup *popup);
extern void popup_on_button_clicked_default_cb(GtkButton *button,
					       gpointer data);
extern void popup_destroy(struct t_popup *popup);

#endif
