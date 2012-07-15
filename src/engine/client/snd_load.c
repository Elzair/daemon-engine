/*
===========================================================================
This file is part of Daemon.

OpenTrem is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenTrem is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*
 * snd_load.c
 * Loads a sound module, and redirects calls to it
 */

#include "client.h"
#include "snd_api.h"
#include "snd_codec.h"
#include "snd_local.h"
#include "snd_public.h"

static soundInterface_t si;

cvar_t                *s_volume;
cvar_t                *s_testsound;
cvar_t                *s_khz;
cvar_t                *s_show;
cvar_t                *s_mixahead;
cvar_t                *s_mixPreStep;
cvar_t                *s_musicVolume;
cvar_t                *s_separation;
cvar_t                *s_doppler;

void S_Shutdown( void )
{
	Com_Printf( "Calling Shutdown\n" );

	if( si.Shutdown )
	{
		si.Shutdown( );
	}

	codec_shutdown();

	Cmd_RemoveCommand( "play" );
	Cmd_RemoveCommand( "music" );
	Cmd_RemoveCommand( "stopsound" );
	Cmd_RemoveCommand( "soundlist" );
	Cmd_RemoveCommand( "soundinfo" );
}

void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx )
{
	Com_Printf( "Calling StartSound\n" );
	if( si.StartSound )
	{
		si.StartSound( origin, entnum, entchannel, sfx );
	}
}

void S_StartSoundEx( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx )
{
	Com_Printf( "Calling StartSoundEx\n" );
	if( si.StartSoundEx )
	{
		si.StartSoundEx( origin, entnum, entchannel, sfx );
	}
}

void S_StartLocalSound( sfxHandle_t sfx, int channelNum )
{
	Com_Printf( "Calling StartLocalSound\n" );
	if( si.StartLocalSound )
	{
		si.StartLocalSound( sfx, channelNum );
	}
}

void S_StartBackgroundTrack( const char *intro, const char *loop )
{
	Com_Printf( "Calling StartBackgroundTrack\n" );
	if( si.StartBackgroundTrack )
	{
		si.StartBackgroundTrack( intro, loop );
	}
}

void S_StopBackgroundTrack( void )
{
	Com_Printf( "Calling StopBackgroundTrack\n" );
	if( si.StopBackgroundTrack )
	{
		si.StopBackgroundTrack( );
	}
}

void S_RawSamples( int stream, int samples, int rate, int width, int channels,
                   const byte *data, float volume, int entityNum )
{
	Com_Printf( "Calling RawSamples\n" );
	if( si.RawSamples )
	{
		si.RawSamples( stream, samples, rate, width, channels, data, volume, entityNum );
	}
}

void S_StopAllSounds( void )
{
	Com_Printf( "Calling StopAllSounds\n" );
	if( si.StopAllSounds )
	{
		si.StopAllSounds( );
	}
}

void S_ClearLoopingSounds( qboolean killall )
{
	//Com_Printf( "Calling ClearLoopingSounds\n" );
	if( si.ClearLoopingSounds )
	{
		si.ClearLoopingSounds( killall );
	}
}

void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx )
{
	//Com_Printf( "Calling AddLoopingSounds\n" );
	if( si.AddLoopingSound )
	{
		si.AddLoopingSound( entityNum,  origin, velocity, sfx );
	}
}

void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx )
{
	Com_Printf( "Calling AddRealLoopingSound\n" );
	if( si.AddRealLoopingSound )
	{
		si.AddRealLoopingSound( entityNum, origin, velocity, sfx );
	}
}

void S_StopLoopingSound( int entityNum )
{
	Com_Printf( "Calling StopLoopingSound\n" );
	if( si.StopLoopingSound )
	{
		si.StopLoopingSound( entityNum );
	}
}

void S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[ 3 ], int inwater )
{
	//Com_Printf( "Calling Respatialize\n" );
	if( si.Respatialize )
	{
		si.Respatialize( entityNum, origin, axis, inwater );
	}
}

void S_UpdateEntityPosition( int entityNum, const vec3_t origin )
{
	//Com_Printf( "Calling UpdateEntityPosition\n" );
	if( si.UpdateEntityPosition )
	{
		si.UpdateEntityPosition( entityNum, origin );
	}
}

void S_Update( void )
{
	//Com_Printf( "Calling Update\n" );
	if( si.Update )
	{
		si.Update( );
	}
}

void S_DisableSounds( void )
{
	Com_Printf( "Calling DisableSounds\n" );
	if( si.DisableSounds )
	{
		si.DisableSounds( );
	}
}

void S_BeginRegistration( void )
{
	Com_Printf( "Calling BeginRegistration\n" );
	if( si.BeginRegistration )
	{
		si.BeginRegistration( );
	}
}

