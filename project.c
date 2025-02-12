#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#include <sqlite3.h>

int damage_flag;
int health_spell = 0, damage_spell = 0, speed_spell = 0;
int health = 100, round_gold = 0, hungry = 0;
int deamon_health = 5, fire_health = 10, giant_health = 15, snake_health = 20, undeed_health = 30;
int select_color = 0, select_hard = 0;
int flooor = 0;
char select_gun = 'm';
int number_dagger = 0, number_arrow = 0, number_wand = 0;
int number_sword = 0;
char map[4][24][80];
char UserName[25], Email[30], Password[25], RemWord[25];
int character_row, character_column;
int pillar_row, pillar_column;
int map_door[4][24][80];
struct room
{
    int row;
    int column;
    int lenght;
    int width;
    int door_row;
    int door_column;
    int door_left_row;
    int door_left_column;
    int door_right_row;
    int door_right_column;
};

#define DB_NAME "game.db"
#define ROWS_PER_PAGE 20
#define WIDTH 20
#define HEIGHT 55
#define FLOORS 4
#define ROOMS_PER_FLOOR 8
#define ROOMS_PER_WIDTH 2
#define ROOMS_PER_HEIGHT 3

#define WALL '|'
#define VERTICAL_WALL '_'
#define FLOOR '.'
#define DOOR '+'
#define HIDDEN_DOOR '/'
#define SHOW_HIDDEN_DOOR '?'
#define LOCK_DOOR '@'
#define CORRIDOR '#'
#define PILLAR 'O'
#define WINDOWW '='
#define STAIR '<'
#define PLAYER '1'
#define TRAP '^'
#define GOLD "🪙"
#define GOLD_IN_MAP 'g'
#define BLACK_GOLD_IN_MAP 'i'
#define SIMPLE_FOOD "🍞"
#define SIMPLE_FOOD_IN_MAP 'f'
#define GREAT_FOOD "🦐"
#define GREAT_FOOD_IN_MAP 'u'
#define MAGICAL_FOOD "🧁"
#define MAGICAL_FOOD_IN_MAP 'x'
#define CORRUPT_FOOD "💀"
#define MACE_IN_MAP 'm'
#define MACE_IN_MAP_2 'X'
#define MACE "⚒"	// گرز
#define DAGGER "🗡" // خنجر
#define DAGGER_IN_MAP 'd'
#define DAGGER_IN_MAP_2 'Z'
#define MAGIC_WAND "🪄" // عصای جادویی
#define MAGIC_WAND_IN_MAP 'w'
#define MAGIC_WAND_IN_MAP_2 'r'
#define NORMAL_ARROW "➳" // تیر عادی
#define NORMAL_ARROW_IN_MAP 'a'
#define NORMAL_ARROW_IN_MAP_2 'R'
#define SWORD "⚔" // شمشیر
#define SWORD_IN_MAP 's'
#define SWORD_IN_MAP_2 'v'
#define HEALTH "🩺"
#define HEALTH_IN_MAP 'h'
#define SPEED "🚀"
#define SPEED_IN_MAP 'y'
#define DAMAGE "💥"
#define DAMAGE_IN_MAP 'M'
#define FINISH_GAME "🏆"
#define FINISH_GAME_IN_MAP 'z'
#define MONSTER_DEAMON 'D'
#define MONSTER_FIRE_BREATHING 'F'
#define MONSTER_GIANT 'G'
#define MONSTER_SNAKE 'S'
#define MONSTER_UNDEAD 'U'
#define FULLNESS_MAX 50
#define WEAPONS_COUNT 5
#define POTIONS_COUNT 3

int level = 1; 
int color = 0; 
int song = 0;
char last_pos;
typedef struct
{
	int flooor, x, y;  // مختصات شروع زیرجدول
	int width, height; // ابعاد زیرجدول
	int visited;	   // 1:visited 0:
	int dead_end;	   // 1:yes 0:no
	int kind;		   // 0:simple 1:Treasure 2:Enchant 3:Nightmare
} Room;

typedef struct
{
	int flooor, x, y;
} Cell;

typedef struct
{
	Cell cell;
	int visited;
} Stair;

typedef struct
{
	char name;
	int health;
	Cell cell;
	int moves;
	char last_pos;
} Monster;

struct User
{
	char username[50];
	int scores;
	int golds;
	int finish_games;
	char time_left[20];
};

enum MonsterPower
{
	D = 1,
	F = 2,
	G = 3,
	S = 4,
	U = 6
};

int every_thing_visible = 0;
Room rooms[FLOORS * ROOMS_PER_FLOOR];
Monster monsters[FLOORS * ROOMS_PER_FLOOR * 5] = {0};
int stairs[FLOORS][WIDTH][HEIGHT];
int foods[5] = {0, 0, 0, 0, 0};		 // 1:simple 2:great 3:magical 4:corrupt
int foods_time[5] = {0, 0, 0, 0, 0}; // 1:simple 2:great 3:magical 4:corrupt
int fullness = FULLNESS_MAX + 5;
int weapons[5] = {0, 0, 0, 0, 0};						// MACE DAGGER MAGIC_WAND NORMAL_ARROW SWORD
char weapons_icons[5][5] = {"⚒", "🗡", "🪄", "➳", "⚔"}; // MACE DAGGER MAGIC_WAND NORMAL_ARROW SWORD
int current_weapon = 0;
char *weapons_names[WEAPONS_COUNT] = {"Mace", "Dagger", "Magic Wand", "Normal Arrow", "Sward"};
char weapons_short_names[WEAPONS_COUNT] = {MACE_IN_MAP, DAGGER_IN_MAP, MAGIC_WAND_IN_MAP, NORMAL_ARROW_IN_MAP, SWORD_IN_MAP};
char weapons_short_names_2[WEAPONS_COUNT] = {MACE_IN_MAP_2, DAGGER_IN_MAP_2, MAGIC_WAND_IN_MAP_2, NORMAL_ARROW_IN_MAP_2, SWORD_IN_MAP_2};
int weapon_range[WEAPONS_COUNT] = {1, 5, 10, 5, 1};	  // برد سلاح‌ها
int weapon_power[WEAPONS_COUNT] = {5, 12, 15, 5, 10}; // قدرت سلاح‌ها
int potions[3] = {0, 0, 0};							  // HEALTH SPEED DAMAGE
int potions_left[3] = {0, 0, 0};					  // HEALTH SPEED DAMAGE
char potions_icons[3][5] = {"🩺", "🚀", "💥"};		  // HEALTH SPEED DAMAGE
char *potions_names[POTIONS_COUNT] = {"Health Potion", "Speed Potion", "Damage Potion"};
int speed_moving = 1;
int hp = 100;
int gold = 0;
int golds = 0;
int recovery_health = 1;
int games = 0;
char timee[20];

struct room a[8];

// Functions
char *print_error(const char *message);
sqlite3 *connect_to_database(const char *db_name);
int execute_query(sqlite3 *db, const char *query);
int create_users_table(sqlite3 *db);
int add_user(sqlite3 *db);
int start_database();
int menu_start();
int username();
int email();
int password();
int rand_password();
int remind();
int sign_up();
int login();
int guest();
int username_login();
int password_log();
int remind_log();
int pre_game();
int settings();
int profile();
void map_create();
int move_map();
int print_map();
void set_color();
void score();
int mmenu();
int login_user();
char *remind_word();
int update_user();
void music_play(char *address);
void kill_mp3();
void print_mapM();
int map_create_door();

int mmenu(){
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    set_color();

    menu_start();
    int select = menu_start();
    if (select == 0)
        sign_up();
    else if (select == 1)
        login();
    else
        guest();

    while(1){
        select = pre_game();
        if (select == 0 || select == 1)
            break;
        else if (select == 2){
            score();
            continue;
        }
        else if (select == 3){
            profile();
            continue;
        }
        else{
            settings();
            continue;
        }
    }
    map_create();
    move_map();
    print_map();
    refresh();
    endwin();
    return 0;

}

int main()
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 24; j++){
            for(int k = 0; k < 80; k++){
                stairs[i][j][k] = 0;
            }
        }
    }
    

    music_play("1.mp3");

    start_database();

    setlocale(LC_ALL, "");
    mmenu();
    
}

int username()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    curs_set(0);
    char *sign_up_items[5] = {"UserName:", "E-Mail:", "Password:", "Word Reminder:", "creat random passowrd (press 'r')"};
    WINDOW *sign = newwin(11, 40, 7, 20);
    keypad(sign, TRUE);
    wclear(sign);
    box(sign, 0, 0);
    int index = 0;
    noecho();
    for (int i = 0; i < 5; i++)
    {

        if (i == 0)
        {
            wattron(sign, COLOR_PAIR(2));
            wattron(sign, A_BOLD);
            mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
            wattroff(sign, A_BOLD);
            wattroff(sign, COLOR_PAIR(2));
            wrefresh(sign);
        }

        else if (i == 1)
        {
            mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 2)
        {
            mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 3)
        {
            mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 4)
        {
            mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
            refresh();
        }
    }
    while (1)
    {
        wmove(sign, 1, 18 + index);
        int c = mvwgetch(sign, 1, 18 + index);
        if (c == KEY_BACKSPACE)
            if (index > 0)
            {
                mvwprintw(sign, 1, (18 + index), "%c", ' ');
                --index;
                continue;
            }
            else
            {
                continue;
            }

        if (c == '\n')
        {
            if (login_user())
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "USERNAME CURRENTLY EXISTS !");
                napms(850);
                attroff(COLOR_PAIR(3));
                refresh();
                continue;
            }
            UserName[index] = '\0';

            if (UserName[0] == '\0')
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "USERNAME IS EMPTY !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum USERNAME lenght is 12!");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                UserName[index] = c;
                ++index;
                mvwprintw(sign, 1, (18 + index), "%c", c);
            }
        }
    }
}

int email()
{
    int index = 0;
    noecho();
    clear();
    refresh();
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *sign_up_items[5] = {"UserName:", "E-Mail:", "Password:", "Word Reminder:", "creat random passowrd (press 'r')"};
    WINDOW *sign = newwin(11, 40, 7, 20);
    keypad(sign, TRUE);
    wclear(sign);
    box(sign, 0, 0);
    for (int i = 0; i < 5; i++)
    {

        if (i == 1)
        {
            wattron(sign, COLOR_PAIR(2));
            wattron(sign, A_BOLD);
            mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
            wattroff(sign, A_BOLD);
            wattroff(sign, COLOR_PAIR(2));
            wrefresh(sign);
        }

        else if (i == 0)
        {
            mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 2)
        {
            mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 3)
        {
            mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 4)
        {
            mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
            refresh();
        }
    }

    while (1)
    {
        wmove(sign, 3, 18 + index);
        int c = mvwgetch(sign, 3, 18 + index);
        if (c == KEY_BACKSPACE)
        {
            if (index > 0)
            {
                mvwprintw(sign, 3, (18 + index), "%c", ' ');
                --index;
                continue;
            }
            else
            {
                continue;
            }
        }

        if (c == '\n')
        {
            Email[index] = '\0';
            int countats = 0, countdot = 0, banner = 0;
            for (int i = 0; i < strlen(Email); i++)
            {
                if (Email[i] == '@')
                    countats = i;
                else if (Email[i] == '.')
                    countdot = i;
                else if ('a' <= Email[i] && Email[i] <= 'z')
                    banner = 1;
            }

            if (strlen(Email) < 5)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE LENGHT OF THE MINIMUM ALLOWED E-MAIL IS 5 !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else if ((0 >= countats) || (countats >= (countdot - 1)) || (countdot >= (strlen(Email) - 1)) || (banner == 0))
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE E-MAIL HAS NOT ENTERED THE CORRECT FORMAT !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    if (i == 2)
                    {
                        wattron(sign, COLOR_PAIR(2));
                        wattron(sign, A_BOLD);
                        mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
                        wattroff(sign, A_BOLD);
                        wattroff(sign, COLOR_PAIR(2));
                        wrefresh(sign);
                    }
                    else if (i == 0)
                    {
                        mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
                        refresh();
                    }
                    else if (i == 1)
                    {
                        mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
                        refresh();
                    }
                    else if (i == 3)
                    {
                        mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
                        refresh();
                    }
                    else if (i == 4)
                    {
                        mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
                        refresh();
                    }
                }
                // save in file
                return 0;
            }
        }

        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum E-MAIL lenght is 14 !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                Email[index] = c;
                ++index;
                mvwprintw(sign, 3, (18 + index), "%c", c);
            }
        }
    }
}

int password()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *sign_up_items[5] = {"UserName:", "E-Mail:", "Password:", "Word Reminder:", "creat random passowrd (press 'r')"};
    WINDOW *sign = newwin(11, 40, 7, 20);
    keypad(sign, TRUE);
    wclear(sign);
    box(sign, 0, 0);
    int index = 0;
    noecho();
    for (int i = 0; i < 5; i++)
    {
        if (i == 2)
        {
            wattron(sign, COLOR_PAIR(2));
            wattron(sign, A_BOLD);
            mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
            wattroff(sign, A_BOLD);
            wattroff(sign, COLOR_PAIR(2));
            wrefresh(sign);
        }
        else if (i == 0)
        {
            mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 1)
        {
            mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 3)
        {
            mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 4)
        {
            mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
            refresh();
        }
    }
    while (1)
    {
        wmove(sign, 5, 18 + index);
        int c = mvwgetch(sign, 5, 18 + index);
        if (c == KEY_BACKSPACE)
            if (index > 0)
            {
                mvwprintw(sign, 5, (18 + index), "%c", ' ');
                --index;
                continue;
            }
            else
            {
                continue;
            }

        if (c == '\n')
        {
            int uppercase = 0, lowercase = 0, digit = 0;
            Password[index] = '\0';
            for (int i = 0; i < strlen(Password); i++)
            {
                if ('A' <= Password[i] && Password[i] <= 'Z')
                    uppercase = 1;
                else if ('a' <= Password[i] && Password[i] <= 'z')
                    lowercase = 1;
                else if ('0' <= Password[i] && Password[i] <= '9')
                    digit = 1;
            }

            if (strlen(Password) < 7)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE LENGHT OF THE MINIMUM ALLOWED PASSWORD IS 7 !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else if (uppercase == 0)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE PASSWORD REQUIRES AT LEAST ONE UPPERCASE LETTER !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else if (lowercase == 0)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE PASSWORD REQUIRES AT LEAST ONE LOWERCASE LETTER !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else if (digit == 0)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "THE PASSWORD REQUIRES AT LEAST ONE DIGIT LETTER !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }

            // save in file
            return 0;
        }

        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum PASSWORD lenght is 12!");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                Password[index] = c;
                ++index;
                mvwprintw(sign, 5, (18 + index), "%c", '*');
            }
        }
    }
}

int rand_password()
{
    char upper, lower;
    int digit, randchar;
    srand(time(NULL));
    int lenght = (rand() % 12);
    for (int i = 0; i <= lenght; i++)
    {
        randchar = (rand() % 3);

        if (randchar == 0)
        {
            upper = (rand() % 26);
            Password[i] = ('A' + upper);
            Password[i + 1] = '\0';
        }
        else if (randchar == 1)
        {
            lower = (rand() % 26);
            Password[i] = ('a' + lower);
            Password[i + 1] = '\0';
        }
        if (randchar == 2)
        {
            digit = (rand() % 10);
            Password[i] = ('0' + digit);
            Password[i + 1] = '\0';
        }
    }
    attron(COLOR_PAIR(3));
    mvprintw(1, 1, "Your PASSWORD is : %s", Password);
    refresh();
    napms(8000);
    attroff(COLOR_PAIR(3));
    move(1, 1);
    clrtoeol();
    refresh();
    return 0;
}

