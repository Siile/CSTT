#include <engine/shared/config.h>

#include <game/mapitems.h>

#include <game/server/entities/flag.h>
#include <game/server/entities/bomb.h>
#include <game/server/entities/character.h>
#include <game/server/entities/pickup.h>
#include <game/server/entities/superexplosion.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "cstt.h"

#include <game/server/ai.h>
#include <game/server/ai/cstt_ai.h>


enum WinStatus
{
	TERRORISTS_WIN = 1,
	COUNTERTERRORISTS_WIN = 2,
	DRAW = -1,
};


CGameControllerCSTT::CGameControllerCSTT(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	GameServer()->Collision()->GenerateWaypoints();
	
	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "%d waypoints generated, %d connections created", GameServer()->Collision()->WaypointCount(), GameServer()->Collision()->ConnectionCount());
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
	
	
	m_pGameType = "CSTT";
	m_GameFlags = GAMEFLAG_TEAMS|GAMEFLAG_FLAGS;

	//char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Creating CSTT controller");
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
		
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
	m_BombActionTimer = 0;
	
	Restart();
	
	m_BroadcastTimer = 0;
}



void CGameControllerCSTT::DropPickup(vec2 Pos, int PickupType, vec2 Force, int PickupSubtype)
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





	
	
bool CGameControllerCSTT::OnEntity(int Index, vec2 Pos)
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


void CGameControllerCSTT::CreateDroppables()
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
	
	
int CGameControllerCSTT::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	//IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	
	int HadBomb = 0;
	
	pVictim->GetPlayer()->m_InterestPoints = 0;
	
	int DropWeapon = pVictim->m_ActiveCustomWeapon;
	
	if (DropWeapon != HAMMER_BASIC && DropWeapon != GUN_PISTOL) // g_Config.m_SvWeaponDrops == 1 && 
	{
		DropPickup(pVictim->m_Pos, POWERUP_WEAPON, pVictim->m_LatestHitVel, DropWeapon);
	}
	else
	{
		// drop pickup
		if (pVictim->HasAmmo())
			DropPickup(pVictim->m_Pos, POWERUP_ARMOR, pVictim->m_LatestHitVel, 0);
		else
			DropPickup(pVictim->m_Pos, POWERUP_HEALTH, pVictim->m_LatestHitVel, 0);
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
		

	if (WeaponID != WEAPON_GAME)
		pVictim->GetPlayer()->m_ForceToSpectators = true;
	
	return HadBomb;
}



void CGameControllerCSTT::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAIcstt(GameServer(), pChr->GetPlayer());
}


CBomb *CGameControllerCSTT::GetBomb()
{
	return m_pBomb;
}




bool CGameControllerCSTT::CanCharacterSpawn(int ClientID)
{
	return GameServer()->m_CanRespawn;
}

void CGameControllerCSTT::DoWincheck()
{

}

bool CGameControllerCSTT::CanBeMovedOnBalance(int ClientID)
{
	return false;
}

void CGameControllerCSTT::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;
	
	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierBlue = FLAG_ATSTAND;
	
	
	if(m_pBomb)
	{
		if(m_pBomb->m_pCarryingCharacter && m_pBomb->m_pCarryingCharacter->GetPlayer())
			pGameDataObj->m_FlagCarrierRed = m_pBomb->m_pCarryingCharacter->GetPlayer()->GetCID();
		else
			pGameDataObj->m_FlagCarrierRed = FLAG_TAKEN;
	}
	else
		pGameDataObj->m_FlagCarrierRed = FLAG_MISSING;
}



void CGameControllerCSTT::ClearPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Hide();
	}
}

void CGameControllerCSTT::RespawnPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i])
			m_apPickup[i]->Respawn();
	}
}

void CGameControllerCSTT::FlashPickups()
{
	for (int i = 0; i < m_PickupCount; i++)
	{
		if (m_apPickup[i] && !m_apPickup[i]->m_Dropable && m_apPickup[i]->m_SpawnTick <= 0)
			m_apPickup[i]->m_Flashing = true;
	}
}





int CGameControllerCSTT::CountPlayers()
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


int CGameControllerCSTT::CountPlayersAlive()
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



