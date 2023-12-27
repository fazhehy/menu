//
// Created by fazhehy on 2023/12/24.
//

#include "key.h"

uint8_t get_key_num()
{
    if (KEY1_VALUE() == 0)
    {
        HAL_Delay(20);
        while (KEY1_VALUE() == 0);
        HAL_Delay(20);
        return 1;
    }

    if (KEY2_VALUE() == 0)
    {
        HAL_Delay(20);
        while (KEY2_VALUE() == 0);
        HAL_Delay(20);

        return 2;
    }

    if (KEY3_VALUE() == 0)
    {
        HAL_Delay(20);
        while (KEY3_VALUE() == 0);
        HAL_Delay(20);

        return 3;
    }

    if (KEY4_VALUE() == 0)
    {
        HAL_Delay(20);
        while (KEY4_VALUE() == 0);
        HAL_Delay(20);

        return 4;
    }

    return 0;
}