int remind()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *sign_up_items[5] = {"UserName:", "E-Mail:", "Password:", "Word Reminder:", "creat random passowrd (press 'r')"};
    WINDOW *sign = newwin(11, 40, 7, 20);
    keypad(sign, TRUE);
    wclear(sign);
    box(sign, 0, 0);
    int index = 0;
    noecho();
    for (int i = 0; i < 5; i++)
    {

        if (i == 3)
        {
            wattron(sign, COLOR_PAIR(2));
            wattron(sign, A_BOLD);
            mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
            wattroff(sign, A_BOLD);
            wattroff(sign, COLOR_PAIR(2));
            wrefresh(sign);
        }

        else if (i == 1)
        {
            mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 2)
        {
            mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 4)
        {
            mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 0)
        {
            mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
            refresh();
        }
    }
    while (1)
    {
        wmove(sign, 7, 18 + index);
        int c = mvwgetch(sign, 7, 18 + index);
        if (c == KEY_BACKSPACE)
            if (index > 0)
            {
                mvwprintw(sign, 7, (18 + index), "%c", ' ');
                --index;
                continue;
            }
            else
            {
                continue;
            }

        if (c == '\n')
        {
            // if(exist){
            //     attron(COLOR_PAIR(3));
            //     mvprintw(1, 1, "%s", "USERNAME CURRENTLY EXISTS !");
            //     napms(850);
            //     attroff(COLOR_PAIR(3));
            //     refresh();
            //     continue;
            // }
            RemWord[index] = '\0';

            if (RemWord[0] == '\0')
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "REMINDWORD IS EMPTY !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                // save in file
                return 0;
            }
        }
        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum REMINDWORD lenght is 12!");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                RemWord[index] = c;
                ++index;
                mvwprintw(sign, 7, (18 + index), "%c", c);
            }
        }
    }
}

int sign_up()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *sign_up_items[5] = {"UserName:", "E-Mail:", "Password:", "Word Reminder:", "creat random passowrd (press 'r')"};
    WINDOW *sign = newwin(11, 40, 7, 20);
    keypad(sign, TRUE);
    wclear(sign);
    box(sign, 0, 0);
    for (int i = 0; i < 5; i++)
    {

        if (i == 0)
        {
            wattron(sign, COLOR_PAIR(2));
            wattron(sign, A_BOLD);
            mvwprintw(sign, 1, 2, "%s", sign_up_items[i]);
            wattroff(sign, A_BOLD);
            wattroff(sign, COLOR_PAIR(2));
            wrefresh(sign);
        }

        else if (i == 2)
        {
            mvwprintw(sign, 5, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 1)
        {
            mvwprintw(sign, 3, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 3)
        {
            mvwprintw(sign, 7, 2, "%s", sign_up_items[i]);
            refresh();
        }
        else if (i == 4)
        {
            mvwprintw(sign, 9, 4, "%s", sign_up_items[i]);
            refresh();
        }
    }

    wrefresh(sign);
    curs_set(0);
    username();
    email();
    int cd = getchar();
    if (cd == 'r')
    {
        rand_password();
    }
    else
    {
        password();
    }
    remind();

    sqlite3 *db = connect_to_database(DB_NAME);
    if (add_user(db) != 0)
    {
        sqlite3_close(db);
        return FALSE;
    }
    sqlite3_close(db);
    clear();
    refresh();
    return TRUE;
}

int login()
{

    clear();
    refresh();
    keypad(stdscr, TRUE);
    curs_set(0);
    char *log_items[3] = {"UserName:", "Password:", "Forget PASSWORD (press 'f')"};
    WINDOW *log = newwin(7, 46, 7, 17);
    keypad(log, TRUE);
    wclear(log);
    box(log, 0, 0);
    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            wattron(log, COLOR_PAIR(2));
            wattron(log, A_BOLD);
            mvwprintw(log, 1, 2, "%s", log_items[i]);
            wattroff(log, A_BOLD);
            wattroff(log, COLOR_PAIR(2));
            wrefresh(log);
        }

        else if (i == 1)
        {
            mvwprintw(log, 3, 2, "%s", log_items[i]);
            refresh();
        }

        else if (i == 2)
        {
            mvwprintw(log, 5, 9, "%s", log_items[i]);
            refresh();
        }
    }
    wrefresh(log);
    username_login();
    wmove(log, 3, 18);
    int c = mvwgetch(log, 3, 18);
    if (c == 'f')
    {
        remind_log();
    }
    password_log();
    clear();
    refresh();
    return 0;
}

int username_login()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    curs_set(0);
    char *log_items[3] = {"UserName:", "Password:", "Forget PASSWORD (press 'f')"};
    WINDOW *log = newwin(7, 46, 7, 17);
    keypad(log, TRUE);
    wclear(log);
    box(log, 0, 0);
    int index = 0;
    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            wattron(log, COLOR_PAIR(2));
            wattron(log, A_BOLD);
            mvwprintw(log, 1, 2, "%s", log_items[i]);
            wattroff(log, A_BOLD);
            wattroff(log, COLOR_PAIR(2));
            wrefresh(log);
        }

        else if (i == 1)
        {
            mvwprintw(log, 3, 2, "%s", log_items[i]);
            wrefresh(log);
        }

        else if (i == 2)
        {
            mvwprintw(log, 5, 9, "%s", log_items[i]);
            wrefresh(log);
        }
    }
    while (1)
    {
        wmove(log, 1, 18 + index);
        int c = mvwgetch(log, 1, 18 + index);
        if (c == KEY_BACKSPACE)
            if (index > 0)
            {
                mvwprintw(log, 1, (18 + index), "%c", ' ');
                --index;
                continue;
            }
            else
            {
                continue;
            }

        if (c == '\n')
        {
            UserName[index] = '\0';
            if (login_user() == 0)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "USERNAME NOT EXISTS !");
                refresh();
                napms(850);
                mvprintw(1, 1, "%s", "                                   ");
                refresh();
                attroff(COLOR_PAIR(3));
                refresh();
                continue;
            }

            else
            {
                return 0;
            }
        }
        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum USERNAME lenght is 12!");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                UserName[index] = c;
                ++index;
                mvwprintw(log, 1, (18 + index), "%c", c);
            }
        }
    }
}

int remind_log()
{
    attron(COLOR_PAIR(3));
    mvprintw(1, 1, "YOUR REMIND WORD IS %s", remind_word());
    refresh();
    napms(2500);
    attroff(COLOR_PAIR(3));
    move(1, 1);
    clrtoeol();
    refresh();
}

int password_log()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    char *log_items[3] = {"UserName:", "Password:", "Forget PASSWORD (press 'f')"};
    WINDOW *log = newwin(7, 46, 7, 17);
    keypad(log, TRUE);
    wclear(log);
    box(log, 0, 0);
    int index = 0;
    noecho();
    for (int i = 0; i < 3; i++)
    {
        if (i == 1)
        {
            wattron(log, COLOR_PAIR(2));
            wattron(log, A_BOLD);
            mvwprintw(log, 3, 2, "%s", log_items[i]);
            wattroff(log, A_BOLD);
            wattroff(log, COLOR_PAIR(2));
            wrefresh(log);
        }
        else if (i == 0)
        {
            mvwprintw(log, 1, 2, "%s", log_items[i]);
            wrefresh(log);
        }
        else if (i == 2)
        {
            mvwprintw(log, 5, 9, "%s", log_items[i]);
            wrefresh(log);
        }
    }
    while (1)
    {
        wmove(log, 3, 18 + index);
        int c = mvwgetch(log, 3, 18 + index);
        if (c == KEY_BACKSPACE)
            if (index > 0)
            {
                mvwprintw(log, 3, (18 + index), "%c", ' ');
                --index;
                refresh();
                continue;
            }
            else
            {
                continue;
            }

        if (c == '\n')
        {
            int uppercase = 0, lowercase = 0, digit = 0;
            Password[index] = '\0';

            if (login_user() == 2)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "WRONG PASSWORD !");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            return 0;
        }

        else
        {
            if (index > 11)
            {
                attron(COLOR_PAIR(3));
                mvprintw(1, 1, "%s", "The maximum PASSWORD lenght is 12!");
                refresh();
                napms(850);
                attroff(COLOR_PAIR(3));
                move(1, 1);
                clrtoeol();
                refresh();
                continue;
            }
            else
            {
                Password[index] = c;
                ++index;
                mvwprintw(log, 3, (18 + index), "%c", '*');
            }
        }
        wrefresh(log);
    }
}
int guest() {
    strcpy(UserName, "Guest mode");
    return 0;
    pre_game();
}
int pre_game()
{

    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    int select = 0;
    char *start_items[5] = {"NEW GAME", "OLD GAME", "SCORE BORD", "PROFILE", "SETTINGS"};
    WINDOW *menu = newwin(11, 20, 6, 30);
    keypad(menu, TRUE);
    wclear(menu);
    box(menu, 0, 0);

    for (int i = 0; i < 5; i++)
    {
        if (i == 0)
        {
            wattron(menu, COLOR_PAIR(1));
            wattron(menu, A_BOLD);
            mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
            wattroff(menu, A_BOLD);
            wattroff(menu, COLOR_PAIR(1));
        }

        else if (i == 1)
            mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
        else if (i == 2)
            mvwprintw(menu, 2 * i + 1, 5, "%s", start_items[i]);
        else if (i == 3)
            mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
        else if (i == 4)
            mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
        move(1, 1);
        clrtoeol();
        mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
        refresh();
    }

    wrefresh(menu);
    while (1)
    {
        wrefresh(menu);
        int c = wgetch(menu);
        if (c == KEY_UP)
        {
            wrefresh(menu);
            if (select == 0)
            {
                select = 4;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            else
            {
                --select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 5; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 5, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 3)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 4)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }
                }
                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);

                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);

                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 5, "%s", start_items[i]);

                    if (i == 3)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);

                    if (i == 4)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                }
            }
        }

        else if (c == KEY_DOWN)
        {
            if (select == 4)
            {
                select = 0;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }
            else
            {
                ++select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 5; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 5, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 3)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 4)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }
                }

                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 5, "%s", start_items[i]);
                    if (i == 3)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                    if (i == 4)
                        mvwprintw(menu, 2 * i + 1, 6, "%s", start_items[i]);
                }
            }
        }

        else if (c == '\n')
        {
            clear();
            wrefresh(menu);
            delwin(menu);
            return select;
        }
        wrefresh(menu);
    }
}

void score()
{
    initscr();
	clear();
	noecho();
	cbreak();
	curs_set(0);
	keypad(stdscr, TRUE);

	start_color();

	struct User users[100];
	int count = 0;
	int current_user_number = -1;

	sqlite3 *db = connect_to_database(DB_NAME);
	if (!db)
	{
		return;
	}

	const char *query = "SELECT Username, HP, Golds, Games, Time FROM Users;";
	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		print_error(sqlite3_errmsg(db));
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
	    strcpy(users[count].username, (char *)sqlite3_column_text(stmt, 0));
		users[count].golds = sqlite3_column_int(stmt, 2);
		strcpy(users[count].time_left, (char *)sqlite3_column_text(stmt, 1));
		users[count].finish_games = sqlite3_column_int(stmt, 3);
		users[count].scores = (users[count].golds) + (sqlite3_column_int(stmt, 1) / 20) + users[count].finish_games;
		count++;
	}

	for (int i = 0; i < count - 1; i++)
	{
		for (int j = i + 1; j < count; j++)
		{
			if (users[j].scores > users[i].scores)
			{
				struct User temp = users[i];
				users[i] = users[j];
				users[j] = temp;
			}
		}
	}

	for (int i = 0; i < count; i++)
	{
		if (strcmp(users[i].username, UserName) == 0)
		{
			current_user_number = i;
		}
	}

	int current_page = 0;
	int total_pages = (count + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
	int scroll_offset = 0;

	strcat(users[0].username, " (king)");
	strcat(users[1].username, " (master)");
	strcat(users[2].username, " (goat)");

	while (1)
	{
		clear();
		int start_row = 2;
		int start_col = (COLS - 60) / 2;

		mvprintw(start_row - 2, start_col, "Top Players - Page %d/%d", current_page + 1, total_pages);
		mvprintw(start_row, start_col, "Rank  Username          Scores  Golds  Games  Sign-Up");
		mvprintw(start_row + 1, start_col, "-------------------------------------------------------");

		int start_idx = current_page * ROWS_PER_PAGE;
		int end_idx = (start_idx + ROWS_PER_PAGE < count) ? start_idx + ROWS_PER_PAGE : count;

		int visible_lines = LINES - (start_row + 5); 
		int visible_start_idx = start_idx + scroll_offset;
		int visible_end_idx = (visible_start_idx + visible_lines < end_idx) ? visible_start_idx + visible_lines : end_idx;

		for (int i = visible_start_idx; i < visible_end_idx; i++)
		{
            move(start_row + 2 + (i - visible_start_idx), start_col);
			if (i == 0)
			{
				addwstr(L"🥇\t");
			}
			else if (i == 1)
			{
				addwstr(L"🥈\t");
			}
			else if (i == 2)
			{
				addwstr(L"🥉\t");
			}
			else
				printw("%d\t", i + 1);

			if (i < 3)
				attron(A_BOLD);
			if (i == 0)
				attron(COLOR_PAIR(4));
			else if (i == 1)
				attron(COLOR_PAIR(1));
			else if (i == 2)
				attron(COLOR_PAIR(6));

			if (i == current_user_number)
			{
				attron(COLOR_PAIR(5));
			}

			printw("%-16.16s %-7d %-6d %-6d %-9s",
					 users[i].username,
					 users[i].scores,
					 users[i].golds,
					 users[i].finish_games,
					 users[i].time_left);

			if (i < 3)
				attroff(A_BOLD);
			if (i == 0)
				attroff(COLOR_PAIR(4));
			else if (i == 1)
				attroff(COLOR_PAIR(1));
			else if (i == 2)
				attroff(COLOR_PAIR(6));

			if (i == current_user_number)
			{
				attroff(COLOR_PAIR(5));
			}
		}

		mvprintw(LINES - 2, (COLS - 40) / 2, "Up/Down: Scroll, Left/Right: Pages, Q: Quit");
		refresh();

		int ch = getch();
		if (ch == 'q' || ch == 'Q')
		{
			break;
		}
		else if (ch == KEY_RIGHT)
		{
			if (current_page < total_pages - 1)
			{
				current_page++;
				scroll_offset = 0; 
			}
		}
		else if (ch == KEY_LEFT)
		{
			if (current_page > 0)
			{
				current_page--;
				scroll_offset = 0; 
			}
		}
		else if (ch == KEY_DOWN)
		{
			if ((scroll_offset + visible_lines) < (end_idx - start_idx))
			{
				scroll_offset++;
			}
		}
		else if (ch == KEY_UP)
		{
			if (scroll_offset > 0)
			{
				scroll_offset--;
			}
		}
	}

	clear();
	refresh();

	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	endwin();
}

