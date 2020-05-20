/*
Simple implementation of BWT encoder with a bitmap compression algorithm
Copyright (C) 2020  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef BWT_H
#define	BWT_H

int encode(int size_in, unsigned char* in_data, int size_buffer_out, unsigned char* buffer_out);
int decode(int size_in, unsigned char* in_data, int size_buffer_out, unsigned char* buffer_out);

#endif	/* BWT_H */