sfxHandle_t S_RegisterSound( const char *sample, qboolean compressed )
{
	Com_Printf( "Calling RegisterSound\n" );
	if ( si.RegisterSound )
	{
		return si.RegisterSound( sample, compressed );
	}
	else
	{
		return 0;
	}
}

void S_ClearSoundBuffer( void )
{
	Com_Printf( "Calling ClearSoundBuffer\n" );
	if( si.ClearSoundBuffer )
	{
		si.ClearSoundBuffer( );
	}
}

int S_SoundDuration( sfxHandle_t handle )
{
	Com_Printf( "Calling SoundDuration\n" );
	if( si.SoundDuration )
	{
		return si.SoundDuration( handle );
	}
	else
	{
		return 0;
	}
}

int S_GetVoiceAmplitude( int entnum )
{
	Com_Printf( "Calling GetVoiceAmplitude\n" );
	if ( si.GetVoiceAmplitude )
	{
		return si.GetVoiceAmplitude( entnum );
	}
	else
	{
		return 0;
	}
}
int S_GetSoundLength( sfxHandle_t sfxHandle )
{
	Com_Printf( "Calling GetSoundLength\n" );
	if ( si.GetSoundLength )
	{
		return si.GetSoundLength( sfxHandle );
	}
	else
	{
		return 0;
	}
}

#ifdef USE_VOIP
void S_StartCapture( void )
{
	Com_Printf( "Calling StartCapture\n" );
	if ( si.StartCapture )
	{
		si.StartCapture( );
	}
}

int S_AvailableCaptureSamples( void )
{
	Com_Printf( "Calling AvailableCaptureSamples\n" );
	if ( si.AvailableCaptureSamples )
	{
		return si.AvailableCaptureSamples( );
	}
	else
	{
		return 0;
	}
}

void S_Capture( int samples, byte *data )
{
	Com_Printf( "Calling Capture\n" );
	if ( si.Capture )
	{
		si.Capture( samples, data );
	}
}

void S_StopCapture( void )
{
	Com_Printf( "Calling Stop Capture\n" );
	if ( si.StopCapture )
	{
		si.StopCapture( );
	}
}

void S_MasterGain( float gain )
{
	Com_Printf( "Calling MasterGain\n" );
	if ( si.MasterGain )
	{
		si.MasterGain( gain );
	}
}

#endif

int S_GetCurrentSoundTime( void )
{
	Com_Printf( "Calling GetCurrentSoundTime\n" );
	if( si.GetCurrentSoundTime )
	{
		return si.GetCurrentSoundTime( );
	}
	else
	{
		return 0;
	}
}



void S_Init( void )
{
	cvar_t *cv;
	qboolean  started = qfalse;

	Com_Printf("%s", _( "------ Initializing Sound -----\n" ));

	cv = Cvar_Get( "s_initsound", "1", 0 );
	if ( !cv->integer )
	{
		Com_Printf( "Sound Disabled\n" );
		return;
	}
	else
	{
		codec_init();

		Com_Printf( "S_Init()_debug: cvars starting" );

		s_volume = Cvar_Get( "s_volume", "0.8", CVAR_ARCHIVE );
		s_musicVolume = Cvar_Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
		s_separation = Cvar_Get( "s_separation", "0.5", CVAR_ARCHIVE );
		s_doppler = Cvar_Get( "s_doppler", "1", CVAR_ARCHIVE );
		s_khz = Cvar_Get( "s_khz", "22", CVAR_ARCHIVE );
		s_mixahead = Cvar_Get( "s_mixahead", "0.2", CVAR_ARCHIVE );

		s_mixPreStep = Cvar_Get( "s_mixPreStep", "0.05", CVAR_ARCHIVE );
		s_show = Cvar_Get( "s_show", "0", CVAR_CHEAT );
		s_testsound = Cvar_Get( "s_testsound", "0", CVAR_CHEAT );

		Com_Printf( "S_Init()_debug: cvars done" );

		cv = Cvar_Get( "s_usemodule", "1", CVAR_ARCHIVE );
		if( cv->integer )
		{
			started = S_AL_Init( &si );
		}

		if( !started )
		{
			Com_Printf( "Using builtin sound backend\n" );
			SOrig_Init( &si );
		}
		else
		{
			Com_Printf( "Using OpenAL sound backend\n" );
		}

		Cmd_AddCommand( "play", si.Play_f );
		Cmd_AddCommand( "music", si.Music_f );
		Cmd_AddCommand( "s_list", si.SoundList_f );
		Cmd_AddCommand( "s_info", si.SoundInfo_f );
		Cmd_AddCommand( "s_stop", si.StopAllSounds );
	}
	Com_Printf( "------------------------------------\n" );
}
