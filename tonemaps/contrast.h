/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 *   Lux Renderer is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Lux Renderer is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This project is based on PBRT ; see http://www.pbrt.org               *
 *   Lux Renderer website : http://www.luxrender.net                       *
 ***************************************************************************/

// contrast.cpp*
#include "tonemap.h"
#include "paramset.h"

namespace lux
{

// ContrastOp Declarations
class ContrastOp : public ToneMap {
public:
	ContrastOp(float day) { displayAdaptationY = day; }
	virtual ~ContrastOp() { }
	virtual void Map(vector<XYZColor> &xyz, u_int xRes, u_int yRes, float maxDisplayY) const;
	static ToneMap *CreateToneMap(const ParamSet &ps);
private:
	float displayAdaptationY;
};

}//namespace lux
