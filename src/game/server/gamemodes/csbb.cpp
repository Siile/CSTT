#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/flag.h>
#include <game/server/entities/bomb.h>
#include <game/server/entities/character.h>
#include <game/server/entities/pickup.h>
#include <game/server/entities/arrow.h>
#include <game/server/entities/superexplosion.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "csbb.h"

#include <game/server/ai.h>
#include <game/server/ai/csbb_ai.h>


enum WinStatus
{
	TERRORISTS_WIN = 1,
	COUNTERTERRORISTS_WIN = 2,
	DRAW = -1,
};


CGameControllerCSBB::CGameControllerCSBB(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "CSBB";
	m_GameFlags = GAMEFLAG_TEAMS|GAMEFLAG_FLAGS;

	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Creating CSTT controller");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
		
	for (int i = 0; i < MAX_PICKUPS; i++)
		m_apPickup[i] = NULL;
	
	for (int i = 0; i < MAX_BOMBAREAS; i++)
		m_apBombArea[i] = NULL;

	m_BombAreaCount = 0;
	
	m_pBomb = NULL;
	
	m_PickupCount = 0;
	m_PickupDropCount = 0;
	m_DroppablesCreated = false;
	
	m_NewGame = false;
	
	m_BombSoundTimer = 0;
	
	Restart();
	
	m_BroadcastTimer = 0;
}



void CGameControllerCSBB::DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype)
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && m_apPickup[i]->m_Dropable && m_apPickup[i]->m_Life <= 0 && m_apPickup[i]->GetType() == PickupType)
		{
			m_apPickup[i]->m_Pos = Pos;
			m_apPickup[i]->RespawnDropable();
			if (m_apPickup[i]->GetType() == POWERUP_WEAPON)
				m_apPickup[i]->SetSubtype(PickupSubtype);
			
			m_apPickup[i]->m_Vel = Force;
			return;
		}
	}
}


	
	
bool CGameControllerCSBB::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnNonPickupEntity(Index, Pos))
		return true;

	if (!m_DroppablesCreated)
		CreateDroppables();


	// create bomb if not created
	if (!m_pBomb)
	{
		char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Creating bomb entity");
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
		
		GameServer()->GenerateArrows();
		
		CBomb *B = new CBomb(&GameServer()->m_World);
		B->m_Pos = vec2(0, 0);
		B->m_Hide = true;
		m_pBomb = B;
		GameServer()->m_World.InsertEntity(B);
	}
	
	
	// bomb areas (blue flags)
	if (Index == ENTITY_FLAGSTAND_BLUE && m_BombAreaCount < MAX_BOMBAREAS)
	{
		char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Creating bomb area entity");
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
		
		CFlag *F = new CFlag(&GameServer()->m_World, TEAM_BLUE);
		F->m_StandPos = Pos;
		F->m_Pos = Pos;
		m_apBombArea[m_BombAreaCount++] = F;
		GameServer()->m_World.InsertEntity(F);
		return true;
	}
	
	return false;
}


