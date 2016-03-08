/*  =========================================================================
    glar_node_fsm - No title state machine engine

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: glar_node.xml, or
     * The code generation script that built this file: fsm_c
    ************************************************************************
    =========================================================================
*/


//  ---------------------------------------------------------------------------
//  State machine constants

typedef enum {
    start_state = 1,
    as_console_state = 2,
    as_robot_state = 3,
    emergency_state = 4,
    defaults_state = 5
} state_t;

typedef enum {
    NULL_event = 0,
    console_event = 1,
    robot_event = 2,
    console_command_event = 3,
    whisper_event = 4,
    shout_event = 5,
    button_on_event = 6,
    button_off_event = 7,
    nothing_event = 8,
    finished_event = 9,
    join_event = 10,
    leave_event = 11,
    other_event = 12
} event_t;

//  Names for state machine logging and error reporting
static char *
s_state_name [] = {
    "(NONE)",
    "start",
    "as_console",
    "as_robot",
    "emergency",
    "defaults"
};

static char *
s_event_name [] = {
    "(NONE)",
    "console",
    "robot",
    "console_command",
    "whisper",
    "shout",
    "button_on",
    "button_off",
    "nothing",
    "finished",
    "join",
    "leave",
    "other"
};

//  Action prototypes
static void join_network_as_console (glar_node_t *self);
static void wait_for_activity (glar_node_t *self);
static void join_network_as_robot (glar_node_t *self);
static void shout_command_to_robots (glar_node_t *self);
static void print_command_results (glar_node_t *self);
static void execute_the_command (glar_node_t *self);
static void show_at_rest_sequence (glar_node_t *self);
static void signal_button_on (glar_node_t *self);
static void start_emergency_sequence (glar_node_t *self);
static void check_for_activity (glar_node_t *self);
static void signal_button_off (glar_node_t *self);
static void stop_emergency_sequence (glar_node_t *self);
static void leave_network (glar_node_t *self);
static void signal_peer_joined (glar_node_t *self);
static void signal_peer_left (glar_node_t *self);

//  This is the context block for a FSM thread; use the setter
//  methods to set the FSM properties.

typedef struct {
    glar_node_t *parent;        //  Parent class
    bool animate;               //  Animate state machine
    state_t state;              //  Current state
    event_t event;              //  Current event
    event_t next_event;         //  The next event
    event_t exception;          //  Exception event, if any
    uint64_t cycles;            //  Track the work done
} fsm_t;

