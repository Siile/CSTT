#ifndef GAME_SERVER_UPGRADELIST_H
#define GAME_SERVER_UPGRADELIST_H

#include <cstring>
#include <game/generated/protocol.h>


#define MAX_GRENADES 5


struct CWeapon
{
	char m_Name[64];
	char m_BuyCmd[64];
	int m_ParentWeapon;
	int m_ProjectileType;
	int m_Sound;
	int m_Cost;
	int m_Damage;
	bool m_FullAuto;
	float m_BulletSpread;
	int m_ShotSpread;
	int m_ClipSize;
	int m_PowerupSize;
	int m_MaxAmmo;
	float m_BulletLife;
	int m_BulletReloadTime;
	int m_ClipReloadTime;
	float m_Knockback;
	float m_SelfKnockback;
	int m_Extra1;
	int m_Require;


	CWeapon(const char *Name,
			const char *BuyCmd,
			int ParentWeapon,
			int ProjectileType,
			int Sound,
			int Require,
			int Cost,
			int Damage,
			int Extra1,
			bool FullAuto,
			float BulletSpread,
			int ShotSpread,
			int ClipSize,
			int PowerupSize,
			int MaxAmmo,
			float BulletLife,
			int BulletReloadTime,
			int ClipReloadTime,
			float Knockback,
			float SelfKnockback)
	{
	    str_copy(m_Name, Name, sizeof(m_Name));
	    str_copy(m_BuyCmd, BuyCmd, sizeof(m_BuyCmd));
		m_ParentWeapon = ParentWeapon;
		m_ProjectileType = ProjectileType;
		m_Sound = Sound;
		m_Require = Require;
		m_Cost = Cost;
		m_Damage = Damage;
		m_Extra1 = Extra1;
		m_FullAuto = FullAuto;
		m_BulletSpread = BulletSpread;
		m_ShotSpread = ShotSpread;
		m_ClipSize = ClipSize;
		m_PowerupSize = PowerupSize;
		m_MaxAmmo = MaxAmmo;
		m_BulletLife = BulletLife;
		m_BulletReloadTime = BulletReloadTime;
		m_ClipReloadTime = ClipReloadTime;
		m_Knockback = Knockback;
		m_SelfKnockback = SelfKnockback;
	}
};

enum CustomWeapons
{
	HAMMER_BASIC,
	HAMMER_THUNDER,
	SWORD_KATANA,
	SWORD_LIGHTNING,
	GUN_PISTOL,
	GUN_UZI,
	GUN_MAGNUM,
	GUN_TASER,
	SHOTGUN_DOUBLEBARREL,
	SHOTGUN_COMBAT,
	GRENADE_GRENADELAUNCHER,
	GRENADE_DOOMLAUNCHER,
	GRENADE_ELECTROLAUNCHER,
	RIFLE_ASSAULTRIFLE,
	RIFLE_HEAVYRIFLE,
	RIFLE_LIGHTNINGRIFLE,
	RIFLE_STORMRIFLE,
	RIFLE_LASERRIFLE,
	RIFLE_ELECTRO,
	RIFLE_DOOMRAY,
	GRENADE_GRENADE,
	NUM_CUSTOMWEAPONS
};


const int BotAttackRange[NUM_CUSTOMWEAPONS] =
{
	120, // HAMMER_BASIC,
	300, // HAMMER_THUNDER,
	430, // SWORD_KATANA,
	410, // SWORD_LIGHTNING,
	750, // GUN_PISTOL,
	750, // GUN_UZI,
	750, // GUN_MAGNUM,
	250, // GUN_TASER,
	400, // SHOTGUN_DOUBLEBARREL,
	400, // SHOTGUN_COMBAT,
	520, // GRENADE_GRENADELAUNCHER,
	600, // GRENADE_DOOMLAUNCHER,
	600, // GRENADE_ELECTROLAUNCHER,
	500, // GRENADE_GRENADE
	780, // RIFLE_ASSAULTRIFLE,
	780, // RIFLE_HEAVYRIFLE,
	430, // RIFLE_LIGHTNINGRIFLE,
	650, // RIFLE_STORMRIFLE,
	740, // RIFLE_LASERRIFLE,
	500, // RIFLE_ELECTRO,
	740, // RIFLE_DOOMRAY,
};


