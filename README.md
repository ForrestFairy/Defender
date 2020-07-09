# Defender

Less complex version of arcade game Defender from 1981 programmed in C.

Objective: Destroy all aliens and save citizens of the Earth. You can destroy aliens by shooting them down with torpedos. If you run into alien then your spaceship will explode and you will lose. Aliens will try to pick up people and escape with them. You can still shoot them down but be careful! You need to catch falling people before they hit ground.

Controls:

    Left Arrow -> Move left
    Right Arrow -> Move right
    Up Arrow -> Move up
    Down Arrow -> Move down
    Space -> Shoot torpedo
    F1 -> End game

Player can get score by:

    Shooting aliens - 100 points
    Catching falling people - 200 points
    Each alibe person in the end - 150 points

To play the game you will need ncurses. To run the game compile defender.c from project/src/main/c folder and run it
