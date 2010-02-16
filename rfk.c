/*  robotfindskitten for maemo
 *  original by Leonard Richardson, 1997
 *  ported to maemo by Thomas Thurman, 2009
 *  suggestions welcome
 *  Compile with:
 *  gcc -Wall -g rfk.c -o rfk `pkg-config --cflags --libs gtk+-2.0 hildon-1 dbus-glib-1 dbus-1`
 */

/*
 TO DO:
  - working demo mode
  - gtkhtml instructions reader
*/

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <glib.h>
#include <hildon/hildon.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <mce/mode-names.h>
#include <mce/dbus-names.h>

#define MCE_SIGNAL_MATCH "type='signal'," \
	        "sender='"    MCE_SERVICE     "'," \
        "path='"      MCE_SIGNAL_PATH "'," \
        "interface='" MCE_SIGNAL_IF   "'"

#define ARENA_WIDTH 25
#define ARENA_HEIGHT 12

#define WINDOW_TITLE "robotfindskitten"
#define WINDOW_TITLE_SHORT "rfk"

#define DEMO_SPEED 200

const int amount_of_random_stuff = 15;

typedef enum {
  STATE_PROLOGUE,
  STATE_PLAYING,
  STATE_EPILOGUE,
  STATE_LAST
} StateOfPlay;

StateOfPlay current_state = STATE_LAST;
GtkWidget* state_widget[STATE_LAST];

GSList *nki = NULL;
guint nki_count = 0;

gboolean portrait_mode = FALSE;

GtkWidget *arena[ARENA_WIDTH][ARENA_HEIGHT];
GtkWidget *window, *robot, *kitten;
int robot_x, robot_y;
int robot_demo_x=-1, robot_demo_y=-1;
gboolean *used = NULL;
gboolean demo_running = FALSE;

GdkPixbuf *robot_pic, *love_pic, *kitten_pic;

const GdkColor black = { 0, };
GdkColor grey = { 0, 0x3333, 0x3333, 0x3333 };

/****************************************************************/
/* Random object descriptions.                                  */
/****************************************************************/

static char *
description (void)
{
  int r;
   
  do
    {
      r = random() % nki_count;
    }
  while (used[r]);
  used[r] = TRUE;

  return g_slist_nth_data (nki, r);
}

/****************************************************************/
/* Placing objects.                                             */
/****************************************************************/

static void
place_in_arena_at_xy (GtkWidget *item, int x, int y)
{
  arena[x][y] = item;

  gtk_table_attach_defaults (GTK_TABLE (state_widget[STATE_PLAYING]),
			     item,
			     x, x+1,
			     y, y+1);

  if (item==robot)
    {
      robot_x = x;
      robot_y = y;
    }
}

static void
place_in_arena_randomly (GtkWidget *item)
{
  int x, y;
   
  do
    {
      x = random() % ARENA_WIDTH;
      y = random() % ARENA_HEIGHT;
    }
  while (arena[x][y]);

  place_in_arena_at_xy (item, x, y);
}

/****************************************************************/
/* Labels representing things the robot might find.             */
/****************************************************************/

static GtkWidget *
random_character (gchar *description)
{
  gchar character[2] = { random() % ('~'-'!') + '!', 0 };
  gchar *escaped_character = g_markup_escape_text (character, -1);
  gchar *markup = g_strdup_printf ("<span color=\"#%02x%02x%02x\">%s</span>",
				   (int) (random() % 0x7F)+0x80,
				   (int) (random() % 0x7F)+0x80,
				   (int) (random() % 0x7F)+0x80,
				   escaped_character);
  GtkWidget *result = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (result), markup);
  g_free (markup);
  g_free (escaped_character);

  g_object_set_data (G_OBJECT (result), "examine", description);

  return result;
}

/****************************************************************/
/* Talking back to the user.                                    */
/****************************************************************/

static gboolean
close_dialogue (gpointer data)
{
  GtkDialog *dialogue = (GtkDialog*) data;
  gtk_dialog_response (dialogue, GTK_RESPONSE_ACCEPT);
  return FALSE; /* don't go round again */
}

static void
show_message (const char *message)
{
  HildonNote* note = HILDON_NOTE
    (hildon_note_new_information (GTK_WINDOW (window),
				  message?message:
				  "Some message was supposed to be here."));
  if (demo_running)
    {
      g_timeout_add (1000, close_dialogue, note);
    }
  gtk_dialog_run (GTK_DIALOG (note));
  gtk_widget_destroy (GTK_WIDGET (note));
}