int login_user()
{
    sqlite3 *db = connect_to_database(DB_NAME);
    if (!db)
    {
        return 0;
    }

    const char *query = "SELECT Password FROM Users WHERE Username = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        print_error(sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, UserName, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const char *stored_password = (const char *)sqlite3_column_text(stmt, 0);
        if (strcmp(stored_password, Password) == 0)
        {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
        else
        {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 2;
        }
    }
    else if (rc == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    else
    {
        print_error(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
}


int settings()
{
    clear();
    refresh();
    keypad(stdscr, TRUE);
    curs_set(0);
    int select = 0, select_song = 0;
    char *start_items[3] = {"COLOR", "HARDEST", "MUSIC"};
    char *color_items[3] = {"GREEN", "RED", "BLUE"};
    char *hard_items[3] = {"EASY", "MEDIUM", "HARD"};
    char *song_items[3] = {"RAP", "MOKH", "LOR"};
    WINDOW *menu = newwin(7, 20, 8, 30);
    keypad(menu, TRUE);
    wclear(menu);
    box(menu, 0, 0);

    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            wattron(menu, COLOR_PAIR(1));
            wattron(menu, A_BOLD);
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
            wattroff(menu, A_BOLD);
            wattroff(menu, COLOR_PAIR(1));

            wattron(menu, COLOR_PAIR(3));
            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
            wattroff(menu, COLOR_PAIR(3));
        }

        else if (i == 1){
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

            wattron(menu, COLOR_PAIR(3));
            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
            wattroff(menu, COLOR_PAIR(3));
        }

        else if (i == 2){
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

            wattron(menu, COLOR_PAIR(3));
            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
            wattroff(menu, COLOR_PAIR(3));

            music_play("1.mp3");
        }
        move(1, 1);
        clrtoeol();
        mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
        refresh();
    }

    wrefresh(menu);
    while (1)
    {
        wrefresh(menu);
        int c = wgetch(menu);
        if (c == KEY_UP)
        {
            wrefresh(menu);
            if (select == 0)
            {
                select = 2;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            else
            {
                --select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));

                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }
                else
                {
                    if (i == 0)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    if (i == 1)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    if (i == 2)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }
            }
        }

        else if (c == KEY_DOWN)
        {
            if (select == 2)
            {
                select = 0;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }
            else
            {
                ++select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }
            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));

                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }

                else
                {
                    if (i == 0)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 1)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 2){
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }
            }
        }

        else if (c == 'n')
        {
            if (select == 0)
            {
                if (select_color == 2)
                    select_color = 0;
                else{
                    ++select_color;
                }
            }
            else if (select == 1)
            {
                if (select_hard == 2)
                    select_hard = 0;
                else
                    ++select_hard;
            }
            else {
                if (select_song == 2)
                    select_song = 0;
                else
                    ++select_song;
            }
            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));

                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }

                else
                {
                    if (i == 0)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 1)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 2){
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }
            }
        }

        else if (c == 'b')
        {
            if (select == 0)
            {
                if (select_color == 0)
                    select_color = 2;
                else
                    --select_color;
            }
            else if (select == 1)
            {
                if (select_hard == 0)
                    select_hard = 2;
                else
                    --select_hard;
            }
            else
            {
                if (select_song == 0)
                    select_song = 2;
                else
                    --select_song;
            }
            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));

                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }

                else
                {
                    if (i == 0)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_color == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_color == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_color == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", color_items[select_color]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 1)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_hard == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(3));
                        }
                        else if (select_hard == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(4));
                        }
                        else if (select_hard == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", hard_items[select_hard]);
                            wattroff(menu, COLOR_PAIR(5));
                        }
                    }
                    if (i == 2){
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        if (select_song == 0)
                        {
                            wattron(menu, COLOR_PAIR(3));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(3));

                            music_play("1.mp3");
                        }
                        else if (select_song == 1)
                        {
                            wattron(menu, COLOR_PAIR(4));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(4));

                            music_play("2.mp3");
                        }
                        else if (select_song == 2)
                        {
                            wattron(menu, COLOR_PAIR(5));
                            mvwprintw(menu, 2 * i + 1, 11, "%s", song_items[select_song]);
                            wattroff(menu, COLOR_PAIR(5));

                            music_play("3.mp3");
                        }
                    }
                }
            }
        }

        else if (c == '\n')
        {
            wrefresh(menu);
            delwin(menu);
            return pre_game();
        }
        wrefresh(menu);
    }
}

int profile()
{
    keypad(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    int select = 0;
    char *start_items[5] = {"USERNAME : ", "SCORE : ", "GOLD : ", "N OF GAMES : ", "EXPERIENCE : "};
    WINDOW *menu = newwin(11, 30, 6, 20);
    keypad(menu, TRUE);
    wclear(menu);
    box(menu, 0, 0);

    for (int i = 0; i < 5; i++)
    {
        if (i == 0)
        {
            wattron(menu, COLOR_PAIR(4));
            wattron(menu, A_BOLD);
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
            wattroff(menu, A_BOLD);
            wattroff(menu, COLOR_PAIR(4));
        }

        else if (i == 1)
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
        else if (i == 2)
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
        else if (i == 3)
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
        else if (i == 4)
            mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
        move(1, 1);
        clrtoeol();
        mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
        refresh();
    }

    wrefresh(menu);
    while (1)
    {
        wrefresh(menu);

        int c = wgetch(menu);
        if (c == KEY_UP)
        {
            wrefresh(menu);
            if (select == 0)
            {
                select = 4;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            else
            {
                --select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);

            

            for (int i = 0; i < 5; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 3)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 4)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }
                }
                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 3)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 4)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                }
            }
        }

        else if (c == KEY_DOWN)
        {
            if (select == 4)
            {
                select = 0;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }
            else
            {
                ++select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 5; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 3)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }

                    else if (i == 4)
                    {
                        wattron(menu, COLOR_PAIR(4));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(4));
                    }
                }

                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                    if (i == 3)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                    if (i == 4)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                }
            }
        }

        else if (c == '\n')
        {
            wrefresh(menu);
            delwin(menu);
            return pre_game();
        }
        wrefresh(menu);


        for (int i = 0; i < 5; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(3));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 18, "%s", UserName);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(3));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(3));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 18, "%d", golds * 1000);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(3));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(3));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 18, "%d", golds);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(3));
                    }

                    else if (i == 3)
                    {
                        wattron(menu, COLOR_PAIR(3));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 18, "%d", games);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(3));
                    }

                    else if (i == 4)
                    {
                        wattron(menu, COLOR_PAIR(3));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 18, "%d", games * 983);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(3));
                    }
                }
                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 3)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);

                    if (i == 4)
                        mvwprintw(menu, 2 * i + 1, 2, "%s", start_items[i]);
                }
            }
    }
}


