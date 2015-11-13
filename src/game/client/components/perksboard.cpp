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
#include "perksboard.h"

CPerksboard::CPerksboard()
{
    OnReset();
}

void CPerksboard::ConKeyPerksboard(IConsole::IResult *pResult, void *pUserData)
{
    ((CPerksboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CPerksboard::OnReset()
{
    m_Active = false;
}

void CPerksboard::OnConsoleInit()
{
    Console()->Register("+perksboard", "", CFGFLAG_CLIENT, ConKeyPerksboard, this, "Show perksboard");
}

void CPerksboard::OnRender()
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
    
    float mx = (m_MousePos.x/(float)Graphics()->ScreenWidth())*pScreen->w;
    float my = (m_MousePos.y/(float)Graphics()->ScreenHeight())*pScreen->h;
    
    int Buttons = 0;
    if(Input()->KeyPressed(KEY_MOUSE_1)) Buttons |= 1;
    if(Input()->KeyPressed(KEY_MOUSE_2)) Buttons |= 2;
    if(Input()->KeyPressed(KEY_MOUSE_3)) Buttons |= 4;
    
    UI()->Update(mx,my,mx*3.0f,my*3.0f,Buttons);
    
    viewRect.Margin(5, &viewRect);
    static int buttonIds[NUM_PERKS] = {0, 0, 0, 0, 0, 0, 0};
    float buttonH = viewRect.h / NUM_PERKS;
    CUIRect buttonRect;
    for (int i=0; i<NUM_PERKS; ++i) {
        viewRect.HSplitTop(buttonH, &buttonRect, &viewRect);
        buttonRect.Margin(5, &buttonRect);
        if (DoButton(&buttonIds[i], g_pData->m_aPerks[i].m_pName, i == m_pClient->m_Snap.m_pLocalInfo->m_WantedPerk, &buttonRect)) {
            CNetMsg_Acl_SetPerk Msg;
            Msg.m_Perk = i;
            Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
        }
    }
    
    Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
    Graphics()->QuadsBegin();
    Graphics()->SetColor(1,1,1,1);
    IGraphics::CQuadItem QuadItem(mx, my, 24, 24);
    Graphics()->QuadsDrawTL(&QuadItem, 1);
    Graphics()->QuadsEnd();
}

void CPerksboard::OnRelease()
{
    m_Active = false;
}

bool CPerksboard::Active()
{
    return m_Active;
}

bool CPerksboard::OnMouseMove(float x, float y)
{
    if (!Active())
        return false;
    
    UI()->ConvertMouseMove(&x, &y);
    m_MousePos.x += x;
    m_MousePos.y += y;
    if(m_MousePos.x < 0) m_MousePos.x = 0;
    if(m_MousePos.y < 0) m_MousePos.y = 0;
    if(m_MousePos.x > Graphics()->ScreenWidth()) m_MousePos.x = Graphics()->ScreenWidth();
    if(m_MousePos.y > Graphics()->ScreenHeight()) m_MousePos.y = Graphics()->ScreenHeight();
    
    return true;
}

bool CPerksboard::OnInput(IInput::CEvent e)
{
    if (!Active())
        return false;
    return false;
}

int CPerksboard::DoButton(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
    RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f)*ButtonColorMul(pID, Checked), CUI::CORNER_ALL, 5.0f);
    CUIRect Temp;
    pRect->HMargin(pRect->h>=20.0f?2.0f:1.0f, &Temp);
    UI()->DoLabel(&Temp, pText, Temp.h*0.8, 0);
    return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

vec4 CPerksboard::ButtonColorMul(const void *pID, int Checked)
{
    if(UI()->ActiveItem() == pID || Checked)
        return vec4(1,1,1,0.5f);
    else if(UI()->HotItem() == pID)
        return vec4(1,1,1,1.5f);
    return vec4(1,1,1,1);
}

