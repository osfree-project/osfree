/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

/* this spec follows the prototype of 'static_foreign_marshaler' */

SOM_IMPORTEXPORT_som SOMEXTERN void SOMLINK 
	somIdMarshal(
		void *latent_param,
		char *foreign_addr,
		SOMD_marshaling_direction_t direction,
		SOMD_marshaling_op_t function,
		CosStream_StreamIO SOMSTAR stream,
		Environment *ev);

