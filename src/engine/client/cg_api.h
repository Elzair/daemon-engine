/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 2011  Dusan Jocic <dusanjocic@msn.com>

Daemon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Daemon is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef CGAPI_H
#define CGAPI_H


#include "../qcommon/q_shared.h"
#include "../qcommon/vm_traps.h"
#include "../renderer/tr_types.h"

#define CGAME_IMPORT_API_VERSION 3

#define CMD_BACKUP               64
#define CMD_MASK                 ( CMD_BACKUP - 1 )
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP

#define MAX_ENTITIES_IN_SNAPSHOT 512

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct
{
	int           snapFlags; // SNAPFLAG_RATE_DELAYED, etc
	int           ping;

	int           serverTime; // server time the message is valid for (in msec)

	byte          areamask[ MAX_MAP_AREA_BYTES ]; // portalarea visibility bits

	playerState_t ps; // complete information about the current player at this time

	int           numEntities; // all of the entities that need to be presented
	entityState_t entities[ MAX_ENTITIES_IN_SNAPSHOT ]; // at the time of this snapshot

	int           numServerCommands; // text based server commands to execute when this
	int           serverCommandSequence; // snapshot becomes current
} snapshot_t;

typedef enum {
	ROCKET_STRING,
	ROCKET_FLOAT,
	ROCKET_INT,
	ROCKET_COLOR
} rocketVarType_t;

typedef enum {
	ROCKETMENU_MAIN,
	ROCKETMENU_CONNECTING,
	ROCKETMENU_LOADING,
	ROCKETMENU_DOWNLOADING,
	ROCKETMENU_INGAME_MENU,
	ROCKETMENU_TEAMSELECT,
	ROCKETMENU_HUMANSPAWN,
	ROCKETMENU_ALIENSPAWN,
	ROCKETMENU_ALIENBUILD,
	ROCKETMENU_HUMANBUILD,
	ROCKETMENU_ARMOURYBUY,
	ROCKETMENU_ALIENEVOLVE,
	ROCKETMENU_CHAT,
	ROCKETMENU_BEACONS,
	ROCKETMENU_ERROR,
	ROCKETMENU_NUM_TYPES
} rocketMenuType_t;

typedef enum {
	RP_QUAKE = 1 << 0,
	RP_EMOTICONS = 1 << 1,
} rocketInnerRMLParseTypes_t;

typedef struct
{
	connstate_t connState;
	int         connectPacketCount;
	int         clientNum;
	char        servername[ MAX_STRING_CHARS ];
	char        updateInfoString[ MAX_STRING_CHARS ];
	char        messageString[ MAX_STRING_CHARS ];
} cgClientState_t;

typedef enum
{
	SORT_HOST,
	SORT_MAP,
	SORT_CLIENTS,
	SORT_PING,
	SORT_GAME,
	SORT_FILTERS,
	SORT_FAVOURITES
} serverSortField_t;

