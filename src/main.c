#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define str(x) #x
#define xstr(x) str(x)

#define SUCCESS 0
#define BUFFER_SIZE 1024

#define MAX_NAME_SIZE 64
#define MAX_DECK_SIZE 100
#define MAX_HAND_SIZE 10
#define COUNT_MANA_TYPES 5
#define SIZE_ALL_CARDS 20

#define CLEAR_SCREEN printf("\033[1;1H\033[2J")

typedef enum Mana {
     VOID, LIGHT, TIME, FIRE, EARTH, LIGHTNING, WATER, NUMBER_OF_ELEMENTS
} Mana;

typedef enum Error { NO_ERROR = 0, ERROR, ALLOCATION_ERROR } Error;

typedef struct Card {
    char name[MAX_NAME_SIZE];
    int mana_cost[NUMBER_OF_ELEMENTS];
    int life_time;
    int (*on_play_effect)(void*);
    int (*on_round_start_effect)(void*);
} Card;

typedef struct CardState {
    int life_time;
} CardState;

typedef struct PlayerState {
    int mana[NUMBER_OF_ELEMENTS];
    Card *deck[MAX_DECK_SIZE];
    Card *hand[MAX_HAND_SIZE];
    Card *draw[MAX_DECK_SIZE];
    Card *discard[MAX_DECK_SIZE];
    Card *in_play[MAX_DECK_SIZE];
    CardState card_state[MAX_DECK_SIZE];
} PlayerState;

typedef struct Game {
    int error;
    int is_running;
    int player_turn;
    char buffer[BUFFER_SIZE];
    Card all_cards[SIZE_ALL_CARDS];
    PlayerState player_state;
} Game;

int print_tjena(void *g) {
    printf("Tjena din jävel\n");
    return SUCCESS;
}

int print_hejsan(void *g) {
    printf("Hejsan din jävel\n");
    return SUCCESS;
}

int increment_void(void *p) {
    Game *g = (Game*) p;
    g->player_state.mana[VOID]++;
    return SUCCESS;
}

int increment_light(void *p) {
    Game *g = (Game*) p;
    g->player_state.mana[LIGHT]++;
    return SUCCESS;
}

int increment_time(void *p) {
    Game *g = (Game*) p;
    g->player_state.mana[TIME]++;
    return SUCCESS;
}

int nothing(void* p) {
    return SUCCESS;
}

void print_mana(int *mana) {
    fprintf(stdout, "%s: \t\t%d\n", xstr(VOID), mana[VOID]); 
    fprintf(stdout, "%s: \t\t%d\n", xstr(LIGHT), mana[LIGHT]);
    fprintf(stdout, "%s: \t\t%d\n", xstr(TIME), mana[TIME]);
    fprintf(stdout, "%s: \t\t%d\n", xstr(FIRE), mana[FIRE]);
    fprintf(stdout, "%s: \t\t%d\n", xstr(EARTH), mana[EARTH]); 
    fprintf(stdout, "%s: \t%d\n", xstr(LIGHTNING), mana[LIGHTNING]); 
    fprintf(stdout, "%s: \t\t%d\n", xstr(WATER), mana[WATER]);
}

// Card design
Card test[SIZE_ALL_CARDS] = {
    {
        .name = "Tjena",
        .mana_cost[VOID] = 0,
        .mana_cost[LIGHT] = 0,
        .mana_cost[TIME] = 0,
        .mana_cost[FIRE] = 0,
        .mana_cost[EARTH] = 0,
        .mana_cost[LIGHTNING] = 0,
        .mana_cost[WATER] = 0,
        .life_time = 5,
        .on_play_effect = print_tjena,
        .on_round_start_effect = print_tjena,
    }, 
    {
        .name = "Hejsan",
        .mana_cost[VOID] = 0,
        .mana_cost[LIGHT] = 0,
        .mana_cost[TIME] = 0,
        .mana_cost[FIRE] = 0,
        .mana_cost[EARTH] = 0,
        .mana_cost[LIGHTNING] = 0,
        .mana_cost[WATER] = 0,
        .life_time = -1,
        .on_play_effect = print_hejsan,
        .on_round_start_effect = print_hejsan,
    }, 
    {
        .name = "Channel Void",
        .mana_cost[VOID] = 0,
        .mana_cost[LIGHT] = 0,
        .mana_cost[TIME] = 0,
        .mana_cost[FIRE] = 0,
        .mana_cost[EARTH] = 0,
        .mana_cost[LIGHTNING] = 0,
        .mana_cost[WATER] = 0,
        .life_time = -1,
        .on_play_effect = nothing,
        .on_round_start_effect = increment_void,
    },
    {
        .name = "Channel Light",
        .mana_cost[VOID] = 0,
        .mana_cost[LIGHT] = 0,
        .mana_cost[TIME] = 0,
        .mana_cost[FIRE] = 0,
        .mana_cost[EARTH] = 0,
        .mana_cost[LIGHTNING] = 0,
        .mana_cost[WATER] = 0,
        .life_time = -1,
        .on_play_effect = nothing,
        .on_round_start_effect = increment_light,
    },
    {
        .name = "Channel Time",
        .mana_cost[VOID] = 0,
        .mana_cost[LIGHT] = 0,
        .mana_cost[TIME] = 0,
        .mana_cost[FIRE] = 0,
        .mana_cost[EARTH] = 0,
        .mana_cost[LIGHTNING] = 0,
        .mana_cost[WATER] = 0,
        .life_time = -1,
        .on_play_effect = nothing,
        .on_round_start_effect = increment_time,
    }
};