void CGameControllerCSBB::CreateDroppables()
{
	for (int i = 0; i < MAX_DROPPABLES; i++)
	{
		// hearts
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_HEALTH, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;

		// armors
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_ARMOR, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
		
		// weapons
		m_apPickup[m_PickupCount] = new CPickup(&GameServer()->m_World, POWERUP_WEAPON, 0);
		m_apPickup[m_PickupCount]->m_Pos = vec2(0, 0);
		m_apPickup[m_PickupCount]->m_Dropable = true;
		m_PickupCount++;
	}
	
	m_DroppablesCreated = true;
}
	
	
int CGameControllerCSBB::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	//IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	
	int HadBomb = 0;
	
	pVictim->GetPlayer()->m_InterestPoints = 0;
	
	int DropWeapon = pVictim->m_ActiveCustomWeapon;
	
	if (DropWeapon != HAMMER_BASIC && DropWeapon != GUN_PISTOL) // g_Config.m_SvWeaponDrops == 1 && 
	{
		DropPickup(pVictim->m_Pos, POWERUP_WEAPON, pVictim->m_LatestHitVel, DropWeapon);
		
		for (int i = 0; i < 2; i++)
		{
			if (frandom()*10 < 3)
				DropPickup(pVictim->m_Pos, POWERUP_ARMOR, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			else
				DropPickup(pVictim->m_Pos, POWERUP_HEALTH, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		}
			
	}
	else
	{
		// drop pickup
		if (pVictim->HasAmmo())
			DropPickup(pVictim->m_Pos, POWERUP_ARMOR, pVictim->m_LatestHitVel, 0);
		else
			DropPickup(pVictim->m_Pos, POWERUP_HEALTH, pVictim->m_LatestHitVel, 0);
		
		for (int i = 0; i < 2; i++)
		{
			if (frandom()*10 < 3)
				DropPickup(pVictim->m_Pos, POWERUP_ARMOR, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			else
				DropPickup(pVictim->m_Pos, POWERUP_HEALTH, pVictim->m_LatestHitVel+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		}
	}

	// drop flags
	CBomb *B = m_pBomb;
	if(B && pKiller && pKiller->GetCharacter() && B->m_pCarryingCharacter == pKiller->GetCharacter())
		HadBomb |= 2;
	if(B && B->m_pCarryingCharacter == pVictim)
	{
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
		B->m_DropTick = Server()->Tick();
		B->m_pCarryingCharacter = 0;
		B->m_Vel = vec2(0,0);
		
		B->m_Status = BOMB_IDLE;

		HadBomb |= 1;
	}
	
	if(pKiller && pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam())
	{
		pKiller->m_Score++;
		pKiller->m_Money += g_Config.m_SvKillMoney;
		pKiller->m_InterestPoints += 60;
	}
		

	//if (WeaponID != WEAPON_GAME)
	//	pVictim->GetPlayer()->m_ForceToSpectators = true;
	pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*g_Config.m_SvRespawnDelayCSBB;
	
	return HadBomb;
}



void CGameControllerCSBB::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAIcsbb(GameServer(), pChr->GetPlayer());
	
	GameServer()->ResetVotes();
}


CBomb *CGameControllerCSBB::GetBomb()
{
	return m_pBomb;
}




bool CGameControllerCSBB::CanCharacterSpawn(int ClientID)
{
	return true;
}

void CGameControllerCSBB::DoWincheck()
{

}

bool CGameControllerCSBB::CanBeMovedOnBalance(int ClientID)
{
	return false;
}

void CGameControllerCSBB::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;
	
	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];


	
	switch (m_DefendingTeam)
	{
	case TEAM_BLUE:
	case TEAM_RED:
		if (m_pBomb && m_pBomb->m_Team == TEAM_RED) pGameDataObj->m_FlagCarrierBlue = FLAG_ATSTAND;
		break;
	default:
		if (m_pBomb && m_pBomb->m_Team == TEAM_RED) pGameDataObj->m_FlagCarrierBlue = FLAG_MISSING;
	}
	
	pGameDataObj->m_FlagCarrierRed = FLAG_ATSTAND;
	pGameDataObj->m_FlagCarrierBlue = FLAG_ATSTAND;
		
	
	/*
	if(m_pBomb)
	{
		if(m_pBomb->m_pCarryingCharacter && m_pBomb->m_pCarryingCharacter->GetPlayer())
			pGameDataObj->m_FlagCarrierRed = m_pBomb->m_pCarryingCharacter->GetPlayer()->GetCID();
		else
			pGameDataObj->m_FlagCarrierRed = FLAG_TAKEN;
	}
	else
		pGameDataObj->m_FlagCarrierRed = FLAG_MISSING;
	*/
}



void CGameControllerCSBB::ClearPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Hide();
	}
}

void CGameControllerCSBB::RespawnPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Respawn();
	}
}