typedef enum cgameImport_s
{
  CG_PRINT = FIRST_VM_SYSCALL,
  CG_ERROR,
  CG_LOG,
  CG_MILLISECONDS,
  CG_CVAR_REGISTER,
  CG_CVAR_UPDATE,
  CG_CVAR_SET,
  CG_CVAR_VARIABLESTRINGBUFFER,
  CG_CVAR_LATCHEDVARIABLESTRINGBUFFER,
  CG_CVAR_VARIABLEINTEGERVALUE,
  CG_CVAR_VARIABLEVALUE,
  CG_CVAR_ADDFLAGS,
  CG_ARGC,
  CG_ARGV,
  CG_ESCAPED_ARGS,
  CG_LITERAL_ARGS,
  CG_GETDEMOSTATE,
  CG_GETDEMOPOS,
  CG_FS_FOPENFILE,
  CG_FS_READ,
  CG_FS_WRITE,
  CG_FS_FCLOSEFILE,
  CG_FS_GETFILELIST,
  CG_FS_GETFILELISTRECURSIVE,
  CG_FS_DELETEFILE,
  CG_FS_LOADPAK,
  CG_FS_LOADMAPMETADATA,
  CG_SENDCONSOLECOMMAND,
  CG_ADDCOMMAND,
  CG_REMOVECOMMAND,
  CG_SENDCLIENTCOMMAND,
  CG_UPDATESCREEN,
  CG_CM_LOADMAP,
  CG_CM_NUMINLINEMODELS,
  CG_CM_INLINEMODEL,
  CG_CM_TEMPBOXMODEL,
  CG_CM_TEMPCAPSULEMODEL,
  CG_CM_POINTCONTENTS,
  CG_CM_TRANSFORMEDPOINTCONTENTS,
  CG_CM_BOXTRACE,
  CG_CM_TRANSFORMEDBOXTRACE,
  CG_CM_CAPSULETRACE,
  CG_CM_TRANSFORMEDCAPSULETRACE,
  CG_CM_BISPHERETRACE,
  CG_CM_TRANSFORMEDBISPHERETRACE,
  CG_CM_MARKFRAGMENTS,
  CG_R_PROJECTDECAL,
  CG_R_CLEARDECALS,
  CG_S_STARTSOUND,
  CG_S_STARTLOCALSOUND,
  CG_S_CLEARLOOPINGSOUNDS,
  CG_S_CLEARSOUNDS,
  CG_S_ADDLOOPINGSOUND,
  CG_S_ADDREALLOOPINGSOUND,
  CG_S_STOPLOOPINGSOUND,
  CG_S_STOPSTREAMINGSOUND,
  CG_S_UPDATEENTITYPOSITION,
  CG_S_RESPATIALIZE,
  CG_S_REGISTERSOUND,
  CG_S_STARTBACKGROUNDTRACK,
  CG_S_FADESTREAMINGSOUND,
  CG_S_STARTSTREAMINGSOUND,
  CG_R_LOADWORLDMAP,
  CG_R_REGISTERMODEL,
  CG_R_REGISTERSKIN,
  CG_R_GETSKINMODEL,
  CG_R_GETMODELSHADER,
  CG_R_REGISTERSHADER,
  CG_R_REGISTERFONT,
  CG_UNUSED_IMPORT_1,
  CG_UNUSED_IMPORT_2,
  CG_R_CLEARSCENE,
  CG_R_ADDREFENTITYTOSCENE,
  CG_R_ADDREFLIGHTSTOSCENE,
  CG_R_ADDPOLYTOSCENE,
  CG_R_ADDPOLYSTOSCENE,
  CG_R_ADDPOLYBUFFERTOSCENE,
  CG_R_ADDLIGHTTOSCENE,
  CG_R_ADDADDITIVELIGHTTOSCENE,
  CG_FS_SEEK,
  CG_R_ADDCORONATOSCENE,
  CG_R_SETFOG,
  CG_R_SETGLOBALFOG,
  CG_R_RENDERSCENE,
  CG_R_SAVEVIEWPARMS,
  CG_R_RESTOREVIEWPARMS,
  CG_R_SETCOLOR,
  CG_R_SETCLIPREGION,
  CG_R_DRAWSTRETCHPIC,
  CG_R_DRAWROTATEDPIC,
  CG_R_DRAWSTRETCHPIC_GRADIENT,
  CG_R_DRAW2DPOLYS,
  CG_R_MODELBOUNDS,
  CG_R_LERPTAG,
  CG_GETGLCONFIG,
  CG_GETGAMESTATE,
  CG_GETCLIENTSTATE,
  CG_GETCURRENTSNAPSHOTNUMBER,
  CG_GETSNAPSHOT,
  CG_GETSERVERCOMMAND,
  CG_GETCURRENTCMDNUMBER,
  CG_GETUSERCMD,
  CG_SETUSERCMDVALUE,
  CG_SETCLIENTLERPORIGIN,
  CG_MEMORY_REMAINING,
  CG_KEY_ISDOWN,
  CG_KEY_GETCATCHER,
  CG_KEY_SETCATCHER,
  CG_KEY_GETKEY,
  CG_KEY_GETOVERSTRIKEMODE,
  CG_KEY_SETOVERSTRIKEMODE,
  CG_PC_ADD_GLOBAL_DEFINE,
  CG_PC_LOAD_SOURCE,
  CG_PC_FREE_SOURCE,
  CG_PC_READ_TOKEN,
  CG_PC_SOURCE_FILE_AND_LINE,
  CG_PC_UNREAD_TOKEN,
  CG_S_STOPBACKGROUNDTRACK,
  CG_REAL_TIME,
  CG_SNAPVECTOR,
  CG_CIN_PLAYCINEMATIC,
  CG_CIN_STOPCINEMATIC,
  CG_CIN_RUNCINEMATIC,
  CG_CIN_DRAWCINEMATIC,
  CG_CIN_SETEXTENTS,
  CG_R_REMAP_SHADER,
  CG_GET_ENTITY_TOKEN,
  CG_INGAME_POPUP,
  CG_INGAME_CLOSEPOPUP,
  CG_KEY_GETBINDINGBUF,
  CG_KEY_SETBINDING,
  CG_PARSE_ADD_GLOBAL_DEFINE,
  CG_PARSE_LOAD_SOURCE,
  CG_PARSE_FREE_SOURCE,
  CG_PARSE_READ_TOKEN,
  CG_PARSE_SOURCE_FILE_AND_LINE,
  CG_KEY_KEYNUMTOSTRINGBUF,
  CG_S_FADEALLSOUNDS,
  CG_R_INPVS,
  CG_UNUSED,
  CG_R_LOADDYNAMICSHADER,
  CG_R_RENDERTOTEXTURE,
  CG_R_FINISH,
  CG_GETDEMONAME,
  CG_R_LIGHTFORPOINT,
  CG_R_REGISTERANIMATION,
  CG_R_CHECKSKELETON,
  CG_R_BUILDSKELETON,
  CG_R_BLENDSKELETON,
  CG_R_BONEINDEX,
  CG_R_ANIMNUMFRAMES,
  CG_R_ANIMFRAMERATE,
  CG_COMPLETE_CALLBACK,
  CG_REGISTER_BUTTON_COMMANDS,
  CG_GETCLIPBOARDDATA,
  CG_QUOTESTRING,
  CG_GETTEXT,
  CG_R_GLYPH,
  CG_R_GLYPHCHAR,
  CG_R_UREGISTERFONT,
  CG_PGETTEXT,
  CG_R_INPVVS,
  CG_NOTIFY_TEAMCHANGE,
  CG_GETTEXT_PLURAL,
  CG_REGISTERVISTEST,
  CG_ADDVISTESTTOSCENE,
  CG_CHECKVISIBILITY,
  CG_UNREGISTERVISTEST,
  CG_SETCOLORGRADING,
  CG_CM_DISTANCETOMODEL,
  CG_R_SCISSOR_ENABLE,
  CG_R_SCISSOR_SET,
  CG_LAN_LOADCACHEDSERVERS,
  CG_LAN_SAVECACHEDSERVERS,
  CG_LAN_ADDSERVER,
  CG_LAN_REMOVESERVER,
  CG_LAN_GETPINGQUEUECOUNT,
  CG_LAN_CLEARPING,
  CG_LAN_GETPING,
  CG_LAN_GETPINGINFO,
  CG_LAN_GETSERVERCOUNT,
  CG_LAN_GETSERVERADDRESSSTRING,
  CG_LAN_GETSERVERINFO,
  CG_LAN_GETSERVERPING,
  CG_LAN_MARKSERVERVISIBLE,
  CG_LAN_SERVERISVISIBLE,
  CG_LAN_UPDATEVISIBLEPINGS,
  CG_LAN_RESETPINGS,
  CG_LAN_SERVERSTATUS,
  CG_LAN_SERVERISINFAVORITELIST,
  CG_GETNEWS,
  CG_LAN_COMPARESERVERS,
  CG_R_GETSHADERNAMEFROMHANDLE,
  CG_PREPAREKEYUP,
  CG_R_SETALTSHADERTOKENS,
  CG_S_UPDATEENTITYVELOCITY,
  CG_S_SETREVERB,
  CG_S_BEGINREGISTRATION,
  CG_S_ENDREGISTRATION,
  CG_ROCKET_INIT,
  CG_ROCKET_SHUTDOWN,
  CG_ROCKET_LOADDOCUMENT,
  CG_ROCKET_LOADCURSOR,
  CG_ROCKET_DOCUMENTACTION,
  CG_ROCKET_GETEVENT,
  CG_ROCKET_DELELTEEVENT,
  CG_ROCKET_REGISTERDATASOURCE,
  CG_ROCKET_DSADDROW,
  CG_ROCKET_DSCHANGEROW,
  CG_ROCKET_DSREMOVEROW,
  CG_ROCKET_DSCLEARTABLE,
  CG_ROCKET_SETINNERRML,
  CG_ROCKET_GETATTRIBUTE,
  CG_ROCKET_SETATTRIBUTE,
  CG_ROCKET_GETPROPERTY,
  CG_ROCKET_SETPROPERYBYID,
  CG_ROCKET_GETEVENTPARAMETERS,
  CG_ROCKET_REGISTERDATAFORMATTER,
  CG_ROCKET_DATAFORMATTERRAWDATA,
  CG_ROCKET_DATAFORMATTERFORMATTEDDATA,
  CG_ROCKET_REGISTERELEMENT,
  CG_ROCKET_SETELEMENTDIMENSIONS,
  CG_ROCKET_GETELEMENTTAG,
  CG_ROCKET_GETELEMENTABSOLUTEOFFSET,
  CG_ROCKET_QUAKETORML,
  CG_ROCKET_SETCLASS,
  CG_ROCKET_INITHUDS,
  CG_ROCKET_LOADUNIT,
  CG_ROCKET_ADDUNITTOHUD,
  CG_ROCKET_SHOWHUD,
  CG_ROCKET_CLEARHUD,
  CG_ROCKET_ADDTEXT,
  CG_ROCKET_CLEARTEXT,
  CG_ROCKET_REGISTERPROPERTY,
  CG_ROCKET_SHOWSCOREBOARD,
  CG_ROCKET_SETDATASELECTINDEX,
  CG_ROCKET_LOADFONT
} cgameImport_t;

