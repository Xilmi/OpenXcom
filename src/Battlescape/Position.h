#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../Engine/Yaml.h"
#include <SDL_types.h>
#include <cmath>
#include <ostream>

namespace OpenXcom
{

/**
 * Easy handling of X-Y-Z coordinates.
 */
class Position
{
public:
	constexpr static int TileXY = 16;
	constexpr static int TileZ = 24;

	Sint16 x, y, z;

	/// Null position constructor.
	constexpr Position() noexcept : x(0), y(0), z(0) {};
	/// X Y Z position constructor.
	constexpr Position(int x_, int y_, int z_) noexcept : x(x_), y(y_), z(z_) {};
	/// Copy constructor.
	constexpr Position(const Position& pos) = default;

	/// Assignment
	constexpr Position& operator=(const Position& pos) = default;

	constexpr Position operator+(const Position& pos) const { return Position(x + pos.x, y + pos.y, z + pos.z); }
	constexpr Position& operator+=(const Position& pos) { x+=pos.x; y+=pos.y; z+=pos.z; return *this; }

	constexpr Position operator-(const Position& pos) const { return Position(x - pos.x, y - pos.y, z - pos.z); }
	constexpr Position& operator-=(const Position& pos) { x-=pos.x; y-=pos.y; z-=pos.z; return *this; }

	constexpr Position operator*(const Position& pos) const { return Position(x * pos.x, y * pos.y, z * pos.z); }
	constexpr Position& operator*=(const Position& pos) { x*=pos.x; y*=pos.y; z*=pos.z; return *this; }
	constexpr Position operator*(const int v) const { return Position(x * v, y * v, z * v); }
	constexpr Position& operator*=(const int v) { x*=v; y*=v; z*=v; return *this; }

	constexpr Position operator/(const Position& pos) const { return Position(x / pos.x, y / pos.y, z / pos.z); }
	constexpr Position& operator/=(const Position& pos) { x/=pos.x; y/=pos.y; z/=pos.z; return *this; }

	constexpr Position operator/(const int v) const { return Position(x / v, y / v, z / v); }

	/// == operator
	constexpr bool operator== (const Position& pos) const
	{
		return x == pos.x && y == pos.y && z == pos.z;
	}
	/// != operator
	constexpr bool operator!= (const Position& pos) const
	{
		return x != pos.x || y != pos.y || z != pos.z;
	}

	/// Convert tile position to voxel position.
	constexpr Position toVoxel() const
	{
		return Position(x * TileXY, y * TileXY, z * TileZ);
	}
	/// Convert voxel position to tile position.
	constexpr Position toTile() const
	{
		return Position(x / TileXY, y / TileXY, z / TileZ);
	}
	/// Clip voxel values to position relative to containing tile.
	constexpr Position clipVoxel() const
	{
		return Position(x % TileXY, y % TileXY, z % TileZ);
	}

	/// Calculates the distance in 3d.
	static float distance(Position pos1, Position pos2)
	{
		return std::sqrt(distanceSq(pos1, pos2));
	}

	/**
	 * Calculates the distance squared between 2 points in 3d. No sqrt(), not floating point math, and sometimes it's all you need.
	 * @param pos1 Position of first square.
	 * @param pos2 Position of second square.
	 * @return Distance.
	 */
	constexpr static int distanceSq(Position pos1, Position pos2)
	{
		int x = pos1.x - pos2.x;
		int y = pos1.y - pos2.y;
		int z = pos1.z - pos2.z;
		return x*x + y*y + z*z;
	}

	/**
	 * Calculates the distance between 2 points in 2d. Rounded up to first INT.
	 * @param pos1 Position of first square.
	 * @param pos2 Position of second square.
	 * @return Distance.
	 */
	static int distance2d(Position pos1, Position pos2)
	{
		return (int)std::ceil(std::sqrt(distance2dSq(pos1, pos2)));
	}

	/**
	 * Calculates the distance squared between 2 points in 2d. No sqrt(), not floating point math, and sometimes it's all you need.
	 * @param pos1 Position of first square.
	 * @param pos2 Position of second square.
	 * @return Distance.
	 */
	constexpr static int distance2dSq(Position pos1, Position pos2)
	{
		int x = pos1.x - pos2.x;
		int y = pos1.y - pos2.y;
		return x*x + y*y;
	}
};

struct PositionComparator
{
	bool operator()(const Position& pos1, const Position& pos2) const
	{
		if (pos1.x < pos2.x)
			return true;
		if (pos1.x > pos2.x)
			return false;

		if (pos1.y < pos2.y)
			return true;
		if (pos1.y > pos2.y)
			return false;

		return pos1.z < pos2.z;
	}
};

inline std::ostream& operator<<(std::ostream& out, const Position& pos)
{
	out << "(" << pos.x << "," << pos.y << ","<< pos.z << ")";
	return out;
}

typedef Position Vector3i;


/**
 * Helper class storing last two position from Projectile
 */
struct LastPositions
{
	LastPositions(Position l) : last{l}, before{l} {};
	LastPositions(Position l, Position b) : last{l}, before{b} {};

	Position last;
	Position before;
};

// helper overloads for (de)serialization
bool read(ryml::ConstNodeRef const& n, Position* val);
void write(ryml::NodeRef* n, Position const& val);

}
