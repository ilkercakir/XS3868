/*
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

GtkWidget *button_box_group1;
GtkWidget *button_box1;
GtkWidget *button1;
GtkWidget *glyphbox1;
GtkWidget *icon1;
GtkWidget *button13;
GtkWidget *glyphbox13;
GtkWidget *icon13;
GtkWidget *button14;
GtkWidget *glyphbox14;
GtkWidget *icon14;
GtkWidget *button15;
GtkWidget *glyphbox15;
GtkWidget *icon15;
GtkWidget *button18;
GtkWidget *glyphbox18;
GtkWidget *icon18;
GtkWidget *button19;
GtkWidget *glyphbox19;
GtkWidget *icon19;
GtkWidget *button20;
GtkWidget *glyphbox20;
GtkWidget *icon20;
GtkWidget *button21;
GtkWidget *glyphbox21;
GtkWidget *icon21;
GtkWidget *button22;
GtkWidget *glyphbox22;
GtkWidget *icon22;
GtkWidget *button23;
GtkWidget *glyphbox23;
GtkWidget *icon23;
GtkWidget *button24;
GtkWidget *glyphbox24;
GtkWidget *icon24;
GtkWidget *button25;
GtkWidget *glyphbox25;
GtkWidget *icon25;
GtkWidget *button26;
GtkWidget *glyphbox26;
GtkWidget *icon26_1;
GtkWidget *icon26_2;
GtkWidget *button27;
GtkWidget *glyphbox27;
GtkWidget *icon27_1;
GtkWidget *icon27_2;
GtkWidget *button28;
GtkWidget *glyphbox28;
GtkWidget *icon28_1;
GtkWidget *icon28_2;
GtkWidget *button29;
GtkWidget *glyphbox29;
GtkWidget *icon29;
GtkWidget *button30;
GtkWidget *glyphbox30;
GtkWidget *icon30;
GtkWidget *button31;
GtkWidget *glyphbox31;
GtkWidget *icon31;
GtkWidget *button32;
GtkWidget *glyphbox32;
GtkWidget *icon32;
GtkWidget *button33;
GtkWidget *glyphbox33;
GtkWidget *icon33;
GtkWidget *button34;
GtkWidget *glyphbox34;
GtkWidget *icon34;
GtkWidget *button35;
GtkWidget *glyphbox35;
GtkWidget *icon35;
GtkWidget *button36;
GtkWidget *glyphbox36;
GtkWidget *icon36;
GtkWidget *button37;
GtkWidget *glyphbox37;
GtkWidget *icon37;
GtkWidget *button38;
GtkWidget *glyphbox38;
GtkWidget *icon38;
GtkWidget *button39;
GtkWidget *glyphbox39;
GtkWidget *icon39;
GtkWidget *button40;
GtkWidget *glyphbox40;
GtkWidget *icon40;
GtkWidget *button41;
GtkWidget *glyphbox41;
GtkWidget *icon41;
GtkWidget *button42;
GtkWidget *glyphbox42;
GtkWidget *icon42;
GtkWidget *button43;
GtkWidget *glyphbox43;
GtkWidget *icon43;

GtkWidget *button_box2v;
GtkWidget *button_box2;
GtkWidget *button2;
GtkWidget *glyphbox2;
GtkWidget *icon2;
GtkWidget *label2;
GtkWidget *button3;
GtkWidget *glyphbox3;
GtkWidget *icon3;
GtkWidget *button4;
GtkWidget *glyphbox4;
GtkWidget *icon4;
GtkWidget *button5;
GtkWidget *glyphbox5;
GtkWidget *icon5;
GtkWidget *button6;
GtkWidget *glyphbox6;
GtkWidget *icon6;
GtkWidget *button7;
GtkWidget *glyphbox7;
GtkWidget *icon7;
GtkWidget *button8;
GtkWidget *glyphbox8;
GtkWidget *icon8;

GtkWidget *button_box3v;
GtkWidget *button_box3;
GtkWidget *button16;
GtkWidget *glyphbox16;
GtkWidget *icon16;
GtkWidget *button17;
GtkWidget *glyphbox17;
GtkWidget *icon17;
GtkWidget *button9;
GtkWidget *glyphbox9;
GtkWidget *icon9;
GtkWidget *button10;
GtkWidget *glyphbox10;
GtkWidget *icon10;
GtkWidget *button11;
GtkWidget *glyphbox11;
GtkWidget *icon11;
GtkWidget *button12;
GtkWidget *glyphbox12;
GtkWidget *icon12;

GtkWidget *button_box4;
GtkWidget *button_box5;
GtkWidget *button_box6;
GtkWidget *button_box7;

GtkWidget *notebook;
GtkWidget *nbpage1;
GtkWidget *nbpage2;
GtkWidget *nbpage3;

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
	FILE *fr, *fw;
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

qstatus q_status(messagequeue *q)
{
	qstatus stat;

	pthread_mutex_lock(&(q->qmutex));
	stat = q->status;
	pthread_mutex_unlock(&(q->qmutex));

	return(stat);
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

	if (c->fr)
	{
		strcpy(indication.txrx, "RX"); 
		while ((s=fgets(&(indication.code[0]), msglength, c->fr)))
		{
			i = strlen(s);
//printf("read_indication_thread : %s length:%d\n", s, i);
			if (i<=1) // <newline>
			{
			}
			else // msg<newline>
			{
				s[i-1] = '\0';
				strcpy(indication.parm, s+2);
				indication.code[2] = '\0';
				indication.desc[0] = '\0';
//printf("adding %s-%s\n", indication.code, indication.parm);
				q_add(&(c->rq), &indication);
			}
			if (q_status(&(c->rq))==IDLE) break;
		}
	}

	c->rret[0] = 0;
	pthread_exit(&(c->rret[0]));
}

void command_description(token *q)
{
	if (!strcmp(q->code, "CA"))
		strcpy(q->desc, "Enter pairing mode, discoverable for 2 min.");
	else if (!strcmp(q->code, "CB"))
		strcpy(q->desc, "Exit pairing, can not be found by peers");
	else if (!strcmp(q->code, "CC"))
		strcpy(q->desc, "Connect to last successfully connected remote device");
	else if (!strcmp(q->code, "CD"))
		strcpy(q->desc, "Disconnect from remote device");
	else if (!strcmp(q->code, "CE"))
		strcpy(q->desc, "Answer call");
	else if (!strcmp(q->code, "CF"))
		strcpy(q->desc, "Reject call");
	else if (!strcmp(q->code, "CG"))
		strcpy(q->desc, "End call");
	else if (!strcmp(q->code, "CH"))
		strcpy(q->desc, "Redial");
	else if (!strcmp(q->code, "CI"))
		strcpy(q->desc, "Voice dial");
	else if (!strcmp(q->code, "CJ"))
		strcpy(q->desc, "Cancel voice dial");
	else if (!strcmp(q->code, "CM"))
		strcpy(q->desc, "Mute/unmute microphone");
	else if (!strcmp(q->code, "CO"))
		strcpy(q->desc, "Transfer call to/from handset");
	else if (!strcmp(q->code, "CQ"))
		strcpy(q->desc, "Release held call, Reject waiting call");
	else if (!strcmp(q->code, "CR"))
		strcpy(q->desc, "Release active call, Accept other call");
	else if (!strcmp(q->code, "CS"))
		strcpy(q->desc, "Hold active call, Accept other call");
	else if (!strcmp(q->code, "CT"))
		strcpy(q->desc, "Conference call");
	else if (!strcmp(q->code, "CW"))
		sprintf(q->desc, "Dial %s", q->parm);
	else if (!strcmp(q->code, "CX"))
		sprintf(q->desc, "Send DTMF %s", q->parm);
	else if (!strcmp(q->code, "CY"))
		strcpy(q->desc, "Query HFP Status");
	else if (!strcmp(q->code, "CZ"))
		strcpy(q->desc, "Reset");
	else if (!strcmp(q->code, "MA"))
		strcpy(q->desc, "Play/Pause music");
	else if (!strcmp(q->code, "MC"))
		strcpy(q->desc, "Stop music");
	else if (!strcmp(q->code, "MD"))
		strcpy(q->desc, "Forward music");
	else if (!strcmp(q->code, "ME"))
		strcpy(q->desc, "Backward music");
	else if (!strcmp(q->code, "MF"))
		strcpy(q->desc, "Query Auto Answer and PowerOn Auto Connection");
	else if (!strcmp(q->code, "MG"))
		strcpy(q->desc, "Enable PowerOn Auto Connection");
	else if (!strcmp(q->code, "MH"))
		strcpy(q->desc, "Disable PowerOn Auto Connection");
	else if (!strcmp(q->code, "MI"))
		strcpy(q->desc, "Connect to AV Source");
	else if (!strcmp(q->code, "MJ"))
		strcpy(q->desc, "Disconnect from AV Source");
	else if (!strcmp(q->code, "MO"))
		strcpy(q->desc, "Query AVRCP Status");
	else if (!strcmp(q->code, "MP"))
		strcpy(q->desc, "Enable Auto Answer");
	else if (!strcmp(q->code, "MQ"))
		strcpy(q->desc, "Disable Auto Answer");
	else if (!strcmp(q->code, "MR"))
		strcpy(q->desc, "Fast Forward");
	else if (!strcmp(q->code, "MS"))
		strcpy(q->desc, "Fast Rewind");
	else if (!strcmp(q->code, "MT"))
		strcpy(q->desc, "Stop Fast Forward/Rewind");
	else if (!strcmp(q->code, "MV"))
		strcpy(q->desc, "Query A2DP status");
	else if (!strcmp(q->code, "MZ"))
		strcpy(q->desc, "Switch two remote devices");
	else if (!strcmp(q->code, "VD"))
		strcpy(q->desc, "Volume down");
	else if (!strcmp(q->code, "VI"))
		strcpy(q->desc, "Start Inquiry");
	else if (!strcmp(q->code, "VJ"))
		strcpy(q->desc, "Cancel Inquiry");
	else if (!strcmp(q->code, "VU"))
		strcpy(q->desc, "Volume up");
	else if (!strcmp(q->code, "VX"))
		strcpy(q->desc, "Power off OOL");
}

void indicator_description(token *q)
{
	int i;
	char *s;

	if (!strcmp(q->code, "II"))
		strcpy(q->desc, "Enter pairing state");
	else if (!strcmp(q->code, "IJ"))
		sprintf(q->desc, "Exit pairing mode and enter listening, %s", q->parm);
	else if (!strcmp(q->code, "IV"))
		strcpy(q->desc, "Connected");
	else if (!strcmp(q->code, "IA"))
		strcpy(q->desc, "Disconnected");
	else if (!strcmp(q->code, "IC"))
		strcpy(q->desc, "Outgoing call");
	else if (!strcmp(q->code, "IF"))
		strcpy(q->desc, "Hang up");
	else if (!strcmp(q->code, "IG"))
		strcpy(q->desc, "Pick up");
	else if (!strcmp(q->code, "IL"))
		strcpy(q->desc, "Held active call, Accepted other call");
	else if (!strcmp(q->code, "IM"))
		strcpy(q->desc, "In conference");
	else if (!strcmp(q->code, "IN"))
		strcpy(q->desc, "Released held call, Rejected waiting call");
	else if (!strcmp(q->code, "IP"))
		sprintf(q->desc, "Phone number length %s", q->parm);
	else if (!strcmp(q->code, "IS"))
		sprintf(q->desc, "Power on init complete, version %s", q->parm);
	else if (!strcmp(q->code, "IT"))
		strcpy(q->desc, "Released active call, Accepted other call");
	else if (!strcmp(q->code, "IR"))
		sprintf(q->desc, "Phone number %s", q->parm);
	else if (!strcmp(q->code, "PE"))
		strcpy(q->desc, "Voice dial start");
	else if (!strcmp(q->code, "PF"))
		strcpy(q->desc, "Voice dial stop");
	else if (!strcmp(q->code, "OK"))
		strcpy(q->desc, "Command accepted");
	else if (!strcmp(q->code, "SW"))
		strcpy(q->desc, "Switch command accepted");
	else if (!strcmp(q->code, "MA"))
		strcpy(q->desc, "AV paused/stopped");
	else if (!strcmp(q->code, "MB"))
		strcpy(q->desc, "AV playing");
	else if (!strcmp(q->code, "MC"))
		strcpy(q->desc, "HFP audio connected");
	else if (!strcmp(q->code, "MD"))
		strcpy(q->desc, "HFP audio disconnected");
	else if (!strcmp(q->code, "MF"))
		sprintf(q->desc, "Auto Answer, Auto Connect <%s>", q->parm);
	else if (!strcmp(q->code, "MG"))
	{
		i = atoi(q->parm);
		switch(i)
		{
			case 1: s="Ready (to be connected)"; break;
			case 2: s="Connecting"; break;
			case 3: s="Connected"; break;
			case 4: s="Outgoing call"; break;
			case 5: s="Incoming call"; break;
			case 6: s="Ongoing call"; break;
			default: s="Unknown"; break;
		}
		sprintf(q->desc, "HFP Status %s, %s", q->parm, s);
	}
	else if (!strcmp(q->code, "ML"))
	{
		i = atoi(q->parm);
		switch(i)
		{
			case 1: s="Ready (to be connected)"; break;
			case 2: s="Connecting"; break;
			case 3: s="Connected"; break;
			default: s="Unknown"; break;
		}		
		sprintf(q->desc, "AVRCP status %s, %s", q->parm, s);
	}
	else if (!strcmp(q->code, "MP"))
		strcpy(q->desc, "Music paused");
	else if (!strcmp(q->code, "MR"))
		strcpy(q->desc, "Music resumed");
	else if (!strcmp(q->code, "MS"))
		strcpy(q->desc, "Music rewind");
	else if (!strcmp(q->code, "MX"))
		strcpy(q->desc, "Music forward");
	else if (!strcmp(q->code, "MU"))
	{
		i = atoi(q->parm);
		switch(i)
		{
			case 1: s="Ready (to be connected)"; break;
			case 2: s="Initializing"; break;
			case 3: s="Signalling Active"; break;
			case 4: s="Connected"; break;
			case 5: s="Streaming"; break;
			default: s="Unknown"; break;
		}
		sprintf(q->desc, "A2DP status %s, %s", q->parm, s);
	}
	else if (!strcmp(q->code, "MY"))
		strcpy(q->desc, "AV disconnected");
	else if (!strcmp(q->code, "PA"))
		sprintf(q->desc, "PA<%s>", q->parm);
	else if (!strcmp(q->code, "PC"))
		strcpy(q->desc, "PC");
	else if (!strcmp(q->code, "AE"))
		strcpy(q->desc, "Audio config error");
	else if (!strcmp(q->code, "AF"))
		strcpy(q->desc, "Audio codec closed");
	else if (!strcmp(q->code, "AS"))
		strcpy(q->desc, "Audio codec in phone call mode");
	else if (!strcmp(q->code, "ER"))
		sprintf(q->desc, "ER<%s>", q->parm);
	else if (!strcmp(q->code, "NO"))
		sprintf(q->desc, "NO<%s>", q->parm);
	else if (!strcmp(q->code, "EP"))
		sprintf(q->desc, "EP<%s>", q->parm);
	else if (!strcmp(q->code, "VO"))
		sprintf(q->desc, "Volume Level %s", q->parm+1);
}

gboolean indication_idle(gpointer data)
{
	node *q = (node *)data;
	token *t = &(q->data);

//printf("indication %s %s-%s %s\n", t->txrx, t->code, t->parm, t->desc);
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

void fill_tx_token(token *t, char *code, char *parm)
{
	strcpy(t->txrx, "TX");
	strcpy(t->code, code);
	strcpy(t->parm, parm);
	t->desc[0] = '\0';
}

void send_message(cp2102 *c, token *t)
{
	char msg[msglength];

	if (c->fw)
	{
		sprintf(msg, "%s%s%s%s", cmdheader, t->code, t->parm, crlf);
		fputs(msg, c->fw);
	}
}

gboolean command_idle(gpointer data)
{
	node *q = (node *)data;
	token *t = &(q->data);

//printf("command %s %s-%s %s\n", t->txrx, t->code, t->parm, t->desc);
	command_description(t);
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
		send_message(c, &(q->data));
		gdk_threads_add_idle(command_idle, (void*)q);
	}

	c->tret[0] = 0;
	pthread_exit(&(c->tret[0]));
}


int set_baud_rate(char* dev, speed_t baud)
{
	int fd;
	struct termios settings;

	if ((fd=open(dev, O_RDWR | O_NONBLOCK))==-1)
	{
		printf("Cannot open %s\n", dev);
		return(0);
	}

	tcgetattr(fd, &settings);
	cfsetospeed(&settings, baud); // baud rate
	cfsetispeed(&settings, baud); // baud rate
	tcsetattr(fd, TCSANOW, &settings); // apply the settings
	tcflush(fd, TCOFLUSH);
	tcflush(fd, TCIFLUSH);

	close(fd);
	return(1);
}

void create_threads(cp2102 *c)
{
	int err;

	q_init(&(c->rq), 10);
	q_init(&(c->tq), 10);

	if (!(c->fr = fopen(c->devicepath, "r+")))
		printf("Cannot open %s for input\n", c->devicepath);
	if (!(c->fw = fopen(c->devicepath, "r+")))
		printf("Cannot open %s for output\n", c->devicepath);

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
	token t;

	q_signalstop(&(c->rq));

	fill_tx_token(&t, "MO", ""); 
	q_add(&(c->tq), &t); // send a query message to get a response to wake up read_indication_thread()

	if ((i=pthread_join(c->rtid[0], NULL)))
		printf("pthread_join error, c->rtid[0], %d\n", i);

	if ((i=pthread_join(c->rtid[1], NULL)))
		printf("pthread_join error, c->rtid[1], %d\n", i);

	q_destroy(&(c->rq));


	q_signalstop(&(c->tq));

	if ((i=pthread_join(c->ttid[0], NULL)))
		printf("pthread_join error, c->ttid[0], %d\n", i);

	q_destroy(&(c->tq));

	fclose(c->fr);
	fclose(c->fw);
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

void page_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
	//cp2102 *c = (cp2102 *)user_data;

//printf("switched to page %d\n", page_num);
}

static void button1_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CA", "");
	q_add(&(c->tq), &t);
}

static void button2_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MA", "");
	q_add(&(c->tq), &t);
}

static void button3_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MC", "");
	q_add(&(c->tq), &t);
}

static void button4_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MD", "");
	q_add(&(c->tq), &t);
}

static void button5_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "ME", "");
	q_add(&(c->tq), &t);
}

static void button6_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MR", "");
	q_add(&(c->tq), &t);
}

static void button7_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MS", "");
	q_add(&(c->tq), &t);
}

static void button8_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MT", "");
	q_add(&(c->tq), &t);
}

static void button9_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MV", "");
	q_add(&(c->tq), &t);
}

static void button10_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MZ", "");
	q_add(&(c->tq), &t);
}

static void button11_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "VD", "");
	q_add(&(c->tq), &t);
}

static void button12_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "VU", "");
	q_add(&(c->tq), &t);
}

static void button13_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CB", "");
	q_add(&(c->tq), &t);
}

static void button14_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CC", "");
	q_add(&(c->tq), &t);
}

static void button15_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CD", "");
	q_add(&(c->tq), &t);
}

static void button16_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MI", "");
	q_add(&(c->tq), &t);
}

static void button17_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "MJ", "");
	q_add(&(c->tq), &t);
}

static void button18_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CE", "");
	q_add(&(c->tq), &t);
}

static void button19_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CF", "");
	q_add(&(c->tq), &t);
}

static void button20_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CG", "");
	q_add(&(c->tq), &t);
}

static void button21_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CH", "");
	q_add(&(c->tq), &t);
}

static void button22_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CI", "");
	q_add(&(c->tq), &t);
}

static void button23_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CJ", "");
	q_add(&(c->tq), &t);
}

static void button24_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CM", "");
	q_add(&(c->tq), &t);
}

static void button25_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CO", "");
	q_add(&(c->tq), &t);
}

static void button26_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CQ", "");
	q_add(&(c->tq), &t);
}

static void button27_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CR", "");
	q_add(&(c->tq), &t);
}

static void button28_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CS", "");
	q_add(&(c->tq), &t);
}

static void button29_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CT", "");
	q_add(&(c->tq), &t);
}

static void button30_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CW", "");
	q_add(&(c->tq), &t);
}

static void button31_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "1");
	q_add(&(c->tq), &t);
}

static void button32_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "2");
	q_add(&(c->tq), &t);
}

static void button33_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "3");
	q_add(&(c->tq), &t);
}

static void button34_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "4");
	q_add(&(c->tq), &t);
}

static void button35_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "5");
	q_add(&(c->tq), &t);
}

static void button36_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "6");
	q_add(&(c->tq), &t);
}

static void button37_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "7");
	q_add(&(c->tq), &t);
}

static void button38_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "8");
	q_add(&(c->tq), &t);
}

static void button39_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "9");
	q_add(&(c->tq), &t);
}

static void button40_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "0");
	q_add(&(c->tq), &t);
}

static void button41_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "*");
	q_add(&(c->tq), &t);
}

static void button42_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CX", "#");
	q_add(&(c->tq), &t);
}

static void button43_clicked(GtkWidget *button, gpointer data)
{
	cp2102 *c = (cp2102 *)data;
	token t;

	fill_tx_token(&t, "CY", "");
	q_add(&(c->tq), &t);
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "tr_TR.UTF-8");
	setup_default_icon("./images/XS3868.jpeg");

	cp2102 c;
	strcpy(c.devicepath, "/dev/ttyUSB0");
	if (!set_baud_rate(c.devicepath, B115200))
		return(0);

	/* This is called in all GTK applications. Arguments are parsed
	 * from the command line and are returned to the application. */
	gtk_init(&argc, &argv);

	/* create a new window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	//gtk_widget_set_size_request(window, 100, 100);
	gtk_window_set_title(GTK_WINDOW(window), "XS3868 via CP2102");
	//gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

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
	gtk_box_pack_start(GTK_BOX(vbox), frame1, TRUE, TRUE, 0);

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(frame1), vbox1);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_set_size_request(scrolled_window, 300, 100);
	//gtk_container_add(GTK_CONTAINER(vbox1), scrolled_window);
	gtk_box_pack_start(GTK_BOX(vbox1), scrolled_window, TRUE, TRUE, 0);

	listview = create_view_and_model();
	gtk_container_add(GTK_CONTAINER(scrolled_window), listview);

// botton_box_group
	button_box_group1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

// buttonbox HFP #1
	//button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	button_box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button1 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button1), "clicked", G_CALLBACK(button1_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button1);
	glyphbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button1), glyphbox1);
	icon1 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon1), "./images/EnterPairing.png");
	gtk_container_add(GTK_CONTAINER(glyphbox1), icon1);

	button13 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button13), "clicked", G_CALLBACK(button13_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button13);
	glyphbox13 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button13), glyphbox13);
	icon13 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon13), "./images/ExitPairing.png");
	gtk_container_add(GTK_CONTAINER(glyphbox13), icon13);

	button14 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button14), "clicked", G_CALLBACK(button14_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button14);
	glyphbox14 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button14), glyphbox14);
	icon14 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon14), "./images/ConnectLast.png");
	gtk_container_add(GTK_CONTAINER(glyphbox14), icon14);

	button15 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button15), "clicked", G_CALLBACK(button15_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button15);
	glyphbox15 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button15), glyphbox15);
	icon15 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon15), "./images/Disconnect.png");
	gtk_container_add(GTK_CONTAINER(glyphbox15), icon15);

// buttonbox HFP #2
	//button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	button_box4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button18 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button18), "clicked", G_CALLBACK(button18_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button18);
	glyphbox18 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button18), glyphbox18);
	icon18 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon18), "./images/AnswerCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox18), icon18);

	button19 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button19), "clicked", G_CALLBACK(button19_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button19);
	glyphbox19 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button19), glyphbox19);
	icon19 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon19), "./images/RejectCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox19), icon19);

	button20 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button20), "clicked", G_CALLBACK(button20_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button20);
	glyphbox20 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button20), glyphbox20);
	icon20 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon20), "./images/EndCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox20), icon20);

	button26 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button26), "clicked", G_CALLBACK(button26_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button26);
	glyphbox26 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button26), glyphbox26);
	icon26_1 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon26_1), "./images/ReleaseHeldCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox26), icon26_1);
	icon26_2 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon26_2), "./images/RejectCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox26), icon26_2);

	button27 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button27), "clicked", G_CALLBACK(button27_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button27);
	glyphbox27 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button27), glyphbox27);
	icon27_1 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon27_1), "./images/ReleaseHeldCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox27), icon27_1);
	icon27_2 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon27_2), "./images/AnswerCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox27), icon27_2);

	button28 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button28), "clicked", G_CALLBACK(button28_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button28);
	glyphbox28 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button28), glyphbox28);
	icon28_1 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon28_1), "./images/HoldCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox28), icon28_1);
	icon28_2 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon28_2), "./images/AnswerCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox28), icon28_2);

	button29 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button29), "clicked", G_CALLBACK(button29_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box4), button29);
	glyphbox29 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button29), glyphbox29);
	icon29 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon29), "./images/ConferenceCall.png");
	gtk_container_add(GTK_CONTAINER(glyphbox29), icon29);

// buttonbox HFP #3
	//button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	button_box5 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button21 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button21), "clicked", G_CALLBACK(button21_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box5), button21);
	glyphbox21 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button21), glyphbox21);
	icon21 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon21), "./images/Redial.png");
	gtk_container_add(GTK_CONTAINER(glyphbox21), icon21);

	button22 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button22), "clicked", G_CALLBACK(button22_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box5), button22);
	glyphbox22 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button22), glyphbox22);
	icon22 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon22), "./images/VoiceDial.png");
	gtk_container_add(GTK_CONTAINER(glyphbox22), icon22);

	button23 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button23), "clicked", G_CALLBACK(button23_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box5), button23);
	glyphbox23 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button23), glyphbox23);
	icon23 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon23), "./images/CancelVoiceDial.png");
	gtk_container_add(GTK_CONTAINER(glyphbox23), icon23);

	button30 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button30), "clicked", G_CALLBACK(button30_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box5), button30);
	glyphbox30 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button30), glyphbox30);
	icon30 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon30), "./images/Dial.png");
	gtk_container_add(GTK_CONTAINER(glyphbox30), icon30);

// buttonbox HFP #4
	//button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	button_box6 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button24 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button24), "clicked", G_CALLBACK(button24_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box6), button24);
	glyphbox24 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button24), glyphbox24);
	icon24 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon24), "./images/MuteUnmuteMic.png");
	gtk_container_add(GTK_CONTAINER(glyphbox24), icon24);

	button25 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button25), "clicked", G_CALLBACK(button25_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box6), button25);
	glyphbox25 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button25), glyphbox25);
	icon25 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon25), "./images/TransCallToFromHS.png");
	gtk_container_add(GTK_CONTAINER(glyphbox25), icon25);

	button43 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button43), "clicked", G_CALLBACK(button43_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box1), button43);
	glyphbox43 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button43), glyphbox43);
	icon43 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon43), "./images/QueryStatus.png");
	gtk_container_add(GTK_CONTAINER(glyphbox43), icon43);

// buttonbox HFP #4
	//button_box1 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box1, GTK_BUTTONBOX_START);
	button_box7 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box1);

	button31 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button31), "clicked", G_CALLBACK(button31_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button31);
	glyphbox31 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button31), glyphbox31);
	icon31 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon31), "./images/001.png");
	gtk_container_add(GTK_CONTAINER(glyphbox31), icon31);

	button32 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button32), "clicked", G_CALLBACK(button32_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button32);
	glyphbox32 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button32), glyphbox32);
	icon32 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon32), "./images/002.png");
	gtk_container_add(GTK_CONTAINER(glyphbox32), icon32);

	button33 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button33), "clicked", G_CALLBACK(button33_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button33);
	glyphbox33 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button33), glyphbox33);
	icon33 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon33), "./images/003.png");
	gtk_container_add(GTK_CONTAINER(glyphbox33), icon33);

	button34 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button34), "clicked", G_CALLBACK(button34_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button34);
	glyphbox34 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button34), glyphbox34);
	icon34 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon34), "./images/004.png");
	gtk_container_add(GTK_CONTAINER(glyphbox34), icon34);

	button35 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button35), "clicked", G_CALLBACK(button35_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button35);
	glyphbox35 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button35), glyphbox35);
	icon35 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon35), "./images/005.png");
	gtk_container_add(GTK_CONTAINER(glyphbox35), icon35);

	button36 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button36), "clicked", G_CALLBACK(button36_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button36);
	glyphbox36 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button36), glyphbox36);
	icon36 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon36), "./images/006.png");
	gtk_container_add(GTK_CONTAINER(glyphbox36), icon36);

	button37 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button37), "clicked", G_CALLBACK(button37_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button37);
	glyphbox37 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button37), glyphbox37);
	icon37 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon37), "./images/007.png");
	gtk_container_add(GTK_CONTAINER(glyphbox37), icon37);

	button38 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button38), "clicked", G_CALLBACK(button38_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button38);
	glyphbox38 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button38), glyphbox38);
	icon38 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon38), "./images/008.png");
	gtk_container_add(GTK_CONTAINER(glyphbox38), icon38);

	button39 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button39), "clicked", G_CALLBACK(button39_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button39);
	glyphbox39 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button39), glyphbox39);
	icon39 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon39), "./images/009.png");
	gtk_container_add(GTK_CONTAINER(glyphbox39), icon39);

	button40 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button40), "clicked", G_CALLBACK(button40_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button40);
	glyphbox40 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button40), glyphbox40);
	icon40 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon40), "./images/010.png");
	gtk_container_add(GTK_CONTAINER(glyphbox40), icon40);

	button41 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button41), "clicked", G_CALLBACK(button41_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button41);
	glyphbox41 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button41), glyphbox41);
	icon41 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon41), "./images/011.png");
	gtk_container_add(GTK_CONTAINER(glyphbox41), icon41);

	button42 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button42), "clicked", G_CALLBACK(button42_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box7), button42);
	glyphbox42 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button42), glyphbox42);
	icon42 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon42), "./images/012.png");
	gtk_container_add(GTK_CONTAINER(glyphbox42), icon42);

// buttonbox A2DP
	button_box2v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	//button_box2 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box2, GTK_BUTTONBOX_START);
	button_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box2);

	gtk_container_add(GTK_CONTAINER(button_box2v), button_box2);

	button2 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button2), "clicked", G_CALLBACK(button2_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button2);
	glyphbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button2), glyphbox2);
	icon2 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon2), "./images/PlayPause.png");
	gtk_container_add(GTK_CONTAINER(glyphbox2), icon2);
	label2 = gtk_label_new("Play/Pause");
	gtk_container_add(GTK_CONTAINER(glyphbox2), label2);

	button3 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button3), "clicked", G_CALLBACK(button3_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button3);
	glyphbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button3), glyphbox3);
	icon3 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon3), "./images/Stop.png");
	gtk_container_add(GTK_CONTAINER(glyphbox3), icon3);

	button4 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button4), "clicked", G_CALLBACK(button4_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button4);
	glyphbox4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button4), glyphbox4);
	icon4 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon4), "./images/Next.png");
	gtk_container_add(GTK_CONTAINER(glyphbox4), icon4);

	button5 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button5), "clicked", G_CALLBACK(button5_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button5);
	glyphbox5 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button5), glyphbox5);
	icon5 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon5), "./images/Previous.png");
	gtk_container_add(GTK_CONTAINER(glyphbox5), icon5);

	button6 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button6), "clicked", G_CALLBACK(button6_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button6);
	glyphbox6 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button6), glyphbox6);
	icon6 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon6), "./images/FastForward.png");
	gtk_container_add(GTK_CONTAINER(glyphbox6), icon6);

	button7 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button7), "clicked", G_CALLBACK(button7_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button7);
	glyphbox7 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button7), glyphbox7);
	icon7 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon7), "./images/FastRewind.png");
	gtk_container_add(GTK_CONTAINER(glyphbox7), icon7);

	button8 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button8), "clicked", G_CALLBACK(button8_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box2), button8);
	glyphbox8 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button8), glyphbox8);
	icon8 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon8), "./images/FFFRStop.png");
	gtk_container_add(GTK_CONTAINER(glyphbox8), icon8);

// buttonbox AVRCP
	button_box3v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	//button_box3 = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	//gtk_button_box_set_layout((GtkButtonBox *)button_box3, GTK_BUTTONBOX_START);
	button_box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	//gtk_container_add(GTK_CONTAINER(vbox), button_box3);

	gtk_container_add(GTK_CONTAINER(button_box3v), button_box3);

	button16 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button16), "clicked", G_CALLBACK(button16_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button16);
	glyphbox16 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button16), glyphbox16);
	icon16 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon16), "./images/ConnectBT.png");
	gtk_container_add(GTK_CONTAINER(glyphbox16), icon16);

	button17 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button17), "clicked", G_CALLBACK(button17_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button17);
	glyphbox17 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button17), glyphbox17);
	icon17 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon17), "./images/DisconnectBT.png");
	gtk_container_add(GTK_CONTAINER(glyphbox17), icon17);

	button9 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button9), "clicked", G_CALLBACK(button9_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button9);
	glyphbox9 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button9), glyphbox9);
	icon9 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon9), "./images/QueryStatus.png");
	gtk_container_add(GTK_CONTAINER(glyphbox9), icon9);

	button10 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button10), "clicked", G_CALLBACK(button10_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button10);
	glyphbox10 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button10), glyphbox10);
	icon10 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon10), "./images/SwitchDevices.png");
	gtk_container_add(GTK_CONTAINER(glyphbox10), icon10);

	button11 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button11), "clicked", G_CALLBACK(button11_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button11);
	glyphbox11 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button11), glyphbox11);
	icon11 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon11), "./images/VolumeDown.png");
	gtk_container_add(GTK_CONTAINER(glyphbox11), icon11);

	button12 = gtk_button_new();
	g_signal_connect(GTK_BUTTON(button12), "clicked", G_CALLBACK(button12_clicked), (void*)&c);
	gtk_container_add(GTK_CONTAINER(button_box3), button12);
	glyphbox12 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(button12), glyphbox12);
	icon12 = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(icon12), "./images/VolumeUp.png");
	gtk_container_add(GTK_CONTAINER(glyphbox12), icon12);

	gtk_container_add(GTK_CONTAINER(button_box_group1), button_box1);
	gtk_container_add(GTK_CONTAINER(button_box_group1), button_box4);
	gtk_container_add(GTK_CONTAINER(button_box_group1), button_box5);
	gtk_container_add(GTK_CONTAINER(button_box_group1), button_box6);
	gtk_container_add(GTK_CONTAINER(button_box_group1), button_box7);

// tabbed notebook
	notebook = gtk_notebook_new();
	nbpage1 = gtk_label_new("HFP");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), button_box_group1, nbpage1);
	nbpage2 = gtk_label_new("A2DP");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), button_box2v, nbpage2);
	nbpage3 = gtk_label_new("AVRCP");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), button_box3v, nbpage3);
	g_signal_connect(GTK_NOTEBOOK(notebook), "switch-page", G_CALLBACK(page_switched), (void*)&c);
	//gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(vbox), notebook);

	gtk_widget_show_all(window);

	gtk_main();

	return(0);
}