typedef enum
{
  CG_INIT,
//  void CG_Init( int serverMessageNum, int serverCommandSequence )
  // called when the level loads or when the renderer is restarted
  // all media should be registered at this time
  // cgame will display loading status by calling SCR_Update, which
  // will call CG_DrawInformation during the loading process
  // reliableCommandSequence will be 0 on fresh loads, but higher for
  // demos or vid_restarts

  CG_SHUTDOWN,
//  void (*CG_Shutdown)( void );
  // oportunity to flush and close any open files

  CG_CONSOLE_COMMAND,
//  qboolean (*CG_ConsoleCommand)( void );
  // a console command has been issued locally that is not recognized by the
  // main game system.
  // use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the
  // command is not known to the game

  CG_DRAW_ACTIVE_FRAME,
//  void (*CG_DrawActiveFrame)( int serverTime, qboolean demoPlayback );
  // Generates and draws a game scene and status information at the given time.
  // If demoPlayback is set, local movement prediction will not be enabled

  CG_CONSOLE_TEXT,
//	void (*CG_ConsoleText)( void );
  //  pass text that has been printed to the console to cgame
  //  use Cmd_Argc() / Cmd_Argv() to read it

  CG_CROSSHAIR_PLAYER,
//  int (*CG_CrosshairPlayer)( void );

  CG_UNUSED_EXPORT_1,
//  UNUSED

  CG_KEY_EVENT,
//  void    (*CG_KeyEvent)( int key, qboolean down );

  CG_MOUSE_EVENT,
//  void    (*CG_MouseEvent)( int dx, int dy );

  CG_VOIP_STRING,
// char *(*CG_VoIPString)( void );
// returns a string of comma-delimited clientnums based on cl_voipSendTarget

  CG_COMPLETE_COMMAND,
// char (*CG_CompleteCommand)( int argNum );
// will callback on all availible completions
// use Cmd_Argc() / Cmd_Argv() to read the command

  CG_INIT_CVARS,
// registers cvars only then shuts down; call instead of CG_INIT for this purpose

  CG_INIT_ROCKET,
// Inits libRocket in the game.

  CG_ROCKET_FRAME,
// Rocket runs through a frame, including event processing

  CG_ROCKET_FORMATDATA,
// Rocket wants some data formatted

  CG_ROCKET_RENDERELEMENT,
// Rocket wants an element renderered

  CG_ROCKET_PROGRESSBARVALUE
// Rocket wants to query the value of a progress bar
} cgameExport_t;