/****************************************************************/
/* Loading the non-kitten objects.                              */
/****************************************************************/

static void
ensure_messages_loaded (void)
{
  FILE *nki_file = NULL;
  gchar *line = NULL;
  gboolean headers = TRUE;

  if (nki_count)
    return;

  nki_file = fopen ("/opt/rfk/non-kitten-items.rfk", "r");

  if (!nki_file)
    {
      show_message ("Could not open list of non-kitten items!  Must quit.");
      exit (EXIT_FAILURE);
    }

  while (!feof (nki_file))
    {
      char newline;
      if (fscanf (nki_file, "%a[^\n]%c", &line, &newline) == EOF)
	{
	  break;
	}

      if (strcmp(line, "")==0)
	{
	  headers = FALSE;
	  fscanf (nki_file, "%c", &newline); 
	  free (line);
	}
      else if (headers)
	{
	  /* we ignore all the headers for now */
	  free (line);
	}
      else
	{
	  nki = g_slist_prepend (nki, line);
	  nki_count++;
	}
    }

  fclose (nki_file);
}

static void
load_images (void)
{
  robot_pic = gdk_pixbuf_new_from_file ("/opt/rfk/rfk-robot.png", NULL);
  love_pic = gdk_pixbuf_new_from_file ("/opt/rfk/rfk-love.png", NULL);
  kitten_pic = gdk_pixbuf_new_from_file ("/opt/rfk/rfk-kitten.png", NULL);
}

/****************************************************************/
/* Stop doing that, and do something else.                      */
/****************************************************************/

static void
switch_state (StateOfPlay new_state)
{
  if (current_state != STATE_LAST)
    {
      gtk_container_remove (GTK_CONTAINER (window), state_widget[current_state]);
    }
  gtk_container_add (GTK_CONTAINER (window), state_widget[new_state]);

  gtk_widget_show_all (window);
  gdk_window_set_events (GTK_WIDGET (window)->window,
			 gdk_window_get_events(GTK_WIDGET (window)->window) | GDK_BUTTON_PRESS_MASK);

  current_state = new_state;
}

/****************************************************************/
/* Things we need DBus for: online help, vibration, and         */
/* orientation.                                                 */
/****************************************************************/

static void
call_dbus (DBusBusType type,
	   char *name,
	   char *path,
	   char *interface,
	   char *method,
	   char *parameter)
{
  DBusGConnection *connection;
  GError *error = NULL;

  DBusGProxy *proxy;

  connection = dbus_g_bus_get (type,
                               &error);
  if (connection == NULL)
    {
      show_message (error->message);
      g_error_free (error);
      return;
    }

  proxy = dbus_g_proxy_new_for_name (connection, name, path, interface);

  dbus_g_proxy_call_no_reply (proxy, method,
		  G_TYPE_STRING, parameter,
		  G_TYPE_INVALID);
}

static gboolean
get_help (gpointer button, gpointer data)
{
  call_dbus (DBUS_BUS_SESSION,
	     "com.nokia.osso_browser",
	     "/com/nokia/osso_browser/request",
	     "com.nokia.osso_browser",
	     "load_url",
	     "/opt/rfk/help.html");
  return FALSE;
}

static void
vibrate (void)
{
  call_dbus (DBUS_BUS_SYSTEM,
	     "com.nokia.mce",
	     "/com/nokia/mce/request",
	     "com.nokia.mce.request",
	     "req_vibrator_pattern_activate",
	     "PatternIncomingMessage");
}


