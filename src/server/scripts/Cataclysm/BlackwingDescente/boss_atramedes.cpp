/*
* Copyright (C) 2011-2012 ProjectStudioMirage <http://www.studio-mirage.fr/>
* Copyright (C) 2011-2012 https://github.com/Asardial
*/

#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "blackwing_descent.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

enum Events
{
    // Groundphase
    EVENT_SONAR_PULSE = 1,
    EVENT_MODULATION,
    EVENT_SONIC_BREATH,
    EVENT_SEARING_FLAMES,
    EVENT_ROARING_FLAME_BREATH,

    // Flightphase
    EVENT_ROARING_FLAME_BREATH_F,
    EVENT_SONAR_BOMB_F,
    EVENT_SONIC_FIREBALL_F,

    EVENT_GROUND,
    EVENT_FLIGHT,
    EVENT_LIFTOFF,
    EVENT_LAND,
};

enum Spells
{
    ATRAMEDES_ENTRY                 = 41442,

    // Bossfight
    // Flightphase
    SPELL_ROARING_FLAME_BREATH      = 78353,
    SPELL_SONAR_BOMB                = 92553,
    SPELL_SONIC_FIREBALL            = 78115,

    // Groundphase
    SPELL_MODULATION                = 77612,
    SPELL_SEARING_FLAME             = 77840,
    SPELL_SONIC_BREATH              = 78075,
    SPELL_TRACKING                  = 78092,

    // Effect
    SPELL_SONAR_PULSE               = 92418,
    SPELL_SONAR_PULSE_SUMMON        = 77673,
    SPELL_SONAR_PULSE_AURA          = 77674,
    SPELL_SONAR_PULSE_DAMAGE        = 92418,
    SPELL_ROARING_ZONE              = 76247,
    SPELL_ATRAMEDES_STUN            = 77611,
};

enum Phases 
{
    PHASE_NULL = 0,
    EVENT_GROUND,
    PHASE_FLIGHT,
};

enum MovementPoints
{
    POINT_FLIGHT            = 1,
    POINT_LAND              = 2,
};

Position const AtramedesFlyPos  = {147.491f, -225.301f, 76.4534f, 3.07607f};
Position const AtramedesLandPos = {147.491f, -225.301f, 75.4534f, 3.07607f};

enum SpiritSpells
{
    SPELL_AVATAR             = 80645, // Tank
    SPELL_BURDEN_OF_CROWN    = 80718, // Tank
    SPELL_CHAIN_LIGHTNING    = 91891, // Caster
    SPELL_STORMBOLT          = 91890,
    SPELL_THUNDERCLAP        = 91889,
    SPELL_WHIRLIND           = 80652,
};

#define ACTION_EVENT_START          1
#define ACTION_PROGRESS_PHASE       2

uint16 const adds[8] =
{
    43119, 43128, 43130, 43122, 43127, 43125, 43129, 43126
};

const Position addsLocations[8] = 
{
    {129.971f, -180.754f, 74.9073f, 4.67909f},
    {147.843f, -265.233f, 74.9073f, 2.06174f},
    {112.498f, -264.905f, 74.9073f, 1.15069f},
    {130.363f, -269.234f, 74.9073f, 1.49626f},
    {161.121f, -253.638f, 74.9073f, 2.47016f},
    {112.887f, -184.105f, 74.9073f, 5.21118f},
    {145.621f, -183.672f, 74.9073f, 4.22944f},
    {160.038f, -196.484f, 74.9073f, 3.92511f}
};

