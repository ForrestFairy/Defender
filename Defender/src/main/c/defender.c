/*
Project defender
Author Szymon Bogusz
*/

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

/*
// backslash to copy -> \
*/

//structs used
typedef struct torpedo {
    //y will be a constant
    int y;
    int x;
    //direction 0 -> torpedo facing right
    //direction 1 -> torpedo facing left
    int directionTor;
    bool occupied;
} torpedo;

typedef struct citizens {
    int y;
    int x;
    //0 -> person just stands on the ground, 1 -> person is flying with alien, 2 -> person is falling down, -1 -> dead person
    int state;
} citizens;

typedef struct alien {
    int y;
    int x;
    //0 -> random movement, 1 -> going for citizen, 2 -> going with citizen straight up, 3 - escaped
    int objective;
    bool alive;
    int indexOfPerson;
} alien;

//funtions
WINDOW *move_player(int height, int width, int starty, int startx, int direction);
void destroy_win(WINDOW *local_win, int starty, int startx);
void draw_player(int starty, int startx, int direction);
int add(torpedo array[], int newY, int newX, int direction);
void draw_mountains();
void draw_torpedo(torpedo toDraw);
void move_torpedos (torpedo array[]);
void draw_people (citizens array[]);
void move_people(citizens array[]);
void draw_alien (alien array[]);
void move_aliens (alien array[], citizens people[]);
void torpedos_aliens (torpedo torArray[], alien alArray[], int * score);
void torpedos_people (torpedo torArray[], citizens people[]);
bool endGamePlayer (alien array[], int y, int x);
bool endGameAlien (alien array[]);
bool endGamePeople (citizens array[]);
void rescue_people (int y, int x, citizens people[], int * score);
int finalize_score (citizens people[], int * score);

//Change variables here to change difficulty by increasing and decreasing amount of aliens/torpedos/people
    int numAli = 7;
    int numTor = 4;
    int numPeo = 5;

