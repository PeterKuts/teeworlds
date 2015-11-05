//
//  ptctf.hpp
//  server
//
//  Created by User on 11/6/15.
//  Copyright © 2015 Peter Kuts. All rights reserved.
//

#ifndef GAME_SERVER_GAMEMODES_PTCTF_H
#define GAME_SERVER_GAMEMODES_PTCTF_H
#include <game/server/gamecontroller.h>
#include <game/server/entity.h>

class CGameControllerPTCTF : public IGameController
{
public:
    class CFlag *m_apFlags[2];
    
    CGameControllerPTCTF(class CGameContext *pGameServer);
    virtual void DoWincheck();
    virtual bool CanBeMovedOnBalance(int ClientID);
    virtual void Snap(int SnappingClient);
    virtual void Tick();
    
    virtual bool OnEntity(int Index, vec2 Pos);
    virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
    virtual void OnMessage(int MsgID, class CUnpacker *pUnpacker, int ClientID);
    virtual void PostReset();
};

#endif