void map_create()
{
    curs_set(0);

    for (int j = 0; j < 24; j++)
    {
        for (int i = 0; i < 80; i++)
        {
            map[flooor][j][i] = ' ';
        }
    }

    srand(time(NULL));
    int count_door = 0;
    int door;

    for (int i = 0; i < 8; i++)
    {
        if (i == 0)
        {
            a[i].width = ((rand() % 4) + 6);
            a[i].lenght = ((rand() % 6) + 6);
            a[i].row = rand() % (10 - a[i].width);
            a[i].column = rand() % (16 - a[i].lenght);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
                stairs[0][j][a[i].column] = 1;
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
                stairs[0][j][a[i].column + a[i].lenght - 1] = 1;
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
                stairs[0][a[i].row][j] = 1;
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
                stairs[0][a[i].row + a[i].width - 1][j] = 1;
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row + a[i].width - 1][door] = '+';
            stairs[0][a[i].row + a[i].width - 1][door] = 1;
            a[i].door_row = a[i].row + a[i].width - 1;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            stairs[0][door][a[i].column + a[i].lenght - 1] = 1;
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                    stairs[0][k][j] = 1;
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){

                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            } 
        }
        else if (i == 1)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = rand() % (10 - a[i].width);
            a[i].column = (rand() % (18 - a[i].lenght) + 20);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row + a[i].width - 1][door] = '+';
            a[i].door_row = a[i].row + a[i].width - 1;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 2)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = rand() % (10 - a[i].width);
            a[i].column = (rand() % (18 - a[i].lenght) + 40);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row + a[i].width - 1][door] = '+';
            a[i].door_row = a[i].row + a[i].width - 1;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 3)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = rand() % (10 - a[i].width);
            a[i].column = (rand() % (18 - a[i].lenght) + 60);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row + a[i].width - 1][door] = '+';
            a[i].door_row = a[i].row + a[i].width - 1;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 4)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = (rand() % (10 - a[i].width) + 12);
            a[i].column = rand() % (18 - a[i].lenght);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row][door] = '+';
            a[i].door_row = a[i].row;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 5)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = (rand() % (10 - a[i].width) + 12);
            a[i].column = (rand() % (18 - a[i].lenght) + 20);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row][door] = '+';
            a[i].door_row = a[i].row;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 6)
        {
            a[i].width = (rand() % 4 + 6);
            a[i].lenght = (rand() % 6 + 6);
            a[i].row = (rand() % (10 - a[i].width) + 12);
            a[i].column = (rand() % (18 - a[i].lenght) + 40);

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row][door] = '+';
            a[i].door_row = a[i].row;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column + a[i].lenght - 1] = '+';
            a[i].door_right_row = door;
            a[i].door_right_column = a[i].column + a[i].lenght - 1;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }
        }
        else if (i == 7)
        {
            a[i].width = 9;
            a[i].lenght = 11;
            a[i].row = 13;
            a[i].column = 67;

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column] = '|';
            }

            for (int j = a[i].row; j < a[i].row + a[i].width; j++)
            {
                map[flooor][j][a[i].column + a[i].lenght - 1] = '|';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row][j] = '*';
            }

            for (int j = a[i].column; j < a[i].column + a[i].lenght; j++)
            {
                map[flooor][a[i].row + a[i].width - 1][j] = '*';
            }

            door = rand() % (a[i].lenght - 2) + a[i].column + 1;
            map[flooor][a[i].row][door] = '+';
            a[i].door_row = a[i].row;
            a[i].door_column = door;

            door = rand() % (a[i].width - 2) + a[i].row + 1;
            map[flooor][door][a[i].column] = '+';
            a[i].door_left_row = door;
            a[i].door_left_column = a[i].column;

            for (int k = a[i].row + 1; k < a[i].row + a[i].width - 1; k++)
            {
                for (int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++)
                {
                    map[flooor][k][j] = '.';
                }
            }

            if ((rand() % 2) == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'O';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1 || rand() % 5 == 2 || rand() % 5 == 3)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'c';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if ((rand() % 5) == 0 || rand() % 5 == 1)
            {
                pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                map[flooor][pillar_row][pillar_column] = 'C';
                if(map[flooor][pillar_row + 1][pillar_column] == '+' || map[flooor][pillar_row - 1][pillar_column] == '+' || map[flooor][pillar_row][pillar_column + 1] == '+' || map[flooor][pillar_row][pillar_column - 1] == '+' || map[flooor][pillar_row][pillar_column + 1] != '.' || map[flooor][pillar_row][pillar_column - 1] != '.')
                    map[flooor][pillar_row][pillar_column] == '.';
            }

            if(select_hard == 0){
                if ((rand() % 2) == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 40 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0 || rand() % 40 == 1 || rand() % 80 == 2){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 1){
                if ((rand() % 3) == 0 || rand() % 3 == 1)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k < a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 60 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }
                    }
                }
            }
            else if(select_hard == 2){
                if ((rand() % 4) == 0 || rand() % 4 == 1 || rand() % 4 == 2)
                {
                    pillar_row = rand() % (a[i].width - 2) + a[i].row + 1;
                    pillar_column = rand() % (a[i].lenght - 2) + a[i].column + 1;
                    map[flooor][pillar_row][pillar_column] = 'T';
                }
                refresh();
                for(int k = a[i].row + 1; k< a[i].row + a[i].width -1; k++){
                    for(int j = a[i].column + 1; j < a[i].column + a[i].lenght - 1; j++){

                        if(rand() % 40 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3){
                                map[flooor][k][j] = 'D';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 4|| rand() % 14 == 5 || rand() % 14 == 6 || rand() % 14 == 7){
                                map[flooor][k][j] = 'F';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'G';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12){
                                map[flooor][k][j] = 'S';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 13){
                                map[flooor][k][j] = 'U';
                                if(map[flooor][k][j - 1] != '.' || map[flooor][k][j + 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 14 == 0 || rand() % 14 == 1 || rand() % 14 == 2 || rand() % 14 == 3 || rand() % 14 == 4 || rand() % 14 == 5){
                                map[flooor][k][j] = 's';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 6|| rand() % 14 == 7){
                                map[flooor][k][j] = 'w';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 8 || rand() % 14 == 9 || rand() % 14 == 10){
                                map[flooor][k][j] = 'a';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 14 == 11 || rand() % 14 == 12 || rand() % 14 == 13){
                                map[flooor][k][j] = 'd';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'm';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 2 == 0){
                                map[flooor][k][j] = 'f'; 
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                        else if(rand() % 80 == 0){
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 't';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'x';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                            if(rand() % 3 == 0){
                                map[flooor][k][j] = 'p';
                                if(map[flooor][k][j + 1] != '.' || map[flooor][k][j - 1] != '.')
                                    map[flooor][k][j] = '.';
                            }
                        }

                    }
                }
            }

            for(int i = 0; i < 4; i++){
                map[i][17][72] = '<';
                map[i][17][71] = '.';
            }
            
        }
        refresh();
        update_user();
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[0].door_column == a[4].door_column)
        {
            for (int i = a[0].door_row + 1; i < a[4].door_row; i++){
                map[flooor][i][a[0].door_column] = '#';
                map_door[flooor][i][a[0].door_column] = 1;
            }
        }
        else if (a[0].door_column > a[4].door_column)
        {
            for (int i = a[0].door_row + 1; i <= ((a[4].door_row + a[0].door_row) / 2); i++)
                map[flooor][i][a[0].door_column] = '#';
            for (int i = a[4].door_column; i < a[0].door_column; i++)
                map[flooor][((a[4].door_row + a[0].door_row) / 2)][i] = '#';
            for (int i = ((a[4].door_row + a[0].door_row) / 2); i < a[4].door_row; i++)
                map[flooor][i][a[4].door_column] = '#';
        }
        else
        {
            for (int i = a[0].door_row + 1; i <= ((a[4].door_row + a[0].door_row) / 2); i++)
                map[flooor][i][a[0].door_column] = '#';
            for (int i = a[0].door_column; i < a[4].door_column; i++)
                map[flooor][((a[0].door_row + a[4].door_row) / 2)][i] = '#';
            for (int i = ((a[4].door_row + a[0].door_row) / 2); i < a[4].door_row; i++)
                map[flooor][i][a[4].door_column] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[1].door_column == a[5].door_column)
        {
            for (int i = a[1].door_row + 1; i < a[5].door_row; i++)
                map[flooor][i][a[1].door_column] = '#';
        }
        else if (a[1].door_column > a[5].door_column)
        {
            for (int i = a[1].door_row + 1; i <= ((a[5].door_row + a[1].door_row) / 2); i++)
                map[flooor][i][a[1].door_column] = '#';
            for (int i = a[5].door_column; i < a[1].door_column; i++)
                map[flooor][((a[5].door_row + a[1].door_row) / 2)][i] = '#';
            for (int i = ((a[5].door_row + a[1].door_row) / 2); i < a[5].door_row; i++)
                map[flooor][i][a[5].door_column] = '#';
        }
        else
        {
            for (int i = a[1].door_row + 1; i <= ((a[5].door_row + a[1].door_row) / 2); i++)
                map[flooor][i][a[1].door_column] = '#';
            for (int i = a[1].door_column; i < a[5].door_column; i++)
                map[flooor][((a[1].door_row + a[5].door_row) / 2)][i] = '#';
            for (int i = ((a[5].door_row + a[1].door_row) / 2); i < a[5].door_row; i++)
                map[flooor][i][a[5].door_column] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[2].door_column == a[6].door_column)
        {
            for (int i = a[2].door_row + 1; i < a[6].door_row; i++)
                map[flooor][i][a[2].door_column] = '#';
        }
        else if (a[2].door_column > a[6].door_column)
        {
            for (int i = a[2].door_row + 1; i <= ((a[6].door_row + a[2].door_row) / 2); i++)
                map[flooor][i][a[2].door_column] = '#';
            for (int i = a[6].door_column; i < a[2].door_column; i++)
                map[flooor][((a[6].door_row + a[2].door_row) / 2)][i] = '#';
            for (int i = ((a[6].door_row + a[2].door_row) / 2); i < a[6].door_row; i++)
                map[flooor][i][a[6].door_column] = '#';
        }
        else
        {
            for (int i = a[2].door_row + 1; i <= ((a[6].door_row + a[2].door_row) / 2); i++)
                map[flooor][i][a[2].door_column] = '#';
            for (int i = a[2].door_column; i < a[6].door_column; i++)
                map[flooor][((a[2].door_row + a[6].door_row) / 2)][i] = '#';
            for (int i = ((a[6].door_row + a[2].door_row) / 2); i < a[6].door_row; i++)
                map[flooor][i][a[6].door_column] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[3].door_column == a[7].door_column)
        {
            for (int i = a[3].door_row + 1; i < a[7].door_row; i++)
                map[flooor][i][a[3].door_column] = '#';
        }
        else if (a[3].door_column > a[7].door_column)
        {
            for (int i = a[3].door_row + 1; i <= ((a[7].door_row + a[3].door_row) / 2); i++)
                map[flooor][i][a[3].door_column] = '#';
            for (int i = a[7].door_column; i < a[3].door_column; i++)
                map[flooor][((a[7].door_row + a[3].door_row) / 2)][i] = '#';
            for (int i = ((a[7].door_row + a[3].door_row) / 2); i < a[7].door_row; i++)
                map[flooor][i][a[7].door_column] = '#';
        }
        else
        {
            for (int i = a[3].door_row + 1; i <= ((a[7].door_row + a[3].door_row) / 2); i++)
                map[flooor][i][a[3].door_column] = '#';
            for (int i = a[3].door_column; i < a[7].door_column; i++)
                map[flooor][((a[3].door_row + a[7].door_row) / 2)][i] = '#';
            for (int i = ((a[7].door_row + a[3].door_row) / 2); i < a[7].door_row; i++)
                map[flooor][i][a[7].door_column] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[0].door_right_row == a[1].door_left_row)
        {
            for (int i = a[0].door_right_column + 1; i < a[1].door_left_column; i++)
            {
                map[flooor][a[0].door_right_row][i] = '#';
            }
        }
        else if (a[0].door_right_row < a[1].door_left_row)
        {
            for (int i = a[0].door_right_column + 1; i <= ((a[0].door_right_column + a[1].door_left_column) / 2); i++)
                map[flooor][a[0].door_right_row][i] = '#';
            for (int i = a[0].door_right_row; i <= a[1].door_left_row; i++)
                map[flooor][i][((a[0].door_right_column + a[1].door_left_column) / 2)] = '#';
            for (int i = ((a[0].door_right_column + a[1].door_left_column) / 2); i < a[1].door_left_column; i++)
                map[flooor][a[1].door_left_row][i] = '#';
        }
        else if (a[0].door_right_row > a[1].door_left_row)
        {
            for (int i = a[0].door_right_column + 1; i <= ((a[0].door_right_column + a[1].door_left_column) / 2); i++)
                map[flooor][a[0].door_right_row][i] = '#';
            for (int i = a[1].door_left_row; i <= a[0].door_right_row; i++)
                map[flooor][i][((a[0].door_right_column + a[1].door_left_column) / 2)] = '#';
            for (int i = ((a[0].door_right_column + a[1].door_left_column) / 2); i < a[1].door_left_column; i++)
                map[flooor][a[1].door_left_row][i] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[1].door_right_row == a[2].door_left_row)
        {
            for (int i = a[1].door_right_column + 1; i < a[2].door_left_column; i++)
            {
                map[flooor][a[1].door_right_row][i] = '#';
            }
        }
        else if (a[1].door_right_row < a[2].door_left_row)
        {
            for (int i = a[1].door_right_column + 1; i <= ((a[1].door_right_column + a[2].door_left_column) / 2); i++)
                map[flooor][a[1].door_right_row][i] = '#';
            for (int i = a[1].door_right_row; i <= a[2].door_left_row; i++)
                map[flooor][i][((a[1].door_right_column + a[2].door_left_column) / 2)] = '#';
            for (int i = ((a[1].door_right_column + a[2].door_left_column) / 2); i < a[2].door_left_column; i++)
                map[flooor][a[2].door_left_row][i] = '#';
        }
        else
        {
            for (int i = a[1].door_right_column + 1; i <= ((a[1].door_right_column + a[2].door_left_column) / 2); i++)
                map[flooor][a[1].door_right_row][i] = '#';
            for (int i = a[2].door_left_row; i <= a[1].door_right_row; i++)
                map[flooor][i][((a[1].door_right_column + a[2].door_left_column) / 2)] = '#';
            for (int i = ((a[1].door_right_column + a[2].door_left_column) / 2); i < a[2].door_left_column; i++)
                map[flooor][a[2].door_left_row][i] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[2].door_right_row == a[3].door_left_row)
        {
            for (int i = a[2].door_right_column + 1; i < a[3].door_left_column; i++)
            {
                map[flooor][a[2].door_right_row][i] = '#';
            }
        }
        else if (a[2].door_right_row < a[3].door_left_row)
        {
            for (int i = a[2].door_right_column + 1; i <= ((a[2].door_right_column + a[3].door_left_column) / 2); i++)
                map[flooor][a[2].door_right_row][i] = '#';
            for (int i = a[2].door_right_row; i <= a[3].door_left_row; i++)
                map[flooor][i][((a[2].door_right_column + a[3].door_left_column) / 2)] = '#';
            for (int i = ((a[2].door_right_column + a[3].door_left_column) / 2); i < a[3].door_left_column; i++)
                map[flooor][a[3].door_left_row][i] = '#';
        }
        else
        {
            for (int i = a[2].door_right_column + 1; i <= ((a[2].door_right_column + a[3].door_left_column) / 2); i++)
                map[flooor][a[2].door_right_row][i] = '#';
            for (int i = a[3].door_left_row; i <= a[2].door_right_row; i++)
                map[flooor][i][((a[2].door_right_column + a[3].door_left_column) / 2)] = '#';
            for (int i = ((a[2].door_right_column + a[3].door_left_column) / 2); i < a[3].door_left_column; i++)
                map[flooor][a[3].door_left_row][i] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[4].door_right_row == a[5].door_left_row)
        {
            for (int i = a[4].door_right_column + 1; i < a[5].door_left_column; i++)
            {
                map[flooor][a[4].door_right_row][i] = '#';
            }
        }
        else if (a[4].door_right_row < a[5].door_left_row)
        {
            for (int i = a[4].door_right_column + 1; i <= ((a[4].door_right_column + a[5].door_left_column) / 2); i++)
                map[flooor][a[4].door_right_row][i] = '#';
            for (int i = a[4].door_right_row; i <= a[5].door_left_row; i++)
                map[flooor][i][((a[4].door_right_column + a[5].door_left_column) / 2)] = '#';
            for (int i = ((a[4].door_right_column + a[5].door_left_column) / 2); i < a[5].door_left_column; i++)
                map[flooor][a[5].door_left_row][i] = '#';
        }
        else
        {
            for (int i = a[4].door_right_column + 1; i <= ((a[4].door_right_column + a[5].door_left_column) / 2); i++)
                map[flooor][a[4].door_right_row][i] = '#';
            for (int i = a[5].door_left_row; i <= a[4].door_right_row; i++)
                map[flooor][i][((a[4].door_right_column + a[5].door_left_column) / 2)] = '#';
            for (int i = ((a[4].door_right_column + a[5].door_left_column) / 2); i < a[5].door_left_column; i++)
                map[flooor][a[5].door_left_row][i] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[5].door_right_row == a[6].door_left_row)
        {
            for (int i = a[5].door_right_column + 1; i < a[6].door_left_column; i++)
            {
                map[flooor][a[5].door_right_row][i] = '#';
            }
        }
        else if (a[5].door_right_row < a[6].door_left_row)
        {
            for (int i = a[5].door_right_column + 1; i <= ((a[5].door_right_column + a[6].door_left_column) / 2); i++)
                map[flooor][a[5].door_right_row][i] = '#';
            for (int i = a[5].door_right_row; i <= a[6].door_left_row; i++)
                map[flooor][i][((a[5].door_right_column + a[6].door_left_column) / 2)] = '#';
            for (int i = ((a[5].door_right_column + a[6].door_left_column) / 2); i < a[6].door_left_column; i++)
                map[flooor][a[6].door_left_row][i] = '#';
        }
        else
        {
            for (int i = a[5].door_right_column + 1; i <= ((a[5].door_right_column + a[6].door_left_column) / 2); i++)
                map[flooor][a[5].door_right_row][i] = '#';
            for (int i = a[6].door_left_row; i <= a[5].door_right_row; i++)
                map[flooor][i][((a[5].door_right_column + a[6].door_left_column) / 2)] = '#';
            for (int i = ((a[5].door_right_column + a[6].door_left_column) / 2); i < a[6].door_left_column; i++)
                map[flooor][a[6].door_left_row][i] = '#';
        }
    }

    for (int j = 0; j < 1; j++)
    {
        if (a[6].door_right_row == a[7].door_left_row)
        {
            for (int i = a[6].door_right_column + 1; i < a[7].door_left_column; i++)
            {
                map[flooor][a[6].door_right_row][i] = '#';
            }
        }
        else if (a[6].door_right_row < a[7].door_left_row)
        {
            for (int i = a[6].door_right_column + 1; i <= ((a[6].door_right_column + a[7].door_left_column) / 2); i++)
                map[flooor][a[6].door_right_row][i] = '#';
            for (int i = a[6].door_right_row; i <= a[7].door_left_row; i++)
                map[flooor][i][((a[6].door_right_column + a[7].door_left_column) / 2)] = '#';
            for (int i = ((a[6].door_right_column + a[7].door_left_column) / 2); i < a[7].door_left_column; i++)
                map[flooor][a[7].door_left_row][i] = '#';
        }
        else
        {
            for (int i = a[6].door_right_column + 1; i <= ((a[6].door_right_column + a[7].door_left_column) / 2); i++)
                map[flooor][a[6].door_right_row][i] = '#';
            for (int i = a[7].door_left_row; i <= a[6].door_right_row; i++)
                map[flooor][i][((a[6].door_right_column + a[7].door_left_column) / 2)] = '#';
            for (int i = ((a[6].door_right_column + a[7].door_left_column) / 2); i < a[7].door_left_column; i++)
                map[flooor][a[7].door_left_row][i] = '#';
        }
    }

    map[0][a[0].row + 2][a[0].column + 2] = '$';
    character_row = a[0].row + 2, character_column = a[0].column + 2;
    
    if(flooor == 1) {
        map[1][17][73] = '$';
        character_row = 17, character_column = 73;
    }

    else if(flooor == 2) {
        map[2][17][73] = '$';
        character_row = 17, character_column = 73;
    }

    else if(flooor == 3){
        map[1][17][73] = '$';
        character_row = 17, character_column = 73;
    }

    refresh();

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 0; j++){
            stairs[i][23][j] = 1;
        }
    }
}
int map_create_door(){
    for(int i = 0; i < 24; i++){
        for(int j = 0; j < 80; j++){
            if(map[flooor][i][j] == '+')
                map_door[flooor][i][j] = 1;
            else if(map[flooor][i][j] == '#')
                map_door[flooor][i][j] = 2;
        }
    }
}