void CGameControllerCSBB::FlashPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && !m_apPickup[i]->m_Dropable && m_apPickup[i]->m_SpawnTick <= 0)
			m_apPickup[i]->m_Flashing = true;
	}
}





int CGameControllerCSBB::CountPlayers()
{
	int NumPlayers = 0;
	int NumBots = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (GameServer()->IsBot(i))
			NumBots++;
		else
			NumPlayers++;
	}
	
	// return 0 and kick the bots if there's only bots playing
	if (NumPlayers == 0 && NumBots > 0)
	{
		GameServer()->KickBots();
		return 0;
	}
	
	return NumPlayers+NumBots;
}


int CGameControllerCSBB::CountPlayersAlive()
{
	int NumPlayersAlive = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
			
		if (!pCharacter->IsAlive())
			continue;
		
		NumPlayersAlive++;
	}
		
	return NumPlayersAlive;
}



int CGameControllerCSBB::CheckLose(bool Broadcast)
{
	// check team & bomb status
	
	if (m_BombDefused)
	{
		m_SkipWinBroadcast = true;
	
		if (m_DefendingTeam == TEAM_BLUE)
			return COUNTERTERRORISTS_WIN;
		if (m_DefendingTeam == TEAM_RED)
			return TERRORISTS_WIN;
	}
	
	
	if (m_pBomb && m_pBomb->m_Status == BOMB_PLANTED)
	{
		if(g_Config.m_SvBombTime > 0 && m_pBomb->m_Timer >= g_Config.m_SvBombTime*Server()->TickSpeed())
		{
			// big ass explosion
			CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_pBomb->m_Pos, m_pBomb->m_Owner, 0, 10, 0, true /* superdamage */);
			GameServer()->m_World.InsertEntity(S);

			m_pBomb->m_Hide = true;

			m_Timeout = true;
			if (m_DefendingTeam == TEAM_BLUE)
			{
				if (Broadcast)
					GameServer()->SendBroadcast("Base destroyed - Terrorists score!", -1, true);
				return TERRORISTS_WIN;
			}
			if (m_DefendingTeam == TEAM_RED)
			{
				if (Broadcast)
					GameServer()->SendBroadcast("Base destroyed - Counter-terrorists score!", -1, true);
				return COUNTERTERRORISTS_WIN;
			}
		}
	}
	else
	{
		// time out, defending team score
		if(m_Timeout || (g_Config.m_SvRoundTime > 0 && m_RoundTick >= g_Config.m_SvRoundTime*Server()->TickSpeed()))
		{
			m_Timeout = true;
			
			if (m_pBomb)
				m_pBomb->m_Hide = true;
			
			if (m_DefendingTeam == TEAM_BLUE)
			{
				if (Broadcast)
					GameServer()->SendBroadcast("Base defend success - Counter-terrorists score!", -1, true);
				return COUNTERTERRORISTS_WIN;
			}
			if (m_DefendingTeam == TEAM_RED)
			{
				if (Broadcast)
					GameServer()->SendBroadcast("Base defend success - Terrorists score!", -1, true);
				return TERRORISTS_WIN;
			}
		}
	}
	
	return 0;
}


void CGameControllerCSBB::RoundRewards(int WinningTeam)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->m_WantedTeam == WinningTeam)
			pPlayer->m_Money += g_Config.m_SvWinMoney;
		else
			pPlayer->m_Money += g_Config.m_SvLoseMoney;
	}
	
	//GameServer()->SwapTeams();
	GameServer()->ResetVotes();
}



CFlag *CGameControllerCSBB::GetClosestBombArea(vec2 Pos)
{
	for (int f = 0; f < MAX_BOMBAREAS; f++)
	{
		if (m_apBombArea[f])
		{
			if (!m_apBombArea[f]->m_Hide)
				return m_apBombArea[f];
		}
	}
	
	return NULL;
}




