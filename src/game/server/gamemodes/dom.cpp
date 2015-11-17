#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "dom.h"

#include <game/server/ai.h>
#include <game/server/ai/dom_ai.h>


CGameControllerDOM::CGameControllerDOM(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "DOM++";
	m_GameFlags = GAMEFLAG_TEAMS|GAMEFLAG_FLAGS;
	
	for (int i = 0; i < MAX_BASES; i++)
	{
		m_apBase[i] = NULL;
		m_aDefenders[i] = 0;
	}
	
	m_ScoreTick = 0;
	m_BaseCount = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aCapturing[i] = -1;
}

	
bool CGameControllerDOM::OnEntity(int Index, vec2 Pos)
{
	// bases (flags)
	if ((Index == ENTITY_FLAGSTAND_RED || Index == ENTITY_FLAGSTAND_BLUE) && m_BaseCount < MAX_BASES)
	{
		char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Creating base entity (flag)");
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "dom", aBuf);
		
		CFlag *F = new CFlag(&GameServer()->m_World, TEAM_BLUE);
		F->m_StandPos = Pos;
		F->m_Pos = Pos;
		m_apBase[m_BaseCount++] = F;
		GameServer()->m_World.InsertEntity(F);
		return true;
	}
	
	if(IGameController::OnEntity(Index, Pos))
		return true;
	
	return false;
}



CFlag *CGameControllerDOM::GetRandomBase(int NotThisTeam)
{
	int i = 0;
	while (i++ < 20)
	{
		int r = frandom()*(MAX_BASES-1);
		if (m_apBase[r] && (NotThisTeam == -1 || m_apBase[r]->m_CaptureTeam != NotThisTeam))
			return m_apBase[r];
	}
	return NULL;
}


CFlag *CGameControllerDOM::GetClosestBase(vec2 Pos, int Team)
{
	CFlag *pClosestFlag = NULL;
	
	for (int f = 0; f < MAX_BASES; f++)
	{
		if (m_apBase[f])
		{
			if (!pClosestFlag)
			{
				pClosestFlag = m_apBase[f];
			}
			else
			{
				if (distance(m_apBase[f]->m_Pos, Pos) < distance(pClosestFlag->m_Pos, Pos) && (Team == -1 || Team != m_apBase[f]->m_CaptureTeam))
				{
					pClosestFlag = m_apBase[f];
				}
			}
		}
	}
	
	return pClosestFlag;
}


CFlag *CGameControllerDOM::GetUndefendedBase(int Team)
{
	int i = rand()%(MAX_BASES-1);
	
	for (int n = 0; n < MAX_BASES; n++)
	{
		if (++i >= MAX_BASES)
			i = 0;
		
		if (m_apBase[i] && (Team == -1 || m_apBase[i]->m_CaptureTeam == Team))
		{
			if (m_aDefenders[i] == 0)
				return m_apBase[i];
		}
	}
	
	return NULL;
}



int CGameControllerDOM::Defenders(CFlag *Base)
{
	for (int i = 0; i < MAX_BASES; i++)
	{
		if (m_apBase[i] && m_apBase[i] == Base)
			return m_aDefenders[i];
	}
	
	return -1;
}
	

int CGameControllerDOM::CountBases(int Team)
{
	int Bases = 0;
	
	for (int i = 0; i < MAX_BASES; i++)
	{
		if (m_apBase[i] && (m_apBase[i]->m_CaptureTeam == Team || Team == -1))
			Bases++;
	}
	
	return Bases;
}



void CGameControllerDOM::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
		pChr->GetPlayer()->m_pAI = new CAIdom(GameServer(), pChr->GetPlayer());

	GameServer()->ResetVotes();
}


int CGameControllerDOM::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if(pKiller && Weapon != WEAPON_GAME)
	{
		// do team scoring
		/*
		if(pKiller == pVictim->GetPlayer() || pKiller->GetTeam() == pVictim->GetPlayer()->GetTeam())
			m_aTeamscore[pKiller->GetTeam()&1]--; // klant arschel
		else
			m_aTeamscore[pKiller->GetTeam()&1]++; // good shit
		*/
	}

	pVictim->GetPlayer()->m_RespawnTick = max(pVictim->GetPlayer()->m_RespawnTick, Server()->Tick()+Server()->TickSpeed()*g_Config.m_SvRespawnDelayTDM);

	return 0;
}


void CGameControllerDOM::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierRed = FLAG_ATSTAND;
	pGameDataObj->m_FlagCarrierBlue = FLAG_ATSTAND;
}





