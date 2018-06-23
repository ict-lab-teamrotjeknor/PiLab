#ifndef _PILAB_POPUP_H
#define _PILAB_POPUP_H
#include <gtk/gtk.h>
#include "pilab-config.h"

typedef void(t_popup_on_button_clicked)(GtkButton *button, gpointer data);

struct t_popup {
	struct t_pilab_config *config;
	const char *glade_file;
	GtkBuilder *builder;
	GtkWidget *dialog;
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *entry_classroom;
	GtkWidget *entry_email;
	GtkWidget *entry_password;
	GtkWidget *entry_address;
	GtkWidget *entry_port;
	t_popup_on_button_clicked *callback_on_button_clicked;
};

extern struct t_popup *popup_create_custom(
	struct t_pilab_config *config,
	t_popup_on_button_clicked *callback_on_button_clicked);
extern struct t_popup *popup_create(struct t_pilab_config *config);
extern void popup_init(struct t_popup *popup, int *argc, char ***argv);
extern void popup_show(struct t_popup *popup);
extern void popup_create_exiting_program_dialog(void);
extern int popup_create_invalid_fields_dialog(struct t_popup *popup,
					      const char *msg);
extern void popup_on_button_clicked_default_cb(GtkButton *button,
					       gpointer data);
extern int popup_try_open_glade_file(struct t_popup *popup, int index);
extern void popup_destroy(struct t_popup *popup);

#endif
