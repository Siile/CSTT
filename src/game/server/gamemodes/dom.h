#ifndef GAME_SERVER_GAMEMODES_DOM_H
#define GAME_SERVER_GAMEMODES_DOM_H
#include <game/server/gamecontroller.h>

#define MAX_BASES 10

class CGameControllerDOM : public IGameController
{
private:
	class CFlag *m_apBase[MAX_BASES]; // flags
	int m_BaseCount;

	// for AI
	int m_aDefenders[MAX_BASES];
	
	// for cleaning broadcast
	int m_aCapturing[MAX_CLIENTS];
	
	void BaseTick();
	
	int m_ScoreTick;
	
public:
	CGameControllerDOM(class CGameContext *pGameServer);

	class CFlag *GetClosestBase(vec2 Pos, int Team = -1);
	class CFlag *GetUndefendedBase(int Team = -1);
	class CFlag *GetRandomBase(int NotThisTeam = -1);
	int Defenders(class CFlag *Base);
	
	int CountBases(int Team = -1);
	
	virtual bool OnEntity(int Index, vec2 Pos);
	void OnCharacterSpawn(class CCharacter *pChr, bool RequestAI = false);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};
#endif
