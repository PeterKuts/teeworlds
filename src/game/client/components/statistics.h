//
//  perksboard.h
//  client
//
//  Created by User on 11/7/15.
//  Copyright © 2015 Peter Kuts. All rights reserved.
//

#ifndef GAME_CLIENT_COMPONENTS_STATISTICS_H
#define GAME_CLIENT_COMPONENTS_STATISTICS_H
#include <game/client/component.h>
#include <engine/keys.h>

class CStatistics : public CComponent
{
    static void ConKeyStatistics(IConsole::IResult *pResult, void *pUserData);
    
    bool m_Active;
    int DoButton(const void *pID, const char *pText, int Checked, const CUIRect *pRect);
    float width;
    void DrawHeader(CUIRect *viewRect);
public:
    CStatistics();
    virtual void OnReset();
    virtual void OnConsoleInit();
    virtual void OnRender();
    virtual void OnRelease();
    bool Active();
};


#endif