void CGameControllerCSBB::EndRound()
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "csbb", "Ending round");
	m_Round++;
	m_DefendingTeam = -1;
	m_GameState = CSBB_ENDING;
	m_RoundTick = 0;
	m_RoundTimeLimit = g_Config.m_SvPreroundTime;
	m_ResetTime = true;
	
	HideBombAreas();
	
	if (m_pBomb)
		m_pBomb->m_Hide = true;
	
	m_pBomb->m_Owner = -1;
	m_pBomb->m_Status = BOMB_IDLE;
	
	if (GameServer()->m_pArrow)
		GameServer()->m_pArrow->m_Hide = true;
}


void CGameControllerCSBB::RoundWinLose()
{
	m_SkipWinBroadcast = false;
	
	int score = CheckLose(true);

	// give points and stuff

	if (score == TERRORISTS_WIN)
	{
		//if (!m_SkipWinBroadcast)
		//	GameServer()->SendBroadcast("Terrorists score", -1, true);
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, -1);
		m_aTeamscore[TEAM_RED]++;
		RoundRewards(TEAM_RED);
		EndRound();
	}
	else if (score == COUNTERTERRORISTS_WIN)
	{
		//if (!m_SkipWinBroadcast)
		//	GameServer()->SendBroadcast("Counter-terrorists score", -1, true);
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, -1);
		m_aTeamscore[TEAM_BLUE]++;
		RoundRewards(TEAM_BLUE);
		EndRound();
	}
}





void CGameControllerCSBB::HideBombAreas()
{
	for (int i = 0; i < MAX_BOMBAREAS; i++)
	{
		if (m_apBombArea[i])
		{
			m_apBombArea[i]->m_Hide = true;
			m_apBombArea[i]->m_UseSnapping = false;
		}
	}
}


void CGameControllerCSBB::NewBase()
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "csbb", "Generating new base");
	
	m_GameState = CSBB_NEWBASE;
	
	m_DefendingTeam = -1;
	
	m_RoundTick = 0;
	m_RoundTimeLimit = 0; // gamecontroller
	m_ResetTime = true; // gamecontroller
	m_Timeout = false;
	m_BombDefused = false;
	m_BombDefuseTimer = 0;
	

	if (m_pBomb)
		m_pBomb->m_Hide = true;
	
	m_pBomb->m_Owner = -1;
	m_pBomb->m_Status = BOMB_IDLE;
	
	
	HideBombAreas();
	
	// get a new base
	int Base = rand()%m_BombAreaCount;
	if (m_BombAreaCount > 1)
	{
		while (Base == m_Base || !m_apBombArea[Base])
			Base = rand()%m_BombAreaCount;
	}
	
	if (m_pBomb)
	{
		m_pBomb->m_Hide = true;
		m_pBomb->m_Status = BOMB_IDLE;
	}
	
	m_apBombArea[Base]->m_Hide = false;
	m_apBombArea[Base]->m_UseSnapping = false;
	m_Base = Base;

	
	// dont clear players broadcast
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->m_BroadcastingCaptureStatus)
			pPlayer->m_BroadcastingCaptureStatus = false;
	}
	

	GameServer()->SendBroadcast("Capture the new base!", -1, true);
	GameServer()->CreateSoundGlobal(SOUND_NINJA_HIT, -1);	
	
	AutoBalance();
}



