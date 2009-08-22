/*  robotfindskitten for maemo
 *  original by Leonard Richardson, 1997
 *  ported to maemo by Thomas Thurman, 2009
 *  suggestions welcome
 *  Compile with:
 *  gcc -Wall -g rfk.c -o rfk `pkg-config --cflags --libs gtk+-2.0 hildon-1`
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <glib.h>
#include <hildon/hildon.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define ARENA_WIDTH 25
#define ARENA_HEIGHT 12

const int amount_of_random_stuff = 15;

const char *explanation =
  "In this game, you are robot (#). "
  "Your job is to find kitten. This task is complicated "
  "by the existence of various things which are not kitten. "
  "Robot must touch items to determine if they are kitten or "
  "not. The game ends when robotfindskitten. You may move "
  "robot about by tapping on any side of robot, or with the "
  "cursor keys.";

GSList *nki = NULL;
guint nki_count = 0;

GtkWidget *arena[ARENA_WIDTH][ARENA_HEIGHT];
GtkWidget *table, *window, *robot, *kitten;
int robot_x, robot_y;
gboolean *used = NULL;

GdkPixbuf *robot_pic, *love_pic, *kitten_pic;
GtkWidget *animation_area;

const GdkColor black = { 0, };

/****************************************************************/
/* Random object descriptions.                                  */
/****************************************************************/

char *
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

void
place_in_arena_at_xy (GtkWidget *item, int x, int y)
{
  arena[x][y] = item;

  gtk_table_attach_defaults (GTK_TABLE (table),
			     item,
			     x, x+1,
			     y, y+1);

  if (item==robot)
    {
      robot_x = x;
      robot_y = y;
    }
}

void
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

GtkWidget *
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

void
show_message (const char *message)
{
  HildonNote* note = HILDON_NOTE
    (hildon_note_new_information (GTK_WINDOW (window),
				  message));
  gtk_dialog_run (GTK_DIALOG (note));
  gtk_widget_destroy (GTK_WIDGET (note));
}

/****************************************************************/
/* Loading the non-kitten objects.                              */
/****************************************************************/
void
ensure_messages_loaded (void)
{
  FILE *nki_file = NULL;
  gchar *line = NULL;
  gboolean headers = TRUE;

  if (nki_count)
    return;

  nki_file = fopen ("/usr/share/rfk/non-kitten-items.rfk", "r");

  if (!nki_file)
    {
      show_message ("Could not open list of non-kitten items!  Must quit.");
      exit (EXIT_FAILURE);
    }

  while (!feof (nki_file))
    {
      char newline;
      fscanf (nki_file, "%a[^\n]%c", &line, &newline);

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

  used = g_malloc0 (nki_count);
}

/****************************************************************/
/* The ending animation.                                        */
/****************************************************************/

static gboolean
ending_animation_quit (gpointer data)
{
  gtk_main_quit ();
  return FALSE;
}

static gboolean
ending_animation_draw (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  /* We only run through once, so just make it static. */
  static int cycle_count = 0;

  static int robot_x = 0;
  static int robot_stop = 0;
  static int kitten_x = 0;
  static int all_y = 0;

  const int stepsize = 3;

  if (!kitten_x)
    {
      all_y = (event->area.height - gdk_pixbuf_get_height (love_pic)) / 2;

      robot_stop = gdk_pixbuf_get_width (robot_pic) + gdk_pixbuf_get_width (love_pic);
      kitten_x = event->area.width - (cycle_count*stepsize + gdk_pixbuf_get_width (kitten_pic));
    }

  gdk_gc_set_foreground (widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			 &black);

  gdk_draw_rectangle (GDK_DRAWABLE(widget->window),
		      widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		      TRUE,
		      0, 0, event->area.width, event->area.height);

  gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
		   widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		   robot_pic, 0, 0,
		   robot_x, all_y,
		   -1, -1,
		   GDK_RGB_DITHER_NONE, 0, 0);

  gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
		   widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		   kitten_pic, 0, 0,
		   kitten_x, all_y,
		   -1, -1,
		   GDK_RGB_DITHER_NONE, 0, 0);

  cycle_count++;
  robot_x += stepsize;
  kitten_x -= stepsize;

  if (robot_x+robot_stop >= kitten_x)
    {
      gdk_draw_pixbuf (GDK_DRAWABLE(widget->window),
		       widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		       love_pic, 0, 0,
		       robot_x + gdk_pixbuf_get_width (robot_pic), all_y,
		       -1, -1,
		       GDK_RGB_DITHER_NONE, 0, 0);

      g_object_unref (love_pic);
      love_pic = NULL;

      g_timeout_add (2000, ending_animation_quit, NULL);
    }

  return TRUE;
}