int CGameControllerCSTT::CheckLose()
{
	// check team & bomb status
	
	if (m_BombDefused)
	{
		return COUNTERTERRORISTS_WIN;
	}
	
	
	int Red = 0, Blue = 0;
	
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
		
		if (pPlayer->GetTeam() == TEAM_RED)
			Red++;
		
		if (pPlayer->GetTeam() == TEAM_BLUE)
			Blue++;
	}
	
	/* console debugging
	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "status: %d - %d", Red, Blue);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
	*/
	
	if (m_pBomb && m_pBomb->m_Status == BOMB_PLANTED)
	{
		// bomb planted and ready to explode
		if(m_Timeout || Blue == 0)
		{
			return TERRORISTS_WIN;
		}
		
		if(g_Config.m_SvBombTime > 0 && m_pBomb->m_Timer >= g_Config.m_SvBombTime*Server()->TickSpeed())
		{
			// big ass explosion
			CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_pBomb->m_Pos, m_pBomb->m_Owner, 0, 10, 0, true /* superdamage */);
			GameServer()->m_World.InsertEntity(S);
							
			m_pBomb->m_Hide = true;
			
			m_Timeout = true;
			return TERRORISTS_WIN;
		}
	}
	else
	{
		// time out, counter-terrorists win
		if(m_Timeout || (g_Config.m_SvRoundTime > 0 && m_RoundTick >= g_Config.m_SvRoundTime*Server()->TickSpeed()))
		{
			m_Timeout = true;
			return COUNTERTERRORISTS_WIN;
		}
		
		// check tees left only if bomb isn't planted
		if (Red > 0 && Blue == 0)
			return TERRORISTS_WIN;
		
		if (Red == 0 && Blue > 0)
			return COUNTERTERRORISTS_WIN;

		if (Red == 0 && Blue == 0)
			return DRAW;
	}
	
	return 0;
}


void CGameControllerCSTT::RoundRewards(int WinningTeam)
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
}



void CGameControllerCSTT::SaveWeapons()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		pPlayer->SaveWeapons();
	}
}



void CGameControllerCSTT::EndTheShit()
{
	EndRound();
	m_GameState = GAMESTATE_NEWGAME;
	m_NewGame = true;
	
	// if no map change?
	GameServer()->SwapTeams();
}



void CGameControllerCSTT::RoundWinLose()
{
	int win = CheckLose();

	// give points and stuff

	if (win == TERRORISTS_WIN)
	{
		GameServer()->SendBroadcast("Terrorists win", -1, true);
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, -1);
		m_aTeamscore[TEAM_RED]++;
		RoundRewards(TEAM_RED);
	}
	else if (win == COUNTERTERRORISTS_WIN)
	{
		GameServer()->SendBroadcast("Counter-terrorists win", -1, true);
		GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, -1);
		m_aTeamscore[TEAM_BLUE]++;
		RoundRewards(TEAM_BLUE);
	}
	else // draw
	{
		GameServer()->SendBroadcast("Nobody wins", -1, true);
		GameServer()->CreateSoundGlobal(SOUND_TEE_CRY, -1);
		RoundRewards(-1);
	}
}



void CGameControllerCSTT::Restart()
{
	char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Restarting game");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cstt", aBuf);
	
	m_RoundTimeLimit = 0; // gamecontroller
	m_ResetTime = true; // gamecontroller
	
	m_NewGame = false;
	
	ClearPickups();
	m_GameState = GAMESTATE_NEWGAME;
	m_RoundTick = 0;
	m_Round = 0;
	
	m_BombDefuseTimer = 0;
	m_BombDefused = false;
	
	m_aTeamscore[TEAM_RED] = 0;
	m_aTeamscore[TEAM_BLUE] = 0;
	
	GameServer()->SendBroadcast("", -1);
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aDefusing[i] = false;
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
	
		pPlayer->m_CanShop = false;
		pPlayer->NewRound();
		pPlayer->DisableShopping();
	}
	
	GameServer()->m_FreezeCharacters = false;
	GameServer()->m_CanRespawn = true;
	
	m_BombCarrierTurn = frandom()*MAX_CLIENTS;
	
	m_RoundTimeLimit = 0; // gamecontroller protected
	
	if (m_pBomb)
		m_pBomb->m_Hide = true;
	
	GameServer()->ResetVotes();
}