void            trap_Print( const char *string );
void NORETURN   trap_Error( const char *string );
int             trap_Milliseconds( void );
void            trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void            trap_Cvar_Update( vmCvar_t *vmCvar );
void            trap_Cvar_Set( const char *var_name, const char *value );
void            trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void            trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );
int             trap_Cvar_VariableIntegerValue( const char *var_name );
float           trap_Cvar_VariableValue( const char *var_name );
void            trap_Cvar_AddFlags( const char *var_name, int flags );
int             trap_Argc( void );
void            trap_Argv( int n, char *buffer, int bufferLength );
void            trap_EscapedArgs( char *buffer, int bufferLength );
void            trap_LiteralArgs( char *buffer, int bufferLength );
int             trap_GetDemoState( void );
int             trap_GetDemoPos( void );
int             trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void            trap_FS_Read( void *buffer, int len, fileHandle_t f );
void            trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void            trap_FS_FCloseFile( fileHandle_t f );
int             trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int             trap_FS_GetFileListRecursive( const char *path, const char *extension, char *listbuf, int bufsize );
int             trap_FS_Delete( const char *filename );
qboolean        trap_FS_LoadPak( const char *pak, const char *prefix );
void            trap_FS_LoadAllMapMetadata( void );
void            trap_SendConsoleCommand( const char *text );
void            trap_AddCommand( const char *cmdName );
void            trap_RemoveCommand( const char *cmdName );
void            trap_SendClientCommand( const char *s );
void            trap_UpdateScreen( void );
void            trap_CM_LoadMap( const char *mapname );
int             trap_CM_NumInlineModels( void );
clipHandle_t    trap_CM_InlineModel( int index );
clipHandle_t    trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
clipHandle_t    trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs );
int             trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int             trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void            trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, int skipmask );
void            trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, int skipmask, const vec3_t origin, const vec3_t angles );
void            trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, int skipmask );
void            trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, int skipmask, const vec3_t origin, const vec3_t angles );
void            trap_CM_BiSphereTrace( trace_t *results, const vec3_t start, const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask, int skipmask );
void            trap_CM_TransformedBiSphereTrace( trace_t *results, const vec3_t start, const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask, int skipmask, const vec3_t origin );
int             trap_CM_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );
void            trap_R_ProjectDecal( qhandle_t hShader, int numPoints, vec3_t *points, vec4_t projection, vec4_t color, int lifeTime, int fadeTime );
void            trap_R_ClearDecals( void );
void            trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void            trap_S_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags );
void            trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );

//void trap_S_ClearLoopingSounds(void);
void            trap_S_ClearLoopingSounds( qboolean killall );
void            trap_S_ClearSounds( qboolean killmusic );

//void trap_S_AddLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume, int soundTime);
void            trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );

//void trap_S_AddRealLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range, int volume, int soundTime);
void            trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void            trap_S_StopLoopingSound( int entityNum );
void            trap_S_StopStreamingSound( int entityNum );
int             trap_S_SoundDuration( sfxHandle_t handle );
void            trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );
int             trap_S_GetVoiceAmplitude( int entityNum );
int             trap_S_GetSoundLength( sfxHandle_t sfx );
int             trap_S_GetCurrentSoundTime( void );

void            trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[ 3 ], int inwater );
sfxHandle_t     trap_S_RegisterSound( const char *sample, qboolean compressed );
void            trap_S_StartBackgroundTrack( const char *intro, const char *loop );
void            trap_S_FadeBackgroundTrack( float targetvol, int time, int num );
int             trap_S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation );
void            trap_R_LoadWorldMap( const char *mapname );
qhandle_t       trap_R_RegisterModel( const char *name );
qhandle_t       trap_R_RegisterSkin( const char *name );
qboolean        trap_R_GetSkinModel( qhandle_t skinid, const char *type, char *name );
qhandle_t       trap_R_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap );
qhandle_t       trap_R_RegisterShader( const char *name, RegisterShaderFlags_t flags );
void            trap_R_RegisterFont( const char *fontName, const char *fallbackName, int pointSize, fontMetrics_t * );
void            trap_R_Glyph( fontHandle_t, const char *str, glyphInfo_t *glyph );
void            trap_R_GlyphChar( fontHandle_t, int ch, glyphInfo_t *glyph );
void            trap_R_UnregisterFont( fontHandle_t );

