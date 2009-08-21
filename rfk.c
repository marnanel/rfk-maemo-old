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

static char* messages[] =
  {
    "\"I pity the fool who mistakes me for kitten!\", sez Mr. T.",
    "That's just an old tin can.",
    "It's an altar to the horse god.",
    "A box of dancing mechanical pencils. They dance! They sing!",
    "It's an old Duke Ellington record.",
    "A box of fumigation pellets.",
    "A digital clock. It's stuck at 2:17 PM.",
    "That's just a charred human corpse.",
    "I don't know what that is, but it's not kitten.",
    "An empty shopping bag. Paper or plastic?",
    "Could it be... a big ugly bowling trophy?",
    "A coat hanger hovers in thin air. Odd.",
    "Not kitten, just a packet of Kool-Aid(tm).",
    "A freshly-baked pumpkin pie.",
    "A lone, forgotten comma, sits here, sobbing.",
    "ONE HUNDRED THOUSAND CARPET FIBERS!!!!!",
    "It's Richard Nixon's nose!",
    "It's Lucy Ricardo. \"Aaaah, Ricky!\", she says.",
    "You stumble upon Bill Gates' stand-up act.",
    "Just an autographed copy of the Kama Sutra.",
    "It's the Will Rogers Highway. Who was Will Rogers, anyway?",
    "It's another robot, more advanced in design than you but strangely immobile.",
    "Leonard Richardson is here, asking people to lick him.",
    "It's a stupid mask, fashioned after a beagle.",
    "Your State Farm Insurance(tm) representative!",
    "It's the local draft board.",
    "Seven 1/4\" screws and a piece of plastic.",
    "An 80286 machine.",
    "One of those stupid \"Homes of the Stars\" maps.",
    "A signpost saying \"TO KITTEN\". It points in no particular direction.",
    "A hammock stretched between a tree and a volleyball pole.",
    "A Texas Instruments of Destruction calculator.",
    "It's a dark, amphorous blob of matter.",
    "Just a pincushion.",
    "It's a mighty zombie talking about some love and prosperity.",
    "\"Dear robot, you may have already won our 10 MILLION DOLLAR prize...\"",
    "It's just an object.",
    "A mere collection of pixels.",
    "A badly dented high-hat cymbal lies on its side here.",
    "A marijuana brownie.",
    "A plush Chewbacca.",
    "Daily hunger conditioner from Australasia",
    "Just some stuff.",
    "Why are you touching this when you should be finding kitten?",
    "A glorious fan of peacock feathers.",
    "It's some compromising photos of Babar the Elephant.",
    "A copy of the Weekly World News. Watch out for the chambered nautilus!",
    "It's the proverbial wet blanket.",
    "A \"Get Out of Jail Free\" card.",
    "An incredibly expensive \"Mad About You\" collector plate.",
    "Paul Moyer's necktie.",
    "A haircut and a real job. Now you know where to get one!",
    "An automated robot-hater. It frowns disapprovingly at you.",
    "An automated robot-liker. It smiles at you.",
    "It's a black hole. Don't fall in!",
    "Just a big brick wall.",
    "You found kitten! No, just kidding.",
    "Heart of Darkness brand pistachio nuts.",
    "A smoking branding iron shaped like a 24-pin connector.",
    "It's a Java applet.",
    "An abandoned used-car lot.",
    "A shameless plug for Maemo.",
    "A shameless plug for the UCLA Linux Users Group: http://linux.ucla.edu/",
    "A can of Spam Lite.",
    "This is another fine mess you've gotten us into, Stanley.",
    "It's scenery for \"Waiting for Godot\".",
    "This grain elevator towers high above you.",
    "A Mentos wrapper.",
    "It's the constellation Pisces.",
    "It's a fly on the wall. Hi, fly!",
    "This kind of looks like kitten, but it's not.",
    "It's a banana! Oh, joy!",
    "A helicopter has crashed here.",
    "Carlos Tarango stands here, doing his best impression of Pat Smear.",
    "A patch of mushrooms grows here.",
    "A patch of grape jelly grows here.",
    "A spindle, and a grindle, and a bucka-wacka-woom!",
    "A geyser sprays water high into the air.",
    "A toenail? What good is a toenail?",
    "You've found the fish! Not that it does you much good in this game.",
    "A Buttertonsils bar.",
    "One of the few remaining discoes.",
    "Ah, the uniform of a Revolutionary-era minuteman.",
    "A punch bowl, filled with punch and lemon slices.",
    "It's nothing but a G-thang, baby.",
    "IT'S ALIVE! AH HA HA HA HA!",
    "This was no boating accident!",
    "Wait! This isn't the poker chip! You've been tricked! DAMN YOU, MENDEZ!",
    "A livery stable! Get your livery!",
    "It's a perpetual immobility machine.",
    "\"On this spot in 1962, Henry Winkler was sick.\"",
    "There's nothing here; it's just an optical illusion.",
    "The World's Biggest Motzah Ball!",
    "A tribe of cannibals lives here. They eat Malt-O-Meal for breakfast, you know.",
    "This appears to be a rather large stack of trashy romance novels.",
    "Look out! Exclamation points!",
    "A herd of wild coffee mugs slumbers here.",
    "It's a limbo bar! How low can you go?",
    "It's the horizon. Now THAT'S weird.",
    "A vase full of artificial flowers is stuck to the floor here.",
    "A large snake bars your way.",
    "A pair of saloon-style doors swing slowly back and forth here.",
    "It's an ordinary bust of Beethoven... but why is it painted green?",
    "It's TV's lovable wisecracking Crow! \"Bite me!\", he says.",
    "Hey, look, it's war. What is it good for? Absolutely nothing. Say it again.",
    "It's the amazing self-referential thing that's not kitten.",
    "A flamboyant feather boa. Now you can dress up like Carol Channing!",
    "\"Sure hope we get some rain soon,\" says Farmer Joe.",
    "\"How in heck can I wash my neck if it ain't gonna rain no more?\" asks Farmer Al.",
    "\"Topsoil's all gone, ma,\" weeps Lil' Greg.",
    "This is a large brown bear. Oddly enough, it's currently peeing in the woods.",
    "A team of arctic explorers is camped here.",
    "This object here appears to be Louis Farrakhan's bow tie.",
    "This is the world-famous Chain of Jockstraps.",
    "A trash compactor, compacting away.",
    "This toaster strudel is riddled with bullet holes!",
    "It's a hologram of a crashed helicopter.",
    "This is a television. On screen you see a robot strangely similar to yourself.",
    "This balogna has a first name, it's R-A-N-C-I-D.",
    "A salmon hatchery? Look again. It's merely a single salmon.",
    "It's a rim shot. Ba-da-boom!",
    "It's creepy and it's kooky, mysterious and spooky. It's also somewhat ooky.",
    "This is an anagram.",
    "This object is like an analogy.",
    "It's a symbol. You see in it a model for all symbols everywhere.",
    "The object pushes back at you.",
    "A traffic signal. It appears to have been recently vandalized.",
    "\"There is no kitten!\" cackles the old crone. You are shocked by her blasphemy.",
    "This is a Lagrange point. Don't come too close now.",
    "The dirty old tramp bemoans the loss of his harmonica.",
    "Look, it's Fanny the Irishman!",
    "What in blazes is this?",
    "It's the instruction manual for a previous version of this game.",
    "A brain cell. Oddly enough, it seems to be functioning.",
    "Tea and/or crumpets.",
    "This jukebox has nothing but Cliff Richards albums in it.",
    "It's a Quaker Oatmeal tube, converted into a drum.",
    "This is a remote control. Being a robot, you keep a wide berth.",
    "It's a roll of industrial-strength copper wire.",
    "Oh boy! Grub! Er, grubs.",
    "A puddle of mud, where the mudskippers play.",
    "Plenty of nothing.",
    "Look at that, it's the Crudmobile.",
    "Just Walter Mattheau and Jack Lemmon.",
    "Two crepes, two crepes in a box.",
    "An autographed copy of \"Primary Colors\", by Anonymous.",
    "Another rabbit? That's three today!",
    "It's a segmentation fault. Core dumped, by the way.",
    "A historical marker showing the actual location of /dev/null.",
    "Thar's Mobius Dick, the convoluted whale. Arrr!",
    "It's a charcoal briquette, smoking away.",
    "A pizza, melting in the sun.",
    "It's a \"HOME ALONE 2: Lost in New York\" novelty cup.",
    "A stack of 7 inch floppies wobbles precariously.",
    "It's nothing but a corrupted floppy. Coaster anyone?",
    "A section of glowing phosphor cells sings a song of radiation to you.",
    "This TRS-80 III is eerily silent.",
    "A toilet bowl occupies this space.",
    "This peg-leg is stuck in a knothole!",
    "It's a solitary vaccuum tube.",
    "This corroded robot is clutching a mitten.",
    "\"Hi, I'm Anson Williams, TV's 'Potsy'.\"",
    "This subwoofer was blown out in 1974.",
    "Three half-pennies and a wooden nickel.",
    "It's the missing chapter to \"A Clockwork Orange\".",
    "It's a burrito stand flyer. \"Taqueria El Ranchito\".",
    "This smiling family is happy because they eat LARD.",
    "Roger Avery, persona un famoso de los Estados Unidos.",
    "Ne'er but a potted plant.",
    "A parrot, kipping on its back.",
    "A forgotten telephone switchboard.",
    "A forgotten telephone switchboard operator.",
    "It's an automated robot-disdainer. It pretends you're not there.",
    "It's a portable hole. A sign reads: \"Closed for the winter\".",
    "Just a moldy loaf of bread.",
    "A little glass tub of Carmex. ($.89) Too bad you have no lips.",
    "A Swiss-Army knife. All of its appendages are out. (toothpick lost)",
    "It's a zen simulation, trapped within an ASCII character.",
    "It's a copy of \"The Rubaiyat of Spike Schudy\".",
    "It's \"War and Peace\" (unabridged, very small print).",
    "A willing, ripe tomato bemoans your inability to digest fruit.",
    "A robot comedian. You feel amused.",
    "It's KITT, the talking car.",
    "Here's Pete Peterson. His batteries seem to have long gone dead.",
    "\"Blup, blup, blup\", says the mud pot.",
    "More grist for the mill.",
    "Grind 'em up, spit 'em out, they're twigs.",
    "The boom box cranks out an old Ethel Merman tune.",
    "It's \"Finding kitten\", published by O'Reilly and Associates.",
    "Pumpkin pie spice.",
    "It's the Bass-Matic '76! Mmm, that's good bass!",
    "\"Lend us a fiver 'til Thursday\", pleas Andy Capp.",
    "It's a tape of '70s rock. All original hits! All original artists!",
    "You've found the fabled America Online disk graveyard!",
    "Empty jewelboxes litter the landscape.",
    "It's the astounding meta-object.",
    "Ed McMahon stands here, lost in thought. Seeing you, he bellows, \"YES SIR!\"",
    "...thingy???",
    "It's 1000 secrets the government doesn't want you to know!",
    "The letters O and R.",
    "A magical... magic thing.",
    "It's a moment of silence.",
    "It's Sirhan-Sirhan, looking guilty.",
    "It's \"Chicken Soup for the Kitten-seeking Soulless Robot.\"",
    "It is a set of wind-up chatter teeth.",
    "It is a cloud shaped like an ox.",
    "You see a snowflake here, melting slowly.",
    "It's a big block of ice. Something seems to be frozen inside it.",
    "Vladimir Lenin's casket rests here.",
    "It's a copy of \"Zen and The Art of Robot Maintenance\".",
    "This invisible box contains a pantomime horse.",
    "A mason jar lies here open. It's label reads: \"do not open!\".",
    "A train of thought chugs through here.",
    "This jar of pickles expired in 1957.",
    "Someone's identity disk lies here.",
    "\"Yes!\" says the bit.",
    "\"No!\" says the bit.",
    "A dodecahedron bars your way.",
    "Mr. Hooper is here, surfing.",
    "It's a big smoking fish.",
    "You have new mail in /var/spool/robot",
    "Just a monitor with the blue element burnt out.",
    "A pile of coaxial plumbing lies here.",
    "It's a rotten old shoe.",
    "It's a hundred-dollar bill.",
    "It's a Dvorak keyboard.",
    "It's a cardboard box full of 8-tracks.",
    "Just a broken hard drive containg the archives of Nerth Pork.",
    "A broken metronome sits here, it's needle off to one side.",
    "A sign reads: \"Go home!\"",
    "A sign reads: \"No robots allowed!\"",
    "It's the handheld robotfindskitten game, by Tiger.",
    "This particular monstrosity appears to be ENIAC.",
    "This is a tasty-looking banana creme pie.",
    "A wireframe model of a hot dog rotates in space here.",
    "Just the empty husk of a locust.",
    "You disturb a murder of crows.",
    "It's a copy of the robotfindskitten EULA.",
    "It's Death.",
    "It's an autographed copy of \"Secondary Colors,\" by Bob Ross.",
    "It is a marzipan dreadnought that appears to have melted and stuck.",
    "It's a DVD of \"Crouching Monkey, Hidden Kitten\", region encoded for the moon.",
    "It's Kieran Hervold.  Damn dyslexia!",
    "A non-descript box of crackers.",
    "Carbonated Water, High Fructose Corn Syrup, Color, Phosphoric Acid, Flavors, Caffeine.",
    "\"Move along! Nothing to see here!\"",
    "It's the embalmed corpse of Vladimir Lenin.",
    "A coupon for one free steak-fish at your local family diner.",
    "A set of keys to a 2001 Rolls Royce. Worthless."
  };

GtkWidget *arena[ARENA_WIDTH][ARENA_HEIGHT];
GtkWidget *table, *window, *robot, *kitten;
int robot_x, robot_y;
gboolean used[G_N_ELEMENTS (messages)] = { 0, };

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
      r = random() % G_N_ELEMENTS (messages);
    }
  while (used[r]);

  used[r] = TRUE;
  return messages[r];
	
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
  g_set_application_name ("robot finds kitten");
  srandom (time(0));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "robot finds kitten");
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

  return 0;
}