void CGameControllerCSTT::GiveBombToPlayer()
{
	if (!m_pBomb)
		return;
	
	m_pBomb->m_Hide = false;
	int BombCarrier = -1;
	
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		int i = c + m_BombCarrierTurn;
		
		if (i >= MAX_CLIENTS)
			i -= MAX_CLIENTS;
	
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if(pPlayer->GetTeam() != TEAM_RED)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		BombCarrier = i;
		break;
	}
		
	m_BombCarrierTurn = BombCarrier+1;
	
	if (BombCarrier >= 0)
	{
		m_pBomb->m_Owner = BombCarrier;
		m_pBomb->m_pCarryingCharacter = GameServer()->m_apPlayers[BombCarrier]->GetCharacter();
		m_pBomb->m_Status = BOMB_CARRYING;
	}
}


void CGameControllerCSTT::StartCountdown()
{
	m_RoundTimeLimit = g_Config.m_SvPreroundTime; // gamecontroller
	m_ResetTime = true; // gamecontroller
	
	ClearPickups();

	m_GameState = GAMESTATE_STARTING;
	m_RoundTick = 0;
	m_Round++;
	
	// respawn & freeze players
	GameServer()->m_FreezeCharacters = true;
	GameServer()->m_CanRespawn = true;
	
	// kill characters
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		m_aDefusing[c] = false;
		
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (pCharacter)
			pCharacter->Die(pPlayer->GetCID(), WEAPON_GAME, true);
		
		//pPlayer->JoinTeam();
		
		pPlayer->m_CanShop = true;
	}
	
	GameServer()->SendBroadcast("Prepare for battle (shop using vote menu)", -1, true);
	GameServer()->ResetVotes();
}



void CGameControllerCSTT::StartRound()
{
	m_RoundTimeLimit = g_Config.m_SvRoundTime; // gamecontroller
	m_ResetTime = true; // gamecontroller
	
	m_Timeout = false;
	
	m_BombDefuseTimer = 0;
	m_BombDefused = false;
	
	m_GameState = GAMESTATE_ROUND;
	m_RoundTick = 0;

	// unfreeze players, prevent respawning until end of round
	GameServer()->m_FreezeCharacters = false;
	GameServer()->m_CanRespawn = false;
	
	GameServer()->SendBroadcast("GO!", -1);
	GameServer()->CreateSoundGlobal(SOUND_NINJA_HIT, -1);
	
}



CFlag *CGameControllerCSTT::GetRandomBombArea()
{
	int i = 0;
	while (i++ < 20)
	{
		int r = frandom()*(MAX_BOMBAREAS-1);
		if (m_apBombArea[r] && m_apBombArea[r]->m_Team == TEAM_BLUE)
			return m_apBombArea[r];
	}
	return NULL;
}

CFlag *CGameControllerCSTT::GetClosestBombArea(vec2 Pos)
{
	CFlag *pClosestFlag = NULL;
	
	for (int f = 0; f < MAX_BOMBAREAS; f++)
	{
		if (m_apBombArea[f])
		{
			if (m_apBombArea[f]->m_Team != TEAM_BLUE)
				continue;
			
			if (!pClosestFlag)
			{
				pClosestFlag = m_apBombArea[f];
			}
			else
			{
				if (abs(m_apBombArea[f]->m_Pos.x - Pos.x) + abs(m_apBombArea[f]->m_Pos.y - Pos.y) < abs(pClosestFlag->m_Pos.x - Pos.x) + abs(pClosestFlag->m_Pos.y - Pos.y))
				{
					pClosestFlag = m_apBombArea[f];
				}
			}
		}
	}
	
	return pClosestFlag;
}



void CGameControllerCSTT::AutoBalance()
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

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
			
		if (!pCharacter->IsAlive())
			continue;
		
		if (pPlayer->GetTeam() == TEAM_RED || pPlayer->m_WantedTeam == TEAM_RED)
		{
			if (!pPlayer->m_IsBot)
				Red++;
			else
			{
				RedBotID = i;
				RedBots++;
			}
		}
		
		if (pPlayer->GetTeam() == TEAM_BLUE || pPlayer->m_WantedTeam == TEAM_BLUE)
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
	if ((Red+RedBots) < g_Config.m_SvPreferredTeamSize && (Blue+BlueBots) < g_Config.m_SvPreferredTeamSize)
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
	else
	// too many bots
	if ((Red+RedBots) > g_Config.m_SvPreferredTeamSize && (Blue+BlueBots) > g_Config.m_SvPreferredTeamSize)
	{
		if (RedBots > 1 && BlueBots > 1)
		{
			GameServer()->KickBot(BlueBotID);
			GameServer()->KickBot(RedBotID);
		}
	}
}


