﻿/*
 * XS3868.c
 * 
 * Copyright 2019 pc <pc@pc-ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
/*
Using Geany Editor
Compile with gcc -Wall -c "%f" $(pkg-config --cflags gtk+-3.0)
Link with gcc -Wall -o "%e" "%f" $(pkg-config --cflags gtk+-3.0) -lpthread -lm $(pkg-config --libs gtk+-3.0)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <locale.h>

#define msglength 20 // rx tx max message length
#define cmdheader "AT#"
#define crlf "\r\n"  // carriage return, line feed

GtkWidget *window;
GtkWidget *vbox;
GtkWidget *vbox1;
GtkWidget *listview;
GtkWidget *frame1;
GtkWidget *scrolled_window;
GtkListStore *store;
GtkTreeIter iter;
GtkWidget *button_box1;
GtkWidget *button1;

enum
{
	COL_TXRX = 0,
	COL_CODE,
	COL_PARM,
	COL_DESCRIPTION,
	NUM_COLS
};

typedef enum
{
	IDLE = 0,
	RUNNING
}qstatus;

typedef struct
{
	char txrx[3];
	char code[msglength];
	char parm[30];
	char desc[100];
}token;

typedef struct messagenode node;
struct messagenode
{
	node *prev;
	node *next;
	token data;
};

typedef struct
{
	node *n;
	int qLength, qMaxLength;
	qstatus status;
	pthread_mutex_t qmutex;
	pthread_cond_t qlowcond;
	pthread_cond_t qhighcond;
}messagequeue;

typedef struct
{
	char devicepath[50];
	FILE *f;
	messagequeue rq, tq;
	pthread_t rtid[2], ttid[2];
	int rret[2], tret[2];
}cp2102;

// Message Queue

void q_init(messagequeue *q, int maxLength)
{
	int ret;

	q->n = NULL;
	q->qLength = 0;
	q->qMaxLength = maxLength;

	if((ret=pthread_mutex_init(&(q->qmutex), NULL))!=0 )
		printf("qmutex init failed, %d\n", ret);

	if((ret=pthread_cond_init(&(q->qlowcond), NULL))!=0 )
		printf("qlowcond init failed, %d\n", ret);

	if((ret=pthread_cond_init(&(q->qhighcond), NULL))!=0 )
		printf("qhighcond init failed, %d\n", ret);

	q->status = RUNNING;
}

void q_add(messagequeue *q,  token *t)
{
	node *p;

	pthread_mutex_lock(&(q->qmutex));
	while (q->qLength>=q->qMaxLength)
	{
		//printf("Queue sleeping, overrun\n");
		pthread_cond_wait(&(q->qhighcond), &(q->qmutex));
	}

	p = malloc(sizeof(node));
//printf("malloc q %d\n", sizeof(node));
	if (q->n == NULL)
	{
		p->next = p;
		p->prev = p;
		q->n = p;
	}
	else
	{
		p->next = q->n;
		p->prev = q->n->prev;
		q->n->prev = p;
		p->prev->next = p;
	}
	p->data = (*t);

	q->qLength++;

	//condition = true;
	pthread_cond_signal(&(q->qlowcond)); // Should wake up *one* thread
	pthread_mutex_unlock(&(q->qmutex));
}

node* q_remove_element(node **q)
{
	node *p;

	if ((*q)->next == (*q))
	{
		p=*q;
		*q = NULL;
	}
	else
	{
		p = (*q);
		(*q) = (*q)->next;
		(*q)->prev = p->prev;
		(*q)->prev->next = (*q);
	}
	return p;
}

node* q_remove(messagequeue *q)
{
	node *p;

	pthread_mutex_lock(&(q->qmutex));
	while(!(q->n) && (q->status == RUNNING)) // queue empty
	{
		//printf("Queue sleeping, underrun\n");
		pthread_cond_wait(&(q->qlowcond), &(q->qmutex));
	}

	if (q->status == RUNNING)
	{
		p = q_remove_element(&(q->n));
		q->qLength--;

		//condition = true;
		pthread_cond_signal(&(q->qhighcond)); // Should wake up *one* thread
	}
	else
	{
		while((p = q->n))
		{
			p = q_remove_element(&(q->n));
			q->qLength--;
			free(p);
		}
	}
	pthread_mutex_unlock(&(q->qmutex));

	return p;
}

void q_signalstop(messagequeue *q)
{
	pthread_mutex_lock(&(q->qmutex));
	q->status = IDLE;
	pthread_cond_signal(&(q->qlowcond)); // Should wake up *one* thread
	pthread_mutex_unlock(&(q->qmutex));
}

void q_destroy(messagequeue *q)
{
	pthread_mutex_destroy(&(q->qmutex));
	pthread_cond_destroy(&(q->qlowcond));
	pthread_cond_destroy(&(q->qhighcond));
}

// indication processor threads

static gpointer read_indication_thread(gpointer args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	cp2102 *c = (cp2102 *)args;
	token indication;

	int i;
	char *s;

	strcpy(indication.txrx, "RX"); 
	while ((s=fgets(&(indication.code[0]), msglength, c->f)))
	{
		i = strlen(s);
		printf("read_indication_thread : %s length:%d\n", s, i);
		if (i<=1) // <newline>
		{
		}
		else // msg<newline>
		{
			s[i-1] = '\0';
			strcpy(indication.parm, s+2);
			indication.code[2] = '\0';
			indication.desc[0] = '\0';
			printf("adding %s-%s\n", indication.code, indication.parm);
			q_add(&(c->rq), &indication);
		}
	}

	c->rret[0] = 0;
	pthread_exit(&(c->rret[0]));
}

void indicator_description(token *q)
{
	if (!strcmp(q->code, "II"))
		strcpy(q->desc, "In pairing state");
	else if (!strcmp(q->code, "IJ"))
		strcpy(q->desc, "Exit pairing state");
}

gboolean indication_idle(gpointer data)
{
	node *q = (node *)data;
	token *t = &(q->data);

	printf("indication %s %s-%s %s\n", t->txrx, t->code, t->parm, t->desc);
	indicator_description(t);
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COL_TXRX, t->txrx, COL_CODE, t->code, COL_PARM, t->parm, COL_DESCRIPTION, t->desc, -1);

	free(q);

	return(FALSE);
}

static gpointer process_indication_thread(gpointer args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	cp2102 *c = (cp2102 *)args;
	node *q;

	while((q = q_remove(&(c->rq))))
	{
		gdk_threads_add_idle(indication_idle, (void*)q);
	}

	c->rret[1] = 0;
	pthread_exit(&(c->rret[1]));
}

void send_message(cp2102 *c, token *t)
{
	char msg[msglength];

	sprintf(msg, "%s%s%s%s", cmdheader, t->code, t->parm, crlf);
	printf("sending -%s-\n", msg);
	fputs(msg, c->f);
}

gboolean command_idle(gpointer data)
{
	node *q = (node *)data;
	token *t = &(q->data);

	printf("command %s %s%s %s\n", t->txrx, t->code, t->parm, t->desc);
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COL_TXRX, t->txrx, COL_CODE, t->code, COL_PARM, t->parm, COL_DESCRIPTION, t->desc, -1);

	free(q);

	return(FALSE);
}

static gpointer write_command_thread(gpointer args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	cp2102 *c = (cp2102 *)args;
	node *q;

	while((q = q_remove(&(c->tq))))
	{
		gdk_threads_add_idle(command_idle, (void*)q);
		send_message(c, &(q->data));
	}

	c->tret[0] = 0;
	pthread_exit(&(c->tret[0]));
}


void set_baud_rate(char* dev, speed_t baud)
{
	int fd;
	struct termios settings;

	fd = open(dev, O_RDWR | O_NONBLOCK);

	tcgetattr(fd, &settings);
	cfsetospeed(&settings, baud); // baud rate
	cfsetispeed(&settings, baud); // baud rate
	tcsetattr(fd, TCSANOW, &settings); // apply the settings
	tcflush(fd, TCOFLUSH);
	tcflush(fd, TCIFLUSH);

	close(fd);
}

void create_threads(cp2102 *c)
{
	q_init(&(c->rq), 10);
	q_init(&(c->tq), 10);

	if (!(c->f = fopen(c->devicepath, "r+")))
	{
		printf("Cannot open %s\n", c->devicepath);
	}

	int err;
	err = pthread_create(&(c->rtid[0]), NULL, &read_indication_thread, (void *)c);
	if (err)
	{}

	err = pthread_create(&(c->rtid[1]), NULL, &process_indication_thread, (void *)c);
	if (err)
	{}

	err = pthread_create(&(c->ttid[0]), NULL, &write_command_thread, (void *)c);
	if (err)
	{}
}

void terminate_threads(cp2102 *c)
{
	int i;

//	fclose(c->f);
//	if ((i=pthread_join(c->rtid[0], NULL)))
//		printf("pthread_join error, c->rtid[0], %d\n", i);

	q_signalstop(&(c->rq));
	if ((i=pthread_join(c->rtid[1], NULL)))
		printf("pthread_join error, c->rtid[1], %d\n", i);

	q_destroy(&(c->rq));


	q_signalstop(&(c->tq));
	if ((i=pthread_join(c->ttid[0], NULL)))
		printf("pthread_join error, c->ttid[0], %d\n", i);

	q_destroy(&(c->tq));
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	cp2102 *c = (cp2102 *)data;

	terminate_threads(c);
	return FALSE; // return FALSE to emit destroy signal
}

static void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void realize(GtkWidget *widget, gpointer data)
{
	cp2102 *c = (cp2102 *)data;

	create_threads(c);
}

void setup_default_icon(char *filename)
{
	GdkPixbuf *pixbuf;
	GError *err;

	err = NULL;
	pixbuf = gdk_pixbuf_new_from_file(filename, &err);

	if (pixbuf)
	{
		GList *list;      

		list = NULL;
		list = g_list_append(list, pixbuf);
		gtk_window_set_default_icon_list(list);
		g_list_free(list);
		g_object_unref(pixbuf);
    	}
}

static GtkTreeModel* create_and_fill_model()
{
	store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	//gtk_list_store_append(store, &iter);
	//gtk_list_store_set(store, &iter, COL_TXRX, "0", COL_CODE, "0", COL_PARM, "0", COL_DESCRIPTION, "0", -1);

	return GTK_TREE_MODEL(store);
}

static GtkWidget* create_view_and_model()
{
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
	GtkWidget *view;

	view = gtk_tree_view_new();

	// Column 1
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, "ID", renderer, "text", COL_TXRX, NULL);

	// Column 2
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), -1, "Code", renderer, "text", COL_CODE, NULL);

	// Column 3
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), -1, "Parameter", renderer, "text", COL_PARM, NULL);

	// Column 4
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), -1, "Description", renderer, "text", COL_DESCRIPTION, NULL);

	model = create_and_fill_model();

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

/* The tree view has acquired its own reference to the model, so we can drop ours. That way the model will
   be freed automatically when the tree view is destroyed */
	g_object_unref(model);

	return view;
}