//main function
int main () {
    //things used to operate the game
    WINDOW *my_win;
    int ch;
    time_t t;
    srand((unsigned) time(&t));

    //initialize player variables
    int startx, starty, width, height, direction;

    //prepare numTor torpedos
    torpedo torpedos[numTor];
    for (int i = 0; i < numTor; i++) {
        torpedos[i].occupied = false;
        torpedos[i].y = -1;
    }

    //create numPeo people on Earth
    citizens people[numPeo];
    for (int i = 0; i < numPeo; i++) {
        people[i].y = 23;
        people[i].x = rand() % 65 + 3;
        people[i].state = 0;
    }

    //create numAli aliens in space
    alien aliens[numAli];
    for (int i = 0; i < numAli; i++) {
        aliens[i].objective = 0;
        do {
            aliens[i].x = rand() % 65 + 3;
            aliens[i].y = rand() % 15 + 2;  
        } while (aliens[i].x > 25 && aliens[i].x < 45
                    && aliens[i].y > 6 && aliens[i].y < 14);
        
        aliens[i].alive = true;
        aliens[i].indexOfPerson = -1;
    }

    //alienCooldown shows how fast will aliens move
    int alienCooldown = 40;

    initscr();
    noecho();
    curs_set(0);    //hiding cursor
    timeout(30);    //torpedos do not wait for input
    keypad(stdscr, TRUE);
//map is from x 0 to 68
//and from y 2 to 22
    height = 3;
    width = 10;
    //player starts in the middle of the screen facing right
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    direction = 0;

    //instead of speed player will change amount of moves left/right and up/down
    int verticalMove = 0;
    int horizontalMove = 0;
    int score = 0;
    refresh();
    my_win = move_player(height, width, starty, startx, direction);

//check where player wants to move ship and move it there
    while((ch = getch()) != KEY_F(1)) {

        switch (ch) {
            //1. clear whole window
            //2. move every piece
            //3. print all

            //moves spaceship left
            case KEY_LEFT:
                direction = 1;
                if (startx + horizontalMove - 3 >= 1) horizontalMove -= 3;
                break; 
            //moves spaceship right
            case KEY_RIGHT:
                direction = 0;
                if (startx + horizontalMove + 3 <= 72) horizontalMove += 3;
                break;
            //moves spaceship up
            case KEY_UP:
                if (starty + verticalMove - 1 >= 2) verticalMove -= 1;
                break;
            //moves spaceship down
            case KEY_DOWN:
                if (starty + verticalMove + 1 <= 23) verticalMove += 1;
                break;
            //if space clicked -> fire
            case 32:
                if (direction == 0) {
                    int temp = add(torpedos, starty, startx + 8, direction);
                    if (temp != -1) draw_torpedo(torpedos[temp]);   
                }
                else {
                    int temp = add(torpedos, starty, startx - 6, direction);
                    if (temp != -1) draw_torpedo(torpedos[temp]);
                }
                break;
        }
              
        // moving using steps to take
        if (verticalMove > 0) {
            verticalMove--;
            if (verticalMove > 5 || verticalMove % 2 == 0) {
                starty++;
            }
        }
        if (verticalMove < 0) {
            verticalMove++;
            if (verticalMove < -5 || verticalMove % 2 == 0) {
                starty--;
            }
        }
        if (horizontalMove < 0) {
            horizontalMove++;
            if (horizontalMove < -15 || horizontalMove % 2 == 0) {
                startx--;
            }
        }
        if (horizontalMove > 0) {
            horizontalMove--;
            if (horizontalMove > 15 || horizontalMove % 2 == 0) {
                startx++;
            }
        }

        //clear everything
        clear();
        wrefresh(my_win);

        //draw everything
        //first mountains
        draw_mountains();
        //move torpedos and print them
        move_torpedos(torpedos);
        //draw player
        draw_player(starty, startx, direction);
        //move torpedos again so they are faster then ship
        move_torpedos(torpedos);
        draw_people(people);
        //aliens are moving from time to time
        alienCooldown--;
        //people moves with aliens
        if (alienCooldown == 0) {
            alienCooldown = 40;
            move_aliens(aliens, people);
            move_people(people);
        }
        //check all possible collisions
        rescue_people(starty, startx, people, &score);
        torpedos_aliens(torpedos, aliens, &score);
        torpedos_people (torpedos, people);
        draw_alien(aliens);
        //Display current score
        mvprintw(0, 30, "Press F1 to exit");
        mvprintw(0, 0, "Your score: %d", score);
        mvprintw(0, 68, "? - citizen");

        //there are 3 options to end game not including F1
        //Option 1 -> player run into alien
        if (endGamePlayer(aliens, starty, startx)) {
            mvprintw(0, 0, "Your final is score: %d   To end the game click enter", score);
            for (int c = ' '; c != 10; c = getch()){}
            endwin();
            exit(0);
        }
        //Option 2 -> all aliens are destroyed
        if (endGameAlien(aliens)) {
            while (alienCooldown > 0) alienCooldown--;
            clear();
            wrefresh(my_win);

            finalize_score(people, &score);

            mvprintw((LINES - height) / 2, (COLS - width) / 2 - 7, "Congratulations!!!");
            mvprintw((LINES - height) / 2 + 1, (COLS - width) / 2 - 7, "You saved the planet!");
            mvprintw((LINES - height) / 2 + 2, (COLS - width) / 2 - 7, "Your final score is: %d", score);
            mvprintw((LINES - height) / 2 + 5, (COLS - width) / 2 - 7, "Click enter to leave the game");
            for (int c = ' '; c != 10; c = getch()){}
            endwin();
            exit(0);
        }
        //Oprion 3 -> all people are dead
        if (endGamePeople(people)) {
            while (alienCooldown > 0) alienCooldown--;
            clear();
            wrefresh(my_win);

            finalize_score(people, &score);

            mvprintw((LINES - height) / 2, (COLS - width) / 2 - 7, "Oh no! You lost");
            mvprintw((LINES - height) / 2 + 1, (COLS - width) / 2 - 7, "You have to save people of Earth");
            mvprintw((LINES - height) / 2 + 2, (COLS - width) / 2 - 7, "Your final score is: %d", score);
            mvprintw((LINES - height) / 2 + 5, (COLS - width) / 2 - 7, "Click enter to leave the game");
            for (int c = ' '; c != 10; c = getch()){}
            endwin();
            exit(0);
        }
    }
    //if F1 was clicked just end game
    endwin();

    return 0;
}

//function to create new spaceship
WINDOW *move_player (int height, int width, int starty, int startx, int direction) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);

    draw_player(starty, startx, direction);

    wrefresh(local_win);
    draw_player(starty, startx, direction);
    return local_win;
}

