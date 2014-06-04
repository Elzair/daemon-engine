/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Daemon GPL Source Code (Daemon Source Code).

Daemon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Daemon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Daemon Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the Daemon
Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville,
Maryland 20850 USA.

===========================================================================
*/

// sv_bot.c

#include "server.h"

/*
==================
SV_BotAllocateClient
==================
*/
int SV_BotAllocateClient( int clientNum )
{
	int      i;
	int      firstSlot = std::max( 1, sv_privateClients->integer );
	client_t *cl;

	// Arnout: added possibility to request a clientnum
	if ( clientNum > 0 )
	{
		if ( clientNum >= sv_maxclients->integer )
		{
			return -1;
		}

		cl = &svs.clients[ clientNum ];

		if ( cl->state != CS_FREE )
		{
			return -1;
		}
		else
		{
			i = clientNum;
		}
	}
	else
	{
		// find a free client slot which was occupied by a bot (doesn't matter which)
		for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
		{
			if ( cl->state == CS_FREE && cl->gentity && ( cl->gentity->r.svFlags & SVF_BOT ) )
			{
				break;
			}
		}

		// find any free client slot
		if ( i == sv_maxclients->integer )
		{
			for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
			{
				// Wolfenstein, never use the first slot, otherwise if a bot connects before the first client on a listen server, game won't start
				// Also, never use reserved slots since that messes up the reported player count
				if ( i < firstSlot )
				{
					continue;
				}

				// done.
				if ( cl->state == CS_FREE )
				{
					break;
				}
			}
		}
	}

	if ( i == sv_maxclients->integer )
	{
		return -1;
	}

	cl->gentity = SV_GentityNum( i );
	cl->gentity->s.number = i;
	cl->state = CS_ACTIVE;
	cl->lastPacketTime = svs.time;
	cl->netchan.remoteAddress.type = NA_BOT;
	cl->rate = 16384;

	return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum )
{
	client_t *cl;

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer )
	{
		Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
	}

	cl = &svs.clients[ clientNum ];
	cl->state = CS_FREE;
	cl->name[ 0 ] = 0;
/*
	if ( cl->gentity )
	{
		cl->gentity->r.svFlags &= ~SVF_BOT;
	}
*/
}

//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
int SV_BotGetConsoleMessage( int client, char *buf, int size )
{
	client_t *cl;
	int      index;

	cl = &svs.clients[ client ];
	cl->lastPacketTime = svs.time;

	if ( cl->reliableAcknowledge == cl->reliableSequence )
	{
		return qfalse;
	}

	cl->reliableAcknowledge++;
	index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );

	if ( !cl->reliableCommands[ index ][ 0 ] )
	{
		return qfalse;
	}

	//Q_strncpyz( buf, cl->reliableCommands[index], size );
	return qtrue;
}

