/*
 * main.c  —  Entry point for the 2-D ASCII Graphics Editor
 */

#include "menu.h"
#include <stdio.h>

int main(void)
{
    menu_init();
    menu_run();
    menu_cleanup();

    printf("Goodbye from 2D ASCII Graphics Editor!\n");
    return 0;
}