//function to destroy previous ship
void destroy_win(WINDOW *local_win, int starty, int startx) {
    wrefresh(local_win);
    delwin(local_win);
    return;
}

//drawing function for player's spaceship
void draw_player(int starty, int startx, int direction) {
    
    //direction = 0 -> ship facing right
    if (direction == 0) {
        mvprintw(starty - 1, startx, "|\\_____");
        mvprintw(starty, startx - 2, "»»|____*_\\");
    }
    //direction = 1 -> ship facing left
    else {
        mvprintw(starty - 1, startx - 1, "_____/|");
        mvprintw(starty, startx - 2, "/_*____|««");
    }
    return;
}

//drawing function for torpedos
void draw_torpedo(torpedo toDraw) {
    //direction 0 -> torpedo face right
    if (toDraw.directionTor == 0) mvprintw(toDraw.y, toDraw.x, "-==>");

    //direction 1 -> torpedo face left
    else mvprintw(toDraw.y, toDraw.x, "<==-");
}

//function to move all currently used torpedos and draw them
void move_torpedos (torpedo array[]) {
    for (int i = 0; i < numTor; i++) {
        if (array[i].occupied) {
            if (array[i].directionTor == 0) {
                array[i].x++;
                if (array[i].x > 72) {
                    array[i].occupied = false;
                    continue;
                }
                draw_torpedo(array[i]);
            }
            else {
                array[i].x--;
                if (array[i].x < 1) {
                    array[i].occupied = false;
                    continue;
                }
                draw_torpedo(array[i]);
            }
        } 
    }
}

//function called when player wants to fire torpedo
int add(torpedo array[], int newY, int newX, int direction) {
    //if there is unused torpedo then create new torpedo and return index of it
    for (int i = 0; i < numTor; i++) {
        if (array[i].occupied == false) {
            array[i].occupied = true;
            array[i].x = newX;
            array[i].y = newY;
            array[i].directionTor = direction;
            return i;
        }
    }
    //if all torpedos are in use then return -1
    return -1;
}

//draw mountains in background
void draw_mountains() {
    mvprintw(20, 0, "                          /\\");
    mvprintw(21, 0, "         /\\              /  \\___       ___                _             /\\_");
    mvprintw(22, 0, "   /\\___/  \\        ____/       \\     /   \\___      __   / \\_/\\        /   \\___");
    mvprintw(23, 0, "__/         \\______/             \\___/        \\____/  \\_/      \\______/        \\_");
}

//drawing function for people
void draw_people (citizens array[]) {
    for (int i = 0; i < numPeo; i++) {
        if (array[i].state >= 0) mvprintw(array[i].y, array[i].x, "?");
    }
}

//function to move all alive people
void move_people(citizens array[]) {
    for (int i = 0; i < numPeo; i++) {
        //if state = 1 person is flying with alien to the top of screen
        if (array[i].state == 1) {
            array[i].y--;
            //if citizen is brought to the top of the screen we think of them as they were dead
            if (array[i].y < 1) {
                array[i].state = -1;
            }
        }
        //if state = 2 person is falling down
        if (array[i].state == 2) {
            array[i].y++;
            if (array[i].y > 22) array[i].state = -1;
        }
    } 
}

//drawing function for aliens
void draw_alien (alien array[]) {
    for (int i = 0; i < numAli; i++) {
        if(array[i].alive == true) mvprintw(array[i].y, array[i].x, "(<..>)");
    }  
}