/***********
** Atramedes
************/
class boss_atramedes : public CreatureScript
{
public:
    boss_atramedes() : CreatureScript("boss_atramedes") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_atramedesAI(creature);
    }

    struct boss_atramedesAI : public BossAI
    {
        boss_atramedesAI(Creature* creature) : BossAI(creature, DATA_ATRAMEDES)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        Phases phase;

        uint32 FlyCount;

        void Reset()
        {
            _Reset();
            DespawnMinions();

            me->SetFlying(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            instance->SetData(DATA_ATRAMEDES, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            if (Creature *pCloche = me->GetCreature(*me, instance->GetData64(DATA_CLOCHE)))
                pCloche->AI()->DoAction(ACTION_PROGRESS_PHASE);
            initEvents();
            instance->SetData(DATA_ATRAMEDES, IN_PROGRESS);
        }

        void initEvents()
        {
            phase = PHASE_GROUND;
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            events.ScheduleEvent(EVENT_SONAR_PULSE, 8000, PHASE_GROUND);
            events.ScheduleEvent(EVENT_MODULATION, urand(15000,22000), PHASE_GROUND);
            events.ScheduleEvent(EVENT_SONIC_BREATH, 11000, PHASE_GROUND);
            events.ScheduleEvent(EVENT_SEARING_FLAMES, urand(15000,22000), PHASE_GROUND);
            events.ScheduleEvent(EVENT_ROARING_FLAME_BREATH, urand(12000,22000), PHASE_GROUND);
            events.ScheduleEvent(EVENT_FLIGHT, 80000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (phase == PHASE_GROUND)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_GROUND:
                            phase = PHASE_GROUND;
                            events.SetPhase(PHASE_GROUND);
                            me->SetFlying(false);
                            me->SetLevitate(false);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            DoResetThreat();
                            events.ScheduleEvent(EVENT_LAND, 5500, 0, PHASE_GROUND);
                            return;
                        case EVENT_SONAR_PULSE:
                            DoCast(SPELL_SONAR_PULSE_SUMMON);
                            events.ScheduleEvent(EVENT_SONAR_PULSE, 8000, PHASE_GROUND);
                            break;
                        case EVENT_MODULATION:
                            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_MODULATION);
                            events.ScheduleEvent(EVENT_MODULATION, urand(15000,22000), PHASE_GROUND);
                            break;
                        case EVENT_SONIC_BREATH:
                            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_SONIC_BREATH);
                            events.ScheduleEvent(EVENT_SONIC_BREATH, 11000, PHASE_GROUND);
                            break;
                        case EVENT_SEARING_FLAMES:
                            DoCast(me->getVictim(), SPELL_SEARING_FLAME);
                            events.ScheduleEvent(EVENT_SONAR_PULSE, urand(15000,22000), PHASE_GROUND);
                            break;
                        case EVENT_ROARING_FLAME_BREATH:
                            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_ROARING_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_ROARING_FLAME_BREATH, urand(12000,22000), PHASE_GROUND);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
            else
            {
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_FLIGHT:
                            phase = PHASE_FLIGHT;
                            events.SetPhase(PHASE_FLIGHT);
                            me->SetFlying(true);
                            me->SetLevitate(true);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->RemoveAllAuras();
                            DoResetThreat();
                            float x, y, z;
                            me->GetMotionMaster()->MovePoint(0, x, y, z+10);
                            events.ScheduleEvent(EVENT_SONIC_FIREBALL_F, 2000, PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_SONAR_BOMB_F, urand(12000,16000), PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_ROARING_FLAME_BREATH_F, urand(8000,16000), PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_GROUND, 45000, PHASE_GROUND);
                            ++FlyCount;
                            return;
                        case EVENT_SONIC_FIREBALL_F:
                            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_SONIC_FIREBALL);
                            events.ScheduleEvent(EVENT_SONIC_FIREBALL_F, 2000, PHASE_FLIGHT);
                            break;
                        case EVENT_ROARING_FLAME_BREATH_F:
                            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_ROARING_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_ROARING_FLAME_BREATH_F, urand(8000,16000), PHASE_FLIGHT);
                            break;
                        case EVENT_SONAR_BOMB_F:
                            DoCastToAllHostilePlayers(SPELL_SONAR_BOMB);
                            events.ScheduleEvent(EVENT_SONAR_BOMB_F, urand(12000,16000), PHASE_FLIGHT);
                            break;
                    }
                }
            }
        }

    private:
        inline void DespawnMinions()
        {
            DespawnCreatures(NPC_SONAR_PULSE);
        }

        void DespawnCreatures(uint32 entry)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, 200.0f);

            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            instance->SetData(DATA_ATRAMEDES, DONE);
        }
    };
};

/*******************
** Event Ancien Bell
********************/

/***************
** Trashs Spirit
****************/
class npc_spirit : public CreatureScript
{
public:
    npc_spirit() : CreatureScript("npc_spirit") { }