static DBusHandlerResult
mce_filter_func (DBusConnection * connection,
                 DBusMessage * message, gpointer dummy)
{
  DBusError error;
  char *rotation, *stand, *face;
  int x, y, z;
	
  if (!dbus_message_is_signal (message, MCE_SIGNAL_IF, MCE_DEVICE_ORIENTATION_SIG))
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  dbus_error_init (&error);
  if (dbus_message_get_args (message,
                             &error,
                             DBUS_TYPE_STRING, &rotation,
                             DBUS_TYPE_STRING, &stand,
                             DBUS_TYPE_STRING, &face,
                             DBUS_TYPE_INT32,  &x,
                             DBUS_TYPE_INT32,  &y,
                             DBUS_TYPE_INT32,  &z, DBUS_TYPE_INVALID))
    {
      /* Rotate main window */
      if (strcmp (rotation, MCE_ORIENTATION_PORTRAIT)==0)
        {		
          portrait_mode = TRUE;
          hildon_gtk_window_set_portrait_flags (GTK_WINDOW (window), HILDON_PORTRAIT_MODE_REQUEST|HILDON_PORTRAIT_MODE_SUPPORT);
	  gtk_window_set_title (GTK_WINDOW (window), WINDOW_TITLE_SHORT);
        }
      else
        {
          portrait_mode = FALSE;
          hildon_gtk_window_set_portrait_flags (GTK_WINDOW (window), HILDON_PORTRAIT_MODE_SUPPORT);
	  gtk_window_set_title (GTK_WINDOW (window), WINDOW_TITLE);
        }
    }
  else
    {
      g_warning ("%s: %s\n", error.name, error.message);
      dbus_error_free (&error);
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}	    

static void
check_for_rotation (void)
{
  DBusConnection *connection = dbus_bus_get (DBUS_BUS_SYSTEM, NULL);
  
  call_dbus (DBUS_BUS_SYSTEM,
	     MCE_SERVICE,
	     MCE_REQUEST_PATH,
	     MCE_REQUEST_IF,
	     "req_accelerometer_enable",
	     NULL);

  dbus_bus_add_match (connection, MCE_SIGNAL_MATCH, NULL);

  dbus_connection_add_filter (connection,
                               (DBusHandleMessageFunction) mce_filter_func,
                               NULL, NULL);
  
  hildon_gtk_window_set_portrait_flags (GTK_WINDOW (window),
                                        HILDON_PORTRAIT_MODE_SUPPORT);
}

/****************************************************************/
/* The ending animation.                                        */
/****************************************************************/

gboolean animation_running = FALSE;

static gboolean
ending_animation_quit (gpointer data)
{
  switch_state (STATE_PROLOGUE);
  return FALSE;
}

static gboolean
ending_animation_draw (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  static int cycle_count = 0;

  static int robot_x = 0;
  static int robot_stop = 0;
  static int kitten_x = 0;
  static int all_y = 0;
  static GdkGC *gc = NULL;

  const int stepsize = 3;
  static int love_size = 40;

  if (!kitten_x)
    {
      gc = gdk_gc_new (GDK_DRAWABLE (widget->window));

      all_y = (event->area.height - gdk_pixbuf_get_height (love_pic)) / 2;

      robot_stop = gdk_pixbuf_get_width (robot_pic) + gdk_pixbuf_get_width (love_pic);
      kitten_x = event->area.width - (cycle_count*stepsize + gdk_pixbuf_get_width (kitten_pic));
    }

  gdk_gc_set_foreground (gc, &black);

  gdk_draw_rectangle (GDK_DRAWABLE (widget->window),
		      gc,
		      TRUE,
		      0, 0, event->area.width, event->area.height);

  gdk_draw_pixbuf (GDK_DRAWABLE (widget->window),
		   gc,
		   robot_pic, 0, 0,
		   robot_x, all_y,
		   -1, -1,
		   GDK_RGB_DITHER_NONE, 0, 0);

  gdk_draw_pixbuf (GDK_DRAWABLE (widget->window),
		   gc,
		   kitten_pic, 0, 0,
		   kitten_x, all_y,
		   -1, -1,
		   GDK_RGB_DITHER_NONE, 0, 0);

  if (robot_x+robot_stop < kitten_x)
    {
      cycle_count++;
      robot_x += stepsize;
      kitten_x -= stepsize;
    }
  else
    {
      GdkPixbuf *scaled_love_pic =
	gdk_pixbuf_scale_simple (love_pic,
				 love_size,
				 love_size,
				 GDK_INTERP_BILINEAR);

      gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
		       gc,
		       scaled_love_pic, 0, 0,
		       robot_x + gdk_pixbuf_get_width (robot_pic) +
		       (gdk_pixbuf_get_width (love_pic)-love_size)/2,
		       all_y + (gdk_pixbuf_get_height (love_pic)-love_size)/2,
		       -1, -1,
		       GDK_RGB_DITHER_NONE, 0, 0);

      love_size += 10;

      if (love_size >= gdk_pixbuf_get_width (love_pic))
	{
	  /* all done! */
	  
	  vibrate ();
	  
	  animation_running = FALSE;

	  g_timeout_add (2000, ending_animation_quit, NULL);

	  gdk_gc_unref (gc);
	  love_size = 40;
	  cycle_count = 0;
	  robot_x = 0;
	  robot_stop = 0;
	  kitten_x = 0;
	  all_y = 0;
	  gc = NULL;
	}
    }

  return TRUE;
}

