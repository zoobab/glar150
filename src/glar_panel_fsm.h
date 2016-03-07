/*  =========================================================================
    glar_panel_fsm - No title state machine engine

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: glar_panel.xml, or
     * The code generation script that built this file: fsm_c
    ************************************************************************
    =========================================================================
*/


//  ---------------------------------------------------------------------------
//  State machine constants

typedef enum {
    have_command_state = 1
} state_t;

typedef enum {
    NULL_event = 0,
    bitmap_event = 1,
    long_pause_event = 2,
    short_pause_event = 3,
    long_poll_event = 4,
    short_poll_event = 5,
    repeat_event = 6,
    finished_event = 7
} event_t;

//  Names for state machine logging and error reporting
static char *
s_state_name [] = {
    "(NONE)",
    "have_command"
};

static char *
s_event_name [] = {
    "(NONE)",
    "bitmap",
    "long_pause",
    "short_pause",
    "long_poll",
    "short_poll",
    "repeat",
    "finished"
};

//  Action prototypes
static void collect_full_bitmap (glar_panel_t *self);
static void set_leds_as_specified (glar_panel_t *self);
static void get_next_command (glar_panel_t *self);
static void do_long_sleep (glar_panel_t *self);
static void do_short_sleep (glar_panel_t *self);
static void prepare_long_poll (glar_panel_t *self);
static void prepare_short_poll (glar_panel_t *self);
static void start_sequence_again (glar_panel_t *self);
static void prepare_blocking_poll (glar_panel_t *self);

//  This is the context block for a FSM thread; use the setter
//  methods to set the FSM properties.

typedef struct {
    glar_panel_t *parent;       //  Parent class
    bool animate;               //  Animate state machine
    state_t state;              //  Current state
    event_t event;              //  Current event
    event_t next_event;         //  The next event
    event_t exception;          //  Exception event, if any
    uint64_t cycles;            //  Track the work done
} fsm_t;

static fsm_t *
fsm_new (glar_panel_t *parent)
{
    fsm_t *self = (fsm_t *) zmalloc (sizeof (fsm_t));
    if (self) {
        self->state = have_command_state;
        self->event = NULL_event;
        self->parent = parent;
    }
    return self;
}

static void
fsm_destroy (fsm_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        fsm_t *self = *self_p;
        free (self);
        *self_p = NULL;
    }
}

static void
fsm_set_next_event (fsm_t *self, event_t next_event)
{
    if (self)
        self->next_event = next_event;
}

static void
fsm_set_exception (fsm_t *self, event_t exception)
{
    if (self)
        self->exception = exception;
}

static void
fsm_set_animate (fsm_t *self, bool animate)
{
    if (self)
        self->animate = animate;
}

static uint64_t
fsm_cycles (fsm_t *self)
{
    if (self)
        return self->cycles;
    else
        return 0;
}

//  Stops annoying compiler warnings on unused functions
void
glar_panel_not_used (fsm_t *self)
{
    fsm_set_next_event (NULL, NULL_event);
    fsm_set_exception (NULL, NULL_event);
    fsm_set_animate (NULL, 0);
    fsm_cycles (NULL);
}


//  Execute state machine until it has no next event. Before calling this
//  you must have set the next event using fsm_set_next_event(). Ends when
//  there is no next event set.

static void
fsm_execute (fsm_t *self)
{
    while (self->next_event != NULL_event) {
        self->cycles++;
        self->event = self->next_event;
        self->next_event = NULL_event;
        self->exception = NULL_event;
        if (self->animate) {
            zsys_debug ("glar_panel: %s:", s_state_name [self->state]);
            zsys_debug ("glar_panel:        %s", s_event_name [self->event]);
        }
        if (self->state == have_command_state) {
            if (self->event == bitmap_event) {
                if (!self->exception) {
                    //  collect_full_bitmap
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ collect_full_bitmap");
                    collect_full_bitmap (self->parent);
                }
                if (!self->exception) {
                    //  set_leds_as_specified
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ set_leds_as_specified");
                    set_leds_as_specified (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == long_pause_event) {
                if (!self->exception) {
                    //  do_long_sleep
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ do_long_sleep");
                    do_long_sleep (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == short_pause_event) {
                if (!self->exception) {
                    //  do_short_sleep
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ do_short_sleep");
                    do_short_sleep (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == long_poll_event) {
                if (!self->exception) {
                    //  prepare_long_poll
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ prepare_long_poll");
                    prepare_long_poll (self->parent);
                }
            }
            else
            if (self->event == short_poll_event) {
                if (!self->exception) {
                    //  prepare_short_poll
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ prepare_short_poll");
                    prepare_short_poll (self->parent);
                }
            }
            else
            if (self->event == repeat_event) {
                if (!self->exception) {
                    //  start_sequence_again
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ start_sequence_again");
                    start_sequence_again (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  prepare_blocking_poll
                    if (self->animate)
                        zsys_debug ("glar_panel:            $ prepare_blocking_poll");
                    prepare_blocking_poll (self->parent);
                }
            }
            else {
                //  Handle unexpected internal events
                zsys_warning ("glar_panel: unhandled event %s in %s",
                    s_event_name [self->event], s_state_name [self->state]);
                exit (-1);
            }
        }
        //  If we had an exception event, interrupt normal programming
        if (self->exception) {
            if (self->animate)
                zsys_debug ("glar_panel:            ! %s", s_event_name [self->exception]);
            self->next_event = self->exception;
        }
        else
        if (self->animate)
            zsys_debug ("glar_panel:            > %s", s_state_name [self->state]);
    }
}
