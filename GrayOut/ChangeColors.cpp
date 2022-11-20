#include "ChangeColors.h"

#include <Windows.h>
#include <magnification.h>

#include <iostream>
#include <sstream>

#include "Utils.h"



void ChangeColors::Init()
{
    W32_ABORT_IF_EQ(MagInitialize(), FALSE);
}


void ChangeColors::EnableGrayscale()
{

    MAGCOLOREFFECT grayScale = {
        0.3f,  0.3f,  0.3f,  0.0f,  0.0f,
        0.6f,  0.6f,  0.6f,  0.0f,  0.0f,
        0.1f,  0.1f,  0.1f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    };

    W32_ABORT_IF_EQ(MagSetFullscreenColorEffect(&grayScale), FALSE);
}


void ChangeColors::DisableGrayscale()
{
    MAGCOLOREFFECT regular = {
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  1.0f
    };

    W32_ABORT_IF_EQ(MagSetFullscreenColorEffect(&regular), FALSE);
}
