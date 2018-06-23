#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200112L
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <wiringPi.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include "pilab-string.h"
#include "ds18b20.h"
#include "pilab-config.h"
#include "pilab-log.h"
#include "pilab-popup.h"
#include "pilab-host-device.h"
#include "pilab-api-client.h"
#include "pilab-api-calls.h"
#include "pilab-json-parser.h"
#include "pilab-gpio-device.h"
#include "pilab-lcd.h"
#include "pilab-time.h"

struct t_pilab_config *pilab_config(char *config_file_path)
{
	struct t_pilab_config *config;
	config = config_create_custom(config_file_path);
	if (!config) {
		pilab_log(LOG_ERROR,
			  "Could not create a pilab config instance.");
		exit(EXIT_FAILURE);
	}
	return config;
}

struct t_api_client *pilab_client(struct t_pilab_config *config)
{
	struct t_api_client *client;
	/* create a new client */
	client = api_client_create(config);
	if (!client) {
		pilab_log(LOG_ERROR, "Could not create a api client instance.");
		exit(EXIT_FAILURE);
	}
	return client;
}

/* A special feature for MR Minuto */
void pilab_grandma_needs_a_prompt(struct t_pilab_config *config, int *argc,
				  char ***argv)
{
	struct t_popup *popup;
	/* first read of config */
	if (config_read_configuration_file(config)) {
		/* check if the config file has the necessary configrations */
		if (config_has_necessary_configurations(config) < 1) {
			popup = popup_create(config);
			if (!popup) {
				pilab_log(LOG_ERROR,
					  "Could not create a popup instance.");
				exit(EXIT_FAILURE);
			}
			popup_init(popup, argc, argv);

			/* wipe the config */
			/* config_clear_file(config); */
			/* prompt again */
			popup_show(popup);
		}
	}
}

void pilab_read_config(struct t_pilab_config *config)
{
	/* read configuration (again, we need the newly added values) */
	if (config_read_configuration_file(config)) {
		if (config_has_necessary_configurations(config) < 1) {
			pilab_log(
				LOG_ERROR,
				"Double reading of configuration file resulted in no results, please make sure there is a valid config and read/write operations are properly executed!");
			exit(EXIT_FAILURE);
		}
	}
}

struct t_host_device *pilab_host()
{
	struct t_host_device *new_host;

	new_host = host_device_create();
	if (!new_host) {
		pilab_log(LOG_ERROR,
			  "Could not create a host device instance.");
		exit(EXIT_FAILURE);
	}

	host_device_read_in_sensor_modules(new_host);

	return new_host;
}

void *pilab_worker(void *arg)
{
	struct t_api_client *client, *new_client;
	struct t_pilist *arg_list;
	struct t_host_device *host;
	struct t_slave_device *slave;
	int analog_value, digital_value;
	float value;
	char char_value[20];

	arg_list = (struct t_pilist *)arg;
	slave = (struct t_slave_device *)pilist_get_data(arg_list, 0);
	client = (struct t_api_client *)pilist_get_data(arg_list, 1);
	host = (struct t_host_device *)pilist_get_data(arg_list, 2);

	if (!client) {
		pilab_log(
			LOG_ERROR,
			"Could not find client in the thread pilab_worker for %s",
			slave->get_name(slave->instance));
		hashtable_free(host->slave_devices_lookup);
		if (host->lcd)
			lcd_free(host->lcd);
		free(host);
		pilist_free(arg_list);
		exit(EXIT_FAILURE);
	}

	new_client = api_client_create(client->config);
	api_client_set_cookie(new_client, client->cookie);

	analog_value = slave->analog_read(slave->instance,
					  slave->get_pinbase(slave->instance));
	digital_value = slave->digital_read(
		slave->instance, slave->get_pinbase(slave->instance));

	if (analog_value >= digital_value) {
		value = analog_value;
		if (string_strcmp(slave->get_name(slave->instance),
				  "ds18b20") == 0)
			value = 1.0 * analog_value / 10;
	} else {
		value = digital_value;
	}

	/* make it a 1 precision floating point number */
	sprintf(char_value, "%.1f", value);

	pilab_add_data(new_client, char_value);

	api_client_free_minimal(new_client);

	pilab_log(LOG_DEBUG, "Adding reading: %s", char_value);

	return 0;
}

/* Go up in Day,Week,Month,Year (Keyboard letter "W") (keycode: 119) */
/* Go down in Day,Week,Month,Year (Keyboard letter "S") (keycode: 115) */
/* Scroll up 50 pixels (Keyboard letter "R") (keycode: 114) */
/* Scroll down 50 pixels (Keyboard letter "F") (keycode 102) */
/* Go to previous Day,Week,Month,Year (Keyboard letter "A") (keycode: 97) */
/* Go to next Day,Week,Month,Year (Keyboard letter "D") (keycode: 100) */