void CGameControllerCSBB::CaptureBase()
{
	if (m_Base < 0 || m_Base >= m_BombAreaCount)
	{
		NewBase();
		return;
	}
	
	if (!m_apBombArea[m_Base])
	{
		NewBase();
		return;
	}
	
	bool Red = false;
	bool Blue = false;
	
	// check for players within base range
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
		
		
		if (distance(m_apBombArea[m_Base]->m_Pos, pCharacter->m_Pos) < g_Config.m_SvBaseCaptureDistance && pCharacter->m_Pos.y < m_apBombArea[m_Base]->m_Pos.y+48)
		{
			if (pPlayer->GetTeam() == TEAM_RED)
				Red = true;
			if (pPlayer->GetTeam() == TEAM_BLUE)
				Blue = true;
		}
	}
	
	if (Red && Blue)
	{
		m_RedCaptureTime = 0;
		m_BlueCaptureTime = 0;
	}
	
	if (!Red)
	{
		m_RedCaptureTime = 0;
		if (Blue)
		{
			// blue team capturing the base
			m_BlueCaptureTime++;
		}
	}
	
	if (!Blue)
	{
		m_BlueCaptureTime = 0;
		if (Red)
		{
			// red team capturing the base
			m_RedCaptureTime++;
		}
	}
	
	// broadcast to players capturing the base
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pPlayer->m_IsBot)
			continue;
		
		if (((pPlayer->GetTeam() == TEAM_RED && Red && !Blue) || (pPlayer->GetTeam() == TEAM_BLUE && Blue && !Red)) &&
			(distance(m_apBombArea[m_Base]->m_Pos, pCharacter->m_Pos) < g_Config.m_SvBaseCaptureDistance && pCharacter->m_Pos.y < m_apBombArea[m_Base]->m_Pos.y+48))
		{
			pPlayer->m_BroadcastingCaptureStatus = true;
			GameServer()->SendBroadcast("Capturing the base", pPlayer->GetCID());
		}
		else if (pPlayer->m_BroadcastingCaptureStatus)
		{
			pPlayer->m_BroadcastingCaptureStatus = false;
			GameServer()->SendBroadcast("", pPlayer->GetCID());
		}
	}
	
	
	bool BaseCaptured = false;
	
	if (m_RedCaptureTime > g_Config.m_SvBaseCaptureTime*Server()->TickSpeed())
	{
		GameServer()->SendBroadcast("Terrorists captured the base!", -1, true);
		
		BaseCaptured = true;
		m_DefendingTeam = TEAM_RED;
		
		for (int i = 0; i < MAX_BOMBAREAS; i++)
		{
			if (m_apBombArea[i])
				m_apBombArea[i]->m_Team = TEAM_RED;
		}
		if (m_pBomb)
			m_pBomb->m_Team = TEAM_BLUE;
	}
	else if (m_BlueCaptureTime > g_Config.m_SvBaseCaptureTime*Server()->TickSpeed())
	{
		GameServer()->SendBroadcast("Counter-terrorists captured the base!", -1, true);
		
		BaseCaptured = true;
		m_DefendingTeam = TEAM_BLUE;
		
		for (int i = 0; i < MAX_BOMBAREAS; i++)
		{
			if (m_apBombArea[i])
				m_apBombArea[i]->m_Team = TEAM_BLUE;
		}
		if (m_pBomb)
			m_pBomb->m_Team = TEAM_RED;
	}
	
	
	if (BaseCaptured)
	{
		m_BlueCaptureTime = 0;
		m_RedCaptureTime = 0;
		
		m_GameState = CSBB_DEFENDING;
		m_RoundTick = 0;
		m_RoundTimeLimit = g_Config.m_SvRoundTime;
		m_ResetTime = true;
		
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, -1);	
		GiveBombToPlayer();
	}
}





void CGameControllerCSBB::Restart()
{
	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Restarting game");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
	
	m_RedCaptureTime = 0;
	m_BlueCaptureTime = 0;
	
	m_Base = -1;
	m_DefendingTeam = -1;
	
	m_RoundTimeLimit = 0; // gamecontroller
	m_ResetTime = true; // gamecontroller
	
	m_NewGame = false;
	
	ClearPickups();
	m_GameState = CSBB_NEWBASE;
	m_RoundTick = 0;
	m_Round = 0;
	
	m_BombDefuseTimer = 0;
	m_BombDefused = false;
	
	m_aTeamscore[TEAM_RED] = 0;
	m_aTeamscore[TEAM_BLUE] = 0;
	
	GameServer()->SendBroadcast("", -1);
	
	m_BombActionTimer = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aDefusing[i] = false;	
		m_aPlanting[i] = 0;
		m_aBombActionTimer[i] = 0;
		
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
	
		pPlayer->m_CanShop = false;
		pPlayer->NewRound();
	}
	
	GameServer()->m_FreezeCharacters = false;
	GameServer()->m_CanRespawn = true;
	
	m_BombCarrierTurn = frandom()*MAX_CLIENTS;
	
	m_RoundTimeLimit = 0; // gamecontroller protected
	
	if (m_pBomb)
		m_pBomb->m_Hide = true;
	
	for (int i = 0; i < MAX_BOMBAREAS; i++)
	{
		if (m_apBombArea[i])
		{
			m_apBombArea[i]->m_Hide = true;
			m_apBombArea[i]->m_UseSnapping = false;
		}
	}
	
	GameServer()->ResetVotes();
}






