/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_ADVANCED_VARIABLES_H
#define GAME_ADVANCED_VARIABLES_H
#undef GAME_ADVANCED_VARIABLES_H // this file will be included several times


// client

// server
MACRO_CONFIG_INT(AdvRespawnDelay, adv_respawn_delay, 500, 0, 10000, CFGFLAG_SERVER, "Time needed to respawn after death (milis)")
MACRO_CONFIG_INT(AdvRespawnDelaySelfKill, adv_respawn_delay_self_kill, 3000, 0, 10000, CFGFLAG_SERVER, "Time needed to respawn after suicide (milis)")
MACRO_CONFIG_INT(AdvRespawnDelayWorldKill, adv_respawn_delay_world_kill, 500, 0, 10000, CFGFLAG_SERVER, "Time needed to respawn after falling (milis)")

MACRO_CONFIG_INT(ActfGrabScore, actf_grab_score, 1, -1000, 1000, CFGFLAG_SERVER, "Team score for grabbing the flag in ACTF mode")
MACRO_CONFIG_INT(ActfReturnScore, actf_return_score, 0, -1000, 1000, CFGFLAG_SERVER, "Team score for grabbing the flag in ACTF mode")
MACRO_CONFIG_INT(ActfCaptureScore, actf_capture_score, 100, -1000, 1000, CFGFLAG_SERVER, "Team score for returning the flag in ACTF mode")

MACRO_CONFIG_INT(ActfPlayerGrabScore, actf_player_grab_score, 1, -1000, 1000, CFGFLAG_SERVER, "Player score for grabbing the flag in ACTF mode")
MACRO_CONFIG_INT(ActfPlayerReturnScore, actf_player_return_score, 1, -1000, 1000, CFGFLAG_SERVER, "Player score for returning the flag in ACTF mode")
MACRO_CONFIG_INT(ActfPlayerCaptureScore, actf_player_capture_score, 5, -1000, 1000, CFGFLAG_SERVER, "Player score for capturing the flag in ACTF mode")

// debug

#endif
