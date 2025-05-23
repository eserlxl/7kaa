/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename   : OAI_DEFE.CPP
//Description: AI on defense

#include <ALL.h>
#include <OTALKRES.h>
#include <ONATION.h>
#include <OFIRM.h>

//----- Begin of function Nation::ai_defend -----//
//
// <int> attackerUnitRecno - unit recno of the attacker.
//
int Nation::ai_defend(int attackerUnitRecno)
{
	//--- don't call for defense too frequently, only call once 7 days (since this function will be called every time our king/firm/town is attacked, so this filtering is necessary ---//

	if( info.game_date < ai_last_defend_action_date+7 )
		return 0;

	ai_last_defend_action_date = info.game_date;

	//---------- analyse the situation first -----------//

	Unit* attackerUnit = unit_array[attackerUnitRecno];

	err_when( attackerUnit->nation_recno == nation_recno );

	int attackerXLoc = attackerUnit->next_x_loc();
	int attackerYLoc = attackerUnit->next_y_loc();
	Location *loc = world.get_loc(attackerXLoc, attackerYLoc);

	if( !attackerUnit->is_visible() )
	{
		//***BUGHERE
		// Find the enemy building as the target since the attacker disappeared into one.
		// Search around last known location. This search should not be necessary, but
		// the attacker cannot be found in game information easily right now.

		#define DIMENSION 3
		#define TEST_LIMIT DIMENSION*DIMENSION
		int xShift, yShift, checkXLoc, checkYLoc, firmRecno;
		int haveFirm = 0;

		for( int i=2; i<=TEST_LIMIT; i++)
		{
			misc.cal_move_around_a_point(i, DIMENSION, DIMENSION, xShift, yShift);

			checkXLoc = attackerXLoc+xShift;
			checkYLoc = attackerYLoc+yShift;
			if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
				continue;

			loc = world.get_loc(checkXLoc, checkYLoc);
			firmRecno = loc->firm_recno();
			if( firm_array.is_deleted(firmRecno) )
				continue;

			Firm* firmPtr = firm_array[firmRecno];
			if( firmPtr->nation_recno == attackerUnit->nation_recno )
			{
				haveFirm = 1;
				attackerXLoc = firmPtr->loc_x1;
				attackerYLoc = firmPtr->loc_y1;
				loc = world.get_loc(attackerXLoc, attackerYLoc);
				break;
			}
		}

		err_when( !haveFirm );
		if( !haveFirm )
			return 0; //***BUGHERE
	}

	if( loc->has_unit(UNIT_LAND) && loc->has_unit(UNIT_AIR) &&
		loc->unit_recno(UNIT_LAND) == attackerUnit->sprite_recno )
	{
		Unit* airUnit = unit_array[loc->unit_recno(UNIT_AIR)];
		err_when( airUnit->nation_recno != nation_recno &&
			airUnit->nation_recno != attackerUnit->nation_recno );
		// ai can't distinguish between air and land unit past this point
		// see Nation::get_target_nation_recno
		if( airUnit->nation_recno != nation_recno &&
				airUnit->nation_recno != attackerUnit->nation_recno )
			return 0; //***BUGHERE
	}

	int hasWar;

	int enemyCombatLevel = mobile_defense_combat_level( attackerXLoc, attackerYLoc,
		attackerUnit->nation_recno, 0, hasWar );		// 0-don't return immediately even if there is war around this town

	//-- the value returned is enemy strength minus your own strength, so if it's positive, it means that your enemy is stronger than you, otherwise you're stronger than your enemy --//

	int attackCombatLevel = ai_attack_target(attackerXLoc, attackerYLoc, enemyCombatLevel, 1);		// 1-defense mode

	//------ request military aid from allies ----//

	if( attackCombatLevel < enemyCombatLevel && attackerUnit->nation_recno )
	{
		ai_request_military_aid();
	}

	return 1;
}
//----- End of function Nation::ai_defend -----//


//----- Begin of function Nation::ai_request_military_aid -----//
//
// Request allied nations to provide immediate military aid.
//
int Nation::ai_request_military_aid()
{
	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( get_relation(i)->status != NATION_ALLIANCE )
			continue;

		if( should_diplomacy_retry(TALK_REQUEST_MILITARY_AID, i) )
		{
			talk_res.ai_send_talk_msg(i, nation_recno, TALK_REQUEST_MILITARY_AID);
			return 1;
		}
	}

	return 0;
}
//----- End of function Nation::ai_request_military_aid -----//
