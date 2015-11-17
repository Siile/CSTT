#ifndef GAME_SERVER_AI_DOM_AI_H
#define GAME_SERVER_AI_DOM_AI_H
#include <game/server/ai.h>
#include <game/server/gamecontext.h>

class CAIdom : public CAI
{
public:
	CAIdom(CGameContext *pGameServer, CPlayer *pPlayer);

	virtual void DoBehavior();
	void OnCharacterSpawn(class CCharacter *pChr);

private:
	int m_SkipMoveUpdate;
	
	class CFlag *m_TargetBase;
};

#endif