static gboolean
ending_animation_step (gpointer data)
{
  if (animation_running)
    {
      gdk_window_invalidate_rect (state_widget[STATE_EPILOGUE]->window,
				  NULL, TRUE);

      return TRUE;
    }
  else
    return FALSE;
}

static void
ending_animation ()
{
  if (current_state!=STATE_EPILOGUE)
    {
      animation_running = TRUE;
      g_timeout_add (10, ending_animation_step, NULL);
    }
}

/****************************************************************/
/* Moving the robot.  Way to go, robot!                         */
/****************************************************************/

typedef struct {
  guint gdk_key;
  gchar vi_key; /* or nethack equivalent */
  guint8 move_x;
  guint8 move_y;
} direction;

direction directions[] = {
  { GDK_Home,      'y', -1, -1 },
  { GDK_Left,      'h', -1,  0 },
  { GDK_End,       'b', -1,  1 },
  { GDK_Down,      'j',  0,  1 },
  { GDK_Page_Down, 'n',  1,  1 },
  { GDK_Right,     'l',  1,  0 },
  { GDK_Page_Up,   'u',  1, -1 },
  { GDK_Up,        'k',  0, -1 }
};

static gboolean
move_robot (guint8 whichway)
{
  GtkWidget *new_space;
  gint8 dx, dy;
  
  dx = directions[whichway].move_x;
  dy = directions[whichway].move_y;

  const char *found;

  if (robot_x+dx<0 ||
      robot_y+dy<0 ||
      robot_x+dx>=ARENA_WIDTH ||
      robot_y+dy>=ARENA_HEIGHT)
    return TRUE;

  new_space = arena[robot_x+dx][robot_y+dy];
  found = g_object_get_data (G_OBJECT (new_space), "examine");

  if (found && *found)
    {
      show_message (found);

      if (new_space == kitten)
	{
	  switch_state (STATE_EPILOGUE);
	}

      robot_demo_x = robot_demo_y = -1;
      g_object_set_data (G_OBJECT (new_space), "visited", "");
      return TRUE;
    }
  else
    {
      /* just an ordinary move into an empty space */

      g_object_ref (new_space);

      gtk_container_remove (GTK_CONTAINER (state_widget[STATE_PLAYING]), robot);
      gtk_container_remove (GTK_CONTAINER (state_widget[STATE_PLAYING]), new_space);

      place_in_arena_at_xy (new_space, robot_x, robot_y);
      place_in_arena_at_xy (robot, robot_x+dx, robot_y+dy);

      g_object_unref (new_space);

      return FALSE;
    }
}

static void
move_robot_randomly (void)
{
  guint8 whichway;
  gint8 dx, dy;
  guint8 i;

  for (i=0; i<100; i++)
    {
      whichway = random() % 8;
      dx = directions[whichway].move_x;
      dy = directions[whichway].move_y;

      /* Can't move off the edge. */
      if (robot_x+dx<0 ||
	  robot_y+dy<0 ||
	  robot_x+dx>=ARENA_WIDTH ||
	  robot_y+dy>=ARENA_HEIGHT)
	continue;

      /* Can't move onto an occupied square randomly. */
      if (g_object_get_data (G_OBJECT (arena[robot_x+dx][robot_y+dy]),
			     "examine"))
	continue;

      /* Success! */
      move_robot (whichway);
      return;
    }

  /* if we get here, robot is stuck or just very unlucky;
   * either way we do nothing.
   */
}

static gboolean
item_is_investigatable (guint8 x, guint8 y)
{
  GObject *item = G_OBJECT (arena[x][y]);

  if (g_object_get_data (item, "examine")==NULL)
    /* Empty space */
    return FALSE;

  if (g_object_get_data (item, "visited")!=NULL)
    /* Been there, done that */
    return FALSE;

  return TRUE;
}