void            trap_R_ClearScene( void );
void            trap_R_AddRefEntityToScene( const refEntity_t *re );
void            trap_R_AddRefLightToScene( const refLight_t *light );
void            trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts );
void            trap_R_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
void            trap_R_AddPolyBufferToScene( polyBuffer_t *pPolyBuffer );
void            trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void            trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void            trap_GS_FS_Seek( fileHandle_t f, long offset, fsOrigin_t origin );
void            trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible );
void            trap_R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density );
void            trap_R_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque );
void            trap_R_RenderScene( const refdef_t *fd );
void            trap_R_SaveViewParms( void );
void            trap_R_RestoreViewParms( void );
void            trap_R_SetColor( const float *rgba );
void            trap_R_SetClipRegion( const float *region );
void            trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void            trap_R_DrawRotatedPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );
void            trap_R_DrawStretchPicGradient( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType );
void            trap_R_Add2dPolys( polyVert_t *verts, int numverts, qhandle_t hShader );
void            trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int             trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void            trap_GetGlconfig( glconfig_t *glconfig );
void            trap_GetGameState( gameState_t *gamestate );
void            trap_GetClientState( cgClientState_t *cstate );
void            trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );
qboolean        trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );
qboolean        trap_GetServerCommand( int serverCommandNumber );
int             trap_GetCurrentCmdNumber( void );
qboolean        trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );
void            trap_SetUserCmdValue( int stateValue, int flags, float sensitivityScale, int mpIdentClient );
void            trap_SetClientLerpOrigin( float x, float y, float z );
int             trap_MemoryRemaining( void );
qboolean        trap_Key_IsDown( int keynum );
int             trap_Key_GetCatcher( void );
void            trap_Key_SetCatcher( int catcher );
int             trap_Key_GetKey( const char *binding );
qboolean        trap_Key_GetOverstrikeMode( void );
void            trap_Key_SetOverstrikeMode( qboolean state );
int             trap_PC_AddGlobalDefine( const char *define );
int             trap_PC_LoadSource( const char *filename );
int             trap_PC_FreeSource( int handle );
int             trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int             trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
int             trap_PC_UnReadToken( int handle );
void            trap_S_StopBackgroundTrack( void );
int             trap_RealTime( qtime_t *qtime );
void            trap_SnapVector( float *v );
int             trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits );
e_status        trap_CIN_StopCinematic( int handle );
e_status        trap_CIN_RunCinematic( int handle );
void            trap_CIN_DrawCinematic( int handle );
void            trap_CIN_SetExtents( int handle, int x, int y, int w, int h );
void            trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean        trap_GetEntityToken( char *buffer, int bufferSize );
void            trap_UI_Popup( int arg0 );
void            trap_UI_ClosePopup( const char *arg0 );
void            trap_Key_GetBindingBuf( int keynum, int team, char *buf, int buflen );
void            trap_Key_SetBinding( int keynum, int team, const char *binding );
int             trap_Parse_AddGlobalDefine( const char *define );
int             trap_Parse_LoadSource( const char *filename );
int             trap_Parse_FreeSource( int handle );
int             trap_Parse_ReadToken( int handle, pc_token_t *pc_token );
int             trap_Parse_SourceFileAndLine( int handle, char *filename, int *line );
void            trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void            trap_CG_TranslateString( const char *string, char *buf );
void            trap_S_FadeAllSound( float targetvol, int time, qboolean stopsounds );
qboolean        trap_R_inPVS( const vec3_t p1, const vec3_t p2 );
qboolean        trap_R_inPVVS( const vec3_t p1, const vec3_t p2 );
qboolean        trap_R_LoadDynamicShader( const char *shadername, const char *shadertext );
void            trap_R_RenderToTexture( int textureid, int x, int y, int w, int h );
void            trap_R_Finish( void );
void            trap_GetDemoName( char *buffer, int size );
void            trap_S_StartSoundVControl( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int volume );
int             trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );

qhandle_t       trap_R_RegisterAnimation( const char *name );
int             trap_R_CheckSkeleton( refSkeleton_t *skel, qhandle_t hModel, qhandle_t hAnim );
int             trap_R_BuildSkeleton( refSkeleton_t *skel, qhandle_t anim, int startFrame, int endFrame, float frac, qboolean clearOrigin );
int             trap_R_BlendSkeleton( refSkeleton_t *skel, const refSkeleton_t *blend, float frac );
int             trap_R_BoneIndex( qhandle_t hModel, const char *boneName );
int             trap_R_AnimNumFrames( qhandle_t hAnim );
int             trap_R_AnimFrameRate( qhandle_t hAnim );

void            trap_CompleteCallback( const char *complete );

void            trap_RegisterButtonCommands( const char *cmds );

void            trap_GetClipboardData( char *, int, clipboard_t );
void            trap_QuoteString( const char *, char*, int );
void            trap_Gettext( char *buffer, const char *msgid, int bufferLength );
void            trap_Pgettext( char *buffer, const char *ctxt, const char *msgid, int bufferLength );
void            trap_GettextPlural( char *buffer, const char *msgid, const char *msgid2, int number, int bufferLength );

void            trap_notify_onTeamChange( int newTeam );

qhandle_t       trap_RegisterVisTest();
void            trap_AddVisTestToScene( qhandle_t hTest, vec3_t pos,
					float depthAdjust, float area );
