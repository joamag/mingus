/*
 Mingus Virtual Machine
 Copyright (C) 2008 Jo�o Magalh�es

 This file is part of Mingus Virtual Machine.

 Mingus Virtual Machine is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Mingus Virtual Machine is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Mingus Virtual Machine. If not, see <http://www.gnu.org/licenses/>.

 __author__    = Jo�o Magalh�es <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Jo�o Magalh�es
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

int main(int argc, const char *argv[]) {
	FILE *file = fopen("C:/calc.mia", "rb");
	fclose(file);
	return 0;
}