void CGameControllerCSBB::GiveBombToPlayer()
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "csbb", "Giving bomb to a player");
	
	if (!m_pBomb)
		return;
	
	m_pBomb->m_Hide = true;
	int BombCarrier = -1;
	
	m_pBomb->m_Owner = -1;
	m_pBomb->m_pCarryingCharacter = NULL;
	m_pBomb->m_Status = BOMB_CARRYING;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{	
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		pCharacter->m_BombStatus = BOMB_CARRYING;
	}
}






void CGameControllerCSBB::AutoBalance()
{
	int Red = 0, Blue = 0;
	int RedBots = 0, BlueBots = 0;
	
	// skip
	// return;
	
	int RedBotID = -1;
	int BlueBotID = -1;
	
	
	// count players
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		/*
		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
			
		if (!pCharacter->IsAlive())
			continue;
		*/
		
		if (pPlayer->GetTeam() == TEAM_RED)
		{
			if (!pPlayer->m_IsBot)
				Red++;
			else
			{
				RedBotID = i;
				RedBots++;
			}
		}
		
		if (pPlayer->GetTeam() == TEAM_BLUE)
		{
			if (!pPlayer->m_IsBot)
				Blue++;
			else
			{
				BlueBotID = i;
				BlueBots++;
			}
		}
	}
	
	if (Red+Blue == 0)
		return;
	

	// not enough players
	if ((Red+RedBots) < 4 && (Blue+BlueBots) < 4)
	{
		GameServer()->AddBot();
		GameServer()->AddBot();
	}
	
	// add bots when needed, as many as needed
	if (abs((Red+RedBots) - (Blue+BlueBots)) > 0)
	{
		for (int i = 0; i < abs((Red+RedBots) - (Blue+BlueBots)); i++)
			GameServer()->AddBot();
		
		char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Adding %d bots for balance", abs((Red+RedBots) - (Blue+BlueBots)));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}
	
	// too many bots
	if ((Red+RedBots) > 5 && (Blue+BlueBots) > 5)
	{
		if (RedBots > 1 && BlueBots > 1)
		{
			GameServer()->KickBot(BlueBotID);
			GameServer()->KickBot(RedBotID);
		}
	}
}