float           trap_CheckVisibility( qhandle_t hTest );
void            trap_UnregisterVisTest( qhandle_t hTest );
void            trap_SetColorGrading( int slot, qhandle_t hShader );
float           trap_CM_DistanceToModel( const vec3_t loc, clipHandle_t model );
void            trap_R_ScissorEnable( qboolean enable );
void            trap_R_ScissorSet( int x, int y, int w, int h );
void            trap_LAN_LoadCachedServers( void );
void            trap_LAN_SaveCachedServers( void );
int             trap_LAN_AddServer( int source, const char *name, const char *addr );
void            trap_LAN_RemoveServer( int source, const char *addr );
int             trap_LAN_GetPingQueueCount( void );
void            trap_LAN_ClearPing( int n );
void            trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void            trap_LAN_GetPingInfo( int n, char *buf, int buflen );
int             trap_LAN_GetServerCount( int source );
void            trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void            trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int             trap_LAN_GetServerPing( int source, int n );
void            trap_LAN_MarkServerVisible( int source, int n, qboolean visible );
int             trap_LAN_ServerIsVisible( int source, int n );
qboolean        trap_LAN_UpdateVisiblePings( int source );
void            trap_LAN_ResetPings( int n );
int             trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
qboolean        trap_LAN_ServerIsInFavoriteList( int source, int n );
qboolean        trap_GetNews( qboolean force );
int             trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void            trap_R_GetShaderNameFromHandle( const qhandle_t shader, char *out, int len );
void            trap_PrepareKeyUp( void );
void            trap_R_SetAltShaderTokens( const char * );
void            trap_S_UpdateEntityVelocity( int entityNum, const vec3_t velocity );
void            trap_S_SetReverb( int slotNum, const char* presetName, float ratio );
void            trap_S_BeginRegistration( void );
void            trap_S_EndRegistration( void );
void            trap_Rocket_Init( void );
void            trap_Rocket_Shutdown( void );
void            trap_Rocket_LoadDocument( const char *path );
void            trap_Rocket_LoadCursor( const char *path );
void            trap_Rocket_DocumentAction( const char *name, const char *action );
qboolean        trap_Rocket_GetEvent( void );
void            trap_Rocket_DeleteEvent( void );
void            trap_Rocket_RegisterDataSource( const char *name );
void            trap_Rocket_DSAddRow( const char *name, const char *table, const char *data );
void            trap_Rocket_DSChangeRow( const char *name, const char *table, int row, const char *data );
void            trap_Rocket_DSRemoveRow( const char *name, const char *table, int row );
void            trap_Rocket_DSClearTable( const char *name, const char *table );
void            trap_Rocket_SetInnerRML( const char *RML, int parseFlags );
void            trap_Rocket_GetAttribute( const char *attribute, char *out, int length );
void            trap_Rocket_SetAttribute( const char *attribute, const char *value );
void            trap_Rocket_GetProperty( const char *name, void *out, int len, rocketVarType_t type );
void            trap_Rocket_SetProperty( const char *property, const char *value );
void            trap_Rocket_GetEventParameters( char *params, int length );
void            trap_Rocket_RegisterDataFormatter( const char *name );
void            trap_Rocket_DataFormatterRawData( int handle, char *name, int nameLength, char *data, int dataLength );
void            trap_Rocket_DataFormatterFormattedData( int handle, const char *data, qboolean parseQuake );
void            trap_Rocket_RegisterElement( const char *tag );
void            trap_Rocket_SetElementDimensions( float x, float y );
void            trap_Rocket_GetElementTag( char *tag, int length );
void            trap_Rocket_GetElementAbsoluteOffset( float *x, float *y );
void            trap_Rocket_QuakeToRML( const char *in, char *out, int length );
void            trap_Rocket_SetClass( const char *in, qboolean activate );
void            trap_Rocket_InitializeHuds( int size );
void            trap_Rocket_LoadUnit( const char *path );
void            trap_Rocket_AddUnitToHud( int weapon, const char *id );
void            trap_Rocket_ShowHud( int weapon );
void            trap_Rocket_ClearHud( int weapon );
void            trap_Rocket_AddTextElement( const char *text, const char *Class, float x, float y );
void            trap_Rocket_ClearText( void );
void            trap_Rocket_RegisterProperty( const char *name, const char *defaultValue, qboolean inherited, qboolean force_layout, const char *parseAs );
void            trap_Rocket_ShowScoreboard( const char *name, qboolean show );
void            trap_Rocket_SetDataSelectIndex( int index );
void            trap_Rocket_LoadFont( const char *font );
#endif
