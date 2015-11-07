//
//  perksboard.h
//  client
//
//  Created by User on 11/7/15.
//  Copyright © 2015 Peter Kuts. All rights reserved.
//

#ifndef GAME_CLIENT_COMPONENTS_PERKSBOARD_H
#define GAME_CLIENT_COMPONENTS_PERKSBOARD_H
#include <game/client/component.h>
#include <engine/keys.h>

class CPerksboard : public CComponent
{
    static void ConKeyPerksboard(IConsole::IResult *pResult, void *pUserData);
    
    bool m_Active;
    vec2 m_MousePos;
public:
    CPerksboard();
    virtual void OnReset();
    virtual void OnConsoleInit();
    virtual void OnRender();
    virtual void OnRelease();
    virtual bool OnMouseMove(float x, float y);
    virtual bool OnInput(IInput::CEvent e);
    bool Active();
};


#endif
