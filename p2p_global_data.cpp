/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2011  University of Florida
Copyright (C) 2011  Kyungyong Lee (klee@acis.ufl.edu), University of Florida

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//#include <stdio.h>
#include <iostream>
#include "p2p_action.h"
#include "son_util.h"
#include "p2p_message.h"

using namespace Starsky;
using namespace std;

GlobalClass::GlobalClass(){
}

P2PRdResponse::P2PRdResponse(){
  Response = new map<int,int>();
}

P2PRdResponse::~P2PRdResponse(){
  delete Response;
}


P2PStat::P2PStat(){
  TotalNodes = 0;
  MaxDepth = 0;
}
