#ifndef GAME_SERVER_CLASSABILITIES_H
#define GAME_SERVER_CLASSABILITIES_H

#include <cstring>
#include <game/generated/protocol.h>


enum Classes
{
	CLASS_SOLDIER,
	CLASS_MEDIC,
	CLASS_TECHNICIAN,
	NUM_CLASSES
};




struct CAbility
{
	char m_aName[64];
	char m_aChatMsg[128];
	int m_Require;
	int m_Class;
	int m_Cost;
	
	CAbility(const char *Name,
			const char *ChatMsg,
			int RequiredClass,
			int Require,
			int Cost)
	{
		str_copy(m_aName, Name, sizeof(m_aName));
		str_copy(m_aChatMsg, ChatMsg, sizeof(m_aChatMsg));
		m_Class = RequiredClass;
		m_Require = Require;
		m_Cost = Cost;
	}
};



enum Abilities
{
	// soldier
	BULLET_DAMAGE1, // done
	BULLET_DAMAGE2, // done
	FAST_RELOAD, // done
	
	MELEE_DAMAGE1, // done
	MELEE_DAMAGE2, // done
	MELEE_SPEED1, // done
	MELEE_LIFESTEAL, // done
	
	// medic
	STORE_HEALTH, // done
	MEDKIT_EXPANSION, // done
	EXPLOSIVE_HEARTS1, // done
	EXPLOSIVE_HEARTS2, // done
	//HEARTSTRIKE, // 
	FAST_RESPAWN, // done
	SHOTGUN_SPREAD1, // done
	
	// technician
	FAST_BOMB_ACTION, // done
	EXPLOSIVE_BELT, // done
	BOMBRADAR, // done
	EXPLOSION_DAMAGE1, // done
	ELECTRO_DAMAGE1, // done
	ELECTRO_REACH1, // done
	ELECTRO_GRENADES, // done
	//MOTOR_HOOK,
	
	// for all classes
	BODYARMOR, // done
	
	// for soldier
	HEAVYBODYARMOR, // done
	ANTIIMPACTARMOR, // done
	
	// for medic
	ANTIEXPLOSIONARMOR, // done
	
	NUM_ABILITIES
};



const CAbility aAbilities[NUM_ABILITIES] =
{
	// soldier
	CAbility("Bullet damage +1",
			"Your assault rifles and hand guns now deal +1 damage.",
			CLASS_SOLDIER, // class
			-1, // require
			1), // cost
	CAbility("Bullet damage +2",
			"Your assault rifles and hand guns now deal +2 damage.",
			CLASS_SOLDIER, // class
			BULLET_DAMAGE1, // require
			1), // cost
	CAbility("Fast reload",
			"You now reload 33% faster.",
			CLASS_SOLDIER, // class
			-1, // require
			1), // cost
	CAbility("Melee damage +2",
			"Your swords and hammers now deal +2 damage.",
			CLASS_SOLDIER, // class
			-1, // require
			1), // cost
	CAbility("Melee damage +4",
			"Your swords and hammers now deal +4 damage.",
			CLASS_SOLDIER, // class
			MELEE_DAMAGE1, // require
			1), // cost
	CAbility("Melee speed up",
			"Your swords and hammers now reload faster.",
			CLASS_SOLDIER, // class
			MELEE_DAMAGE1, // require
			1), // cost
	CAbility("Melee lifesteal",
			"Your swords and hammers now heal 33% of damage dealt.",
			CLASS_SOLDIER, // class
			MELEE_DAMAGE1, // require
			1), // cost
			
	// medic
	CAbility("Medkit - store hearts for later use",
			"You now can store up to 3 hearts and use them with heart emoticon.",
			CLASS_MEDIC, // class
			-1, // require
			1), // cost
	CAbility("Medkit expansion",
			"You now can store up to 6 hearts.",
			CLASS_MEDIC, // class
			STORE_HEALTH, // require
			1), // cost
	CAbility("Explosive hearts",
			"Your stored hearts now explode on enemy contact.",
			CLASS_MEDIC, // class
			STORE_HEALTH, // require
			1), // cost
	CAbility("Super explosive hearts",
			"Your stored hearts now explode with more power.",
			CLASS_MEDIC, // class
			EXPLOSIVE_HEARTS1, // require
			1), // cost
			/*
	CAbility("Heart strike - cost 2",
			"You can now summon heart strike from above with emoticon 8 (target / sushi).",
			CLASS_MEDIC, // class
			EXPLOSIVE_HEARTS1, // require
			2), // cost
			*/
	CAbility("Fast respawn",
			"You now respawn 33% faster.",
			CLASS_MEDIC, // class
			-1, // require
			1), // cost
	CAbility("Shotgun spread +1",
			"Your shotguns now shoots an extra pellet.",
			CLASS_MEDIC, // class
			-1, // require
			1), // cost
			
			
	// technician
	CAbility("Plant & defuse bomb faster",
			"You can now plant and defuse bombs 33% faster.",
			CLASS_TECHNICIAN, // class
			-1, // require
			1), // cost
	CAbility("Explosive belt",
			"You now explode to bits on death.",
			CLASS_TECHNICIAN, // class
			EXPLOSION_DAMAGE1, // require
			1), // cost
	CAbility("Bomb radar",
			"The blue dot now keeps track of bomb's location.",
			CLASS_TECHNICIAN, // class
			FAST_BOMB_ACTION, // require
			1), // cost
	CAbility("Explosion damage +1",
			"Your explosions now deal +1 damage.",
			CLASS_TECHNICIAN, // class
			-1, // require
			1), // cost
	CAbility("Electro damage +1",
			"Your electro rifle and taser now deal +1 damage.",
			CLASS_TECHNICIAN, // class
			-1, // require
			1), // cost
	CAbility("Electro reach up",
			"Your electro rifle and taser now reach longer.",
			CLASS_TECHNICIAN, // class
			ELECTRO_DAMAGE1, // require
			1), // cost
	CAbility("Electro grenades",
			"Your grenades are now electric.",
			CLASS_TECHNICIAN, // class
			ELECTRO_DAMAGE1, // require
			1), // cost
			/*
	CAbility("Motor hook - not done yet",
			"Your hook has now motor in it.",
			CLASS_TECHNICIAN, // class
			-1, // require
			1), // cost
			*/
			
	// for all abilities
	CAbility("Bodyarmor",
			"You now take -1 damage from all sources.",
			-1, // class
			-1, // require
			1), // cost
			
	// soldier
	CAbility("Heavy bodyarmor",
			"You now take -2 damage from all sources.",
			CLASS_SOLDIER, // class
			BODYARMOR, // require
			1), // cost
	CAbility("Anti impact armor",
			"Bullets, explosions and recoil now affects you less.",
			CLASS_SOLDIER, // class
			BODYARMOR, // require
			1), // cost
			
	// medic
	CAbility("Anti explosion armor",
			"You now take -1 damage from explosions.",
			CLASS_MEDIC, // class
			BODYARMOR, // require
			1) // cost
};



#endif