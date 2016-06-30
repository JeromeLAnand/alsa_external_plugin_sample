/*
 * Sample Dummy algo for ALSA plugin
 *
 * @file dummy_algo.h
 *
 * @brief dummy algo copy source audio data into Destination duffer,
 * without doing any processing.
 *  
 * Copyright (c) 2016 by Jerome Anand <jeromelanand@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* 
	Dummy algo init
	Do Algo Initilization here 
*/
int dummy_algo_init();

/* 
 	Dummy algo close
	Do Algo De-Initilization here 
	  
*/
void dummy_algo_close();

/*
	Dummy algo transfer
	Do Audio Data processing here.
*/ 
int dummy_algo_transfer( void *dst, void *src, int size);