void CGameControllerCSBB::Tick()
{
	IGameController::Tick();

	GameServer()->UpdateSpectators();
	
	if(GameServer()->m_World.m_ResetRequested || GameServer()->m_World.m_Paused)
		return;	

	m_RoundTick++;
		
	// no actual players (bots kicked if human_players == 0)
	if (CountPlayers() < 1 || m_NewGame)
	{
		GameServer()->m_CanRespawn = true;
	
		if (m_Round != 0 || m_GameState != CSBB_NEWBASE)
			Restart();
		
		m_RoundTick = 0;
	}
	else
	{
		if (CountPlayers() == 1)
		{
			Restart();
			AutoBalance();
			return;
		}
		
		if (m_GameState == CSBB_NEWBASE)
		{
			CaptureBase();
			
			if (GameServer()->m_pArrow)
			{
				GameServer()->m_pArrow->m_Hide = false;
				GameServer()->m_pArrow->m_Target = m_apBombArea[m_Base]->m_Pos;
			}
		}
		if (m_GameState == CSBB_DEFENDING)
		{

			if (GameServer()->m_pArrow)
			{
				GameServer()->m_pArrow->m_Hide = false;
				GameServer()->m_pArrow->m_Target = m_apBombArea[m_Base]->m_Pos;
			}
			
			RoundWinLose();
		}
		if (m_GameState == CSBB_ENDING)
		{
			if (m_RoundTick >= g_Config.m_SvPreroundTime*Server()->TickSpeed())
			{
				/*
				if (m_Round >= g_Config.m_SvNumRounds)
				{
					EndRound();
					m_NewGame = true;
					return;
				}
				*/
				
				NewBase();
				//AutoBalance();
				
				if (GameServer()->m_pArrow)
					GameServer()->m_pArrow->m_Hide = true;
			}
		}
	}
	
	
	
	


	GameServer()->UpdateAI();

	

	// warm welcome
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;
		
		if (!pPlayer->m_Welcomed && !pPlayer->m_IsBot)
		{
			GameServer()->SendBroadcast("Welcome to Counter-Strike: Base Bombing", pPlayer->GetCID(), true);
			pPlayer->m_Welcomed = true;
		}
	}
	
	
	
	
	// the bomb (red flag)
	CBomb *B = m_pBomb;
	
	if (!B)
		return;
	
	
	
	/*
	// always update bomb position
	if(B->m_pCarryingCharacter)
	{
		B->m_Pos = B->m_pCarryingCharacter->m_Pos;
	}
	else
	{
		if (B->m_Status == BOMB_CARRYING || B->m_Status == BOMB_IDLE)
		{
			B->m_Vel.y += GameServer()->m_World.m_Core.m_Tuning.m_Gravity;
			GameServer()->Collision()->MoveBox(&B->m_Pos, &B->m_Vel, vec2(B->ms_PhysSize, B->ms_PhysSize), 0.5f);
			B->m_Status = BOMB_IDLE;
		}
	}
	*/
	
	
	if (m_Timeout || m_BombDefused || m_GameState != CSBB_DEFENDING)
		return;

	
	if (B->m_Status == BOMB_PLANTED)
	{
		B->m_Timer++;
		
		// bomb ticking sound
		int Time = Server()->TickSpeed();
		
		if (Server()->TickSpeed() / 30 + GetTimeLeft()*8 < Time)
			Time = Server()->TickSpeed() / 20 + GetTimeLeft()*4;
		
		if (++m_BombSoundTimer >= Time)
		{
			m_BombSoundTimer = 0;
			GameServer()->CreateSound(B->m_Pos, SOUND_CHAT_SERVER);
		}
		
		// bomb defusing
		//CCharacter *apCloseCCharacters[MAX_CLIENTS];
		//int Num = GameServer()->m_World.FindEntities(B->m_Pos, CFlag::ms_PhysSize * 2, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		
		bool DefusingBomb = false;
		
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[i];
			if(!pPlayer)
				continue;

			CCharacter *pCharacter = pPlayer->GetCharacter();
			if (!pCharacter)
				continue;
		
			if(!pCharacter->IsAlive() || pPlayer->GetTeam() != m_DefendingTeam)
				continue;
			
			// check distance
			if (abs(pCharacter->m_Pos.x - B->m_Pos.x) < 150 && abs(pCharacter->m_Pos.y - B->m_Pos.y) < 150 &&
				pCharacter->IsGrounded())
			{
				DefusingBomb = true;
				m_aDefusing[i] = true;
				
				pPlayer->m_InterestPoints += 7;
							
				if (m_BombDefuseTimer == 0)
				{
					GameServer()->SendBroadcast("Defusing bomb", pPlayer->GetCID());
					//GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, pPlayer->GetCID());
				}
			}
			else
			{
				if (m_aDefusing[i])
				{
					m_aDefusing[i] = false;
					GameServer()->SendBroadcast("", pPlayer->GetCID());
				}
			}
		}
		
		if (DefusingBomb)
		{
			// bomb defusing sound
			if (++m_BombActionTimer >= Server()->TickSpeed()/4)
			{
				m_BombActionTimer = 0;
				GameServer()->CreateSound(B->m_Pos, SOUND_BODY_LAND);
			}
			
			if (++m_BombDefuseTimer >= g_Config.m_SvBombDefuseTime*Server()->TickSpeed())
			{
				B->m_Hide = true;
				m_BombDefused = true;
				if (m_DefendingTeam == TEAM_RED)
					GameServer()->SendBroadcast("Bomb defused - Terrorists score!", -1, true);
				if (m_DefendingTeam == TEAM_BLUE)
					GameServer()->SendBroadcast("Bomb defused - Counter-terrorists score!", -1, true);
				GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, -1);
							
				m_RoundTimeLimit = 0; // gamecontroller
				m_ResetTime = true; // gamecontroller
			}
		}
		else
		{
			m_BombDefuseTimer = 0;
		}
		
		return;
	}
	else
	{
		for (int c = 0; c < MAX_CLIENTS; c++)
		{
			bool BombPlantable = false;
			CPlayer *pPlayer = GameServer()->m_apPlayers[c];
			if(!pPlayer)
				continue;
			
			if(pPlayer->GetTeam() == m_DefendingTeam)
				continue;

			CCharacter *pCharacter = pPlayer->GetCharacter();
			if (!pCharacter)
				continue;
		
			if(!pCharacter->IsAlive() || pPlayer->GetTeam() == m_DefendingTeam)
				continue;
				
			if (m_Base >= 0)
			{
				if (m_apBombArea[m_Base] && !m_apBombArea[m_Base]->m_Hide)
				{
					
					// check distance
					if (abs(m_apBombArea[m_Base]->m_Pos.x - pCharacter->m_Pos.x) < 200 && abs(m_apBombArea[m_Base]->m_Pos.y - pCharacter->m_Pos.y) < 200 &&
						pCharacter->IsGrounded())
					{
						BombPlantable = true;
						//GameServer()->SendBroadcast("Inside range", pPlayer->GetCID());
						
						if (pCharacter->m_BombStatus != BOMB_PLANTING)
						{
							pCharacter->m_BombStatus = BOMB_PLANTING;
							m_aPlanting[c] = 0;
							
							GameServer()->SendBroadcast("Planting bomb", pPlayer->GetCID());
						}
						else if (pCharacter->m_BombStatus == BOMB_PLANTING)
						{
							// bomb planting sound
							if (++m_aBombActionTimer[c] >= Server()->TickSpeed()/4)
							{
								m_aBombActionTimer[c] = 0;
								GameServer()->CreateSound(B->m_Pos, SOUND_BODY_LAND);
							}
							
							pPlayer->m_InterestPoints += 6;
							
							if (++m_aPlanting[c] >= g_Config.m_SvBombPlantTime*Server()->TickSpeed())
							{
								pPlayer->m_InterestPoints += 120;
								
								B->m_pCarryingCharacter = NULL;
								B->m_Status = BOMB_PLANTED;
								pCharacter->m_BombStatus = BOMB_PLANTED;
								m_aPlanting[c] = 0;
								B->m_Timer = 0;
								GameServer()->SendBroadcast("Bomb planted!", -1, true);
								GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, -1);
								
								B->m_Hide = false;
								B->m_Pos = pCharacter->m_Pos;
								B->m_Owner = c;
								
								m_RoundTimeLimit = g_Config.m_SvBombTime; // gamecontroller
								m_ResetTime = true; // gamecontroller
								
								return;
							}
						}
					}
				}
			}
			
			if (!BombPlantable)
			{
				pCharacter->m_BombStatus = BOMB_CARRYING;
				if (m_aPlanting[c] > 0)
					GameServer()->SendBroadcast("", c);
				
				m_aPlanting[c] = 0;
			}
		}
	}


	// don't add anything relevant here! possible return; above!
}