static gboolean
move_robot_demo (gpointer dummy)
{
  gint8 whichway;
  gint8 dx, dy;

  if (current_state!=STATE_PLAYING)
    return FALSE;

  if (robot_demo_x == -1)
    {
      /* Find an item to investigate. */
      /* STUB: randomise robot_demo_x & y*/

      robot_demo_x = robot_demo_y = 0;

      while (!item_is_investigatable (robot_demo_x,
				      robot_demo_y))
	{
	  robot_demo_x++;
	  if (robot_demo_x==ARENA_WIDTH)
	    {
	      robot_demo_x=0;
	      robot_demo_y++;
	      if (robot_demo_y==ARENA_HEIGHT)
		{
		  robot_demo_y=0;
		}
	    }
	}
    }
  
  whichway = ((int) (7+atan2 (robot_x-robot_demo_x, robot_y-robot_demo_y)*4/M_PI)) % 8;
  dx = directions[whichway].move_x;
  dy = directions[whichway].move_y;

  if (!(robot_x+dx == robot_demo_x && robot_y+dy == robot_demo_y)
      &&
      g_object_get_data (G_OBJECT (arena[robot_x+dx][robot_y+dy]),
			 "examine"))
    {
      /* Can't move onto an occupied square, unless it's the target. */
      move_robot_randomly ();
    }
  else
    {
      move_robot (whichway);
    }

  return TRUE;
}

/****************************************************************/
/* Event handlers.                                              */
/****************************************************************/

static gboolean
on_window_clicked (GtkWidget      *widget,
		   GdkEventButton *event,
		   gpointer        user_data)
{
  int quarter_width, quarter_height;
  int quadrant;
  int directions[] = { 0, 7, 6, 1, -1, 5, 2, 3, 4 };

  if (current_state!=STATE_PLAYING)
    {
      return TRUE;
    }

  quarter_width = window->allocation.width/4;
  quarter_height = window->allocation.height/4;

  if (event->x < quarter_width)
    quadrant = 0;
  else if (event->x > quarter_width*3)
    quadrant = 2;
  else
    quadrant = 1;

  if (event->y < quarter_height)
    /* nothing */;
  else if (event->y > quarter_height*3)
    quadrant += 6;
  else
    quadrant += 3;

  quadrant = directions[quadrant];

  if (quadrant >= 0)
    move_robot (quadrant);

  return TRUE;
}

static gboolean
on_key_pressed (GtkWidget      *widget,
		GdkEventKey    *event,
		gpointer        user_data)
{
  gint i;
  guint keyval = event->keyval;

  if (current_state!=STATE_PLAYING)
    {
      return FALSE;
    }

  if (keyval>='A' && keyval<='Z')
    {
      keyval += ('a'-'A');
    }

  for (i=0; i<G_N_ELEMENTS(directions); i++)
    {
      if (keyval==directions[i].gdk_key ||
	  keyval==directions[i].vi_key)
	{
          int whichway = i;

	  if (portrait_mode)
             whichway = (whichway+6)%8;

	  if (event->state & GDK_SHIFT_MASK)
	    {
	      while (!move_robot (whichway))
		{
		  /* keep going, robot! */
		}
	    }
	  else
	    {
	      move_robot (whichway);
	    }
	  return FALSE;
	}
    }

  if (keyval=='q' && event->state & GDK_CONTROL_MASK)
    {
      /* secret debugging key */
      show_message (gtk_label_get_text (GTK_LABEL (kitten)));
    }
  else if (keyval=='r')
    {
      move_robot_randomly ();
    }
  else if (keyval=='d')
    {
      move_robot_demo (NULL);
    }

  return FALSE;
}

static void
play_game (gpointer button, gpointer data)
{
  demo_running = FALSE;
  switch_state (STATE_PLAYING);
}

static void
play_demo (gpointer button, gpointer data)
{
  demo_running = TRUE;
  g_timeout_add (DEMO_SPEED, move_robot_demo, NULL);
  switch_state (STATE_PLAYING);
}

static void
restart (gpointer button, gpointer data)
{
  if (current_state == STATE_EPILOGUE)
    {
      show_message ("Have patience while robotfindskitten.");
    }
  else
    {
      switch_state (STATE_PROLOGUE);
    }
}

