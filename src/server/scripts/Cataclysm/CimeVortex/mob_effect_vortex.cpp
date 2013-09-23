/*
* Copyright (C) 2011-2012 Project TBEmu <http://www.trueblood-servers.com/>
* By Asardial
*/

#include "ScriptPCH.h"
#include "the_vortex_pinnacle.h"

#define SPELL_TWISTER_TRIGGER 55883 // efffect joueur tornade 

/********************
** Grand Vent Hurlant
*********************/
#define SPELL_HOWLING_GALE_AURA 85084 // Aura
#define SPELL_HOWLING_GALE 85158 // Vent

class npc_howling_gale : public CreatureScript
{
    public:
        npc_howling_gale() : CreatureScript("npc_howling_gale") { }

        struct npc_howling_galeAI : public ScriptedAI
        {
		npc_howling_galeAI(Creature * pCreature) : ScriptedAI(pCreature) {}

		bool uiTwistingWindFixe;

		void Reset()
		{
			uiTwistingWindFixe = false;
			me->AddAura(SPELL_HOWLING_GALE_AURA, me);
		}

		void UpdateAI(const uint32 uiDiff)
		{
                if (!UpdateVictim())
                    return;
	
			if(!uiTwistingWindFixe) // Si proche Joueur
			{
				DoCast(SPELL_HOWLING_GALE);
				uiTwistingWindFixe = true;
			}
		}
	};

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_howling_galeAI(creature);
    }
};

/***************
** Vortex Speed
***************/
#define SPELL_VORTEX_SPEED_AURA 59551 // Aura Golem Vortex
#define SPELL_VORTEX_SPEED 83141 // Sprint

class npc_sprint_vortex : public CreatureScript
{
    public:
        npc_sprint_vortex() : CreatureScript("npc_sprint_vortex") { }

        struct npc_sprint_vortexAI : public ScriptedAI
        {
		npc_sprint_vortexAI(Creature * pCreature) : ScriptedAI(pCreature) {}

		bool uiSprintVortexFixe;
			
		void Reset()
		{
			uiSprintVortexFixe = false;
			me->AddAura(SPELL_VORTEX_SPEED_AURA, me);
		}

		void UpdateAI(const uint32 uiDiff)
		{
                if (!UpdateVictim())
                    return;
				
			if(!uiSprintVortexFixe) // Si proche Joueur
			{
				DoCast(SPELL_VORTEX_SPEED);
				uiSprintVortexFixe = true;
			}
		}
	};

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sprint_vortexAI(creature);
    }
};

void AddSC_mob_effect_cimevortex()
{
    new npc_howling_gale();
    new npc_sprint_vortex();
}