void CGameControllerDOM::BaseTick()
{
	for (int i = 0; i < MAX_BASES; i++)
	{
		if (m_apBase[i])
		{
			m_aDefenders[i] = 0;

			int Capturing = 0;
		
			for (int c = 0; c < MAX_CLIENTS; c++)
			{
				CPlayer *pPlayer = GameServer()->m_apPlayers[c];
				if(!pPlayer)
					continue;
				
				if(!pPlayer->GetCharacter())
					continue;
				
				if (distance(m_apBase[i]->m_Pos, pPlayer->GetCharacter()->m_Pos) > g_Config.m_SvBaseCaptureDistance)
				{
					if (m_aCapturing[c] == i)
					{
						m_aCapturing[c] = -1;
						GameServer()->SendBroadcast("", pPlayer->GetCID());
					}
					
					continue;
				}
				else
				{
					if (GameServer()->Collision()->IntersectLine(m_apBase[i]->m_Pos, pPlayer->GetCharacter()->m_Pos, NULL, NULL))
					{
						if (m_aCapturing[c] == i)
						{
							m_aCapturing[c] = -1;
							GameServer()->SendBroadcast("", pPlayer->GetCID());
						}
						
						continue;
					}
				}
				
				int Team = pPlayer->GetTeam();
				
				// capturing
				if (Team == TEAM_RED)
				{
					m_apBase[i]->m_CapturePoints -= 1;
					Capturing = -1;
				}
				if (Team == TEAM_BLUE)
				{
					m_apBase[i]->m_CapturePoints += 1;
					Capturing = 1;
				}
				
				if (Team != m_apBase[i]->m_CaptureTeam)
				{
					GameServer()->SendBroadcast("Capturing the base", pPlayer->GetCID());
					m_aCapturing[c] = i;
				}
				else
				{
					m_aDefenders[i]++;
				}
			}
			
			if (Capturing == 0)
				m_apBase[i]->m_CapturePoints *= 0.95f;


			// captured
			if (m_apBase[i]->m_CapturePoints <= -g_Config.m_SvBaseCaptureTreshold)
			{
				m_apBase[i]->m_CapturePoints = -g_Config.m_SvBaseCaptureTreshold;
				
				if (m_apBase[i]->m_CaptureTeam != TEAM_RED)
				{
					m_apBase[i]->m_CaptureTeam = TEAM_RED;
					
					char aBuf[128];
					str_format(aBuf, sizeof(aBuf), "Red team captured base %d!", i+1);
					GameServer()->SendBroadcast(aBuf, -1, true);
					
					// play sound
					for (int x = 0; x < MAX_CLIENTS; x++)
					{
						CPlayer *P = GameServer()->m_apPlayers[x];
						if(!P)
							continue;
						
						if (P->GetTeam() == TEAM_RED)
							GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, x);
						else
							GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, x);
					}
				}
			}				
			if (m_apBase[i]->m_CapturePoints >= g_Config.m_SvBaseCaptureTreshold)
			{
				m_apBase[i]->m_CapturePoints = g_Config.m_SvBaseCaptureTreshold;
				
				if (m_apBase[i]->m_CaptureTeam != TEAM_BLUE)
				{
					m_apBase[i]->m_CaptureTeam = TEAM_BLUE;
					
					char aBuf[128];
					str_format(aBuf, sizeof(aBuf), "Blue team captured base %d!", i+1);
					GameServer()->SendBroadcast(aBuf, -1, true);
					
					// play sound
					for (int x = 0; x < MAX_CLIENTS; x++)
					{
						CPlayer *P = GameServer()->m_apPlayers[x];
						if(!P)
							continue;
						
						if (P->GetTeam() == TEAM_BLUE)
							GameServer()->CreateSoundGlobal(SOUND_CTF_CAPTURE, x);
						else
							GameServer()->CreateSoundGlobal(SOUND_CTF_DROP, x);
					}
				}
			}
		}
	}
}


void CGameControllerDOM::Tick()
{
	IGameController::Tick();
	AutoBalance();
	GameServer()->UpdateAI();
	
	BaseTick();
	
	if (m_ScoreTick + Server()->TickSpeed()*5 <= Server()->Tick())
	{
		m_ScoreTick = Server()->Tick();
		m_aTeamscore[TEAM_RED] += CountBases(TEAM_RED);
		m_aTeamscore[TEAM_BLUE] += CountBases(TEAM_BLUE);
	}
	
	int PlayerCount = 0;
	
	// warm welcome
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;
		
		PlayerCount++;
		
		if (!pPlayer->m_Welcomed && !pPlayer->m_IsBot)
		{
			GameServer()->SendBroadcast("Welcome to Domination++", pPlayer->GetCID(), true);
			pPlayer->m_Welcomed = true;
		}
	}
	
	if (PlayerCount == 0)
	{
		m_aTeamscore[TEAM_RED] = 0;
		m_aTeamscore[TEAM_BLUE] = 0;
	}
	
	
	// dont show more than one base (flag) to a player
	for (int f = 0; f < MAX_BASES; f++)
	{
		if (m_apBase[f])
			m_apBase[f]->ResetDistanceInfo();
	}
	
	
	// find the closest base to a player (and snap it later)
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[c];
		if(!pPlayer)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
	
		CFlag *pClosestBase = GetClosestBase(pCharacter->m_Pos);
		
		if (pClosestBase)
			pClosestBase->m_ClosestFlagToCharacter[c] = true;
	}
}