static void
set_up_board (void)
{
  guint x, y;

  if (current_state==STATE_PLAYING)
    {
      /* end of the game; clean up */

      for (x=0; x < ARENA_WIDTH; x++)
	for (y=0; y < ARENA_HEIGHT; y++)
	  if (arena[x][y])
	    {
	      gtk_container_remove (GTK_CONTAINER (state_widget[STATE_PLAYING]),
				    arena[x][y]);
	      arena[x][y] = NULL;
	    }

      g_object_unref (robot);
      g_object_unref (kitten);
      robot_demo_x = robot_demo_y = -1;
    }
  else
    {
      /* make everything new */
  
      g_free (used);
      used = g_malloc0 (nki_count * sizeof(gboolean));

      robot = gtk_label_new ("#");
      g_object_ref (robot);
      kitten = random_character ("You found kitten!  Way to go, robot!");
      g_object_ref (kitten);

      place_in_arena_randomly (robot);
      place_in_arena_randomly (kitten);
      gtk_widget_modify_bg (kitten, GTK_STATE_NORMAL, &grey);

      if (nki_count < amount_of_random_stuff)
	{
	  /* sanity check failed */
	  show_message ("There are too few non-kitten items to play a meaningful game.");
	  exit (EXIT_FAILURE);
	}

      for (x=0; x < amount_of_random_stuff; x++)
	place_in_arena_randomly (random_character (description ()));

      for (x=0; x < ARENA_WIDTH; x++)
	for (y=0; y < ARENA_HEIGHT; y++)
	  if (!arena[x][y])
	    place_in_arena_at_xy (gtk_label_new (NULL), x, y);
    }
}

static gboolean
backdrop_draw (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GdkGC *gc = NULL;
  int quarter_width = event->area.width / 4;
  int quarter_height = event->area.height / 4;

  gc = gdk_gc_new (GDK_DRAWABLE (widget->window));
  gdk_gc_set_foreground (gc, &grey);
  gdk_draw_rectangle (GDK_DRAWABLE (widget->window),
		  gc,
		  TRUE,
		  quarter_width, 0,
		  quarter_width*2, quarter_height);
   gdk_draw_rectangle (GDK_DRAWABLE (widget->window),
		  gc,
		  TRUE,
		  quarter_width, quarter_height*3,
		  quarter_width*2, quarter_height);
   gdk_draw_rectangle (GDK_DRAWABLE (widget->window),
		  gc,
		  TRUE,
		  0, quarter_height,
		  quarter_width, quarter_height*2);
   gdk_draw_rectangle (GDK_DRAWABLE (widget->window),
		  gc,
		  TRUE,
		  quarter_width*3, quarter_height,
		  quarter_width, quarter_height*2);
  return FALSE;
}

