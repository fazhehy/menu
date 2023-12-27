//
// Created by fazhehy on 2023/12/26.
//

#include "common_inc.h"

void Main()
{
    oled_init();
    menu_init();
    for (;;)
    {
        menu_task();
    }
}