static fsm_t *
fsm_new (glar_node_t *parent)
{
    fsm_t *self = (fsm_t *) zmalloc (sizeof (fsm_t));
    if (self) {
        self->state = start_state;
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
glar_node_not_used (fsm_t *self)
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
            zsys_debug ("glar_node: %s:", s_state_name [self->state]);
            zsys_debug ("glar_node:         %s", s_event_name [self->event]);
        }
        if (self->state == start_state) {
            if (self->event == console_event) {
                if (!self->exception) {
                    //  join_network_as_console
                    if (self->animate)
                        zsys_debug ("glar_node:             $ join_network_as_console");
                    join_network_as_console (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
                if (!self->exception)
                    self->state = as_console_state;
            }
            else
            if (self->event == robot_event) {
                if (!self->exception) {
                    //  join_network_as_robot
                    if (self->animate)
                        zsys_debug ("glar_node:             $ join_network_as_robot");
                    join_network_as_robot (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
                if (!self->exception)
                    self->state = as_robot_state;
            }
            else {
                //  Handle unexpected internal events
                zsys_warning ("glar_node: unhandled event %s in %s",
                    s_event_name [self->event], s_state_name [self->state]);
                exit (-1);
            }
        }
        else
        if (self->state == as_console_state) {
            if (self->event == console_command_event) {
                if (!self->exception) {
                    //  shout_command_to_robots
                    if (self->animate)
                        zsys_debug ("glar_node:             $ shout_command_to_robots");
                    shout_command_to_robots (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == whisper_event) {
                if (!self->exception) {
                    //  print_command_results
                    if (self->animate)
                        zsys_debug ("glar_node:             $ print_command_results");
                    print_command_results (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  leave_network
                    if (self->animate)
                        zsys_debug ("glar_node:             $ leave_network");
                    leave_network (self->parent);
                }
            }
            else
            if (self->event == join_event) {
                if (!self->exception) {
                    //  signal_peer_joined
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_joined");
                    signal_peer_joined (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == leave_event) {
                if (!self->exception) {
                    //  signal_peer_left
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_left");
                    signal_peer_left (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == other_event) {
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_on_event) {
                if (!self->exception) {
                    //  signal_button_on
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_on");
                    signal_button_on (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_off_event) {
                if (!self->exception) {
                    //  signal_button_off
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_off");
                    signal_button_off (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else {
                //  Handle all other events
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
        }
        else
        if (self->state == as_robot_state) {
            if (self->event == shout_event) {
                if (!self->exception) {
                    //  execute_the_command
                    if (self->animate)
                        zsys_debug ("glar_node:             $ execute_the_command");
                    execute_the_command (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_on_event) {
                if (!self->exception) {
                    //  signal_button_on
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_on");
                    signal_button_on (self->parent);
                }
                if (!self->exception) {
                    //  start_emergency_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ start_emergency_sequence");
                    start_emergency_sequence (self->parent);
                }
                if (!self->exception) {
                    //  check_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ check_for_activity");
                    check_for_activity (self->parent);
                }
                if (!self->exception)
                    self->state = emergency_state;
            }
            else
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  leave_network
                    if (self->animate)
                        zsys_debug ("glar_node:             $ leave_network");
                    leave_network (self->parent);
                }
            }
            else
            if (self->event == join_event) {
                if (!self->exception) {
                    //  signal_peer_joined
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_joined");
                    signal_peer_joined (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == leave_event) {
                if (!self->exception) {
                    //  signal_peer_left
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_left");
                    signal_peer_left (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == other_event) {
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_off_event) {
                if (!self->exception) {
                    //  signal_button_off
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_off");
                    signal_button_off (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else {
                //  Handle all other events
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
        }
        else
        if (self->state == emergency_state) {
            if (self->event == button_off_event) {
                if (!self->exception) {
                    //  signal_button_off
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_off");
                    signal_button_off (self->parent);
                }
                if (!self->exception) {
                    //  stop_emergency_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ stop_emergency_sequence");
                    stop_emergency_sequence (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
                if (!self->exception)
                    self->state = as_robot_state;
            }
            else
            if (self->event == nothing_event) {
                if (!self->exception) {
                    //  check_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ check_for_activity");
                    check_for_activity (self->parent);
                }
            }
            else
            if (self->event == whisper_event) {
                if (!self->exception) {
                    //  check_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ check_for_activity");
                    check_for_activity (self->parent);
                }
            }
            else
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  leave_network
                    if (self->animate)
                        zsys_debug ("glar_node:             $ leave_network");
                    leave_network (self->parent);
                }
            }
            else
            if (self->event == join_event) {
                if (!self->exception) {
                    //  signal_peer_joined
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_joined");
                    signal_peer_joined (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == leave_event) {
                if (!self->exception) {
                    //  signal_peer_left
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_left");
                    signal_peer_left (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == other_event) {
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_on_event) {
                if (!self->exception) {
                    //  signal_button_on
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_on");
                    signal_button_on (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else {
                //  Handle all other events
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
        }
        else
        if (self->state == defaults_state) {
            if (self->event == finished_event) {
                if (!self->exception) {
                    //  leave_network
                    if (self->animate)
                        zsys_debug ("glar_node:             $ leave_network");
                    leave_network (self->parent);
                }
            }
            else
            if (self->event == join_event) {
                if (!self->exception) {
                    //  signal_peer_joined
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_joined");
                    signal_peer_joined (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == leave_event) {
                if (!self->exception) {
                    //  signal_peer_left
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_peer_left");
                    signal_peer_left (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == other_event) {
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_on_event) {
                if (!self->exception) {
                    //  signal_button_on
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_on");
                    signal_button_on (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else
            if (self->event == button_off_event) {
                if (!self->exception) {
                    //  signal_button_off
                    if (self->animate)
                        zsys_debug ("glar_node:             $ signal_button_off");
                    signal_button_off (self->parent);
                }
                if (!self->exception) {
                    //  show_at_rest_sequence
                    if (self->animate)
                        zsys_debug ("glar_node:             $ show_at_rest_sequence");
                    show_at_rest_sequence (self->parent);
                }
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
            else {
                //  Handle all other events
                if (!self->exception) {
                    //  wait_for_activity
                    if (self->animate)
                        zsys_debug ("glar_node:             $ wait_for_activity");
                    wait_for_activity (self->parent);
                }
            }
        }
        //  If we had an exception event, interrupt normal programming
        if (self->exception) {
            if (self->animate)
                zsys_debug ("glar_node:             ! %s", s_event_name [self->exception]);
            self->next_event = self->exception;
        }
        else
        if (self->animate)
            zsys_debug ("glar_node:             > %s", s_state_name [self->state]);
    }
}