void CGameControllerCSTT::Tick()
{
	IGameController::Tick();

	GameServer()->UpdateSpectators();
	
	if(GameServer()->m_World.m_ResetRequested || GameServer()->m_World.m_Paused)
		return;	

	m_RoundTick++;
		
	// no actual players (bots kicked if human_players == 0)
	if (CountPlayers() <= 1 || m_NewGame)
	{
		GameServer()->m_CanRespawn = true;
	
		if (m_Round != 0 || m_GameState != GAMESTATE_NEWGAME)
			Restart();
		
		m_RoundTick = 0;

		if (CountPlayers() == 1)
			AutoBalance();
	}
	else
	{
		// first player join or new game
		if (m_GameState == GAMESTATE_NEWGAME)
		{
			//if (m_RoundTick == 60)
			//	GameServer()->SendBroadcast("First round starting in 3...", -1);
			if (m_RoundTick == 120)
				GameServer()->SendBroadcast("First round starting in 2...", -1, true);
			if (m_RoundTick == 180)
				GameServer()->SendBroadcast("First round starting in 1...", -1, true);
			if (m_RoundTick == 240)
				StartCountdown();
		}
		else
		if (m_GameState == GAMESTATE_STARTING)
		{
			if (m_RoundTick == (g_Config.m_SvPreroundTime-3)*Server()->TickSpeed() ||
				m_RoundTick == (g_Config.m_SvPreroundTime-2.5f)*Server()->TickSpeed())
			{
				AutoBalance();
			}
		
			if (m_RoundTick == (g_Config.m_SvPreroundTime-2)*Server()->TickSpeed())
			{				
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Round %d / %d", m_Round, g_Config.m_SvNumRounds);
				GameServer()->SendBroadcast(aBuf, -1, true);
				
				GiveBombToPlayer();
			}
			if (m_RoundTick >= g_Config.m_SvPreroundTime*Server()->TickSpeed())
				StartRound();
		}
		else
		if (m_GameState == GAMESTATE_ROUND)
		{
			if (CheckLose() != 0)
			{
				m_RoundTimeLimit = 0; // gamecontroller
				m_ResetTime = true; // gamecontroller
				m_GameState = GAMESTATE_ENDING;
				m_RoundTick = 0;
			}
		}
		else
		if (m_GameState == GAMESTATE_ENDING)
		{
			// final wincheck with some delay
			if (m_RoundTick == 50)
				RoundWinLose();
			
			// new round starting
			if (m_RoundTick >= 300)
			{
				if (m_Round >= g_Config.m_SvNumRounds)
				{
					EndTheShit();
					return;
				}
				else
				{
					SaveWeapons();
					StartCountdown();
				}
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
			GameServer()->SendBroadcast("Welcome to Counter-Strike: Tee Time", pPlayer->GetCID(), true);
			pPlayer->m_Welcomed = true;
		}
	}
	
	
	
		
	// dont show more than one bomb area (blue flag) to a player
	for (int f = 0; f < MAX_BOMBAREAS; f++)
	{
		if (m_apBombArea[f])
			m_apBombArea[f]->ResetDistanceInfo();
	}
	
	
	// find the closest bomb area to a player (and snap it later)
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
	
		CFlag *pClosestBombArea = GetClosestBombArea(pCharacter->m_Pos);
		
		if (pClosestBombArea)
			pClosestBombArea->m_ClosestFlagToCharacter[c] = true;
	}
	
	

	
	// the bomb (red flag)
	CBomb *B = m_pBomb;
	
	if (!B)
		return;
	
	
	
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
	
	
	if (m_Timeout || m_BombDefused || m_GameState != GAMESTATE_ROUND)
		return;

	
	if (B->m_Status == BOMB_PLANTED)
	{
		B->m_Timer++;
		
		// bomb ticking sound
		int Time = Server()->TickSpeed();
		
		if (Server()->TickSpeed() / 30 + GetTimeLeft()*8 < Time)
			Time = Server()->TickSpeed() / 20 + GetTimeLeft()*4;
		
		if (++m_BombSoundTimer >= Time)
		//if (++m_BombSoundTimer >= Server()->TickSpeed() / ((GetTimeLeft() < 4) + 1))
		{
			m_BombSoundTimer = 0;
			GameServer()->CreateSound(B->m_Pos, SOUND_CHAT_SERVER);
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
		
			if(!pCharacter->IsAlive() || pCharacter->GetPlayer()->GetTeam() != TEAM_BLUE)
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
				GameServer()->SendBroadcast("Bomb defused!", -1, true);
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
	

	//
	if(B->m_pCarryingCharacter && B->m_Status != BOMB_PLANTED)
	{
		bool BombPlantable = false;
		
		// check if carrying tee is planting the bomb
		for (int i = 0; i < MAX_BOMBAREAS; i++)
		{
			if (m_apBombArea[i])
			{
				// check distance
				if (abs(m_apBombArea[i]->m_Pos.x - B->m_Pos.x) < 200 && abs(m_apBombArea[i]->m_Pos.y - B->m_Pos.y) < 200 &&
					B->m_pCarryingCharacter->IsGrounded())
				{
					BombPlantable = true;
					
					if (B->m_Status == BOMB_CARRYING)
					{
						B->m_Status = BOMB_PLANTING;
						B->m_Timer = 0;
						
						GameServer()->SendBroadcast("Planting bomb", B->m_pCarryingCharacter->GetPlayer()->GetCID());
						//GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, B->m_pCarryingCharacter->GetPlayer()->GetCID());
					}
					else if (B->m_Status == BOMB_PLANTING)
					{
						// bomb planting sound
						if (++m_BombActionTimer >= Server()->TickSpeed()/4)
						{
							m_BombActionTimer = 0;
							GameServer()->CreateSound(B->m_Pos, SOUND_BODY_LAND);
						}
						
						B->m_pCarryingCharacter->GetPlayer()->m_InterestPoints += 6;
						
						if (++B->m_Timer >= g_Config.m_SvBombPlantTime*Server()->TickSpeed())
						{
							B->m_pCarryingCharacter->GetPlayer()->m_InterestPoints += 120;
							
							B->m_pCarryingCharacter = NULL;
							B->m_Status = BOMB_PLANTED;
							B->m_Timer = 0;
							GameServer()->SendBroadcast("Bomb planted!", -1, true);
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, -1);
							
							m_RoundTimeLimit = g_Config.m_SvBombTime; // gamecontroller
							m_ResetTime = true; // gamecontroller
							
							return;
						}
					}
				}
			}
		}
		
		if (!BombPlantable && B->m_Status == BOMB_PLANTING)
		{
			B->m_Status = BOMB_CARRYING;
			B->m_Timer = 0;
			GameServer()->SendBroadcast("", B->m_pCarryingCharacter->GetPlayer()->GetCID());
		}
	}
	else if (B->m_Status == BOMB_IDLE)
	{
		// pick up the bomb!
		CCharacter *apCloseCCharacters[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(B->m_Pos, CFlag::ms_PhysSize * 1.3f, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		
		for(int i = 0; i < Num; i++)
		{
			if(!apCloseCCharacters[i]->IsAlive() || apCloseCCharacters[i]->GetPlayer()->GetTeam() != TEAM_RED || GameServer()->Collision()->IntersectLine(B->m_Pos, apCloseCCharacters[i]->m_Pos, NULL, NULL))
				continue;

			B->m_pCarryingCharacter = apCloseCCharacters[i];
			B->m_Status = BOMB_CARRYING;
			B->m_Owner = apCloseCCharacters[i]->GetPlayer()->GetCID();
			B->m_pCarryingCharacter->GetPlayer()->m_Score += 1;
			
			
			B->m_pCarryingCharacter->GetPlayer()->m_InterestPoints += 120;

			// console printing
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "bomb_grab player='%d:%s'",
				B->m_pCarryingCharacter->GetPlayer()->GetCID(),
				Server()->ClientName(B->m_pCarryingCharacter->GetPlayer()->GetCID()));
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			// flag pick sound effect
			/*
					for(int c = 0; c < MAX_CLIENTS; c++)
					{
						CPlayer *pPlayer = GameServer()->m_apPlayers[c];
						if(!pPlayer)
							continue;

						if(pPlayer->GetTeam() == TEAM_SPECTATORS && pPlayer->m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[pPlayer->m_SpectatorID] && GameServer()->m_apPlayers[pPlayer->m_SpectatorID]->GetTeam() == fi)
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
						else if(pPlayer->GetTeam() == fi)
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
						else
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, c);
					}
					// demo record entry
					GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, -2);
					break;
				}
			}
			*/
		}
		
		
	}	

	// don't add anything relevant here! possible return; above!
}