static void
set_up_widgets (void)
{
  GtkWidget *middle = gtk_hbox_new (FALSE, 0);
  GtkWidget *buttons = gtk_hbox_new (TRUE, 0);
  GtkWidget *explain = NULL, *button, *intro;
  const char *explanation =
    "In this game, you are robot (#). "
    "Your job is to find kitten. This task is complicated "
    "by the existence of various things which are not kitten. "
    "Robot must touch items to determine if they are kitten or "
    "not. The game ends when robotfindskitten. You may move "
    "robot about by tapping on any side of the screen, or with the "
    "arrow keys.";
  GKeyFile *desktop = g_key_file_new ();
  gchar *version;
  guint x, y;
  HildonAppMenu *menu = HILDON_APP_MENU (hildon_app_menu_new ());
  
  /* The window */

  window = hildon_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), WINDOW_TITLE);
  gtk_widget_modify_bg (window, GTK_STATE_NORMAL, &black);
  g_signal_connect (G_OBJECT (window), "button-press-event", G_CALLBACK (on_window_clicked), NULL);
  g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (on_key_pressed), NULL);
  g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

  gdk_colormap_alloc_color (gdk_colormap_get_system (), &grey, TRUE, TRUE);

  /* The prologue */

  /* Get the rather odd version string.  The RFK spec says that
   * it should read v<major>.<minor>.<number-of-NKIs>.
   */
  if (g_key_file_load_from_file (desktop,
				 "/usr/share/applications/hildon/rfk.desktop",
				 G_KEY_FILE_NONE,
				 NULL))
    {
      version = g_strdup_printf("v%s.%d",
				g_key_file_get_value (desktop, "Desktop Entry", "Version", NULL),
				nki_count);
      g_key_file_free (desktop);
    }
  else
    {
      version = g_strdup("");
    }

  button = hildon_button_new_with_text (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_THUMB_HEIGHT,
					     HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
					     "Play", NULL);
  g_signal_connect (button, "clicked", G_CALLBACK (play_game), NULL);
  gtk_box_pack_end (GTK_BOX (buttons), button, TRUE, TRUE, 0);


  button = hildon_button_new_with_text (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_THUMB_HEIGHT,
					     HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
					     "Demo", NULL);
  g_signal_connect (button, "clicked", G_CALLBACK (play_demo), NULL);
  gtk_box_pack_end (GTK_BOX (buttons), button, TRUE, TRUE, 0);

  button = hildon_button_new_with_text (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_THUMB_HEIGHT,
					     HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
					     "Help", NULL);
  g_signal_connect (button, "clicked", G_CALLBACK (get_help), NULL);
  gtk_box_pack_end (GTK_BOX (buttons), button, TRUE, TRUE, 0);

  /* and another help button, this time for the menu */
  button = gtk_button_new_with_label ("Help");
  g_signal_connect (button, "clicked", G_CALLBACK (get_help), NULL);
  hildon_app_menu_append (menu, GTK_BUTTON (button));

  button = gtk_button_new_with_label ("Restart");
  g_signal_connect (button, "clicked", G_CALLBACK (restart), NULL);
  hildon_app_menu_append (menu, GTK_BUTTON (button));

  gtk_widget_show_all (GTK_WIDGET (menu));
  hildon_window_set_app_menu (HILDON_WINDOW (window), menu);

  explain = gtk_label_new (explanation);
  gtk_label_set_line_wrap (GTK_LABEL (explain), TRUE);

  gtk_box_pack_end (GTK_BOX (middle), gtk_image_new_from_pixbuf (kitten_pic), FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (middle), explain, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (middle), gtk_image_new_from_pixbuf (robot_pic), FALSE, FALSE, 0);

  intro = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_end (GTK_BOX (intro), buttons, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (intro), middle, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (intro), gtk_label_new (version), FALSE, FALSE, 0);
  g_free (version);

  state_widget[STATE_PROLOGUE] = intro;

  /* The game itself */
  state_widget[STATE_PLAYING] = gtk_table_new (ARENA_HEIGHT, ARENA_WIDTH, TRUE);
  g_signal_connect (state_widget[STATE_PLAYING], "parent-set", G_CALLBACK (set_up_board), NULL);

  for (x=0; x < ARENA_WIDTH; x++)
    for (y=0; y < ARENA_HEIGHT; y++)
      arena[x][y] = NULL;
  
//  GtkWidget *backdrop = gtk_drawing_area_new ();
  if (1)
  g_signal_connect (G_OBJECT (state_widget[STATE_PLAYING]),
		  "expose_event",
		  G_CALLBACK (backdrop_draw), NULL);
  /*
  gtk_table_attach_defaults (GTK_TABLE (state_widget[STATE_PLAYING]),
			     backdrop,
			     0, 10,
			     0, 10);
			     */

  /* The epilogue */
  state_widget[STATE_EPILOGUE] =  gtk_drawing_area_new ();
  g_signal_connect (state_widget[STATE_EPILOGUE], "parent-set", G_CALLBACK (ending_animation), NULL);
  g_signal_connect (G_OBJECT (state_widget[STATE_EPILOGUE]),
		    "expose_event", G_CALLBACK (ending_animation_draw), NULL);

  for (x=0; x<STATE_LAST; x++)
    {
      /* so we don't lose them when we take them offscreen */
      g_object_ref (state_widget[x]);
    }
}

/****************************************************************/
/* Let's kick the whole thing off...                            */
/****************************************************************/

int
main (gint argc,
      gchar **argv)
{
  gtk_init (&argc, &argv);
  g_set_application_name ("robotfindskitten");
  srandom (time(0));

  ensure_messages_loaded ();
  load_images ();

  set_up_widgets ();
  check_for_rotation ();
  switch_state (STATE_PROLOGUE);
  
  gtk_main ();

  return EXIT_SUCCESS;
}