void print_card(Card *c, size_t n) {
    fprintf(stdout, "%lu: %s\n", n, c->name);
    print_mana(c->mana_cost);
    fprintf(stdout, "\n");
}

void print_hand(PlayerState *p) {
    CLEAR_SCREEN;
    for (size_t i = 0; i < MAX_HAND_SIZE; ++i) {
        if (p->hand[i]) {
            print_card(p->hand[i], i);
        }
    }
    fprintf(stdout, "\n");
}

size_t get_index_of_first_empty_in_play(PlayerState *p) {
    for (size_t i = 0; i < MAX_DECK_SIZE; ++i) {
        if (!p->in_play[i]) {
            return i;
        }
    }
    exit(ERROR);
    return -1;
}

void handle_input(int input, Game *g) {
    PlayerState *p = &g->player_state;
    if (p->hand[input]) {
        printf("Played card(%d): %s\n", input, p->hand[input]->name);
        if (p->hand[input]->on_play_effect(g)) {
            exit(ERROR);
        };
        size_t index = get_index_of_first_empty_in_play(p);
        p->in_play[index] = p->hand[input];
        p->card_state[index].life_time = p->hand[input]->life_time;
        p->hand[input] = NULL;
    } else {
        printf("Card out of range!\n");
    }
}

int is_valid_input(int input, Game *g) {
    int isnumeric = 1;
    char *p = g->buffer;
    while(*p && isnumeric && p <= &g->buffer[BUFFER_SIZE]) {
        isnumeric = isdigit(*p);
        ++p;
    }
    return isnumeric && input <= MAX_HAND_SIZE;
}

void allocate_resources(Game *g) {
    g->error = NO_ERROR;
    g->is_running = 1;
    g->player_turn = 1;

    memset(g->player_state.hand,        '\0', sizeof(Card*) * MAX_HAND_SIZE);
    memset(g->player_state.deck,        '\0', sizeof(Card*) * MAX_DECK_SIZE);
    memset(g->player_state.draw,        '\0', sizeof(Card*) * MAX_DECK_SIZE);
    memset(g->player_state.discard,     '\0', sizeof(Card*) * MAX_DECK_SIZE);
    memset(g->player_state.in_play,     '\0', sizeof(Card*) * MAX_DECK_SIZE);
    memset(g->player_state.card_state,  '\0', sizeof(CardState) * MAX_DECK_SIZE);
    memset(g->all_cards,                '\0', sizeof(Card) * SIZE_ALL_CARDS);
    memset(g->player_state.mana,        '\0', sizeof(int) * NUMBER_OF_ELEMENTS);
}

void clean_up_resources(Game *g) {
}

int check_input(char *str) {
    return strcmp(str, "error") == 0 ? ERROR : NO_ERROR;
}

void reset_mana(PlayerState *p) {
    for (size_t i = 0; i < NUMBER_OF_ELEMENTS; ++i) {
        p->mana[i] = 0;
    }
}

void handle_draw(PlayerState *p) {

}

void handle_life_time(Game *g) {
    PlayerState *p = &g->player_state;
    for (size_t i = 0; i < MAX_DECK_SIZE; ++i) {
        if (!p->in_play[i]) {
            continue; 
        }

        printf("Found card %s with life_time: %d\n", p->in_play[i]->name, p->card_state[i].life_time);
        p->in_play[i]->on_round_start_effect(g);
        if (p->card_state[i].life_time > 0) { 
            p->card_state[i].life_time -= 1;
        }

        if (p->card_state[i].life_time == 0) {
            p->in_play[i] = NULL;
            memset(&p->card_state[i],'\0',sizeof(CardState));
        }
    }
}

void handle_round_start(Game *g) {
    PlayerState *p = &g->player_state;
    g->player_turn = 1;
    reset_mana(p);
    handle_life_time(g);
    handle_draw(p);
}

void handle_enemy_round(Game *g) {

}

void run_application(Game *game) {
    game->player_state.hand[0] = &test[0];
    game->player_state.hand[1] = &test[1];
    game->player_state.hand[2] = &test[2];
    game->player_state.hand[3] = &test[3];
    game->player_state.hand[4] = &test[4];
    game->player_state.hand[5] = &test[2];

    int player_input;
    scanf("%s", game->buffer);
    while (game->is_running) {

        handle_enemy_round(game);
        handle_round_start(game);
        
        do {
            print_hand(&game->player_state);
            fprintf(stdout, "--- PLAYER ---\n");
            print_mana(game->player_state.mana);

            scanf("%s", game->buffer);
            player_input = atoi(game->buffer);
            game->is_running =  (strcmp(game->buffer, "q") != 0); 
            game->player_turn = (strcmp(game->buffer, "d") != 0);

            if (is_valid_input(player_input, game)) {
                handle_input(player_input, game);
                scanf("%s", game->buffer);
            }
            game->error = check_input(game->buffer);
            if (game->error) exit(game->error);

        } while (game->player_turn && game->is_running);
    }
    CLEAR_SCREEN;
}

void run(void) {
    Game game;
    allocate_resources  (&game);
    run_application     (&game);
    clean_up_resources  (&game);
}

int main(int argc, char **argv) {
    run();
    return SUCCESS;
}
