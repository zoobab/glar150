/*  =========================================================================
    glar_lamp_fsm - No title state machine engine

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: glar_lamp.xml, or
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
    lamp_on_event = 1,
    lamp_off_event = 2,
    maybe_event = 3,
    sleep_event = 4,
    repeat_event = 5,
    finished_event = 6
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
    "lamp_on",
    "lamp_off",
    "maybe",
    "sleep",
    "repeat",
    "finished"
};

//  Action prototypes
static void set_lamp_on (glar_lamp_t *self);
static void get_next_command (glar_lamp_t *self);
static void set_lamp_off (glar_lamp_t *self);
static void collect_timeout_value (glar_lamp_t *self);
static void sleep_as_specified (glar_lamp_t *self);
static void start_sequence_again (glar_lamp_t *self);
static void prepare_blocking_poll (glar_lamp_t *self);

//  This is the context block for a FSM thread; use the setter
//  methods to set the FSM properties.

typedef struct {
    glar_lamp_t *parent;        //  Parent class
    bool animate;               //  Animate state machine
    state_t state;              //  Current state
    event_t event;              //  Current event
    event_t next_event;         //  The next event
    event_t exception;          //  Exception event, if any
    uint64_t cycles;            //  Track the work done
} fsm_t;

static fsm_t *
fsm_new (glar_lamp_t *parent)
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
glar_lamp_not_used (fsm_t *self)
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
            zsys_debug ("glar_lamp: %s:", s_state_name [self->state]);
            zsys_debug ("glar_lamp:         %s", s_event_name [self->event]);
        }
        if (self->state == have_command_state) {
            if (self->event == lamp_on_event) {
                if (!self->exception) {
                    //  set_lamp_on
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ set_lamp_on");
                    set_lamp_on (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == lamp_off_event) {
                if (!self->exception) {
                    //  set_lamp_off
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ set_lamp_off");
                    set_lamp_off (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == maybe_event) {
                if (!self->exception) {
                    //  collect_timeout_value
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ collect_timeout_value");
                    collect_timeout_value (self->parent);
                }
            }
            else
            if (self->event == sleep_event) {
                if (!self->exception) {
                    //  collect_timeout_value
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ collect_timeout_value");
                    collect_timeout_value (self->parent);
                }
                if (!self->exception) {
                    //  sleep_as_specified
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ sleep_as_specified");
                    sleep_as_specified (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == repeat_event) {
                if (!self->exception) {
                    //  start_sequence_again
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ start_sequence_again");
                    start_sequence_again (self->parent);
                }
                if (!self->exception) {
                    //  get_next_command
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ get_next_command");
                    get_next_command (self->parent);
                }
            }
            else
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  prepare_blocking_poll
                    if (self->animate)
                        zsys_debug ("glar_lamp:             $ prepare_blocking_poll");
                    prepare_blocking_poll (self->parent);
                }
            }
            else {
                //  Handle unexpected internal events
                zsys_warning ("glar_lamp: unhandled event %s in %s",
                    s_event_name [self->event], s_state_name [self->state]);
                exit (-1);
            }
        }
        //  If we had an exception event, interrupt normal programming
        if (self->exception) {
            if (self->animate)
                zsys_debug ("glar_lamp:             ! %s", s_event_name [self->exception]);
            self->next_event = self->exception;
        }
        else
        if (self->animate)
            zsys_debug ("glar_lamp:             > %s", s_state_name [self->state]);
    }
}
