/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "ctf.h"

CGameControllerCTF::CGameControllerCTF(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
    mem_zero(m_apFlags, sizeof(class CFlag *) * TEAMS_COUNT);
	m_pGameType = "CTF";
	m_GameFlags = GAMEFLAG_TEAMS|GAMEFLAG_FLAGS;
}

bool CGameControllerCTF::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnEntity(Index, Pos))
		return true;

	int Team = -1;
	if(Index == ENTITY_FLAGSTAND_RED) Team = TEAM_RED;
	if(Index == ENTITY_FLAGSTAND_BLUE) Team = TEAM_BLUE;
    if(Index == ENTITY_FLAGSTAND_YELLOW) Team = TEAM_YELLOW;
    if(Team == -1 || m_apFlags[Team])
		return false;

	CFlag *F = new CFlag(&GameServer()->m_World, Team);
	F->m_StandPos = Pos;
	F->m_Pos = Pos;
	m_apFlags[Team] = F;
	GameServer()->m_World.InsertEntity(F);
	return true;
}

int CGameControllerCTF::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	int HadFlag = 0;

	// drop flags
	for(int i = 0; i < TEAMS_COUNT; i++)
	{
		CFlag *F = m_apFlags[i];
        if(F && pKiller && pKiller->GetCharacter() && F->m_pCarryingCharacter == pKiller->GetCharacter()) {
			HadFlag |= 2;
        }
		if(F && F->m_pCarryingCharacter == pVictim)
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
			F->m_DropTick = Server()->Tick();
			F->m_pCarryingCharacter = 0;
			F->m_Vel = vec2(0,0);

			if(pKiller && pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam())
				pKiller->m_Score++;

			HadFlag |= 1;
		}
	}

	return HadFlag;
}

void CGameControllerCTF::DoWincheck()
{
	if(m_GameOverTick == -1 && !m_Warmup)
	{
		// check score win condition
        bool scoreLimitReached = false;
        if (g_Config.m_SvScorelimit > 0) {
            for (int i = 0; i < TEAMS_COUNT; ++i) {
                scoreLimitReached |= m_aTeamscore[i] > g_Config.m_SvScorelimit;
            }
        }
        bool timeLimitReached = g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60;
		if(scoreLimitReached || timeLimitReached)
		{
            int tScores[TEAMS_COUNT];
            mem_copy(tScores, m_aTeamscore, sizeof(int)*TEAMS_COUNT);
            for (int i = 0; i < TEAMS_COUNT; ++i) {
                int maxIdx = i;
                for (int j = i+1; j < TEAMS_COUNT; ++j) {
                    if (tScores[j] > tScores[maxIdx]) {
                        maxIdx = j;
                    }
                }
                if (maxIdx != i) {
                    tScores[maxIdx] ^= tScores[i];
                    tScores[i] ^= tScores[maxIdx];
                    tScores[maxIdx] ^= tScores[i];
                }
            }
			if(m_SuddenDeath) {
                if (tScores[0]/100 != tScores[1]/100) {
                    EndRound();
                }
			} else {
                if(m_aTeamscore[0] != m_aTeamscore[1]) {
					EndRound();
                } else {
					m_SuddenDeath = 1;
                }
			}
		}
	}
}

bool CGameControllerCTF::CanBeMovedOnBalance(int ClientID)
{
	CCharacter* Character = GameServer()->m_apPlayers[ClientID]->GetCharacter();
	if(Character)
	{
		for(int fi = 0; fi < TEAMS_COUNT; fi++)
		{
			CFlag *F = m_apFlags[fi];
			if(F && F->m_pCarryingCharacter == Character)
				return false;
		}
	}
	return true;
}

void CGameControllerCTF::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];
    pGameDataObj->m_TeamscoreYellow = m_aTeamscore[TEAM_YELLOW];

