#pragma once

#include "Core/Application.hpp"

int main()
{
    auto app = Application();
    
    app.Initialize();

    while (!app.GetQuit())
    {
        app.Update();
    }
    
    app.Destroy();
}