static void button1_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	strcpy(t.txrx, "TX");
	strcpy(t.code, "CA");
	t.parm[0] = '\0';
	strcpy(t.desc, "Set pairing, waiting for the remote device to connect. Discoverable for 2 min.");
printf("adding\n");
	q_add(&(c->tq), &t);
printf("added command %s %s %s\n", t.txrx, t.code, t.parm);
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "tr_TR.UTF-8");
	setup_default_icon("./images/XS3868.jpeg");

	cp2102 c;
	strcpy(c.devicepath, "/dev/ttyUSB0");
	set_baud_rate(c.devicepath, B115200);

	/* This is called in all GTK applications. Arguments are parsed
	 * from the command line and are returned to the application. */
	gtk_init(&argc, &argv);

	/* create a new window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	//gtk_widget_set_size_request(window, 100, 100);
	gtk_window_set_title(GTK_WINDOW(window), "XS3868");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	/* When the window is given the "delete-event" signal (this is given
	* by the window manager, usually by the "close" option, or on the
	* titlebar), we ask it to call the delete_event () function
	* as defined above. The data passed to the callback
	* function is NULL and is ignored in the callback function. */
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), (void*)&c);
    
	/* Here we connect the "destroy" event to a signal handler.  
	* This event occurs when we call gtk_widget_destroy() on the window,
	* or if we return FALSE in the "delete-event" callback. */
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), (void*)&c);

	g_signal_connect(window, "realize", G_CALLBACK(realize), (void*)&c);

// vertical box
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(window), vbox);

// command - indicator frame
	frame1 = gtk_frame_new("Commands / Indications");
	gtk_container_add(GTK_CONTAINER(vbox), frame1);

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(frame1), vbox1);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_set_size_request(scrolled_window, 300, 100);
	gtk_container_add(GTK_CONTAINER(vbox1), scrolled_window);

	listview = create_view_and_model();
	gtk_container_add(GTK_CONTAINER(scrolled_window), listview);

// buttonbox
	button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button1 = gtk_button_new_with_label("Set Pairing");
	g_signal_connect(GTK_BUTTON(button1), "clicked", G_CALLBACK(button1_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button1);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}