#define CHECK_FLAG(_TEAM_, _CARRIER_) \
    if(m_apFlags[_TEAM_]) \
    { \
        if(m_apFlags[_TEAM_]->m_AtStand) { \
            pGameDataObj->_CARRIER_ = FLAG_ATSTAND; \
        } else if(m_apFlags[_TEAM_]->m_pCarryingCharacter && m_apFlags[_TEAM_]->m_pCarryingCharacter->GetPlayer()) { \
            pGameDataObj->_CARRIER_ = m_apFlags[_TEAM_]->m_pCarryingCharacter->GetPlayer()->GetCID(); \
        } else { \
            pGameDataObj->_CARRIER_ = FLAG_TAKEN; \
        } \
    } else { \
        pGameDataObj->_CARRIER_ = FLAG_MISSING;\
    }

    CHECK_FLAG(TEAM_RED, m_FlagCarrierRed)
    CHECK_FLAG(TEAM_BLUE, m_FlagCarrierBlue)
    CHECK_FLAG(TEAM_YELLOW, m_FlagCarrierYellow)
    
#undef CHECK_FLAG
}

void CGameControllerCTF::updateFlags_LeftGame() {
    // Flag hits death-tile or left the game layer, reset it
    for(int fi = 0; fi < TEAMS_COUNT; fi++) {
        CFlag *F = m_apFlags[fi];
        if(!F) {continue;}
        if(GameServer()->Collision()->GetCollisionAt(F->m_Pos.x, F->m_Pos.y)&CCollision::COLFLAG_DEATH || F->GameLayerClipped(F->m_Pos)) {
            GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "flag_return");
            GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
            F->Reset();
        }
    }
}

void CGameControllerCTF::updateFlags_WithCarriers() {
    for(int fi = 0; fi < TEAMS_COUNT; fi++) {
        CFlag *F = m_apFlags[fi];
        if(!F) {continue;}
        if(!F->m_pCarryingCharacter) {continue;}
        F->m_Pos = F->m_pCarryingCharacter->m_Pos;
        for (int ofi = 0; ofi < TEAMS_COUNT; ++ofi) {
            if (ofi == fi) { continue;}
            bool flagIsOnStand = m_apFlags[ofi] && m_apFlags[ofi]->m_AtStand;
            if (!flagIsOnStand) { continue;}
            bool flagColided = distance(F->m_Pos, m_apFlags[ofi]->m_Pos) < CFlag::ms_PhysSize + CCharacter::ms_PhysSize;
            if (!flagColided) { continue;}
            // CAPTURE! \o/
            m_aTeamscore[ofi] += 100;
            F->m_pCarryingCharacter->GetPlayer()->m_Score += 5;
            
            char aBuf[512];
            str_format(aBuf, sizeof(aBuf), "flag_capture player='%d:%s'",
                       F->m_pCarryingCharacter->GetPlayer()->GetCID(),
                       Server()->ClientName(F->m_pCarryingCharacter->GetPlayer()->GetCID()));
            GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
            
            float CaptureTime = (Server()->Tick() - F->m_GrabTick)/(float)Server()->TickSpeed();
            if(CaptureTime <= 60) {
                str_format(aBuf, sizeof(aBuf), "The %s flag was captured by '%s' (%d.%s%d seconds)",
                           (fi == TEAM_RED ? "red": (fi == TEAM_BLUE ? "blue" : "yellow")),
                           Server()->ClientName(F->m_pCarryingCharacter->GetPlayer()->GetCID()), (int)CaptureTime%60, ((int)(CaptureTime*100)%100)<10?"0":"", (int)(CaptureTime*100)%100);
            } else {
                str_format(aBuf, sizeof(aBuf), "The %s flag was captured by '%s'",
                           (fi == TEAM_RED ? "red": (fi == TEAM_BLUE ? "blue" : "yellow")),
                           Server()->ClientName(F->m_pCarryingCharacter->GetPlayer()->GetCID()));
            }
            GameServer()->SendChat(-1, -2, aBuf);
            m_apFlags[fi]->Reset();
            m_apFlags[ofi]->Reset();
            GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
        }
    }
}