    struct npc_spiritAI : public ScriptedAI
    {
        npc_spiritAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 AvatarTimer;
        uint32 CrownTimer;
        uint32 LightningTimer;
        uint32 StormboltTimer;
        uint32 ThunderTimer;
        uint32 WhirlindTimer;

        void Reset()
        {
            AvatarTimer = 10000;
            CrownTimer = 15000;
            LightningTimer = urand(18000,22000);
            StormboltTimer = 22000;
            ThunderTimer = 15000;
            WhirlindTimer = 20000;
        }

        void EnterCombat(Unit* /*who*/)
        {}

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (AvatarTimer <= diff)
            {
                DoCast(me, SPELL_AVATAR);
                AvatarTimer = 10000;
            }
            else AvatarTimer -= diff;

            if (CrownTimer <= diff)
            {
                DoCast(me->getVictim(), SPELL_BURDEN_OF_CROWN);
                CrownTimer = 15000;
            }
            else CrownTimer -= diff;

            if (LightningTimer <= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true), SPELL_CHAIN_LIGHTNING);
                LightningTimer = urand(18000,22000);
            }
            else LightningTimer -= diff;

            if (StormboltTimer <= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 1, 100,true), SPELL_STORMBOLT);
                StormboltTimer = 22000;
            }
            else StormboltTimer -= diff;

            if (ThunderTimer <= diff)
            {
                DoCastAOE(SPELL_THUNDERCLAP);
                ThunderTimer = 15000;
            }
            else ThunderTimer -= diff;

            if (WhirlindTimer <= diff)
            {
                DoCastAOE(SPELL_WHIRLIND);
                WhirlindTimer = 20000;
            }
            else WhirlindTimer -= diff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*who*/)
        {}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spiritAI(creature);
    }
};

/******************
** Trigger + Spells
*******************/
/*************
** Sonar Pulse
**************/
class npc_sonar_pulse : public CreatureScript
{
public:
    npc_sonar_pulse() : CreatureScript("npc_sonar_pulse") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sonar_pulseAI (creature);
    }

    struct npc_sonar_pulseAI : public ScriptedAI
    {
        npc_sonar_pulseAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetReactState(REACT_PASSIVE);
            me->DespawnOrUnsummon(30000);
        }
		
		uint32 timerChangeTarget;
		
		void Reset()
		{
            me->AddAura(SPELL_SONAR_PULSE_AURA, me);
            me->AddAura(SPELL_SONAR_PULSE_DAMAGE, me);
            timerChangeTarget = 13000;
		}

        void UpdateAI(const uint32 diff)
        {
            if (timerChangeTarget <= diff)
            {
                if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                me->GetMotionMaster()->MoveFollow(target, 3, 2);
                timerChangeTarget = 13000;
            } else timerChangeTarget -= diff;
        }
    };
};

/*****************
** Tracking Flames
******************/
class npc_tracking_flames : public CreatureScript
{
public:
    npc_tracking_flames() : CreatureScript("npc_tracking_flames") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tracking_flamesAI (creature);
    }

    struct npc_tracking_flamesAI : public ScriptedAI
    {
        npc_tracking_flamesAI(Creature* creature) : ScriptedAI(creature)
        {
            me->DespawnOrUnsummon(30000);
        }
    };
};

/***************
** Roaring Flame
****************/
class npc_roaring_flame : public CreatureScript
{
public:
    npc_roaring_flame() : CreatureScript("npc_roaring_flame") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_roaring_flameAI (creature);
    }

    struct npc_roaring_flameAI : public Scripted_NoMovementAI
    {
        npc_roaring_flameAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            DoCast(me, SPELL_ROARING_ZONE);

            me->ForcedDespawn(20000);
        }
    };
};

/**********************
** Roaring Flame Target
***********************/
class npc_roaring_flame_target : public CreatureScript
{
public:
    npc_roaring_flame_target() : CreatureScript("npc_roaring_flame_target") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_roaring_flame_targetAI (creature);
    }

    struct npc_roaring_flame_targetAI : public ScriptedAI
    {
        npc_roaring_flame_targetAI(Creature* creature) : ScriptedAI(creature)
        {
            me->DespawnOrUnsummon(30000);
        }
    };
};

