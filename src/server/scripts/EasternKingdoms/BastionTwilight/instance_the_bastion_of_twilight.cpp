/*
* Copyright (C) 2011-2013 OpenEmu <http://www.openemulator.com/>
* 
*/

#include "ScriptPCH.h"
#include "bastion_of_twilight.h"

#define MAX_ENCOUNTER 5

class instance_the_bastion_of_twilight : public InstanceMapScript
{
public:
    instance_the_bastion_of_twilight() : InstanceMapScript("instance_the_bastion_of_twilight", 671) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_the_bastion_of_twilight_InstanceMapScript(map);
    }

    struct instance_the_bastion_of_twilight_InstanceMapScript: public InstanceScript
    {
		instance_the_bastion_of_twilight_InstanceMapScript(Map* map) : InstanceScript(map) {}

		void Initialize()
        {
             for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                 uiEncounter[i] = NOT_STARTED;

             uiWyrmbreaker = 0;
             uiValiona = 0;
             uiTheralion = 0;
             uiFeludius = 0;
			 uiIgnacious = 0;
			 uiArion = 0;
			 uiTerrastra = 0;
			 uiMonstrosity = 0;
			 uiChogall = 0;
			 uiSlateDrake = 0;
			 uiStormRider = 0;
			 uiNetherScion = 0;
			 uiProtoBehemoth = 0 ;
			 uiTimeWarden = 0;
             uiCyclonWinds = 0;
             if (instance->IsHeroic())
                 uiSinestra = 0;
             uiAscendantCouncilPhase = 1;
        }

		bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (uiEncounter[i] == IN_PROGRESS)
                    return true;

             return false;
        }


        void SetData64(uint32 id, uint64 data)
        {
            switch (id)
            {
                case DATA_HB_VALIONA_THERALION:
                    uiValionaTheralionHealth = data ;
                    break;
            }
        }

		void SetData(uint32 type, uint32 data)
		{
			switch (type)
			{
				case DATA_WYRMBREAKER_EVENT:
					uiEncounter[0] = data;
					break;
				case DATA_VALIONA_THERALION_EVENT:
					uiEncounter[1] = data;
					break;
				case DATA_COUNCIL_EVENT:
					uiEncounter[2] = data;
					break;
				case DATA_CHOGALL_EVENT:
					uiEncounter[3] = data;
					break;
                case DATA_SINESTRA_EVENT:
                    uiEncounter[4] = data;
                    break;
			}

			if (data == DONE)
				SaveToDB();
		}

		std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "P S " << uiEncounter[0] << " " << uiEncounter[1]  << " " << uiEncounter[2]  << " " << uiEncounter[3] << " " << uiEncounter[4] << " " << uiRandomDragons[0] << " " << uiRandomDragons[1] << " " << uiRandomDragons[2] << " " << uiHalfusNormalTimer;

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

		void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 data0, data1, data2, data3, data4;
			uint8 data5, data6, data7;
			uint64 data8;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4 >> data5 >> data6 >> data7 >> data8;

            if (dataHead1 == 'P' && dataHead2 == 'S')
            {
                uiEncounter[0] = data0;
                uiEncounter[1] = data1;
                uiEncounter[2] = data2;
                uiEncounter[3] = data3;
				uiEncounter[4] = data4;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (uiEncounter[i] == IN_PROGRESS)
                        uiEncounter[i] = NOT_STARTED;

				if (!instance->IsHeroic())
				{
					srand((unsigned)time(0));
					if (!data5 == NULL)
					{
						uiRandomDragons[0] = data5;
					} else uiRandomDragons[0] = rand() % 3 +1;
					if (!data6 == NULL)
					{
						uiRandomDragons[1] = data6;
					}
                    else
                    {
						switch (uiRandomDragons[0])
						{
							case RANDOM_DRAGON_STORM_RIDER:
								uiRandomDragons[1] = rand() % 1 + 1;
								break;
							case RANDOM_DRAGON_NETHER_SCION:
								uiRandomDragons[1] = rand() % 1 + 2;
								if (uiRandomDragons[1] == RANDOM_DRAGON_NETHER_SCION)
								{
									uiRandomNumber = rand() % 10 + 1;
									if (uiRandomNumber <= 5)
									{
										uiRandomDragons[1] = RANDOM_DRAGON_STORM_RIDER;
									}
                                    else uiRandomDragons[1] = RANDOM_DRAGON_SLATE_DRAKE;
								}
								break;
							case RANDOM_DRAGON_SLATE_DRAKE:
								uiRandomDragons[1] = rand() % 1 + 2;
								break;
						}
					}
					if (!data7 == NULL)
					{
						uiRandomDragons[2] = data7;
					}
                    else
                    {
						uiRandomDragons[2] = rand() % 1 + 1;
					}
                    if (!data8 == 0)
                    {
				        uiHalfusNormalTimer = data8;
                    }
                    else uiHalfusNormalTimer = 604800000;
				}
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void ChangeState(uint64 guid,bool active,bool finalphase)
        {
            Creature * creature = instance->GetCreature(guid);
            uint16 talkid;
            uint16 wayid;
            if (finalphase)
            {
                switch (creature->GetEntry())
                {
                case BOSS_FELUDIUS:
                    talkid = SAY_PHASE3_FELUDIUS;
                    wayid = WALK_FELUDIUS;
                    break;
                }
                creature->AI()->Talk(talkid);
                creature->UpdateWaypointID(wayid);
            }
            if (active)
            {
                creature->RemoveAura(creature->GetAura(8611,guid));
            }
            else
            {
                creature->AddAura(8611,creature);
            }
        }

        void ShiftPhase()
        {
            uiAscendantCouncilPhase++;
            if (uiAscendantCouncilPhase == 2)
            {
                ChangeState(GetData64(DATA_FELUDIUS),false,false);
                ChangeState(GetData64(DATA_IGNACIOUS),false,false);
                ChangeState(GetData64(DATA_ARION),true,false);
                ChangeState(GetData64(DATA_TERRASTRA),true,false);
            }
            else if (uiAscendantCouncilPhase == 3)
            {
            }
        }

		void Update(uint32 diff)
		{
			if (uiHalfusNormalTimer <= diff)
			{
				uiRandomDragons[0] = rand() % 3 +1;
				switch (uiRandomDragons[0])
				{
					case RANDOM_DRAGON_STORM_RIDER:
						uiRandomDragons[1] = rand() % 1 + 1;
						break;
					case RANDOM_DRAGON_NETHER_SCION:
						uiRandomDragons[1] = rand() % 1 + 2;
						if (uiRandomDragons[1] == RANDOM_DRAGON_NETHER_SCION)
						{
							uiRandomNumber = rand() % 10 + 1;
							if (uiRandomNumber <= 5)
							{
								uiRandomDragons[1] = RANDOM_DRAGON_STORM_RIDER;
							} else uiRandomDragons[1] = RANDOM_DRAGON_SLATE_DRAKE;
						}
						break;
					case RANDOM_DRAGON_SLATE_DRAKE:
						uiRandomDragons[1] = rand() % 1 + 2;
						break;
				}
				uiRandomDragons[2] = rand() % 1 + 1;
			} else uiHalfusNormalTimer -= diff;
		}

    private:
        uint64 uiWyrmbreaker;
		uint64 uiValiona;
		uint64 uiTheralion;
		uint64 uiFeludius;
		uint64 uiArion;
		uint64 uiIgnacious;
		uint64 uiTerrastra;
		uint64 uiMonstrosity;
		uint64 uiChogall;
        uint64 uiSinestra;
		uint64 uiSlateDrake;
		uint64 uiStormRider;
		uint64 uiNetherScion;
		uint64 uiProtoBehemoth;
		uint64 uiTimeWarden;
        uint64 uiValionaTheralionHealth;
        uint64 uiCyclonWinds;
		uint32 uiRandomDragons[3];
		uint32 uiRandomNumber;
		uint32 uiHalfusNormalTimer;
		uint32 uiTeamInInstance;
		uint32 uiEncounter[MAX_ENCOUNTER];
        uint8  uiAscendantCouncilPhase;
	};
};

void AddSC_instance_the_bastion_of_twilight()
{
    new instance_the_bastion_of_twilight();
}