static gboolean
ending_animation_step (gpointer data)
{
  if (love_pic)
    {
      gdk_window_invalidate_rect (animation_area->window,
				  NULL, TRUE);

      return TRUE;
    }
  else
    return FALSE;
}

static void
ending_animation ()
{
  robot_pic = gdk_pixbuf_new_from_file ("/usr/share/rfk/rfk-robot.png", NULL);
  love_pic = gdk_pixbuf_new_from_file ("/usr/share/rfk/rfk-love.png", NULL);
  kitten_pic = gdk_pixbuf_new_from_file ("/usr/share/rfk/rfk-kitten.png", NULL);
  animation_area =  gtk_drawing_area_new ();

  gtk_container_remove (GTK_CONTAINER (window), GTK_WIDGET (table));
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (animation_area));
  gtk_widget_show_all (window);

  g_signal_connect (G_OBJECT (animation_area),
		    "expose_event", G_CALLBACK (ending_animation_draw), NULL);
  g_timeout_add (10, ending_animation_step, NULL);
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

gboolean
move_robot (guint8 whichway)
{
  GtkWidget *new_space;
  gint8 dx = directions[whichway].move_x;
  gint8 dy = directions[whichway].move_y;

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
	  ending_animation ();
	}

      return TRUE;
    }
  else
    {
      /* just an ordinary move into an empty space */

      g_object_ref (new_space);

      gtk_container_remove (GTK_CONTAINER (table), robot);
      gtk_container_remove (GTK_CONTAINER (table), new_space);

      place_in_arena_at_xy (new_space, robot_x, robot_y);
      place_in_arena_at_xy (robot, robot_x+dx, robot_y+dy);

      g_object_unref (new_space);

      return FALSE;
    }
}

/****************************************************************/
/* Event handlers.                                              */
/****************************************************************/

gboolean
on_window_clicked (GtkWidget      *widget,
		   GdkEventButton *event,
		   gpointer        user_data)
{
  /** Centre point of robot's representation on screen */
  int rx, ry;
  double angle;

  rx = (robot->allocation.x+robot->allocation.width/2);
  ry = (robot->allocation.y+robot->allocation.height/2);

  angle = atan2(event->x - rx,
		event->y - ry) +
    M_PI +
    M_PI/8;

  move_robot (((int) (angle / (M_PI/4)))-1);

  return TRUE;
}

gboolean
on_key_pressed (GtkWidget      *widget,
		GdkEventKey    *event,
		gpointer        user_data)
{
  gint i;
  guint keyval = event->keyval;

  if (keyval>='A' && keyval<='Z')
    {
      keyval += ('a'-'A');
    }

  for (i=0; i<G_N_ELEMENTS(directions); i++)
    {
      if (keyval==directions[i].gdk_key ||
	  keyval==directions[i].vi_key)
	{
	  if (event->state & GDK_SHIFT_MASK)
	    {
	      while (!move_robot (i))
		{
		  /* keep going, robot! */
		}
	    }
	  else
	    {
	      move_robot (i);
	    }
	  return FALSE;
	}
    }

  return FALSE;
}

/****************************************************************/
/* Let's kick the whole thing off...                            */
/****************************************************************/

int
main (gint argc,
      gchar **argv)
{
  int x, y;

  gtk_init (&argc, &argv);
  g_set_application_name ("robotfindskitten");
  srandom (time(0));

  ensure_messages_loaded ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "robotfindskitten");
  gtk_widget_modify_bg (window, GTK_STATE_NORMAL, &black);
  g_signal_connect (G_OBJECT (window), "button-press-event", G_CALLBACK (on_window_clicked), NULL);
  g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (on_key_pressed), NULL);
  g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (gtk_main_quit), NULL);
	
  table = gtk_table_new (ARENA_HEIGHT, ARENA_WIDTH, TRUE);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (table));

  robot = gtk_label_new ("#");
  g_object_ref (robot);
  kitten = random_character ("You found kitten!  Way to go, robot!");

  place_in_arena_randomly (robot);
  place_in_arena_randomly (kitten);

  for (x=0; x < amount_of_random_stuff; x++)
    place_in_arena_randomly (random_character (description ()));

  for (x=0; x < ARENA_WIDTH; x++)
    for (y=0; y < ARENA_HEIGHT; y++)
      if (!arena[x][y])
	place_in_arena_at_xy (gtk_label_new (NULL), x, y);

  gtk_widget_show_all (window);

  gdk_window_set_events (GTK_WIDGET (window)->window,
			 gdk_window_get_events(GTK_WIDGET (window)->window) | GDK_BUTTON_PRESS_MASK);

	
  show_message (explanation);

  gtk_main ();

  return EXIT_SUCCESS;
}
