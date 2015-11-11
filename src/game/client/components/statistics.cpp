//
//  perksboard.cpp
//  client
//
//  Created by User on 11/7/15.
//  Copyright © 2015 Peter Kuts. All rights reserved.
//

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <game/client/components/scoreboard.h>
#include <game/client/components/menus.h>
#include <game/client/components/chat.h>
#include <game/generated/client_data.h>
#include "statistics.h"

CStatistics::CStatistics()
{
    OnReset();
}

void CStatistics::ConKeyStatistics(IConsole::IResult *pResult, void *pUserData)
{
    ((CStatistics *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CStatistics::OnReset()
{
    m_Active = false;
}

void CStatistics::OnConsoleInit()
{
    Console()->Register("+statistics", "", CFGFLAG_CLIENT, ConKeyStatistics, this, "Show statistics");
}

void CStatistics::OnRender()
{
    if(!Active())
        return;
    
    CUIRect *pScreen = UI()->Screen();
    CUIRect viewRect;
    pScreen->Margin(100, &viewRect);
    Graphics()->MapScreen(pScreen->x, pScreen->y, pScreen->w, pScreen->h);
    Graphics()->BlendNormal();
    Graphics()->TextureSet(-1);
    Graphics()->QuadsBegin();
    Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
    RenderTools()->DrawRoundRect(viewRect.x, viewRect.y, viewRect.w, viewRect.h, 17.0f);
    Graphics()->QuadsEnd();
}

void CStatistics::OnRelease()
{
    m_Active = false;
}

bool CStatistics::Active()
{
    return m_Active;
}

int CStatistics::DoButton(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
    return 0;
}