void CGameControllerCTF::updateFlags_WithoutCarriers() {
    for(int fi = 0; fi < TEAMS_COUNT; fi++) {
        CFlag *F = m_apFlags[fi];
        if(!F) {continue;}
        if(F->m_pCarryingCharacter) {continue;}
        CCharacter *apCloseCCharacters[MAX_CLIENTS];
        int Num = GameServer()->m_World.FindEntities(F->m_Pos, CFlag::ms_PhysSize, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
        for(int i = 0; i < Num; i++) {
            if(!apCloseCCharacters[i]->IsAlive()
               || apCloseCCharacters[i]->GetPlayer()->GetTeam() == TEAM_SPECTATORS
               || GameServer()->Collision()->IntersectLine(F->m_Pos, apCloseCCharacters[i]->m_Pos, NULL, NULL))
            {
                continue;
            }
            
            if(apCloseCCharacters[i]->GetPlayer()->GetTeam() == F->m_Team) {
                // return the flag
                if(!F->m_AtStand) {
                    CCharacter *pChr = apCloseCCharacters[i];
                    pChr->GetPlayer()->m_Score += 1;
                    char aBuf[256];
                    str_format(aBuf, sizeof(aBuf), "flag_return player='%d:%s'",
                               pChr->GetPlayer()->GetCID(),
                               Server()->ClientName(pChr->GetPlayer()->GetCID()));
                    GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
                    GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
                    F->Reset();
                }
                continue;
            }
            
            // take the flag
            if(F->m_AtStand) {
                m_aTeamscore[apCloseCCharacters[i]->GetPlayer()->GetTeam()]++;
                F->m_GrabTick = Server()->Tick();
            }
            
            F->m_AtStand = 0;
            F->m_pCarryingCharacter = apCloseCCharacters[i];
            F->m_pCarryingCharacter->GetPlayer()->m_Score += 1;
            
            char aBuf[256];
            str_format(aBuf, sizeof(aBuf), "flag_grab player='%d:%s'",
                       F->m_pCarryingCharacter->GetPlayer()->GetCID(),
                       Server()->ClientName(F->m_pCarryingCharacter->GetPlayer()->GetCID()));
            GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
            
            for(int c = 0; c < MAX_CLIENTS; c++) {
                CPlayer *pPlayer = GameServer()->m_apPlayers[c];
                if(!pPlayer)
                    continue;
                if(pPlayer->GetTeam() == TEAM_SPECTATORS && pPlayer->m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[pPlayer->m_SpectatorID] && GameServer()->m_apPlayers[pPlayer->m_SpectatorID]->GetTeam() == fi) {
                    GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
                } else if(pPlayer->GetTeam() == fi) {
                    GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
                } else {
                    GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, c);
                }
            }
            // demo record entry
            GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, -2);
            break;
        }
    }
}

void CGameControllerCTF::updateFlags_FreeFly() {
    for(int fi = 0; fi < TEAMS_COUNT; fi++) {
        CFlag *F = m_apFlags[fi];
        if(!F) {continue;}
        if(F->m_pCarryingCharacter || F->m_AtStand) {continue;}
        if(Server()->Tick() > F->m_DropTick + Server()->TickSpeed()*30) {
            GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
            F->Reset();
        } else {
            F->m_Vel.y += GameServer()->m_World.m_Core.m_Tuning.m_Gravity;
            GameServer()->Collision()->MoveBox(&F->m_Pos, &F->m_Vel, vec2(F->ms_PhysSize, F->ms_PhysSize), 0.5f);
        }
    }
}

void CGameControllerCTF::Tick()
{
	IGameController::Tick();

	if(GameServer()->m_World.m_ResetRequested || GameServer()->m_World.m_Paused)
		return;

    updateFlags_LeftGame();
    updateFlags_WithCarriers();
    updateFlags_WithoutCarriers();
    updateFlags_FreeFly();
    
}