/*****************
** Atramedes Gongs
*****************/
class atramedes_gong : public CreatureScript
{
public:
    atramedes_gong() : CreatureScript("atramedes_gong") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if(Creature* un = pCreature->FindNearestCreature(ATRAMEDES_ENTRY,90.0f))
        {
            un->StopMoving();
            un->CastStop();
            pCreature->CastSpell(un, SPELL_ATRAMEDES_STUN, true);
            pCreature->DisappearAndDie();
        }
        return true;
    }
};

/******************************
** Spell Atramedes Sonic Breath
*******************************/
class spell_atramedes_sonic_breath : public SpellScriptLoader
{
public:
	spell_atramedes_sonic_breath() : SpellScriptLoader("spell_atramedes_sonic_breath") { }

	class spell_atramedes_sonic_breathSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_sonic_breathSpellScript);
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_sonic_breathSpellScript();
	}
};

/****************************
** Spell Atramedes Sonar Bomb
*****************************/
class spell_atramedes_sonar_bomb : public SpellScriptLoader
{
public:
	spell_atramedes_sonar_bomb() : SpellScriptLoader("spell_atramedes_sonar_bomb") { }

	class spell_atramedes_sonar_bombSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_sonar_bombSpellScript);
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_sonar_bombSpellScript();
	}
};

/************************************
** Spell Atramedes Roaring Flame Aura
*************************************/
class spell_atramedes_roaring_flame_aura : public SpellScriptLoader
{
public:
	spell_atramedes_roaring_flame_aura() : SpellScriptLoader("spell_atramedes_roaring_flame_aura") { }

	class spell_atramedes_roaring_flame_auraSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_roaring_flame_auraSpellScript);
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_roaring_flame_auraSpellScript();
	}
};

/*******************************
** Spell Atramedes Roaring Flame
********************************/
class RoaringFlameTargetSelector
{
    public:
        RoaringFlameTargetSelector() { }

        bool operator()(Unit* unit)
        {
            return unit->GetTypeId() != TYPEID_PLAYER;
        }
};


class spell_atramedes_roaring_flame : public SpellScriptLoader
{
public:
	spell_atramedes_roaring_flame() : SpellScriptLoader("spell_atramedes_roaring_flame") { }

	class spell_atramedes_roaring_flameSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_roaring_flameSpellScript);

            void FilterTargets(std::list<Unit*>& unitList)
            {
                unitList.remove_if (RoaringFlameTargetSelector());
                uint8 maxSize = uint8(GetCaster()->GetMap()->GetSpawnMode() & 1);
                if (unitList.size() > maxSize)
                    Trinity::RandomResizeList(unitList, maxSize);
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_SONIC_BREATH, false);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_SEARING_FLAME, true);
            }

            void Register()
            {
                OnUnitTargetSelect += SpellUnitTargetFn(spell_atramedes_roaring_flame_init_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_AREA_ENEMY_SRC);
                OnEffectHitTarget += SpellEffectFn(spell_atramedes_roaring_flame_init_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_roaring_flameSpellScript();
	}
};

/*****************************
** Spell Atramedes Sonar Pulse
******************************/
class spell_atramedes_sonar_pulse : public SpellScriptLoader
{
public:
	spell_atramedes_sonar_pulse() : SpellScriptLoader("spell_atramedes_sonar_pulse") { }

	class spell_atramedes_sonar_pulseSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_sonar_pulseSpellScript);
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_sonar_pulseSpellScript();
	}
};

/****************************
** Spell Atramedes Modulation
*****************************/
class spell_atramedes_modulation : public SpellScriptLoader
{
public:
	spell_atramedes_modulation() : SpellScriptLoader("spell_atramedes_modulation") { }

	class spell_atramedes_modulationSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_atramedes_modulationSpellScript);
	};

	SpellScript * GetSpellScript() const
	{
		return new spell_atramedes_modulationSpellScript();
	}
};

void AddSC_boss_atramedes()
{
    new boss_atramedes();
    new npc_spirit();
    new npc_sonar_pulse();
    new npc_tracking_flames();
    new npc_roaring_flame();
    new npc_roaring_flame_target();
    new atramedes_gong();
    new spell_atramedes_sonic_breath();
    new spell_atramedes_sonar_bomb();
    new spell_atramedes_roaring_flame_aura();
    new spell_atramedes_roaring_flame();
    new spell_atramedes_sonar_pulse();
    new spell_atramedes_modulation();
}