enum ProjectileTypes
{
	PROJTYPE_NONE,
	PROJTYPE_HAMMER,
	PROJTYPE_FLYHAMMER,
	PROJTYPE_SWORD,
	PROJTYPE_BULLET,
	PROJTYPE_PELLET,
	PROJTYPE_LASER,
	PROJTYPE_LIGHTNING,
	PROJTYPE_ELECTRO,
	PROJTYPE_GRENADE,
	PROJTYPE_MINE
};

enum WeaponExtraFeature
{
	NO_EXTRA1,
	EXPLOSIVE,
	MEGAROCKETS,
	DOOMROCKETS,
	SLEEPEFFECT,
	ELECTRIC,
	SMOKE,
	NUM_EXTRA_FEATURES
};


const CWeapon aCustomWeapon[NUM_CUSTOMWEAPONS] =
{	
	CWeapon(
		"Hammer",
		"/buy hammer",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		15, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		170, // bullet reload time
		0, // clip reload time
		1.0f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Thunder hammer",
		"/buy thunderhammer",
		WEAPON_HAMMER,
		PROJTYPE_FLYHAMMER,
		SOUND_HAMMER_FIRE,
		HAMMER_BASIC, // require
		75, // cost
		20, // damage
		ELECTRIC, // extra1
		false, // autofire
		1.0f, // bullet spread
		1, // shot spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		2.0f, // bullet life
		400, // bullet reload time
		0, // clip reload time
		0.75f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Katana",
		"/buy katana",
		WEAPON_HAMMER,
		PROJTYPE_SWORD,
		SOUND_HAMMER_FIRE,
		HAMMER_BASIC, // require
		75, // cost
		25, // damage
		0, // extra1
		false, // autofire
		0, // bullet spread
		1, // shot spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0.9f, // bullet life - vanilla ninja life * bulletlife
		600, // bullet reload time
		0, // clip reload time
		0.0f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Storm sword",
		"/buy stormsword",
		WEAPON_HAMMER,
		PROJTYPE_SWORD,
		SOUND_HAMMER_FIRE,
		SWORD_KATANA, // require
		100, // cost
		25, // damage
		ELECTRIC, // extra1
		false, // autofire
		0, // bullet spread
		1, // shot spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0.85f, // bullet life - vanilla ninja life * bulletlife
		550, // bullet reload time
		0, // clip reload time
		0.0f, // knockback
		0.0f // self knockback
		),
	CWeapon(
		"Pistol",
		"/buy pistol",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		0, // cost
		11, // damage
		0, // extra1
		false, // autofire
		0.04f, // bullet spread
		1, // shot spread
		7, // clip size
		7, // powerup size
		70, // max ammo
		0.3f, // bullet life
		160, // bullet reload time
		650, // clip reload time
		7.0f, // bullet knockback
		0.5f // self knockback
		),
	CWeapon(
		"Uzi",
		"/buy uzi",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		GUN_PISTOL, // require
		50, // cost
		7, // damage
		0, // extra1
		true, // autofire
		0.2f, // bullet spread
		1, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0.3f, // bullet life
		100, // bullet reload time
		650, // clip reload time
		5.0f, // bullet knockback
		0.5f // self knockback
		),
	CWeapon(
		"Magnum .9000",
		"/upg magnum",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		GUN_PISTOL, // require
		50, // cost
		11, // damage
		EXPLOSIVE, // extra1
		false, // autofire
		0, // bullet spread
		1, // shot spread
		6, // clip size
		6, // powerup size
		60, // max ammo
		0.35f, // bullet life
		160, // bullet reload time
		700, // clip reload time
		7.0f, // bullet knockback
		2.0f // self knockback
		),
	CWeapon(
		"Taser",
		"/upg taser",
		WEAPON_GUN,
		PROJTYPE_ELECTRO,
		SOUND_RIFLE_FIRE,
		GUN_PISTOL, // require
		40, // cost
		8, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		10, // clip size
		20, // powerup size
		100, // max ammo
		190, // bullet life
		100, // bullet reload time
		700, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Double barrel shotgun",
		"/buy double",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		-1, // require
		75, // cost
		6, // damage
		0, // extra1
		true, // autofire
		0.04f, // bullet spread
		6, // shot spread
		2, // clip size
		6, // powerup size
		20, // max ammo
		0.17f, // bullet life
		225, // bullet reload time
		750, // clip reload time
		5.0f, // bullet knockback
		6.0f // self knockback
		),
	CWeapon(
		"Combat shotgun",
		"/upg combat",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		SHOTGUN_DOUBLEBARREL, // require
		110, // cost
		7, // damage
		0, // extra1
		true, // autofire
		0.03f, // bullet spread
		5, // shot spread
		10, // clip size
		10, // powerup size
		100, // max ammo
		0.2f, // bullet life
		215, // bullet reload time
		1200, // clip reload time
		5.0f, // bullet knockback
		3.0f // self knockback
		),
	CWeapon(
		"Grenade launcher",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		100, // cost
		6, // damage
		MEGAROCKETS, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		6, // clip size
		6, // powerup size
		60, // max ammo
		1.4f, // bullet life
		300, // bullet reload time
		800, // clip reload time
		0.0f, // bullet knockback
		3.0f // self knockback
		),
	CWeapon(
		"Doom launcher",
		"/upg doom",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_GRENADELAUNCHER, // require
		120, // cost
		9, // damage
		DOOMROCKETS, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		4, // clip size
		4, // powerup size
		40, // max ammo
		1.5f, // bullet life
		360, // bullet reload time
		1200, // clip reload time
		0.0f, // bullet knockback
		4.0f // self knockback
		),
	CWeapon(
		"Electro launcher",
		"/buy electrogrenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_GRENADELAUNCHER, // require
		100, // cost
		6, // damage
		ELECTRIC, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		5, // clip size
		5, // powerup size
		50, // max ammo
		1.4f, // bullet life
		300, // bullet reload time
		800, // clip reload time
		0.0f, // bullet knockback
		3.3f // self knockback
		),
	CWeapon(
		"Assault rifle",
		"/buy rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		100, // cost
		13, // damage
		0, // extra1
		true, // autofire
		0.08f, // bullet spread
		1, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0.45f, // bullet life
		120, // bullet reload time
		1200, // clip reload time
		9.0f, // bullet knockback
		2.0f // self knockback
		),
	CWeapon(
		"Heavy assault rifle",
		"/upg rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		RIFLE_ASSAULTRIFLE, // require
		120, // cost
		22, // damage
		0, // extra1
		true, // autofire
		0.08f, // bullet spread
		1, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0.45f, // bullet life
		120, // bullet reload time
		1200, // clip reload time
		13.0f, // bullet knockback
		3.0f // self knockback
		),
	CWeapon(
		"Lightning rifle",
		"/upg lightning",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		-1, // require
		80, // cost
		8, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Storm rifle",
		"/upg storm",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		RIFLE_LIGHTNINGRIFLE, // require
		120, // cost
		7, // damage
		ELECTRIC, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Laser rifle",
		"/buy laser",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		-1, // require
		90, // cost
		18, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		1, // shot spread
		7, // clip size
		7, // powerup size
		70, // max ammo
		0, // bullet life
		225, // bullet reload time
		1200, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Electro rifle",
		"/upg electro",
		WEAPON_RIFLE,
		PROJTYPE_ELECTRO,
		SOUND_RIFLE_FIRE,
		RIFLE_LASERRIFLE, // require
		130, // cost
		8, // damage
		0, // extra1
		true, // autofire
		0, // bullet spread
		2, // shot spread
		20, // clip size
		20, // powerup size
		100, // max ammo
		400, // bullet life
		175, // bullet reload time
		1400, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Doom ray",
		"/upg doomray",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		RIFLE_LASERRIFLE, // require
		130, // cost
		15, // damage
		DOOMROCKETS, // extra1
		false, // autofire
		0, // bullet spread
		1, // shot spread
		4, // clip size
		4, // powerup size
		20, // max ammo
		0, // bullet life
		400, // bullet reload time
		1200, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		),
	CWeapon(
		"Smoke grenade",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		0, // cost
		0, // damage
		SMOKE, // extra1
		false, // autofire
		0, // bullet spread
		0, // shot spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		1.2f, // bullet life
		0, // bullet reload time
		0, // clip reload time
		0.0f, // bullet knockback
		0.0f // self knockback
		)
};



#endif