//function to move all alive aliens 
void move_aliens (alien array[], citizens people[]) {
    //in the beginning every alien is going randomly around
    //in every frame every alien has 1% chance to get objective for specific person
    for (int i = 0; i < numAli; i++) {
        if (array[i].alive) {
            int ran = rand() % 100;
            if (ran == 0) array[i].objective = 1;

            //if objective 0 -> move randomly
            if (array[i].objective == 0) {
                //directions: 0 -> up, 1 -> right, 2 -> down, 3 -> left
                int direction = rand() % 4; 
                if (direction == 0 && array[i].y > 2) array[i].y--;
                if (direction == 1 && array[i].x < 72) array[i].x++;
                if (direction == 2 && array[i].y < 20) array[i].y++;
                if (direction == 3 && array[i].x > 0) array[i].x--;
            }

            //if objective 1 -> go for citizen
            if (array[i].objective == 1) {
                int minDistance = 100;
                for (int j = 0; j < numPeo; j++) {
                    //find closest person and go to them
                    if (abs(people[j].x - array[i].x) < minDistance && people[j].state == 0) {
                        minDistance = abs(people[j].x - array[i].x);
                        array[i].indexOfPerson = j;
                    }
                }
                if (people[array[i].indexOfPerson].x - array[i].x < 0) array[i].x--;
                if (people[array[i].indexOfPerson].x - array[i].x > 0) array[i].x++;
                if (array[i].y < 23) array[i].y++;
                if (array[i].y == 23 && people[array[i].indexOfPerson].x - array[i].x == 0) {
                    array[i].objective = 2;
                    array[i].y--;
                    people[array[i].indexOfPerson].state = 1;
                }
            }

            //if objective 2 -> escape with citizen
            if (array[i].objective == 2) {
                array[i].y--;
                if (array[i].y < 0) {
                    array[i].objective = 3;
                    array[i].alive = false;
                }
            }
        }
        //if alien was holding person and was destroyed state of this person changes to falling down
        else if (array[i].indexOfPerson != -1 && people[array[i].indexOfPerson].state != -1 
                    && array[i].objective == 2) {
            people[array[i].indexOfPerson].state = 2;
            array[i].indexOfPerson = -1;
        }
    }
}

//function which destroys alien if it was hit by torpedo and add 100 points for every destroyed alien
void torpedos_aliens (torpedo torArray[], alien alArray[], int * score) {
    for (int i = 0; i < numTor; i++) {
        if (torArray[i].occupied) {
            for (int j = 0; j < numAli; j++) {
                if (alArray[j].alive) {
                    if (torArray[i].y == alArray[j].y && torArray[i].x - alArray[j].x < 6 &&
                         alArray[j].x - torArray[i].x < 4) {
                            alArray[j].alive = false;
                            alArray[j].y = -1;
                            alArray[j].x = -1;
                            torArray[i].occupied = false;
                            *score += 100;
                            break;
                    }
                }
            }
        }
    }
}

//function which destroys people if they were hit by torpedo 
void torpedos_people (torpedo torArray[], citizens people[]) {
    for (int i = 0; i < numTor; i++) {
        if (torArray[i].occupied) {
            for (int j = 0; j < numPeo; j++) {
                if (torArray[i].y == people[j].y) 
                    if (torArray[i].x == people[j].x || people[j].x - torArray[i].x == 3) {
                        people[j].state = -1;
                        people[j].y = -1;
                        people[j].x = -1;
                        torArray[i].occupied = false;
                        break;
                }
            }
        }
    }
}

//function which finds out if player ran into alien
bool endGamePlayer (alien array[], int y, int x) {
    for (int i = 0; i < numAli; i++) {
        if (array[i].alive) {
            if (y == array[i].y && array[i].x - x < 10 && x - array[i].x < 8) {
                array[i].alive = false;
                clear();
                mvprintw(y - 2, x + 2, "/\\");
                mvprintw(y - 1, x - 1, "</\\**/\\>");
                mvprintw(y, x - 2, "< ****** >");
                mvprintw(y + 1, x - 1, "<\\/**\\/>");
                mvprintw(y + 2, x + 2, "\\/");
                return true;
            }
        }
    }
    return false;
}

//function which finds out if there is no alien left
bool endGameAlien (alien array[]) {
    //if there is no aliens return true -> player win
    //if false -> continue playing
    for (int i = 0; i < numAli; i++) {
        if (array[i].alive) return false;
    }
    return true;
}

//function which finds out if there is no people left
bool endGamePeople (citizens array[]) {
    //if there is no people return true -> player lose
    //if false -> continue playing
    for (int i = 0; i < numPeo; i++) {
        if (array[i].state != -1) return false;
    }
    return true;
}

//function which checks if player saves any falling person and adds 200 for every saved person
void rescue_people (int y, int x, citizens people[], int * score) {
    for (int i = 0; i < numPeo; i++) {
        if (people[i].state == 2) {
            if (y == people[i].y && x - people[i].x < 4 && people[i].x - x < 4) {
                people[i].state = -1;
                people[i].y = -1;
                people[i].x = -1;
                *score += 200;
            }
        }
    }
}

//if player won, increase score by 150 for each alive person
int finalize_score (citizens people[], int * score) {
    //add 150 to score for each alive citizen
    for (int i = 0; i < numPeo; i++) {
        if (people[i].state != -1) score += 150;
    }
}