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

// Filename    : OB_HOMIN.H
// Description : header file for BulletHoming

#ifndef __OB_HOMIN_H
#define __OB_HOMIN_H

#include <OBULLET.h>

struct BulletHomingCrc;
struct BulletHomingGF;

#pragma pack(1)
class BulletHoming : public Bullet
{
public:
	char	max_step;
	char	target_type;
	short	target_recno;
	short	speed;
	short origin2_x, origin2_y;

public:
	BulletHoming();
	~BulletHoming();
	void	init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType); // virtual function from obullet.h
	void	deinit();

	void	process_move();

	int	write_derived_file(File *filePtr);
	int	read_derived_file(File *filePtr);
	void	write_derived_record(BulletHomingGF *r);
	void	read_derived_record(BulletHomingGF *r);

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(BulletHomingCrc *c);
};
#pragma pack()

#endif