void *detect_press()
{
	Display *display;
	unsigned int keycode;
	int pins[] = { 6, 13, 17, 19, 22, 26 };

	for (int i = 0; i < 4; i++)
		pinMode(pins[i], INPUT);

	display = XOpenDisplay(NULL);

	while (1) {
		if (digitalRead(6) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_w);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 6);
			sleep(1.5);
		} else if (digitalRead(13) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_s);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 13);
			sleep(1);
		} else if (digitalRead(17) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_d);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 17);
			sleep(1);
		} else if (digitalRead(19) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_r);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 19);
			sleep(1);
		} else if (digitalRead(22) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_a);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 22);
			sleep(1);
		} else if (digitalRead(26) == HIGH) {
			keycode = XKeysymToKeycode(display, XK_f);
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
			XFlush(display);
			pilab_log(LOG_DEBUG, "button number %d pressed\n", 26);
			sleep(1);
		}
	}

	return 0;
}

#define URL_INFIX_1 "/Schedule/Index?roomName="
#define URL_INFIX_2 "&view=ScheduleViewDay&dateTime="
#define WEBSITE_PORT "8081"

int main(int argc, char *argv[])
{
	static int verbose = 0, debug = 0, exit_value = 0;

	static struct option long_options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "config", required_argument, NULL, 'c' },
		{ "debug", no_argument, NULL, 'd' },
		{ "version", no_argument, NULL, 'v' },
		{ "verbose", no_argument, NULL, 'V' },
		{ 0, 0, 0, 0 }
	};

	char *config_path = NULL;

	const char *usage =
		"Usage: pilab [options] [command]\n"
		"\n"
		"  -h, --help             Show help message and quit.\n"
		"  -c, --config <config>  Specify a config file.\n"
		"  -d, --debug            Enables full logging, including debug information.\n"
		"  -v, --version          Show the version number and quit.\n"
		"  -V, --verbose          Enables more verbose logging.\n"
		"\n";

	int c;
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hCdDvVc:", long_options,
				&option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h': /* help */
			fprintf(stdout, "%s", usage);
			exit(EXIT_SUCCESS);
			break;
		case 'c': /* config */
			config_path = string_strdup(optarg);
			break;
		case 'd': /* debug */
			debug = 1;
			break;
		case 'v': /* version */
			fprintf(stdout, "pilab version " PILAB_VERSION "\n");
			exit(EXIT_SUCCESS);
			break;
		case 'V': /* verbose */
			verbose = 1;
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(EXIT_FAILURE);
		}
	}

	if (debug) {
		pilab_log_init(LOG_DEBUG);
	} else if (verbose) {
		pilab_log_init(LOG_INFO);
	} else {
		pilab_log_init(LOG_ERROR);
	}

	/* initialisation of the program */
	struct t_pilab_config *config;
	struct t_api_client *client;
	struct t_host_device *host;
	struct t_pilist *sensor_list;

	config = pilab_config(config_path);
	client = pilab_client(config);

	pilab_grandma_needs_a_prompt(config, &argc, &argv);
	pilab_read_config(config);

	/* needs to be called before calling pilab_host */
	wiringPiSetupGpio();
	host = pilab_host();
	/* end initialisation of the program */

	/* start work here */
	pilab_login(client);
	pilab_add_pi(client);

	sensor_list = host_device_get_sensor_name_list(host);

	/*
	 * +1 we need a thread for detecting the key presses.
	 */
	const int num_threads = sensor_list->size + 1;
	pthread_t devices[num_threads];

	char *cmd = "";
	cmd = string_strcat_delimiter_recursive(
		cmd, "", 12, "/usr/bin/chromium --no-sandbox ", "'",
		config->address, ":", WEBSITE_PORT, URL_INFIX_1,
		config->classroom, URL_INFIX_2, time_get_time_fmt("%Y-%m-%d"),
		"'", "&");
	pilab_log(LOG_DEBUG, "Executing command: %s", cmd);

	system(cmd);

	if (pthread_create(&devices[0], NULL, &detect_press, NULL)) {
		pilab_log(
			LOG_ERROR,
			"Failed to create a thread for detecting key presses, exiting...");
		goto cleanup;
	}

	while (1) {
		for (int i = 1; i < num_threads; i++) {
			struct t_slave_device *slave;
			char *slave_name;
			slave_name = pilist_get_data(sensor_list, i - 1);
			struct t_pilist *args;
			args = pilist_create();
			if (!args) {
				pilab_log(
					LOG_ERROR,
					"Could not create argument list for thread, exiting...");
				exit_value = EXIT_FAILURE;
				goto cleanup;
			}

			slave = (struct t_slave_device *)hashtable_get(
				host->slave_devices_lookup, slave_name);

			pilist_add(args, host);
			pilist_add_pointer(args, client);
			pilist_add_pointer(args, slave);

			if (pthread_create(&devices[i], NULL, pilab_worker,
					   args)) {
				pilab_log(
					LOG_ERROR,
					"Failed to create a thread for device :%s",
					slave_name);
				exit_value = EXIT_FAILURE;
				goto cleanup;
			}
		}
		/* sleep 5 * one minute */
		sleep(5 * 60);
	}

	return exit_value;

cleanup:
	pilab_log(LOG_INFO, "Shutting down pilab");
	api_client_free(client);
	config_free(config);
	hashtable_free(host->slave_devices_lookup);
	if (host->lcd)
		lcd_free(host->lcd);
	free(host);
	return exit_value;
}