int move_map()
{
    initscr();
    noecho();
    curs_set(0);
    map_create();
    map_create_door();

    while (1)
    {
        if(health <= 0){
            clear();
            refresh();
            WINDOW * lose = newwin(20, 5, 9, 30);
            wclear(lose);
            box(lose, 0, 0);
            mvwprintw(lose, 1, 6, "YOU LOSE !");
            mvwprintw(lose, 3, 7, "GOLD : %d", gold);
            ++games;
            golds += gold;
            update_user();
            getch();
            mmenu();
        }

        if(select_gun == 's' && number_sword <= 0)
            select_gun = 'm';
        else if(select_gun == 'a' && number_arrow <= 0)
            select_gun = 'm';
        else if(select_gun == 'w' && number_wand <= 0)
            select_gun = 'm';
        else if(select_gun == 'd' && number_dagger <= 0)
            select_gun = 'm';
        
        nodelay(stdscr, TRUE);
        timeout(100000);
        int c = getch();

        if (c == 'M')
        {
            print_mapM();
            refresh();
            continue;
        }
        else if(c == ' '){
            damage_flag = 0;
            while(1){

                if(select_gun == 'm'){
                    
                    if(map[flooor][character_row][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }


                    if(map[flooor][character_row][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }


                    if(map[flooor][character_row + 1][character_column] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }


                    if(map[flooor][character_row - 1][character_column] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }

                    
                    if(map[flooor][character_row + 1][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }


                    if(map[flooor][character_row - 1][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }


                    if(map[flooor][character_row + 1][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row = 1][character_column - 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }
                    

                    if(map[flooor][character_row - 1][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'F'){
                        fire_health = fire_health - 5;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'G'){
                        giant_health = giant_health - 5;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'S'){
                        snake_health = snake_health - 5;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 5;
                        health = health - 15;
                    }

                    if(damage_spell > 0){
                        if(damage_flag == 0){
                            damage_flag = 1;
                            --damage_spell;
                            continue;
                        }
                    }

                    break;
                }

                else if(select_gun == 's'){
                    
                    if(map[flooor][character_row][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }


                    if(map[flooor][character_row][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }


                    if(map[flooor][character_row + 1][character_column] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }


                    if(map[flooor][character_row - 1][character_column] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }

                    
                    if(map[flooor][character_row + 1][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }


                    if(map[flooor][character_row - 1][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }


                    if(map[flooor][character_row + 1][character_column - 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row = 1][character_column - 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }
                    

                    if(map[flooor][character_row - 1][character_column + 1] == 'D'){
                        deamon_health = deamon_health - 5;
                        health = health - 5;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'F'){
                        fire_health = fire_health - 10;
                        health = health - 7;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'G'){
                        giant_health = giant_health - 10;
                        health = health - 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'S'){
                        snake_health = snake_health - 10;
                        health = health - 12;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'U'){
                        undeed_health = undeed_health - 10;
                        health = health - 15;
                    }
                    
                    if(damage_spell > 0){
                        if(damage_flag == 0){
                            damage_flag = 1;
                            --damage_spell;
                            continue;
                        }
                    }

                    break;
                }

                int z = getch();

                
                
                if(z != 'h' && z != 'j' && z != 'y' && z != 'l' && z != 'k' && z != 'u' && z != 'n' && z != 'b'){
                        WINDOW *message = newwin(2, 40, 77, 2);
                        wattron(message, COLOR_PAIR(3));
                        mvwprintw(message, 1, 2, "INVALID ORDER !");
                        wrefresh(message);
                        napms(700);
                        wattroff(message, COLOR_PAIR(3));
                        refresh();
                        delwin(message);
                        continue;
                }
                else {


                    if(select_gun == 'd'){
                        if(z == 'h'){
                            if(map[flooor][character_row][character_column - 1] == '|' || map[flooor][character_row][character_column - 1] == 'O')
                            {
                                WINDOW *message = newwin(2, 40, 23, 2);
                                wattron(message, COLOR_PAIR(5));
                                mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                                wattroff(message, COLOR_PAIR(5));
                                wrefresh(message); 
                                napms(700);
                                delwin(message);
                                touchwin(stdscr);
                                clear();
                                refresh();
                                print_map();
                                refresh();
                                break;;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 2] == '|' || map[flooor][character_row][character_column - 2] == 'O')
                            {
                                map[flooor][character_row][character_column - 1] = '1';
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 3] == '|' || map[flooor][character_row][character_column - 3] == 'O')
                            {
                                map[flooor][character_row][character_column - 2] = '1';
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 4] == '|' || map[flooor][character_row][character_column - 4] == 'O')
                            {
                                map[flooor][character_row][character_column - 3] = '1';
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 5] == '|' || map[flooor][character_row][character_column - 5] == 'O')
                            {
                                map[flooor][character_row][character_column - 4] = '1';
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }

                        else if(z == 'l'){
                            if(map[flooor][character_row][character_column + 1] == '|' || map[flooor][character_row][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row][character_column + 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 2] == '|' || map[flooor][character_row][character_column + 2] == 'O')
                            {
                                map[flooor][character_row][character_column + 1] = '1';
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 3] == '|' || map[flooor][character_row][character_column + 3] == 'O')
                            {
                                map[flooor][character_row][character_column + 2] = '1';
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 4] == '|' || map[flooor][character_row][character_column + 4] == 'O')
                            {
                                map[flooor][character_row][character_column + 3] = '1';
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 5] == '|' || map[flooor][character_row][character_column + 5] == 'O')
                            {
                                map[flooor][character_row][character_column + 4] = '1';
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'j'){
                            if(map[flooor][character_row - 1][character_column] == '|' || map[flooor][character_row - 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column] == '|' || map[flooor][character_row - 2][character_column] == 'O')
                            {
                                map[flooor][character_row - 1][character_column] = '1';
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column] == '|' || map[flooor][character_row - 3][character_column] == 'O')
                            {
                                map[flooor][character_row - 2][character_column] = '1';
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column] == '|' || map[flooor][character_row - 4][character_column] == 'O')
                            {
                                map[flooor][character_row - 3][character_column] = '1';
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column] == '|' || map[flooor][character_row - 5][character_column] == 'O')
                            {
                                map[flooor][character_row - 4][character_column] = '1';
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'k'){
                            if(map[flooor][character_row + 1][character_column] == '|' || map[flooor][character_row + 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column] == '|' || map[flooor][character_row + 2][character_column] == 'O')
                            {
                                map[flooor][character_row + 1][character_column] = '1';
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column] == '|' || map[flooor][character_row + 3][character_column] == 'O')
                            {
                                map[flooor][character_row + 2][character_column] = '1';
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column] == '|' || map[flooor][character_row + 4][character_column] == 'O')
                            {
                                map[flooor][character_row + 3][character_column] = '1';
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column] == '|' || map[flooor][character_row + 5][character_column] == 'O')
                            {
                                map[flooor][character_row + 4][character_column] = '1';
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'n'){
                            if(map[flooor][character_row + 1][character_column + 1] == '|' || map[flooor][character_row + 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column + 2] == '|' || map[flooor][character_row + 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column + 1] = '1';
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column + 3] == '|' || map[flooor][character_row + 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column + 2] = '1';
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column + 4] == '|' || map[flooor][character_row + 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column + 3] = '1';
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column + 5] == '|' || map[flooor][character_row + 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column + 4] = '1';
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'u'){
                            if(map[flooor][character_row - 1][character_column + 1] == '|' || map[flooor][character_row - 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column + 2] == '|' || map[flooor][character_row - 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column + 1] = '1';
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column + 3] == '|' || map[flooor][character_row - 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column + 2] = '1';
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column + 4] == '|' || map[flooor][character_row - 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column + 3] = '1';
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column + 5] == '|' || map[flooor][character_row - 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column + 4] = '1';
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'y'){
                            if(map[flooor][character_row - 1][character_column - 1] == '|' || map[flooor][character_row - 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column - 2] == '|' || map[flooor][character_row - 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column - 1] = '1';
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column - 3] == '|' || map[flooor][character_row - 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column - 2] = '1';
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column - 4] == '|' || map[flooor][character_row - 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column - 3] = '1';
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column - 5] == '|' || map[flooor][character_row - 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column - 4] = '1';
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                        else if(z == 'b'){
                            if(map[flooor][character_row + 1][character_column - 1] == '|' || map[flooor][character_row + 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column - 2] == '|' || map[flooor][character_row + 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column - 1] = '1';
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column - 3] == '|' || map[flooor][character_row + 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column - 2] = '1';
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column - 4] == '|' || map[flooor][character_row + 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column - 3] = '1';
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column - 5] == '|' || map[flooor][character_row + 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column - 4] = '1';
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'D'){
                                deamon_health -= 12;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'F'){
                                fire_health -= 12;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'G'){
                                giant_health -= 12;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'S'){
                                snake_health -= 12;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'U'){
                                undeed_health -= 12;
                                health -= 15;
                            }
                            --number_dagger;
                        }
                    }




                    else if(select_gun == 'w'){
                        if(z == 'h'){
                            if(map[flooor][character_row][character_column - 1] == '|' || map[flooor][character_row][character_column - 1] == 'O')
                            {
                                WINDOW *message = newwin(2, 40, 23, 2);
                                wattron(message, COLOR_PAIR(5));
                                mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                                wattroff(message, COLOR_PAIR(5));
                                wrefresh(message); 
                                napms(700);
                                delwin(message);
                                touchwin(stdscr);
                                clear();
                                refresh();
                                print_map();
                                refresh();
                                break;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 2] == '|' || map[flooor][character_row][character_column - 2] == 'O')
                            {
                                map[flooor][character_row][character_column - 1] = '2';
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 3] == '|' || map[flooor][character_row][character_column - 3] == 'O')
                            {
                                map[flooor][character_row][character_column - 2] = '2';
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 4] == '|' || map[flooor][character_row][character_column - 4] == 'O')
                            {
                                map[flooor][character_row][character_column - 3] = '2';
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 5] == '|' || map[flooor][character_row][character_column - 5] == 'O')
                            {
                                map[flooor][character_row][character_column - 4] = '2';
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }

                        else if(z == 'l'){
                            if(map[flooor][character_row][character_column + 1] == '|' || map[flooor][character_row][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row][character_column + 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 2] == '|' || map[flooor][character_row][character_column + 2] == 'O')
                            {
                                map[flooor][character_row][character_column + 1] = '2';
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 3] == '|' || map[flooor][character_row][character_column + 3] == 'O')
                            {
                                map[flooor][character_row][character_column + 2] = '2';
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 4] == '|' || map[flooor][character_row][character_column + 4] == 'O')
                            {
                                map[flooor][character_row][character_column + 3] = '2';
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 5] == '|' || map[flooor][character_row][character_column + 5] == 'O')
                            {
                                map[flooor][character_row][character_column + 4] = '2';
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'j'){
                            if(map[flooor][character_row - 1][character_column] == '|' || map[flooor][character_row - 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column] == '|' || map[flooor][character_row - 2][character_column] == 'O')
                            {
                                map[flooor][character_row - 1][character_column] = '2';
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column] == '|' || map[flooor][character_row - 3][character_column] == 'O')
                            {
                                map[flooor][character_row - 2][character_column] = '2';
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column] == '|' || map[flooor][character_row - 4][character_column] == 'O')
                            {
                                map[flooor][character_row - 3][character_column] = '2';
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column] == '|' || map[flooor][character_row - 5][character_column] == 'O')
                            {
                                map[flooor][character_row - 4][character_column] = '2';
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'k'){
                            if(map[flooor][character_row + 1][character_column] == '|' || map[flooor][character_row + 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column] == '|' || map[flooor][character_row + 2][character_column] == 'O')
                            {
                                map[flooor][character_row + 1][character_column] = '2';
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column] == '|' || map[flooor][character_row + 3][character_column] == 'O')
                            {
                                map[flooor][character_row + 2][character_column] = '2';
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column] == '|' || map[flooor][character_row + 4][character_column] == 'O')
                            {
                                map[flooor][character_row + 3][character_column] = '2';
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column] == '|' || map[flooor][character_row + 5][character_column] == 'O')
                            {
                                map[flooor][character_row + 4][character_column] = '2';
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'n'){
                            if(map[flooor][character_row + 1][character_column + 1] == '|' || map[flooor][character_row + 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column + 2] == '|' || map[flooor][character_row + 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column + 1] = '2';
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column + 3] == '|' || map[flooor][character_row + 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column + 2] = '2';
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column + 4] == '|' || map[flooor][character_row + 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column + 3] = '2';
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column + 5] == '|' || map[flooor][character_row + 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column + 4] = '2';
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'u'){
                            if(map[flooor][character_row - 1][character_column + 1] == '|' || map[flooor][character_row - 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column + 2] == '|' || map[flooor][character_row - 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column + 1] = '2';
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column + 3] == '|' || map[flooor][character_row - 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column + 2] = '2';
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column + 4] == '|' || map[flooor][character_row - 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column + 3] = '2';
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column + 5] == '|' || map[flooor][character_row - 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column + 4] = '2';
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'y'){
                            if(map[flooor][character_row - 1][character_column - 1] == '|' || map[flooor][character_row - 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column - 2] == '|' || map[flooor][character_row - 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column - 1] = '2';
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column - 3] == '|' || map[flooor][character_row - 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column - 2] = '2';
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column - 4] == '|' || map[flooor][character_row - 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column - 3] = '2';
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column - 5] == '|' || map[flooor][character_row - 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column - 4] = '2';
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                        else if(z == 'b'){
                            if(map[flooor][character_row + 1][character_column - 1] == '|' || map[flooor][character_row + 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column - 2] == '|' || map[flooor][character_row + 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column - 1] = '2';
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column - 3] == '|' || map[flooor][character_row + 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column - 2] = '2';
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column - 4] == '|' || map[flooor][character_row + 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column - 3] = '2';
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column - 5] == '|' || map[flooor][character_row + 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column - 4] = '2';
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'D'){
                                deamon_health -= 15;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'F'){
                                fire_health -= 15;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'G'){
                                giant_health -= 15;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'S'){
                                snake_health -= 15;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'U'){
                                undeed_health -= 15;
                                health -= 15;
                            }
                            --number_wand;
                        }
                    }
                    




                    else if(select_gun == 'a'){
                        if(z == 'h'){
                            if(map[flooor][character_row][character_column - 1] == '|' || map[flooor][character_row][character_column - 1] == 'O')
                            {
                                WINDOW *message = newwin(2, 40, 23, 2);
                                wattron(message, COLOR_PAIR(5));
                                mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                                wattroff(message, COLOR_PAIR(5));
                                wrefresh(message); 
                                napms(700);
                                delwin(message);
                                touchwin(stdscr);
                                clear();
                                refresh();
                                print_map();
                                refresh();
                                break;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 2] == '|' || map[flooor][character_row][character_column - 2] == 'O')
                            {
                                map[flooor][character_row][character_column - 1] = '3';
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 3] == '|' || map[flooor][character_row][character_column - 3] == 'O')
                            {
                                map[flooor][character_row][character_column - 2] = '3';
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 4] == '|' || map[flooor][character_row][character_column - 4] == 'O')
                            {
                                map[flooor][character_row][character_column - 3] = '3';
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column - 5] == '|' || map[flooor][character_row][character_column - 5] == 'O')
                            {
                                map[flooor][character_row][character_column - 4] = '3';
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column - 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }

                        else if(z == 'l'){
                            if(map[flooor][character_row][character_column + 1] == '|' || map[flooor][character_row][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row][character_column + 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 2] == '|' || map[flooor][character_row][character_column + 2] == 'O')
                            {
                                map[flooor][character_row][character_column + 1] = '3';
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 3] == '|' || map[flooor][character_row][character_column + 3] == 'O')
                            {
                                map[flooor][character_row][character_column + 2] = '3';
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 4] == '|' || map[flooor][character_row][character_column + 4] == 'O')
                            {
                                map[flooor][character_row][character_column + 3] = '3';
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row][character_column + 5] == '|' || map[flooor][character_row][character_column + 5] == 'O')
                            {
                                map[flooor][character_row][character_column + 4] = '3';
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row][character_column + 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'j'){
                            if(map[flooor][character_row - 1][character_column] == '|' || map[flooor][character_row - 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column] == '|' || map[flooor][character_row - 2][character_column] == 'O')
                            {
                                map[flooor][character_row - 1][character_column] = '3';
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column] == '|' || map[flooor][character_row - 3][character_column] == 'O')
                            {
                                map[flooor][character_row - 2][character_column] = '3';
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column] == '|' || map[flooor][character_row - 4][character_column] == 'O')
                            {
                                map[flooor][character_row - 3][character_column] = '3';
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column] == '|' || map[flooor][character_row - 5][character_column] == 'O')
                            {
                                map[flooor][character_row - 4][character_column] = '3';
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'k'){
                            if(map[flooor][character_row + 1][character_column] == '|' || map[flooor][character_row + 1][character_column] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column] == '|' || map[flooor][character_row + 2][character_column] == 'O')
                            {
                                map[flooor][character_row + 1][character_column] = '3';
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column] == '|' || map[flooor][character_row + 3][character_column] == 'O')
                            {
                                map[flooor][character_row + 2][character_column] = '3';
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column] == '|' || map[flooor][character_row + 4][character_column] == 'O')
                            {
                                map[flooor][character_row + 3][character_column] = '3';
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column] == '|' || map[flooor][character_row + 5][character_column] == 'O')
                            {
                                map[flooor][character_row + 4][character_column] = '3';
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'n'){
                            if(map[flooor][character_row + 1][character_column + 1] == '|' || map[flooor][character_row + 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column + 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column + 2] == '|' || map[flooor][character_row + 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column + 1] = '3';
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column + 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column + 3] == '|' || map[flooor][character_row + 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column + 2] = '3';
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column + 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column + 4] == '|' || map[flooor][character_row + 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column + 3] = '3';
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column + 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column + 5] == '|' || map[flooor][character_row + 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column + 4] = '3';
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column + 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'u'){
                            if(map[flooor][character_row - 1][character_column + 1] == '|' || map[flooor][character_row - 1][character_column + 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column + 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column + 2] == '|' || map[flooor][character_row - 2][character_column + 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column + 1] = '3';
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column + 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column + 3] == '|' || map[flooor][character_row - 3][character_column + 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column + 2] = '3';
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column + 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column + 4] == '|' || map[flooor][character_row - 4][character_column + 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column + 3] = '3';
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column + 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column + 5] == '|' || map[flooor][character_row - 5][character_column + 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column + 4] = '3';
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column + 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'y'){
                            if(map[flooor][character_row - 1][character_column - 1] == '|' || map[flooor][character_row - 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 1][character_column - 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 2][character_column - 2] == '|' || map[flooor][character_row - 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row - 1][character_column - 1] = '3';
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 2][character_column - 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 3][character_column - 3] == '|' || map[flooor][character_row - 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row - 2][character_column - 2] = '3';
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 3][character_column - 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 4][character_column - 4] == '|' || map[flooor][character_row - 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row - 3][character_column - 3] = '3';
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 4][character_column - 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row - 5][character_column - 5] == '|' || map[flooor][character_row - 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row - 4][character_column - 4] = '3';
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row - 5][character_column - 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                        else if(z == 'b'){
                            if(map[flooor][character_row + 1][character_column - 1] == '|' || map[flooor][character_row + 1][character_column - 1] == 'O')
                            {

                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 1][character_column - 1] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 2][character_column - 2] == '|' || map[flooor][character_row + 2][character_column - 2] == 'O')
                            {
                                map[flooor][character_row + 1][character_column - 1] = '3';
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 2][character_column - 2] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 3][character_column - 3] == '|' || map[flooor][character_row + 3][character_column - 3] == 'O')
                            {
                                map[flooor][character_row + 2][character_column - 2] = '3';
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 3][character_column - 3] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 4][character_column - 4] == '|' || map[flooor][character_row + 4][character_column - 4] == 'O')
                            {
                                map[flooor][character_row + 3][character_column - 3] = '3';
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 4][character_column - 4] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }

                            else if(map[flooor][character_row + 5][character_column - 5] == '|' || map[flooor][character_row + 5][character_column - 5] == 'O')
                            {
                                map[flooor][character_row + 4][character_column - 4] = '3';
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'D'){
                                deamon_health -= 5;
                                health -= 5;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'F'){
                                fire_health -= 5;
                                health -= 7;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'G'){
                                giant_health -= 5;
                                health -= 10;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'S'){
                                snake_health -= 5;
                                health -= 12;
                            }
                            else if(map[flooor][character_row + 5][character_column - 5] == 'U'){
                                undeed_health -= 5;
                                health -= 15;
                            }
                            --number_arrow;
                        }
                    }
                    
                    if(damage_spell > 0){
                        if(damage_flag == 0){
                            damage_flag = 1;
                            --damage_spell;
                            continue;
                        }
                    }


                    break;
                }
            }



            if(deamon_health <= 0){
                if(character_row < 12 && character_column < 20){
                    for(int i = 0; i < 12; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 20 && character_column < 40){
                    for(int i = 0; i < 12; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 40 && character_column < 60){
                    for(int i = 0; i < 12; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 60 && character_column < 80){
                    for(int i = 0; i < 12; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20){
                    for(int i = 12; i < 24; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20 && character_column < 40){
                    for(int i = 12; i < 24; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 40 && character_column < 60){
                    for(int i = 12; i < 24; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 60 && character_column < 80){
                    for(int i = 12; i < 24; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'D')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
            }

            else if(fire_health <= 0){
                if(character_row < 12 && character_column < 20){
                    for(int i = 0; i < 12; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 20 && character_column < 40){
                    for(int i = 0; i < 12; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 40 && character_column < 60){
                    for(int i = 0; i < 12; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 60 && character_column < 80){
                    for(int i = 0; i < 12; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20){
                    for(int i = 12; i < 24; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20 && character_column < 40){
                    for(int i = 12; i < 24; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 40 && character_column < 60){
                    for(int i = 12; i < 24; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 60 && character_column < 80){
                    for(int i = 12; i < 24; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'F')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
            }
            else if(giant_health <= 0){
                if(character_row < 12 && character_column < 20){
                    for(int i = 0; i < 12; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 20 && character_column < 40){
                    for(int i = 0; i < 12; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 40 && character_column < 60){
                    for(int i = 0; i < 12; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 60 && character_column < 80){
                    for(int i = 0; i < 12; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20){
                    for(int i = 12; i < 24; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20 && character_column < 40){
                    for(int i = 12; i < 24; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 40 && character_column < 60){
                    for(int i = 12; i < 24; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 60 && character_column < 80){
                    for(int i = 12; i < 24; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'G')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
            }

            else if(snake_health <= 0){
                if(character_row < 12 && character_column < 20){
                    for(int i = 0; i < 12; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 20 && character_column < 40){
                    for(int i = 0; i < 12; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 40 && character_column < 60){
                    for(int i = 0; i < 12; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 60 && character_column < 80){
                    for(int i = 0; i < 12; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20){
                    for(int i = 12; i < 24; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20 && character_column < 40){
                    for(int i = 12; i < 24; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 40 && character_column < 60){
                    for(int i = 12; i < 24; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 60 && character_column < 80){
                    for(int i = 12; i < 24; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'S')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
            }

            else if(undeed_health <= 0){
                if(character_row < 12 && character_column < 20){
                    for(int i = 0; i < 12; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 20 && character_column < 40){
                    for(int i = 0; i < 12; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 40 && character_column < 60){
                    for(int i = 0; i < 12; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row < 12 && character_column > 60 && character_column < 80){
                    for(int i = 0; i < 12; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20){
                    for(int i = 12; i < 24; i++){
                        for(int j = 0; j < 20; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 20 && character_column < 40){
                    for(int i = 12; i < 24; i++){
                        for(int j = 20; j < 40; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 40 && character_column < 60){
                    for(int i = 12; i < 24; i++){
                        for(int j = 40; j < 60; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
                else if(character_row > 12 && character_column > 60 && character_column < 80){
                    for(int i = 12; i < 24; i++){
                        for(int j = 60; j < 80; j++){
                            if(map[flooor][i][j] == 'U')
                                map[flooor][i][j] = '.';
                        }
                    }
                }
            }
            clear();
            print_map();
            refresh();
            update_user();
        }

        else if (c == 'i')
        {
            char *weapon_items[6] = {"GUN", "ace", "agger", "magic wand", "normal arrow", "word"}; 
            WINDOW *menu = newwin(15, 32, 5, 25);
            while (1)
            {
                wclear(menu);
                keypad(menu, TRUE);
                box(menu, 0, 0);
                for (int i = 0; i < 6; i++)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 2, "%s", weapon_items[i]);
                        mvwprintw(menu, 2 * i + 1, 7, "NUMBER");
                        mvwprintw(menu, 2 * i + 1, 15, "DAMAGE");
                        mvwprintw(menu, 2 * i + 1, 23, "SELECTED");
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                        wrefresh(menu);
                    }
                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 2, "m");
                        wattroff(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 3, "%s", weapon_items[i]);
                        mvwprintw(menu, 2 * i + 1, 9, "extreme");
                        mvwprintw(menu, 2 * i + 1, 19, "5");
                        if (select_gun == 'm')
                        {
                            wattron(menu, COLOR_PAIR(2));
                            mvwprintw(menu, 2 * i + 1, 23, "Picked");
                            wattroff(menu, COLOR_PAIR(2));
                        }
                        wrefresh(menu);
                    }
                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 2, "d");
                        wattroff(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 3, "%s", weapon_items[i]);
                        mvwprintw(menu, 2 * i + 1, 10, "%d", number_dagger);
                        mvwprintw(menu, 2 * i + 1, 19, "12");
                        if (select_gun == 'd')
                        {
                            wattron(menu, COLOR_PAIR(2));
                            mvwprintw(menu, 2 * i + 1, 23, "Picked");
                            wattroff(menu, COLOR_PAIR(2));
                        }
                        wrefresh(menu);
                    }
                    else if (i == 3)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", weapon_items[i]);
                        wattron(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 8, "w");
                        wattroff(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 13, "%d", number_wand);
                        mvwprintw(menu, 2 * i + 1, 19, "15");
                        if (select_gun == 'w')
                        {
                            wattron(menu, COLOR_PAIR(2));
                            mvwprintw(menu, 2 * i + 1, 24, "Picked");
                            wattroff(menu, COLOR_PAIR(2));
                        }
                        wrefresh(menu);
                    }
                    else if (i == 4)
                    {
                        mvwprintw(menu, 2 * i + 1, 2, "%s", weapon_items[i]);
                        wattron(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 9, "a");
                        wattroff(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 16, "%d", number_arrow);
                        mvwprintw(menu, 2 * i + 1, 19, "5");
                        if (select_gun == 'a')
                        {
                            wattron(menu, COLOR_PAIR(2));
                            mvwprintw(menu, 2 * i + 1, 24, "Picked");
                            wattroff(menu, COLOR_PAIR(2));
                        }
                        wrefresh(menu);
                    }
                    else if (i == 5)
                    {
                        wattron(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 2, "s");
                        wattroff(menu, COLOR_PAIR(5));
                        mvwprintw(menu, 2 * i + 1, 3, "%s", weapon_items[i]);
                        if(number_sword == 0)
                            mvwprintw(menu, 2 * i + 1, 10, "%d", number_sword);
                        else{
                            mvwprintw(menu, 2 * i + 1, 10, "extreme");
                        }
                        mvwprintw(menu, 2 * i + 1, 19, "10");
                        if (select_gun == 's')
                        {
                            wattron(menu, COLOR_PAIR(2));
                            mvwprintw(menu, 2 * i + 1, 23, "Picked");
                            wattroff(menu, COLOR_PAIR(2));
                        }
                        wrefresh(menu);
                    }
                }

                wrefresh(menu);
                timeout(100000);
                int z = getch();
                if(z == 'i'){
                    if(select_gun != 'x'){
                        wclear(menu);
                        delwin(menu);
                        refresh();
                        print_map();
                        refresh();
                        break;
                    }
                    else {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "Choose a gun !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                }
                else if (z == 'm')
                {
                    if (select_gun == 'm')
                    {
                        select_gun = 'x';
                        continue;
                    }
                    else if (select_gun != 'x')
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "PUT GUN IN BACKPACK !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "SUCCESSFULLY SELECTED !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        select_gun = 'm';
                        continue;
                    }
                    refresh();
                }

                else if (z == 'd')
                {
                    if (select_gun == 'd')
                    {
                        select_gun = 'x';
                        continue;
                    }
                    else if (select_gun != 'x')
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "PUT GUN IN BACKPACK !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else if (number_dagger == 0)
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "THERE IS NO GUN !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "SUCCESSFULLY SELECTED !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        select_gun = 'd';
                        continue;
                    }
                }

                else if (z == 'w')
                {
                    if (select_gun == 'w')
                    {
                        select_gun = 'x';
                        continue;
                    }
                    else if (select_gun != 'x')
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "PUT GUN IN BACKPACK !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else if (number_wand == 0)
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "THERE IS NO GUN !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "SUCCESSFULLY SELECTED !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        select_gun = 'w';
                        continue;
                    }
                }

                else if (z == 'a')
                {
                    if (select_gun == 'a')
                    {
                        select_gun = 'x';
                        continue;
                    }
                    else if (select_gun != 'x')
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "PUT GUN IN BACKPACK !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else if (number_arrow == 0)
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "THERE IS NO GUN !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "SUCCESSFULLY SELECTED !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        select_gun = 'a';
                        continue;
                    }
                }

                else if (z == 's')
                {
                    if (select_gun == 's')
                    {
                        select_gun = 'x';
                        continue;
                    }
                    else if (select_gun != 'x')
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "PUT GUN IN BACKPACK !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        wrefresh(menu);
                        continue;
                    }
                    else if (number_sword == 0)
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "THERE IS NO GUN !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                    }
                    else
                    {
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "SUCCESSFULLY SELECTED !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        select_gun = 's';
                        continue;
                    }
                }
                else{
                        wattron(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "INVALID ORDER !");
                        wrefresh(menu);
                        napms(800);
                        wattroff(menu, COLOR_PAIR(6));
                        mvwprintw(menu, 13, 2, "                         ");
                        wrefresh(menu);
                        continue;
                }
            }
            refresh();
            clear();
            print_map();
            refresh();
        }
        
        
        else if(speed_spell > 0){
            --speed_spell;
            for(int i = 0; i < 2; i++){
                if (c == 'h')
                {

                    if (map[flooor][character_row][character_column - 1] == '|' || map[flooor][character_row][character_column - 1] == 'D' || map[flooor][character_row][character_column - 1] == 'F' || map[flooor][character_row][character_column - 1] == 'G' || map[flooor][character_row][character_column - 1] == 'S' || map[flooor][character_row][character_column - 1] == 'U' || map[flooor][character_row][character_column - 1] == 'O' || map[flooor][character_row][character_column - 1] == ' ' || map[flooor][character_row][character_column - 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row][character_column - 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column - 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_column = character_column - 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                refresh();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_column = character_column - 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                refresh();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row][character_column - 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row][character_column - 1] == 'C'){
                            round_gold += 20;
                        }

                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row][character_column - 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_column = character_column - 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }

                        if(map_door[flooor][character_row][character_column - 1] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'l')
                {

                    if (map[flooor][character_row][character_column + 1] == '|' || map[flooor][character_row][character_column + 1] == 'D' || map[flooor][character_row][character_column + 1] == 'F' || map[flooor][character_row][character_column + 1] == 'G' || map[flooor][character_row][character_column + 1] == 'S' || map[flooor][character_row][character_column + 1] == 'U' || map[flooor][character_row][character_column + 1] == 'O' || map[flooor][character_row][character_column + 1] == ' ' || map[flooor][character_row][character_column + 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row][character_column + 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column + 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_column = character_column + 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_column = character_column + 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row][character_column + 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row][character_column + 1] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row][character_column + 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_column = character_column + 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }

                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        refresh();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'k')
                {

                    if (map[flooor][character_row + 1][character_column] == '|' || map[flooor][character_row + 1][character_column] == 'D' || map[flooor][character_row + 1][character_column] == 'F' || map[flooor][character_row + 1][character_column] == 'G' || map[flooor][character_row + 1][character_column] == 'S' || map[flooor][character_row + 1][character_column] == 'U' || map[flooor][character_row + 1][character_column] == 'O' || map[flooor][character_row + 1][character_column] == ' ' || map[flooor][character_row + 1][character_column] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row + 1][character_column] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row + 1][character_column] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row + 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row + 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row + 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row + 1][character_column] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row + 1][character_column] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row + 1][character_column] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row + 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        refresh();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'j')
                {

                    if (map[flooor][character_row - 1][character_column] == '*' || map[flooor][character_row - 1][character_column] == 'D' || map[flooor][character_row - 1][character_column] == 'F' || map[flooor][character_row - 1][character_column] == 'G' || map[flooor][character_row - 1][character_column] == 'S' || map[flooor][character_row - 1][character_column] == 'U' || map[flooor][character_row - 1][character_column] == 'O' || map[flooor][character_row - 1][character_column] == ' ' || map[flooor][character_row - 1][character_column] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row - 1][character_column] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row - 1][character_column] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row - 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row - 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row - 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row - 1][character_column] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row - 1][character_column] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row - 1][character_column] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row - 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'y')
                {

                    if (map[flooor][character_row - 1][character_column - 1] == '|' || map[flooor][character_row - 1][character_column - 1] == '*' || map[flooor][character_row - 1][character_column - 1] == 'D' || map[flooor][character_row - 1][character_column - 1] == 'F' || map[flooor][character_row - 1][character_column - 1] == 'G' || map[flooor][character_row - 1][character_column - 1] == 'S' || map[flooor][character_row - 1][character_column - 1] == 'U' || map[flooor][character_row - 1][character_column - 1] == 'O' || map[flooor][character_row - 1][character_column - 1] == ' ' || map[flooor][character_row - 1][character_column - 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row - 1][character_column - 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row - 1][character_column - 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row - 1][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                character_column = character_column - 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row - 1][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                character_column = character_column - 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row - 1][character_column] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row - 1][character_column - 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row - 1][character_column - 1] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row - 1][character_column - 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row - 1;
                        character_column = character_column - 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }

                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'n')
                {

                    if (map[flooor][character_row + 1][character_column + 1] == '|' || map[flooor][character_row + 1][character_column + 1] == '*' || map[flooor][character_row + 1][character_column + 1] == 'D' || map[flooor][character_row + 1][character_column + 1] == 'F' || map[flooor][character_row + 1][character_column + 1] == 'G' || map[flooor][character_row + 1][character_column + 1] == 'S' || map[flooor][character_row + 1][character_column + 1] == 'U' || map[flooor][character_row + 1][character_column + 1] == 'O' || map[flooor][character_row + 1][character_column + 1] == ' ' || map[flooor][character_row + 1][character_column + 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row + 1][character_column + 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row + 1][character_column + 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row + 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                character_column = character_column + 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row + 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                character_column = character_column + 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row + 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row + 1][character_column + 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row + 1][character_column + 1] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row + 1][character_column + 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row + 1;
                        character_column = character_column + 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }

                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'u')
                {

                    if (map[flooor][character_row - 1][character_column + 1] == '|' || map[flooor][character_row - 1][character_column + 1] == '*' || map[flooor][character_row - 1][character_column + 1] == 'D' || map[flooor][character_row - 1][character_column + 1] == 'F' || map[flooor][character_row - 1][character_column + 1] == 'G' || map[flooor][character_row - 1][character_column + 1] == 'S' || map[flooor][character_row - 1][character_column + 1] == 'U' || map[flooor][character_row - 1][character_column + 1] == 'O' || map[flooor][character_row - 1][character_column + 1] == ' ' || map[flooor][character_row - 1][character_column + 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row - 1][character_column + 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row - 1][character_column + 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row - 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                character_column = character_column + 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row - 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row - 1;
                                character_column = character_column + 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row - 1][character_column + 1] = '<';
                                map[flooor][character_row][character_column] = '$';
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row - 1][character_column + 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 's'){
                            number_sword += 1;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row - 1][character_column + 1] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row - 1][character_column + 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row - 1;
                        character_column = character_column + 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }

                else if (c == 'b')
                {

                    if (map[flooor][character_row + 1][character_column - 1] == '|' || map[flooor][character_row + 1][character_column - 1] == '*' || map[flooor][character_row + 1][character_column - 1] == 'D' || map[flooor][character_row + 1][character_column - 1] == 'F' || map[flooor][character_row + 1][character_column - 1] == 'G' || map[flooor][character_row + 1][character_column - 1] == 'S' || map[flooor][character_row + 1][character_column - 1] == 'U' || map[flooor][character_row + 1][character_column - 1] == 'O' || map[flooor][character_row + 1][character_column - 1] == ' ' || map[flooor][character_row + 1][character_column - 1] == '*')
                    {
                        WINDOW *message = newwin(2, 40, 23, 2);
                        wattron(message, COLOR_PAIR(5));
                        mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                        wattroff(message, COLOR_PAIR(5));
                        wrefresh(message); 
                        napms(700);
                        delwin(message);
                        touchwin(stdscr);
                        clear();
                        refresh();
                        print_map();
                        refresh();
                        continue;
                    }
                    else if (map[flooor][character_row + 1][character_column - 1] == '<')
                    {
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row + 1][character_column - 1] = '.';
                        map[flooor][character_row][character_column] = '?';

                        while (1)
                        {
                            int x = getch();

                            if (x == '>')
                            {
                                map[flooor][character_row + 1][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                character_column = character_column - 1;
                                flooor++;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // next flooor
                                break;
                            }

                            else if (x == '<')
                            {
                                map[flooor][character_row + 1][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '.';
                                character_row = character_row + 1;
                                character_column = character_column - 1;
                                flooor--;
                                clear();
                                print_map();
                                refresh();
                                move_map();
                                // previous flooor
                                break;
                            }
                            
                            else
                            {
                                map[flooor][character_row + 1][character_column - 1] = '<';
                                map[flooor][character_row][character_column] = '$';
                                clear();
                                print_map();
                                refresh();
                            }
                        }
                    }
                    else
                    {
                        if(map[flooor][character_row + 1][character_column - 1] == 't'){
                            health_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'T') {
                            health -= 5;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'x'){
                            damage_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'p'){
                            speed_spell += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'f'){
                            if(hungry >= 20)
                                hungry -= 20;
                            else
                                hungry = 0; 
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'm'){
                            if(hungry <= 30)
                                hungry += 10;
                            else 
                                hungry = 40;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'd'){
                            number_dagger += 10;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'w'){
                            number_wand += 8;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'a'){
                            number_arrow += 20;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 's'){
                            number_arrow += 1;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'c'){
                            round_gold += 5;
                        }
                        else if(map[flooor][character_row + 1][character_column - 1] == 'C'){
                            round_gold += 20;
                        }
                        
                        if(hungry < 40)
                            ++hungry;
                        if(hungry > 20)
                            --health;
                        if(health <= 0){
                            //lose
                        }
                        map[flooor][character_row][character_column] = '.';
                        map[flooor][character_row + 1][character_column - 1] = '$';

                        if (map_door[flooor][character_row][character_column] == 1){
                            map[flooor][character_row][character_column] = '+';
                            deamon_health = 5;
                            fire_health = 10;
                            giant_health = 15;
                            snake_health = 20;
                            undeed_health = 30;
                        }
                        if (map_door[flooor][character_row][character_column] == 2)
                            map[flooor][character_row][character_column] = '#';

                        character_row = character_row + 1;
                        character_column = character_column - 1;

                        if (map[flooor][character_row - 1][character_column] == '#')
                        {
                            stairs[flooor][character_row - 1][character_column] = 1;
                        }
                        if (map[flooor][character_row + 1][character_column] == '#')
                        {
                            stairs[flooor][character_row + 1][character_column] = 1;
                        }
                        if (map[flooor][character_row][character_column - 1] == '#')
                        {
                            stairs[flooor][character_row][character_column - 1] = 1;
                        }
                        if (map[flooor][character_row][character_column + 1] == '#')
                        {
                            stairs[flooor][character_row][character_column + 1] = 1;
                        }


                        if(map_door[flooor][character_row][character_column] == 1){

                            if(character_column < 20 && character_row < 12){
                                for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                    for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row < 12){
                                for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                    for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row < 12){
                                for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                    for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row < 12){
                                for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                    for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column < 20 && character_row > 12){
                                for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                    for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 20 && character_column < 40 && character_row > 12){
                                for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                    for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 40 && character_column < 60 && character_row > 12){
                                for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                    for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                            else if(character_column > 60 && character_column < 80 && character_row > 12){
                                for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                    for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                        stairs[flooor][j][k] = 1;
                                    }
                                }
                            }
                        }
                        clear();
                        print_map();
                        refresh();
                    }
                    refresh();
                }
                update_user();
            }
        }

        else if(speed_spell <= 0){
            if (c == 'h')
            {

                if (map[flooor][character_row][character_column - 1] == '|' || map[flooor][character_row][character_column - 1] == 'D' || map[flooor][character_row][character_column - 1] == 'F' || map[flooor][character_row][character_column - 1] == 'G' || map[flooor][character_row][character_column - 1] == 'S' || map[flooor][character_row][character_column - 1] == 'U' || map[flooor][character_row][character_column - 1] == 'O' || map[flooor][character_row][character_column - 1] == ' ' || map[flooor][character_row][character_column - 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row][character_column - 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column - 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_column = character_column - 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            refresh();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_column = character_column - 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            refresh();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row][character_column - 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row][character_column - 1] == 'C'){
                        round_gold += 20;
                    }

                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row][character_column - 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_column = character_column - 1;
                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }

                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'l')
            {

                if (map[flooor][character_row][character_column + 1] == '|' || map[flooor][character_row][character_column + 1] == 'D' || map[flooor][character_row][character_column + 1] == 'F' || map[flooor][character_row][character_column + 1] == 'G' || map[flooor][character_row][character_column + 1] == 'S' || map[flooor][character_row][character_column + 1] == 'U' || map[flooor][character_row][character_column + 1] == 'O' || map[flooor][character_row][character_column + 1] == ' ' || map[flooor][character_row][character_column + 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row][character_column + 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column + 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_column = character_column + 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_column = character_column + 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row][character_column + 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row][character_column + 1] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row][character_column + 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_column = character_column + 1;

                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }

                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'k')
            {

                if (map[flooor][character_row + 1][character_column] == '|' || map[flooor][character_row + 1][character_column] == 'D' || map[flooor][character_row + 1][character_column] == 'F' || map[flooor][character_row + 1][character_column] == 'G' || map[flooor][character_row + 1][character_column] == 'S' || map[flooor][character_row + 1][character_column] == 'U' || map[flooor][character_row + 1][character_column] == 'O' || map[flooor][character_row + 1][character_column] == ' ' || map[flooor][character_row + 1][character_column] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row + 1][character_column] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row + 1][character_column] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row + 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row + 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row + 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row + 1][character_column] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row + 1][character_column] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row + 1][character_column] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row + 1;
                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    refresh();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'j')
            {

                if (map[flooor][character_row - 1][character_column] == '*' || map[flooor][character_row - 1][character_column] == 'D' || map[flooor][character_row - 1][character_column] == 'F' || map[flooor][character_row - 1][character_column] == 'G' || map[flooor][character_row - 1][character_column] == 'S' || map[flooor][character_row - 1][character_column] == 'U' || map[flooor][character_row - 1][character_column] == 'O' || map[flooor][character_row - 1][character_column] == ' ' || map[flooor][character_row - 1][character_column] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row - 1][character_column] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row - 1][character_column] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row - 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row - 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row - 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row - 1][character_column] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row - 1][character_column] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row - 1][character_column] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row - 1;
                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'y')
            {

                if (map[flooor][character_row - 1][character_column - 1] == '|' || map[flooor][character_row - 1][character_column - 1] == '*' || map[flooor][character_row - 1][character_column - 1] == 'D' || map[flooor][character_row - 1][character_column - 1] == 'F' || map[flooor][character_row - 1][character_column - 1] == 'G' || map[flooor][character_row - 1][character_column - 1] == 'S' || map[flooor][character_row - 1][character_column - 1] == 'U' || map[flooor][character_row - 1][character_column - 1] == 'O' || map[flooor][character_row - 1][character_column - 1] == ' ' || map[flooor][character_row - 1][character_column - 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row - 1][character_column - 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row - 1][character_column - 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row - 1][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            character_column = character_column - 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row - 1][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            character_column = character_column - 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row - 1][character_column] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row - 1][character_column - 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row - 1][character_column - 1] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row - 1][character_column - 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row - 1;
                    character_column = character_column - 1;

                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }

                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'n')
            {

                if (map[flooor][character_row + 1][character_column + 1] == '|' || map[flooor][character_row + 1][character_column + 1] == '*' || map[flooor][character_row + 1][character_column + 1] == 'D' || map[flooor][character_row + 1][character_column + 1] == 'F' || map[flooor][character_row + 1][character_column + 1] == 'G' || map[flooor][character_row + 1][character_column + 1] == 'S' || map[flooor][character_row + 1][character_column + 1] == 'U' || map[flooor][character_row + 1][character_column + 1] == 'O' || map[flooor][character_row + 1][character_column + 1] == ' ' || map[flooor][character_row + 1][character_column + 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row + 1][character_column + 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row + 1][character_column + 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row + 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            character_column = character_column + 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row + 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            character_column = character_column + 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row + 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row + 1][character_column + 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row + 1][character_column + 1] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row + 1][character_column + 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row + 1;
                    character_column = character_column + 1;

                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }

                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'u')
            {

                if (map[flooor][character_row - 1][character_column + 1] == '|' || map[flooor][character_row - 1][character_column + 1] == '*' || map[flooor][character_row - 1][character_column + 1] == 'D' || map[flooor][character_row - 1][character_column + 1] == 'F' || map[flooor][character_row - 1][character_column + 1] == 'G' || map[flooor][character_row - 1][character_column + 1] == 'S' || map[flooor][character_row - 1][character_column + 1] == 'U' || map[flooor][character_row - 1][character_column + 1] == 'O' || map[flooor][character_row - 1][character_column + 1] == ' ' || map[flooor][character_row - 1][character_column + 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row - 1][character_column + 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row - 1][character_column + 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row - 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            character_column = character_column + 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row - 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row - 1;
                            character_column = character_column + 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row - 1][character_column + 1] = '<';
                            map[flooor][character_row][character_column] = '$';
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row - 1][character_column + 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 's'){
                        number_sword += 1;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row - 1][character_column + 1] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row - 1][character_column + 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row - 1;
                    character_column = character_column + 1;

                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }

            else if (c == 'b')
            {

                if (map[flooor][character_row + 1][character_column - 1] == '|' || map[flooor][character_row + 1][character_column - 1] == '*' || map[flooor][character_row + 1][character_column - 1] == 'D' || map[flooor][character_row + 1][character_column - 1] == 'F' || map[flooor][character_row + 1][character_column - 1] == 'G' || map[flooor][character_row + 1][character_column - 1] == 'S' || map[flooor][character_row + 1][character_column - 1] == 'U' || map[flooor][character_row + 1][character_column - 1] == 'O' || map[flooor][character_row + 1][character_column - 1] == ' ' || map[flooor][character_row + 1][character_column - 1] == '*')
                {
                    WINDOW *message = newwin(2, 40, 23, 2);
                    wattron(message, COLOR_PAIR(5));
                    mvwprintw(message, 0, 1, "INVALID ORDER !"); 
                    wattroff(message, COLOR_PAIR(5));
                    wrefresh(message); 
                    napms(700);
                    delwin(message);
                    touchwin(stdscr);
                    clear();
                    refresh();
                    print_map();
                    refresh();
                    continue;
                }
                else if (map[flooor][character_row + 1][character_column - 1] == '<')
                {
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row + 1][character_column - 1] = '.';
                    map[flooor][character_row][character_column] = '?';

                    while (1)
                    {
                        int x = getch();

                        if (x == '>')
                        {
                            map[flooor][character_row + 1][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            character_column = character_column - 1;
                            flooor++;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // next flooor
                            break;
                        }

                        else if (x == '<')
                        {
                            map[flooor][character_row + 1][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '.';
                            character_row = character_row + 1;
                            character_column = character_column - 1;
                            flooor--;
                            clear();
                            print_map();
                            refresh();
                            move_map();
                            // previous flooor
                            break;
                        }
                        
                        else
                        {
                            map[flooor][character_row + 1][character_column - 1] = '<';
                            map[flooor][character_row][character_column] = '$';
                            clear();
                            print_map();
                            refresh();
                        }
                    }
                }
                else
                {
                    if(map[flooor][character_row + 1][character_column - 1] == 't'){
                        health_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'T') {
                        health -= 5;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'x'){
                        damage_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'p'){
                        speed_spell += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'f'){
                        if(hungry >= 20)
                            hungry -= 20;
                        else
                            hungry = 0; 
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'm'){
                        if(hungry <= 30)
                            hungry += 10;
                        else 
                            hungry = 40;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'd'){
                        number_dagger += 10;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'w'){
                        number_wand += 8;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'a'){
                        number_arrow += 20;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 's'){
                        number_arrow += 1;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'c'){
                        round_gold += 5;
                    }
                    else if(map[flooor][character_row + 1][character_column - 1] == 'C'){
                        round_gold += 20;
                    }
                    
                    if(hungry < 40)
                        ++hungry;
                    if(hungry > 20)
                        --health;
                    if(health <= 0){
                        //lose
                    }
                    map[flooor][character_row][character_column] = '.';
                    map[flooor][character_row + 1][character_column - 1] = '$';

                    if (map_door[flooor][character_row][character_column] == 1){
                        map[flooor][character_row][character_column] = '+';
                        deamon_health = 5;
                        fire_health = 10;
                        giant_health = 15;
                        snake_health = 20;
                        undeed_health = 30;
                    }
                    if (map_door[flooor][character_row][character_column] == 2)
                        map[flooor][character_row][character_column] = '#';

                    character_row = character_row + 1;
                    character_column = character_column - 1;

                    if (map[flooor][character_row - 1][character_column] == '#')
                    {
                        stairs[flooor][character_row - 1][character_column] = 1;
                    }
                    if (map[flooor][character_row + 1][character_column] == '#')
                    {
                        stairs[flooor][character_row + 1][character_column] = 1;
                    }
                    if (map[flooor][character_row][character_column - 1] == '#')
                    {
                        stairs[flooor][character_row][character_column - 1] = 1;
                    }
                    if (map[flooor][character_row][character_column + 1] == '#')
                    {
                        stairs[flooor][character_row][character_column + 1] = 1;
                    }


                    if(map_door[flooor][character_row][character_column] == 1){

                        if(character_column < 20 && character_row < 12){
                            for(int j = a[0].row; j < a[0].row + a[0].width; j++){
                                for(int k = a[0].column; k < a[0].column + a[0].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row < 12){
                            for(int j = a[1].row; j < a[1].row + a[1].width; j++){
                                for(int k = a[1].column; k < a[1].column + a[1].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row < 12){
                            for(int j = a[2].row; j < a[2].row + a[2].width; j++){
                                for(int k = a[2].column; k < a[2].column + a[2].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row < 12){
                            for(int j = a[3].row; j < a[3].row + a[3].width; j++){
                                for(int k = a[3].column; k < a[3].column + a[3].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column < 20 && character_row > 12){
                            for(int j = a[4].row; j < a[4].row + a[4].width; j++){
                                for(int k = a[4].column; k < a[4].column + a[4].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 20 && character_column < 40 && character_row > 12){
                            for(int j = a[5].row; j < a[5].row + a[5].width; j++){
                                for(int k = a[5].column; k < a[5].column + a[5].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 40 && character_column < 60 && character_row > 12){
                            for(int j = a[6].row; j < a[6].row + a[6].width; j++){
                                for(int k = a[6].column; k < a[6].column + a[6].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                        else if(character_column > 60 && character_column < 80 && character_row > 12){
                            for(int j = a[7].row; j < a[7].row + a[7].width; j++){
                                for(int k = a[7].column; k < a[7].column + a[7].lenght; k++){
                                    stairs[flooor][j][k] = 1;
                                }
                            }
                        }
                    }
                    clear();
                    print_map();
                    refresh();
                }
                refresh();
            }
        }
        update_user();
    }
}

void print_mapM(){
    for(int i = 0; i < 24; i++){
        for(int j = 0; j < 80; j++){
            move(i, j);
            printw("%c", map[flooor][i][j]);
            refresh();
        }
    }
    while (1)
    {
        int x = getch();
        if (x == 'M')
        {
            clear();
            print_map();
            break;
        }
        else{
            WINDOW *message = newwin(2, 40, 23, 2);
            wattron(message, COLOR_PAIR(3));
            mvwprintw(message, 1, 2, "INVALID ORDER !");
            wrefresh(message);
            napms(700);
            wattroff(message, COLOR_PAIR(3));
            refresh();
            delwin(message);
            continue;
        }
    }
}

int print_map()
{
    for(int i = 0; i < 24; i++){
        for(int j = 0; j < 80; j++){
            if(stairs[flooor][i][j] == 1){
                if(map[flooor][i][j] == 'T')
                    mvprintw(i, j, ".");
                else if(map[flooor][i][j] == 'm')
                    mvprintw(i, j, "f");
                else{
                    mvprintw(i, j, "%c", map[flooor][i][j]);
                }
            refresh();
            mvprintw(23, 25, "            GOLD : %d     HEALTH : %d     hungry : %d", gold, health, hungry);
            }
            
        }
    }
    return 0;
}

void set_color()
{
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
}

int menu_start()
{

    keypad(stdscr, TRUE);
    ////keypad(stdscr, TRUE);
    curs_set(0);

    int select = 0;
    char *start_items[3] = {"SIGN UP", "LOGIN", "GUEST"};
    WINDOW *menu = newwin(7, 20, 8, 30);
    keypad(menu, TRUE);
    wclear(menu);
    box(menu, 0, 0);

    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            wattron(menu, COLOR_PAIR(1));
            wattron(menu, A_BOLD);
            mvwprintw(menu, 2 * i + 1, 7, "%s", start_items[i]);
            wattroff(menu, A_BOLD);
            wattroff(menu, COLOR_PAIR(1));
        }

        else if (i == 1)
            mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);

        else if (i == 2)
            mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);

        move(1, 1);
        clrtoeol();
        mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
        refresh();
    }

    wrefresh(menu);
    while (1)
    {
        wrefresh(menu);
        int c = wgetch(menu);
        if (c == KEY_UP)
        {
            wrefresh(menu);
            if (select == 0)
            {
                select = 2;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            else
            {
                --select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 7, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }
                }
                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 7, "%s", start_items[i]);

                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);

                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                }
            }
        }

        else if (c == KEY_DOWN)
        {
            if (select == 2)
            {
                select = 0;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }
            else
            {
                ++select;
                move(1, 1);
                clrtoeol();
                mvprintw(1, 1, "YOU'RE SELECTED: %s", start_items[select]);
                refresh();
            }

            wclear(menu);
            box(menu, 0, 0);
            for (int i = 0; i < 3; i++)
            {
                if (i == select)
                {
                    if (i == 0)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 7, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 1)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }

                    else if (i == 2)
                    {
                        wattron(menu, COLOR_PAIR(1));
                        wattron(menu, A_BOLD);
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                        wattroff(menu, A_BOLD);
                        wattroff(menu, COLOR_PAIR(1));
                    }
                }

                else
                {
                    if (i == 0)
                        mvwprintw(menu, 2 * i + 1, 7, "%s", start_items[i]);
                    if (i == 1)
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                    if (i == 2)
                        mvwprintw(menu, 2 * i + 1, 8, "%s", start_items[i]);
                }
            }
        }

        else if (c == '\n')
        {
            wrefresh(menu);
            delwin(menu);
            return select;
        }
        wrefresh(menu);
    }
}

char *print_error(const char *message)
{
    char *error_message = (char *)malloc(500 * sizeof(char));
    if (error_message == NULL)
    {
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }

    snprintf(error_message, 500, "Error: %s\n", message);

    clear();
    printw("%s", error_message);
    refresh();
    sleep(5);

    return error_message;
}

sqlite3 *connect_to_database(const char *db_name)
{
    sqlite3 *db;
    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK)
    {
        print_error(sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

int execute_query(sqlite3 *db, const char *query)
{
    char *err_msg = 0;
    int rc = sqlite3_exec(db, query, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        print_error(err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    return 0;
}

int create_users_table(sqlite3 *db)
{
    const char *query = "CREATE TABLE IF NOT EXISTS Users ("
						"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
						"Username TEXT NOT NULL UNIQUE, "
						"Password TEXT NOT NULL, "
						"Remind TEXT NOT NULL, "
						"Email TEXT NOT NULL, "
						"Level INTEGER DEFAULT 1, "
						"Color INTEGER DEFAULT 0, "
						"Song INTEGER DEFAULT 0, "
						"LastPos CHAR, "
						"Floor INTEGER DEFAULT 0, "
						"EverythingVisible INTEGER DEFAULT 0, "
						"Map BLOB, "
						"Rooms BLOB, "
						"Monsters BLOB, "
						"Stairs BLOB, "
						"Foods BLOB, "
						"FoodsTime BLOB, "
						"Fullness INTEGER DEFAULT 105, "
						"Weapons BLOB, "
						"CurrentWeapon INTEGER DEFAULT 0, "
						"Potions BLOB, "
						"PotionsLeft BLOB, "
						"SpeedMoving INTEGER DEFAULT 1, "
						"HP INTEGER DEFAULT 100, "
						"Gold INTEGER DEFAULT 0, "
						"Golds INTEGER DEFAULT 0, "
						"RecoveryHealth INTEGER DEFAULT 1, "
						"Games INTEGER DEFAULT 0, "
						"Time TEXT);";
	return execute_query(db, query);
}

int add_user(sqlite3 *db)
{
    time_t now = time(NULL);
	if (now == -1)
	{
		perror("time");
		return -1;
	}

	struct tm *local_time = localtime(&now);
	if (local_time == NULL)
	{
		perror("localtime");
		return -1;
	}

	char time_str[20];
	// strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d", local_time);

	sqlite3_stmt *stmt;
	const char *query = "INSERT INTO Users (Username, Password, Email, Level, Color, Song, LastPos, Floor, EverythingVisible, "
						"Map, Rooms, Monsters, Stairs, Foods, FoodsTime, Fullness, Weapons, CurrentWeapon, Potions, PotionsLeft, "
						"SpeedMoving, HP, Gold, Golds, RecoveryHealth, Games, Time, Remind) "
						"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		print_error(sqlite3_errmsg(db));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, UserName, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, Password, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, Email, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, level);
	sqlite3_bind_int(stmt, 5, color);
	sqlite3_bind_int(stmt, 6, song);
	sqlite3_bind_text(stmt, 7, &last_pos, 1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 8, flooor);
	sqlite3_bind_int(stmt, 9, every_thing_visible);
	sqlite3_bind_blob(stmt, 10, map, sizeof(map), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 11, rooms, sizeof(rooms), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 12, monsters, sizeof(monsters), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 13, stairs, sizeof(stairs), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 14, foods, sizeof(foods), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 15, foods_time, sizeof(foods_time), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 16, fullness);
	sqlite3_bind_blob(stmt, 17, weapons, sizeof(weapons), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 18, current_weapon);
	sqlite3_bind_blob(stmt, 19, potions, sizeof(potions), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 20, potions_left, sizeof(potions_left), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 21, speed_moving);
	sqlite3_bind_int(stmt, 22, hp);
	sqlite3_bind_int(stmt, 23, gold);
	sqlite3_bind_int(stmt, 24, golds);
	sqlite3_bind_int(stmt, 25, recovery_health);
	sqlite3_bind_int(stmt, 26, 0);
	sqlite3_bind_text(stmt, 27, time_str, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 28,RemWord, -1, SQLITE_STATIC);
time_str,
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		print_error(sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);
	return 0;
}

int start_database()
{
    sqlite3 *db = connect_to_database(DB_NAME);
    if (!db)
    {
        return 1;
    }

    if (create_users_table(db) != 0)
    {
        sqlite3_close(db);
        return 1;
    }
}


char *remind_word()
{
    sqlite3 *db = connect_to_database(DB_NAME);
    if (!db)
    {
        return 0;
    }

    const char *query = "SELECT Remind FROM Users WHERE Username = ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        print_error(sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, UserName, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        char *stored_remind = (char *)sqlite3_column_text(stmt, 0);
        return stored_remind;
    }
    else if (rc == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return "User not Found";
    }
    else
    {
        print_error(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return "Databse Error";
    }
}

int update_user()
{
	if (strcmp(UserName, "Guest mode") == 0)
	{
		return -4;
	}

	sqlite3 *db = connect_to_database(DB_NAME);
	if (!db)
	{
		return -3;
	}

	const char *query = "UPDATE Users SET "
						"Level = ?, "
						"Color = ?, "
						"Song = ?, "
						"LastPos = ?, "
						"Floor = ?, "
						"EverythingVisible = ?, "
						"Map = ?, "
						"Rooms = ?, "
						"Monsters = ?, "
						"Stairs = ?, "
						"Foods = ?, "
						"FoodsTime = ?, "
						"Fullness = ?, "
						"Weapons = ?, "
						"CurrentWeapon = ?, "
						"Potions = ?, "
						"PotionsLeft = ?, "
						"SpeedMoving = ?, "
						"HP = ?, "
						"Gold = ?, "
						"RecoveryHealth = ? "
						"WHERE Username = ?;";
	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		print_error(sqlite3_errmsg(db));
		sqlite3_close(db);
		return -2;
	}

	sqlite3_bind_int(stmt, 1, level);
	sqlite3_bind_int(stmt, 2, color);
	sqlite3_bind_int(stmt, 3, song);
	sqlite3_bind_text(stmt, 4, &last_pos, 1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, flooor);
	sqlite3_bind_int(stmt, 6, every_thing_visible);
	sqlite3_bind_blob(stmt, 7, map, sizeof(map), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 8, rooms, sizeof(rooms), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 9, monsters, sizeof(monsters), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 10, stairs, sizeof(stairs), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 11, foods, sizeof(foods), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 12, foods_time, sizeof(foods_time), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 13, fullness);
	sqlite3_bind_blob(stmt, 14, weapons, sizeof(weapons), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 15, current_weapon);
	sqlite3_bind_blob(stmt, 16, potions, sizeof(potions), SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 17, potions_left, sizeof(potions_left), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 18, speed_moving);
	sqlite3_bind_int(stmt, 19, hp);
	sqlite3_bind_int(stmt, 20, gold);
	sqlite3_bind_int(stmt, 21, recovery_health);
	sqlite3_bind_text(stmt, 22, UserName, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		print_error(sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return -1;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return 0;
}

void kill_mp3()
{
    system("pgrep mpg123 && pkill mpg123");
}

void music_play(char *address)
{
    char command[256];

    kill_mp3();
    if (address == NULL || strlen(address) == 0)
    {
        return;
    }

    snprintf(command, sizeof(command), "mpg123 \"%s\" > /dev/null 2>&1 &", address);
    system(command);
}
