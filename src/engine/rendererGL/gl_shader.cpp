/*
===========================================================================
Copyright (C) 2010-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of Daemon source code.

Daemon source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Daemon source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// gl_shader.cpp -- GLSL shader handling

#include "gl_shader.h"

#if defined( USE_GLSL_OPTIMIZER )
#include "../../libs/glsl-optimizer/src/glsl/glsl_optimizer.h"

struct glslopt_ctx *s_glslOptimizer;

#endif
// *INDENT-OFF*

GLShader_generic                         *gl_genericShader = NULL;
GLShader_lightMapping                    *gl_lightMappingShader = NULL;
GLShader_vertexLighting_DBS_entity       *gl_vertexLightingShader_DBS_entity = NULL;
GLShader_vertexLighting_DBS_world        *gl_vertexLightingShader_DBS_world = NULL;
GLShader_forwardLighting_omniXYZ         *gl_forwardLightingShader_omniXYZ = NULL;
GLShader_forwardLighting_projXYZ         *gl_forwardLightingShader_projXYZ = NULL;
GLShader_forwardLighting_directionalSun  *gl_forwardLightingShader_directionalSun = NULL;
GLShader_deferredLighting_omniXYZ        *gl_deferredLightingShader_omniXYZ = NULL;
GLShader_deferredLighting_projXYZ        *gl_deferredLightingShader_projXYZ = NULL;
GLShader_deferredLighting_directionalSun *gl_deferredLightingShader_directionalSun = NULL;
GLShader_geometricFill                   *gl_geometricFillShader = NULL;
GLShader_shadowFill                      *gl_shadowFillShader = NULL;
GLShader_reflection                      *gl_reflectionShader = NULL;
GLShader_skybox                          *gl_skyboxShader = NULL;
GLShader_fogQuake3                       *gl_fogQuake3Shader = NULL;
GLShader_fogGlobal                       *gl_fogGlobalShader = NULL;
GLShader_heatHaze                        *gl_heatHazeShader = NULL;
GLShader_screen                          *gl_screenShader = NULL;
GLShader_portal                          *gl_portalShader = NULL;
GLShader_toneMapping                     *gl_toneMappingShader = NULL;
GLShader_contrast                        *gl_contrastShader = NULL;
GLShader_cameraEffects                   *gl_cameraEffectsShader = NULL;
GLShader_blurX                           *gl_blurXShader = NULL;
GLShader_blurY                           *gl_blurYShader = NULL;
GLShader_debugShadowMap                  *gl_debugShadowMapShader = NULL;
GLShader_depthToColor                    *gl_depthToColorShader = NULL;
GLShader_lightVolume_omni                *gl_lightVolumeShader_omni = NULL;
GLShader_deferredShadowing_proj          *gl_deferredShadowingShader_proj = NULL;
GLShader_liquid                          *gl_liquidShader = NULL;
GLShader_volumetricFog                   *gl_volumetricFogShader = NULL;
GLShader_screenSpaceAmbientOcclusion     *gl_screenSpaceAmbientOcclusionShader = NULL;
GLShader_depthOfField                    *gl_depthOfFieldShader = NULL;
GLShader_motionblur                      *gl_motionblurShader = NULL;
GLShaderManager                           gl_shaderManager;

GLShaderManager::~GLShaderManager( )
{
	for ( std::size_t i = 0; i < _shaders.size(); i++ )
	{
		delete _shaders[ i ];
	}
}

void GLShaderManager::freeAll( void )
{
	for ( std::size_t i = 0; i < _shaders.size(); i++ )
	{
		delete _shaders[ i ];
	}

	_shaders.erase( _shaders.begin(), _shaders.end() );

	while ( !_shaderBuildQueue.empty() )
	{
		_shaderBuildQueue.pop();
	}

	_totalBuildTime = 0;
}

void GLShaderManager::UpdateShaderProgramUniformLocations( GLShader *shader, shaderProgram_t *shaderProgram ) const
{
	size_t uniformSize = shader->_uniformStorageSize;
	size_t numUniforms = shader->_uniforms.size();

	// create buffer for storing uniform locations
	shaderProgram->uniformLocations = ( GLint * ) ri.Z_Malloc( sizeof( GLint ) * numUniforms );

	// create buffer for uniform firewall
	shaderProgram->uniformFirewall = ( byte * ) ri.Z_Malloc( uniformSize );

	// update uniforms
	for ( size_t j = 0; j < numUniforms; j++ )
	{
		GLUniform *uniform = shader->_uniforms[ j ];

		uniform->UpdateShaderProgramUniformLocation( shaderProgram );
	}
}

static inline void AddGLSLDefine( std::string &defines, const std::string define, int value )
{
	defines += "#ifndef " + define + "\n#define " + define + " ";
	defines += va( "%d\n", value );
	defines += "#endif\n";
}

static inline void AddGLSLDefine( std::string &defines, const std::string define, float value )
{
	defines += "#ifndef " + define + "\n#define " + define + " ";
	defines += va( "%f\n", value );
	defines += "#endif\n";
}

static inline void AddGLSLDefine( std::string &defines, const std::string define, float v1, float v2 )
{
	defines += "#ifndef " + define + "\n#define " + define + " ";
	defines += va( "vec2( %f, %f )\n", v1, v2 );
	defines += "#endif\n";
}

static inline void AddGLSLDefine( std::string &defines, const std::string define )
{
	defines += "#ifndef " + define + "\n#define " + define + "\n#endif\n";
}

std::string     GLShaderManager::BuildGPUShaderText( const char *mainShaderName,
    const char *libShaderNames,
    GLenum shaderType ) const
{
	char        filename[ MAX_QPATH ];
	GLchar      *mainBuffer = NULL;
	int         mainSize;
	char        *token;
	std::string libsBuffer; // all libs concatenated

	char        **libs = ( char ** ) &libShaderNames;

	std::string shaderText;

	GL_CheckErrors();

	while ( 1 )
	{
		int  libSize;
		char *libBuffer; // single extra lib file

		token = COM_ParseExt2( libs, qfalse );

		if ( !token[ 0 ] )
		{
			break;
		}

		if ( shaderType == GL_VERTEX_SHADER )
		{
			Com_sprintf( filename, sizeof( filename ), "glsl/%s_vp.glsl", token );
			ri.Printf( PRINT_DEVELOPER, "...loading vertex shader '%s'\n", filename );
		}
		else
		{
			Com_sprintf( filename, sizeof( filename ), "glsl/%s_fp.glsl", token );
			ri.Printf( PRINT_DEVELOPER, "...loading fragment shader '%s'\n", filename );
		}

		libSize = ri.FS_ReadFile( filename, ( void ** ) &libBuffer );

		if ( !libBuffer )
		{
			ri.Error( ERR_DROP, "Couldn't load %s", filename );
		}

		// append it to the libsBuffer
		libsBuffer.append(libBuffer);

		ri.FS_FreeFile( libBuffer );
	}

	// load main() program
	if ( shaderType == GL_VERTEX_SHADER )
	{
		Com_sprintf( filename, sizeof( filename ), "glsl/%s_vp.glsl", mainShaderName );
		ri.Printf( PRINT_DEVELOPER, "...loading vertex main() shader '%s'\n", filename );
	}
	else
	{
		Com_sprintf( filename, sizeof( filename ), "glsl/%s_fp.glsl", mainShaderName );
		ri.Printf( PRINT_DEVELOPER, "...loading fragment main() shader '%s'\n", filename );
	}

	mainSize = ri.FS_ReadFile( filename, ( void ** ) &mainBuffer );

	if ( !mainBuffer )
	{
		ri.Error( ERR_DROP, "Couldn't load %s", filename );
	}

	std::string bufferExtra;
	
	bufferExtra.reserve( 4096 );

#if defined( COMPAT_Q3A ) || defined( COMPAT_ET )
	AddGLSLDefine( bufferExtra, "COMPAT_Q3A", 1 );
#endif

#if defined( COMPAT_ET )
	AddGLSLDefine( bufferExtra, "COMPAT_ET", 1 );
#endif

	if( glConfig2.textureRGAvailable ) {
		AddGLSLDefine( bufferExtra, "TEXTURE_RG", 1 );
	}

	AddGLSLDefine( bufferExtra, "r_SpecularExponent", r_specularExponent->value );
	AddGLSLDefine( bufferExtra, "r_SpecularExponent2", r_specularExponent->value );
	AddGLSLDefine( bufferExtra, "r_SpecularScale", r_specularScale->value );

	//AddGLSLDefine( bufferExtra, "r_NormalScale", r_normalScale->value );

	AddGLSLDefine( bufferExtra, "M_PI", static_cast<float>( M_PI ) );
	AddGLSLDefine( bufferExtra, "MAX_SHADOWMAPS", MAX_SHADOWMAPS );
	AddGLSLDefine( bufferExtra, "MAX_SHADER_DEFORM_PARMS", MAX_SHADER_DEFORM_PARMS );

	AddGLSLDefine( bufferExtra, "deform_t" );
	AddGLSLDefine( bufferExtra, "DEFORM_WAVE", DEFORM_WAVE );
	AddGLSLDefine( bufferExtra, "DEFORM_BULGE", DEFORM_BULGE );
	AddGLSLDefine( bufferExtra, "DEFORM_MOVE", DEFORM_MOVE );

	AddGLSLDefine( bufferExtra, "genFunc_t" );
	AddGLSLDefine( bufferExtra, "GF_NONE", static_cast<float>( GF_NONE ) );
	AddGLSLDefine( bufferExtra, "GF_SIN", static_cast<float>( GF_SIN ) );
	AddGLSLDefine( bufferExtra, "GF_SQUARE", static_cast<float>( GF_SQUARE ) );
	AddGLSLDefine( bufferExtra, "GF_TRIANGLE", static_cast<float>( GF_TRIANGLE ) );
	AddGLSLDefine( bufferExtra, "GF_SAWTOOTH", static_cast<float>( GF_SAWTOOTH ) );
	AddGLSLDefine( bufferExtra, "GF_INVERSE_SAWTOOTH", static_cast<float>( GF_INVERSE_SAWTOOTH ) );
	AddGLSLDefine( bufferExtra, "GF_NOISE", static_cast<float>( GF_NOISE ) );

	/*
	AddGLSLDefine( bufferExtra, "deformGen_t" );
	AddGLSLDefine( bufferExtra, "DGEN_WAVE_SIN", static_cast<float>( DGEN_WAVE_SIN ) );
	AddGLSLDefine( bufferExtra, "DGEN_WAVE_SQUARE", static_cast<float>( DGEN_WAVE_SQUARE ) );
	AddGLSLDefine( bufferExtra, "DGEN_WAVE_TRIANGLE", static_cast<float>( DGEN_WAVE_TRIANGLE ) );
	AddGLSLDefine( bufferExtra, "DGEN_WAVE_SAWTOOTH", static_cast<float>( DGEN_WAVE_SAWTOOTH ) );
	AddGLSLDefine( bufferExtra, "DGEN_WAVE_INVERSE_SAWTOOH", static_cast<float>( DGEN_WAVE_INVERSE_SAWTOOTH ) );
	AddGLSLDefine( bufferExtra, "DGEN_BULGE", static_cast<float>( DGEN_BULGE ) );
	AddGLSLDefine( bufferExtra, "DGEN_MOVE", static_cast<float>( DGEN_MOVE ) );

	AddGLSLDefine( bufferExtra, "colorGen_t" );
	AddGLSLDefine( bufferExtra, "CGEN_VERTEX", CGEN_VERTEX );
	AddGLSLDefine( bufferExtra, "CGEN_ONE_MINUS_VERTEX", CGEN_ONE_MINUX_VERTEX );

	AddGLSLDefine( bufferExtra, "alphaGen_t" );
	AddGLSLDefine( bufferExtra, "AGEN_VERTEX", AGEN_VERTEX );
	AddGLSLDefine( bufferExtra, "AGEN_ONE_MINUS_VERTEX", AGEN_ONE_MINUS_VERTEX );
	*/

	float fbufWidthScale = Q_recip( ( float ) glConfig.vidWidth );
	float fbufHeightScale = Q_recip( ( float ) glConfig.vidHeight );

	AddGLSLDefine( bufferExtra, "r_FBufScale", fbufWidthScale, fbufHeightScale );

	float npotWidthScale = 1;
	float npotHeightScale = 1;

	if ( !glConfig2.textureNPOTAvailable )
	{
		npotWidthScale = ( float ) glConfig.vidWidth / ( float ) NearestPowerOfTwo( glConfig.vidWidth );
		npotHeightScale = ( float ) glConfig.vidHeight / ( float ) NearestPowerOfTwo( glConfig.vidHeight );
	}

	AddGLSLDefine( bufferExtra, "r_NPOTScale", npotWidthScale, npotHeightScale );
		
	if ( glConfig.driverType == GLDRV_MESA )
	{
		AddGLSLDefine( bufferExtra, "GLDRV_MESA", 1 );
	}

	if ( glConfig.hardwareType == GLHW_ATI )
	{
		AddGLSLDefine( bufferExtra, "GLHW_ATI", 1 );
	}
	else if ( glConfig.hardwareType == GLHW_ATI_DX10 )
	{
		AddGLSLDefine( bufferExtra, "GLHW_ATI_DX10", 1 );
	}
	else if ( glConfig.hardwareType == GLHW_NV_DX10 )
	{
		AddGLSLDefine( bufferExtra, "GLHW_NV_DX10", 1 );
	}

	if ( r_shadows->integer >= SHADOWING_ESM16 && glConfig2.textureFloatAvailable && glConfig2.framebufferObjectAvailable )
	{
		if ( r_shadows->integer == SHADOWING_ESM16 || r_shadows->integer == SHADOWING_ESM32 )
		{
			AddGLSLDefine( bufferExtra, "ESM", 1 );
		}
		else if ( r_shadows->integer == SHADOWING_EVSM32 )
		{
			AddGLSLDefine( bufferExtra, "EVSM", 1 );
			// The exponents for the EVSM techniques should be less than ln(FLT_MAX/FILTER_SIZE)/2 {ln(FLT_MAX/1)/2 ~44.3}
			//         42.9 is the maximum possible value for FILTER_SIZE=15
			//         42.0 is the truncated value that we pass into the sample
			AddGLSLDefine( bufferExtra, "r_EVSMExponents", 42.0f, 42.0f );
			if ( r_evsmPostProcess->integer )
			{
				AddGLSLDefine( bufferExtra,"r_EVSMPostProcess", 1 );
			}
		}
		else
		{
			AddGLSLDefine( bufferExtra, "VSM", 1 );

			if ( glConfig.hardwareType == GLHW_ATI )
			{
				AddGLSLDefine( bufferExtra, "VSM_CLAMP", 1 );
			}
		}

		if ( ( glConfig.hardwareType == GLHW_NV_DX10 || glConfig.hardwareType == GLHW_ATI_DX10 ) && r_shadows->integer == SHADOWING_VSM32 )
		{
			AddGLSLDefine( bufferExtra, "VSM_EPSILON", 0.000001f );
		}
		else
		{
			AddGLSLDefine( bufferExtra, "VSM_EPSILON", 0.0001f );
		}

		if ( r_lightBleedReduction->value )
		{
			AddGLSLDefine( bufferExtra, "r_LightBleedReduction", r_lightBleedReduction->value );
		}

		if ( r_overDarkeningFactor->value )
		{
			AddGLSLDefine( bufferExtra, "r_OverDarkeningFactor", r_overDarkeningFactor->value );
		}

		if ( r_shadowMapDepthScale->value )
		{
			AddGLSLDefine( bufferExtra, "r_ShadowMapDepthScale", r_shadowMapDepthScale->value );
		}

		if ( r_debugShadowMaps->integer )
		{
			AddGLSLDefine( bufferExtra, "r_debugShadowMaps", r_debugShadowMaps->integer );
		}

		/*
		if(r_softShadows->integer == 1)
		{
			AddGLSLDefine( bufferExtra, "PCF_2X2", 1 );
		}
		else if(r_softShadows->integer == 2)
		{
			AddGLSLDefine( bufferExtra, "PCF_3X3", 1 );
		}
		else if(r_softShadows->integer == 3)
		{
			AddGLSLDefine( bufferExtra, "PCF_4X4", 1 );
		}
		else if(r_softShadows->integer == 4)
		{
			AddGLSLDefine( bufferExtra, "PCF_5X5", 1 );
		}
		else if(r_softShadows->integer == 5)
		{
			AddGLSLDefine( bufferExtra, "PCF_6X6", 1 );
		}
		*/
		if ( r_softShadows->integer == 6 )
		{
			AddGLSLDefine( bufferExtra, "PCSS", 1 );
		}
		else if ( r_softShadows->integer )
		{
			AddGLSLDefine( bufferExtra, "r_PCFSamples", r_softShadows->value + 1.0f );
		}

		if ( r_parallelShadowSplits->integer )
		{
			AddGLSLDefine( bufferExtra, va( "r_ParallelShadowSplits_%d", r_parallelShadowSplits->integer ) );
		}

		if ( r_showParallelShadowSplits->integer )
		{
			AddGLSLDefine( bufferExtra, "r_ShowParallelShadowSplits", 1 );
		}
	}

	if ( r_deferredShading->integer && glConfig2.maxColorAttachments >= 4 && glConfig2.textureFloatAvailable &&
		    glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 4 )
	{
		if ( r_deferredShading->integer == DS_STANDARD )
		{
			AddGLSLDefine( bufferExtra, "r_DeferredShading", 1 );
		}
	}

	if ( r_hdrRendering->integer && glConfig2.framebufferObjectAvailable && glConfig2.textureFloatAvailable )
	{
		AddGLSLDefine( bufferExtra, "r_HDRRendering", 1 );
		AddGLSLDefine( bufferExtra, "r_HDRContrastThreshold", r_hdrContrastThreshold->value );
		AddGLSLDefine( bufferExtra, "r_HDRContrastOffset", r_hdrContrastOffset->value );
		AddGLSLDefine( bufferExtra, va( "r_HDRToneMappingOperator_%d", r_hdrToneMappingOperator->integer ) );
		AddGLSLDefine( bufferExtra, "r_HDRGamma", r_hdrGamma->value );
	}

	if ( r_precomputedLighting->integer )
	{
		AddGLSLDefine( bufferExtra, "r_precomputedLighting", 1 );
	}

	if ( r_heatHazeFix->integer && glConfig2.framebufferBlitAvailable && /*glConfig.hardwareType != GLHW_ATI && glConfig.hardwareType != GLHW_ATI_DX10 &&*/ glConfig.driverType != GLDRV_MESA )
	{
		AddGLSLDefine( bufferExtra, "r_heatHazeFix", 1 );
	}

	if ( r_showLightMaps->integer )
	{
		AddGLSLDefine( bufferExtra, "r_showLightMaps", r_showLightMaps->integer );
	}

	if ( r_showDeluxeMaps->integer )
	{
		AddGLSLDefine( bufferExtra, "r_showDeluxeMaps", r_showDeluxeMaps->integer );
	}

#ifdef EXPERIMENTAL

	if ( r_screenSpaceAmbientOcclusion->integer )
	{
		int             i;
		static vec3_t   jitter[ 32 ];
		static qboolean jitterInit = qfalse;

		if ( !jitterInit )
		{
			for ( i = 0; i < 32; i++ )
			{
				float *jit = &jitter[ i ][ 0 ];

				float rad = crandom() * 1024.0f; // FIXME radius;
				float a = crandom() * M_PI * 2;
				float b = crandom() * M_PI * 2;

				jit[ 0 ] = rad * sin( a ) * cos( b );
				jit[ 1 ] = rad * sin( a ) * sin( b );
				jit[ 2 ] = rad * cos( a );
			}

			jitterInit = qtrue;
		}

		// TODO
	}

#endif

	if ( glConfig2.vboVertexSkinningAvailable )
	{
		AddGLSLDefine( bufferExtra, "r_VertexSkinning", 1 );
		AddGLSLDefine( bufferExtra, "MAX_GLSL_BONES", glConfig2.maxVertexSkinningBones );
	}
	else
	{
		AddGLSLDefine( bufferExtra, "MAX_GLSL_BONES", 4 );
	}

	/*
	if(glConfig.drawBuffersAvailable && glConfig.maxDrawBuffers >= 4)
	{
		AddGLSLDefine( bufferExtra, "GL_ARB_draw_buffers", 1 );
		bufferExtra += "#extension GL_ARB_draw_buffers : enable\n";
	}
	*/

	if ( r_wrapAroundLighting->value )
	{
		AddGLSLDefine( bufferExtra, "r_WrapAroundLighting", r_wrapAroundLighting->value );
	}

	if ( r_halfLambertLighting->integer )
	{
		AddGLSLDefine( bufferExtra, "r_HalfLambertLighting", 1 );
	}

	if ( r_rimLighting->integer )
	{
		AddGLSLDefine( bufferExtra, "r_RimLighting", 1 );
		AddGLSLDefine( bufferExtra, "r_RimExponent", r_rimExponent->value );
	}

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	bufferExtra += "#line 0\n";
	shaderText = bufferExtra + libsBuffer + mainBuffer;

#if 0
	{
		static char msgPart[ 1024 ];
		int         i;
		ri.Printf( PRINT_WARNING, "----------------------------------------------------------\n" );
		ri.Printf( PRINT_WARNING, "CONCATENATED shader '%s' ----------\n", filename );
		ri.Printf( PRINT_WARNING, " BEGIN ---------------------------------------------------\n" );

		for ( i = 0; i < sizeFinal; i += 1024 )
		{
			Q_strncpyz( msgPart, shaderText.c_str() + i, sizeof( msgPart ) );
			ri.Printf( PRINT_ALL, "%s", msgPart );
		}

		ri.Printf( PRINT_WARNING, " END-- ---------------------------------------------------\n" );
	}
#endif

	ri.FS_FreeFile( mainBuffer );

	return shaderText;
}

bool GLShaderManager::buildPermutation( GLShader *shader, size_t i )
{
	std::string compileMacros;
	int  startTime = ri.Milliseconds();
	int  endTime;

	// program already exists
	if ( shader->_shaderPrograms[ i ].program )
	{
		return false;
	}

	if( shader->GetCompileMacrosString( i, compileMacros ) )
	{
		shader->BuildShaderCompileMacros( compileMacros );

		//ri.Printf(PRINT_ALL, "Compile macros: '%s'\n", compileMacros.c_str());

		shaderProgram_t *shaderProgram = &shader->_shaderPrograms[ i ];

		shaderProgram->program = glCreateProgram();
		shaderProgram->attribs = shader->_vertexAttribsRequired; // | _vertexAttribsOptional;

		if ( !LoadShaderBinary( shader, i ) )
		{
			CompileAndLinkGPUShaderProgram(	shader, shaderProgram, compileMacros );
			SaveShaderBinary( shader, i );
		}

		UpdateShaderProgramUniformLocations( shader, shaderProgram );
		GL_BindProgram( shaderProgram );
		shader->SetShaderProgramUniforms( shaderProgram );
		GL_BindProgram( NULL );

		ValidateProgram( shaderProgram->program );
		GL_CheckErrors();

		endTime = ri.Milliseconds();
		_totalBuildTime += ( endTime - startTime );
		return true;
	}
	return false;
}

void GLShaderManager::buildAll( )
{
	while ( !_shaderBuildQueue.empty() )
	{
		GLShader *shader = _shaderBuildQueue.front();
		size_t numPermutations = 1 << shader->GetNumOfCompiledMacros();
		size_t i;

		for( i = 0; i < numPermutations; i++ )
		{
			buildPermutation( shader, i );
		}

		_shaderBuildQueue.pop();
	}

	ri.Printf( PRINT_DEVELOPER, "glsl shaders took %d msec to build\n", _totalBuildTime );

	if( r_recompileShaders->integer )
	{
		ri.Cvar_Set( "r_recompileShaders", "0" );
	}
}

void GLShaderManager::InitShader( GLShader *shader )
{
	shader->_shaderPrograms = std::vector<shaderProgram_t>( 1 << shader->_compileMacros.size() );
	memset( &shader->_shaderPrograms[ 0 ], 0, shader->_shaderPrograms.size() * sizeof( shaderProgram_t ) );

	shader->_uniformStorageSize = 0;
	for ( std::size_t i = 0; i < shader->_uniforms.size(); i++ )
	{
		GLUniform *uniform = shader->_uniforms[ i ];
		uniform->SetLocationIndex( i );
		uniform->SetFirewallIndex( shader->_uniformStorageSize );
		shader->_uniformStorageSize += uniform->GetSize();
	}

	std::string vertexInlines = "";
	shader->BuildShaderVertexLibNames( vertexInlines );

	std::string fragmentInlines = "";
	shader->BuildShaderFragmentLibNames( fragmentInlines );

	shader->_vertexShaderText = BuildGPUShaderText( shader->GetMainShaderName().c_str(), vertexInlines.c_str(), GL_VERTEX_SHADER );
	shader->_fragmentShaderText = BuildGPUShaderText( shader->GetMainShaderName().c_str(), fragmentInlines.c_str(), GL_FRAGMENT_SHADER );
	std::string combinedShaderText= shader->_vertexShaderText + shader->_fragmentShaderText;

	shader->_checkSum = Com_BlockChecksum( combinedShaderText.c_str(), combinedShaderText.length() );
}

bool GLShaderManager::LoadShaderBinary( GLShader *shader, size_t programNum )
{
#ifdef GLEW_ARB_get_program_binary
	GLint          success;
	GLint          fileLength;
	void           *binary;
	byte           *binaryptr;
	GLShaderHeader shaderHeader;
	int            numLoaded = 0;
	int            numProgramsNeeded = 0;

	// we need to recompile the shaders
	if( r_recompileShaders->integer )
	{
		return false;
	}

	// don't even try if the necessary functions aren't available
	if( !glConfig2.getProgramBinaryAvailable )
	{
		return false;
	}

	fileLength = ri.FS_ReadFile( va( "glsl/%s/%s_%u.bin", shader->GetName().c_str(), shader->GetName().c_str(), ( unsigned int ) programNum ), &binary );

	// file empty or not found
	if( fileLength <= 0 )
	{
		return false;
	}

	binaryptr = ( byte* )binary;

	// get the shader header from the file
	memcpy( &shaderHeader, binaryptr, sizeof( shaderHeader ) );
	binaryptr += sizeof( shaderHeader );

	// check if this shader binary is the correct format
	if ( shaderHeader.version != GL_SHADER_VERSION )
	{
		ri.FS_FreeFile( binary );
		return false;
	}

	// make sure this shader uses the same number of macros
	if ( shaderHeader.numMacros != shader->GetNumOfCompiledMacros() )
	{
		ri.FS_FreeFile( binary );
		return false;
	}

	// make sure this shader uses the same macros
	for ( unsigned int i = 0; i < shaderHeader.numMacros; i++ )
	{
		if ( shader->_compileMacros[ i ]->GetType() != shaderHeader.macros[ i ] )
		{
			ri.FS_FreeFile( binary );
			return false;
		}
	}

	// make sure the checksums for the source code match
	if ( shaderHeader.checkSum != shader->_checkSum )
	{
		ri.FS_FreeFile( binary );
		return false;
	}

	// load the shader
	shaderProgram_t *shaderProgram = &shader->_shaderPrograms[ programNum ];
	glProgramBinary( shaderProgram->program, shaderHeader.binaryFormat, binaryptr, shaderHeader.binaryLength );
	glGetProgramiv( shaderProgram->program, GL_LINK_STATUS, &success );

	if ( !success )
	{
		ri.FS_FreeFile( binary );
		return false;
	}

	ri.FS_FreeFile( binary );
	return true;
#else
	return false;
#endif
}
void GLShaderManager::SaveShaderBinary( GLShader *shader, size_t programNum )
{
#ifdef GLEW_ARB_get_program_binary
	GLint                 binaryLength;
	GLuint                binarySize = 0;
	byte                  *binary;
	byte                  *binaryptr;
	GLShaderHeader        shaderHeader;
	shaderProgram_t       *shaderProgram;

	// don't even try if the necessary functions aren't available
	if( !glConfig2.getProgramBinaryAvailable )
	{
		return;
	}

	shaderProgram = &shader->_shaderPrograms[ programNum ];

	memset( &shaderHeader, 0, sizeof( shaderHeader ) );

	// find output size
	binarySize += sizeof( shaderHeader );
	glGetProgramiv( shaderProgram->program, GL_PROGRAM_BINARY_LENGTH, &binaryLength );
	binarySize += binaryLength;

	binaryptr = binary = ( byte* )ri.Hunk_AllocateTempMemory( binarySize );

	// reserve space for the header
	binaryptr += sizeof( shaderHeader );

	// get the program binary and write it to the buffer
	glGetProgramBinary( shaderProgram->program, binaryLength, NULL, &shaderHeader.binaryFormat, binaryptr );

	// set the header
	shaderHeader.version = GL_SHADER_VERSION;
	shaderHeader.numMacros = shader->_compileMacros.size();

	for ( unsigned int i = 0; i < shaderHeader.numMacros; i++ )
	{
		shaderHeader.macros[ i ] = shader->_compileMacros[ i ]->GetType();
	}

	shaderHeader.binaryLength = binaryLength;
	shaderHeader.checkSum = shader->_checkSum;

	// write the header to the buffer
	memcpy( ( void* )binary, &shaderHeader, sizeof( shaderHeader ) );

	ri.FS_WriteFile( va( "glsl/%s/%s_%u.bin", shader->GetName().c_str(), shader->GetName().c_str(), ( unsigned int ) programNum ), binary, binarySize );

	ri.Hunk_FreeTempMemory( binary );
#endif
}
void GLShaderManager::CompileAndLinkGPUShaderProgram( GLShader *shader, shaderProgram_t *program,
    const std::string &compileMacros ) const
{
#ifdef USE_GLSL_OPTIMIZER
	bool        optimize = r_glslOptimizer->integer ? true : false;
#endif

	//ri.Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");
	// header of the glsl shader
	std::string vertexHeader;
	std::string fragmentHeader;

	if ( glConfig.driverType == GLDRV_OPENGL3 )
	{
		// HACK: abuse the GLSL preprocessor to turn GLSL 1.20 shaders into 1.30 ones

		vertexHeader += "#version 130\n";
		fragmentHeader += "#version 130\n";

		vertexHeader += "#define attribute in\n";
		vertexHeader += "#define varying out\n";

		fragmentHeader += "#define varying in\n";

		fragmentHeader += "out vec4 out_Color;\n";
		fragmentHeader += "#define gl_FragColor out_Color\n";

		vertexHeader += "#define textureCube texture\n";
		vertexHeader += "#define texture2D texture\n";
		vertexHeader += "#define texture2DProj textureProj\n";

		fragmentHeader += "#define textureCube texture\n";
		fragmentHeader += "#define texture2D texture\n";
		fragmentHeader += "#define texture2DProj textureProj\n";
	}
	else
	{
		vertexHeader += "#version 120\n";
		fragmentHeader += "#version 120\n";
	}

	// permutation macros
	std::string macrosString;

	if ( !compileMacros.empty() )
	{
		const char *compileMacros_ = compileMacros.c_str();
		char       **compileMacrosP = ( char ** ) &compileMacros_;
		char       *token;

		while ( 1 )
		{
			token = COM_ParseExt2( compileMacrosP, qfalse );

			if ( !token[ 0 ] )
			{
				break;
			}

			macrosString += va( "#ifndef %s\n#define %s 1\n#endif\n", token, token );
		}
	}

	// add them
	std::string vertexShaderTextWithMacros = vertexHeader + macrosString + shader->_vertexShaderText;
	std::string fragmentShaderTextWithMacros = fragmentHeader + macrosString + shader->_fragmentShaderText;
#ifdef USE_GLSL_OPTIMIZER
	if( optimize )
	{
		static char         msgPart[ 1024 ];
		int                 length = 0;
		int                 i;

		glslopt_shader *shaderOptimized = glslopt_optimize( s_glslOptimizer, kGlslOptShaderVertex, vertexShaderTextWithMacros.c_str(), 0 );
		if( glslopt_get_status( shaderOptimized ) )
		{
			vertexShaderTextWithMacros = glslopt_get_output( shaderOptimized );

			ri.Printf( PRINT_DEVELOPER, "----------------------------------------------------------\n" );
			ri.Printf( PRINT_DEVELOPER, "OPTIMIZED VERTEX shader '%s' ----------\n", shader->GetName().c_str() );
			ri.Printf( PRINT_DEVELOPER, " BEGIN ---------------------------------------------------\n" );

			length = strlen( vertexShaderTextWithMacros.c_str() );

			for ( i = 0; i < length; i += 1024 )
			{
				Q_strncpyz( msgPart, vertexShaderTextWithMacros.c_str() + i, sizeof( msgPart ) );
				ri.Printf( PRINT_DEVELOPER, "%s\n", msgPart );
			}

			ri.Printf( PRINT_DEVELOPER, " END-- ---------------------------------------------------\n" );
		}
		else
		{
			const char *errorLog = glslopt_get_log( shaderOptimized );

			length = strlen( errorLog );

			for ( i = 0; i < length; i += 1024 )
			{
				Q_strncpyz( msgPart, errorLog + i, sizeof( msgPart ) );
				ri.Printf( PRINT_WARNING, "%s\n", msgPart );
			}

			ri.Printf( PRINT_WARNING, "^1Couldn't optimize VERTEX shader %s\n", shader->GetName().c_str() );
		}
		glslopt_shader_delete( shaderOptimized );

		glslopt_shader *shaderOptimized1 = glslopt_optimize( s_glslOptimizer, kGlslOptShaderFragment, fragmentShaderTextWithMacros.c_str(), 0 );
		if( glslopt_get_status( shaderOptimized1 ) )
		{
			fragmentShaderTextWithMacros = glslopt_get_output( shaderOptimized1 );

			ri.Printf( PRINT_DEVELOPER, "----------------------------------------------------------\n" );
			ri.Printf( PRINT_DEVELOPER, "OPTIMIZED FRAGMENT shader '%s' ----------\n", shader->GetName().c_str() );
			ri.Printf( PRINT_DEVELOPER, " BEGIN ---------------------------------------------------\n" );

			length = strlen( fragmentShaderTextWithMacros.c_str() );

			for ( i = 0; i < length; i += 1024 )
			{
				Q_strncpyz( msgPart, fragmentShaderTextWithMacros.c_str() + i, sizeof( msgPart ) );
				ri.Printf( PRINT_DEVELOPER, "%s\n", msgPart );
			}

			ri.Printf( PRINT_DEVELOPER, " END-- ---------------------------------------------------\n" );
		}
		else
		{
			const char *errorLog = glslopt_get_log( shaderOptimized1 );

			length = strlen( errorLog );

			for ( i = 0; i < length; i += 1024 )
			{
				Q_strncpyz( msgPart, errorLog + i, sizeof( msgPart ) );
				ri.Printf( PRINT_WARNING, "%s\n", msgPart );
			}

			ri.Printf( PRINT_WARNING, "^1Couldn't optimize FRAGMENT shader %s\n", shader->GetName().c_str() );
		}
		glslopt_shader_delete( shaderOptimized1 );
	}
#endif
	CompileGPUShader( program->program, shader->GetName().c_str(), vertexShaderTextWithMacros.c_str(), vertexShaderTextWithMacros.length(), GL_VERTEX_SHADER );
	CompileGPUShader( program->program, shader->GetName().c_str(), fragmentShaderTextWithMacros.c_str(), fragmentShaderTextWithMacros.length(), GL_FRAGMENT_SHADER );
	BindAttribLocations( program->program );
	LinkProgram( program->program );
}

void GLShaderManager::CompileGPUShader( GLuint program, const char *programName, const char *shaderText, int shaderTextSize, GLenum shaderType ) const
{
	GLuint shader = glCreateShader( shaderType );

	GL_CheckErrors();

	glShaderSource( shader, 1, ( const GLchar ** ) &shaderText, &shaderTextSize );

	// compile shader
	glCompileShader( shader );

	GL_CheckErrors();

	// check if shader compiled
	GLint compiled;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if ( !compiled )
	{
		PrintShaderSource( shader );
		PrintInfoLog( shader, qfalse );
#ifdef NDEBUG
		// In a release build, GLSL shader compiliation usually means that the hardware does
		// not support GLSL shaders and should rely on vanilla instead.
		ri.Cvar_Set( "cl_renderer", "GL" );
#endif
		ri.Error( ERR_DROP, "Couldn't compile %s %s", ( shaderType == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader" ), programName );
	}

	//PrintInfoLog(shader, qtrue);
	//ri.Printf(PRINT_ALL, "%s\n", GLSL_PrintShaderSource(shader));

	// attach shader to program
	glAttachShader( program, shader );
	GL_CheckErrors();

	// mark shader for deletion
	// shader will be deleted when glDeleteProgram is called on the program it is attached to
	glDeleteShader( shader );
	GL_CheckErrors();
}

void GLShaderManager::PrintShaderSource( GLuint object ) const
{
	char        *msg;
	static char msgPart[ 1024 ];
	int         maxLength = 0;
	int         i;

	glGetShaderiv( object, GL_SHADER_SOURCE_LENGTH, &maxLength );

	msg = ( char * ) ri.Hunk_AllocateTempMemory( maxLength );

	glGetShaderSource( object, maxLength, &maxLength, msg );

	for ( i = 0; i < maxLength; i += 1024 )
	{
		Q_strncpyz( msgPart, msg + i, sizeof( msgPart ) );
		ri.Printf( PRINT_ALL, "%s\n", msgPart );
	}

	ri.Hunk_FreeTempMemory( msg );
}

void GLShaderManager::PrintInfoLog( GLuint object, bool developerOnly ) const
{
	char        *msg;
	static char msgPart[ 1024 ];
	int         maxLength = 0;
	int         i;

	printParm_t print = ( developerOnly ) ? PRINT_DEVELOPER : PRINT_ALL;

	if ( glIsShader( object ) )
	{
		glGetShaderiv( object, GL_INFO_LOG_LENGTH, &maxLength );
	}
	else if ( glIsProgram( object ) )
	{
		glGetProgramiv( object, GL_INFO_LOG_LENGTH, &maxLength );
	}
	else
	{
		ri.Printf( print, "object is not a shader or program\n" );
		return;
	}

	msg = ( char * ) ri.Hunk_AllocateTempMemory( maxLength );

	if ( glIsShader( object ) )
	{
		glGetShaderInfoLog( object, maxLength, &maxLength, msg );
		ri.Printf( print, "compile log:\n" );
	}
	else if ( glIsProgram( object ) )
	{
		glGetProgramInfoLog( object, maxLength, &maxLength, msg );
		ri.Printf( print, "link log:\n" );
	}

	for ( i = 0; i < maxLength; i += 1024 )
	{
		Q_strncpyz( msgPart, msg + i, sizeof( msgPart ) );

		ri.Printf( print, "%s\n", msgPart );
	}

	ri.Hunk_FreeTempMemory( msg );
}

void GLShaderManager::LinkProgram( GLuint program ) const
{
	GLint linked;

#ifdef GLEW_ARB_get_program_binary
	// Apparently, this is necessary to get the binary program via glGetProgramBinary
	if( glConfig2.getProgramBinaryAvailable )
	{
		glProgramParameteri( program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE );
	}
#endif
	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS, &linked );

	if ( !linked )
	{
		PrintInfoLog( program, qfalse );
		ri.Error( ERR_DROP, "Shaders failed to link!!!" );
	}
}

void GLShaderManager::ValidateProgram( GLuint program ) const
{
	GLint validated;

	glValidateProgram( program );

	glGetProgramiv( program, GL_VALIDATE_STATUS, &validated );

	if ( !validated )
	{
		PrintInfoLog( program, qfalse );
		ri.Error( ERR_DROP, "Shaders failed to validate!!!" );
	}
}

void GLShaderManager::BindAttribLocations( GLuint program ) const
{
	//if(attribs & ATTR_POSITION)
	glBindAttribLocation( program, ATTR_INDEX_POSITION, "attr_Position" );

	//if(attribs & ATTR_TEXCOORD)
	glBindAttribLocation( program, ATTR_INDEX_TEXCOORD0, "attr_TexCoord0" );

	//if(attribs & ATTR_LIGHTCOORD)
	glBindAttribLocation( program, ATTR_INDEX_TEXCOORD1, "attr_TexCoord1" );

//  if(attribs & ATTR_TEXCOORD2)
//      glBindAttribLocation(program, ATTR_INDEX_TEXCOORD2, "attr_TexCoord2");

//  if(attribs & ATTR_TEXCOORD3)
//      glBindAttribLocation(program, ATTR_INDEX_TEXCOORD3, "attr_TexCoord3");

	//if(attribs & ATTR_TANGENT)
	glBindAttribLocation( program, ATTR_INDEX_TANGENT, "attr_Tangent" );

	//if(attribs & ATTR_BINORMAL)
	glBindAttribLocation( program, ATTR_INDEX_BINORMAL, "attr_Binormal" );

	//if(attribs & ATTR_NORMAL)
	glBindAttribLocation( program, ATTR_INDEX_NORMAL, "attr_Normal" );

	//if(attribs & ATTR_COLOR)
	glBindAttribLocation( program, ATTR_INDEX_COLOR, "attr_Color" );

#if !defined( COMPAT_Q3A ) && !defined( COMPAT_ET )
	//if(attribs & ATTR_PAINTCOLOR)
	glBindAttribLocation( program, ATTR_INDEX_PAINTCOLOR, "attr_PaintColor" );
#endif

	//if(attribs & ATTR_AMBIENTLIGHT)
	glBindAttribLocation( program, ATTR_INDEX_AMBIENTLIGHT, "attr_AmbientLight" );

	//if(attribs & ATTR_DIRECTEDLIGHT)
	glBindAttribLocation( program, ATTR_INDEX_DIRECTEDLIGHT, "attr_DirectedLight" );

	//if(attribs & ATTR_LIGHTDIRECTION)
	glBindAttribLocation( program, ATTR_INDEX_LIGHTDIRECTION, "attr_LightDirection" );

	//if(glConfig2.vboVertexSkinningAvailable)
	{
		glBindAttribLocation( program, ATTR_INDEX_BONE_INDEXES, "attr_BoneIndexes" );
		glBindAttribLocation( program, ATTR_INDEX_BONE_WEIGHTS, "attr_BoneWeights" );
	}

	//if(attribs & ATTR_POSITION2)
	glBindAttribLocation( program, ATTR_INDEX_POSITION2, "attr_Position2" );

	//if(attribs & ATTR_TANGENT2)
	glBindAttribLocation( program, ATTR_INDEX_TANGENT2, "attr_Tangent2" );

	//if(attribs & ATTR_BINORMAL2)
	glBindAttribLocation( program, ATTR_INDEX_BINORMAL2, "attr_Binormal2" );

	//if(attribs & ATTR_NORMAL2)
	glBindAttribLocation( program, ATTR_INDEX_NORMAL2, "attr_Normal2" );
}

bool GLCompileMacro_USE_VERTEX_SKINNING::HasConflictingMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
	for ( size_t i = 0; i < macros.size(); i++ )
	{
		GLCompileMacro *macro = macros[ i ];

		//if(GLCompileMacro_USE_VERTEX_ANIMATION* m = dynamic_cast<GLCompileMacro_USE_VERTEX_ANIMATION*>(macro))
		if ( ( permutation & macro->GetBit() ) != 0 && macro->GetType() == USE_VERTEX_ANIMATION )
		{
			//ri.Printf(PRINT_ALL, "conflicting macro! canceling '%s' vs. '%s'\n", GetName(), macro->GetName());
			return true;
		}
	}

	return false;
}

bool GLCompileMacro_USE_VERTEX_SKINNING::MissesRequiredMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
	return !glConfig2.vboVertexSkinningAvailable;
}

bool GLCompileMacro_USE_VERTEX_ANIMATION::HasConflictingMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
#if 1

	for ( size_t i = 0; i < macros.size(); i++ )
	{
		GLCompileMacro *macro = macros[ i ];

		if ( ( permutation & macro->GetBit() ) != 0 && macro->GetType() == USE_VERTEX_SKINNING )
		{
			//ri.Printf(PRINT_ALL, "conflicting macro! canceling '%s' vs. '%s'\n", GetName(), macro->GetName());
			return true;
		}
	}

#endif
	return false;
}

uint32_t        GLCompileMacro_USE_VERTEX_ANIMATION::GetRequiredVertexAttributes() const
{
	uint32_t attribs = ATTR_NORMAL | ATTR_POSITION2 | ATTR_NORMAL2;

	if ( r_normalMapping->integer )
	{
		attribs |= ATTR_TANGENT2 | ATTR_BINORMAL2;
	}

	return attribs;
}

bool GLCompileMacro_USE_DEFORM_VERTEXES::HasConflictingMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
	return ( glConfig.driverType != GLDRV_OPENGL3 || !r_vboDeformVertexes->integer );
}

bool GLCompileMacro_USE_PARALLAX_MAPPING::MissesRequiredMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
	bool foundUSE_NORMAL_MAPPING = false;

	for ( size_t i = 0; i < macros.size(); i++ )
	{
		GLCompileMacro *macro = macros[ i ];

		if ( ( permutation & macro->GetBit() ) != 0 && macro->GetType() == USE_NORMAL_MAPPING )
		{
			foundUSE_NORMAL_MAPPING = true;
		}
	}

	if ( !foundUSE_NORMAL_MAPPING )
	{
		//ri.Printf(PRINT_ALL, "missing macro! canceling '%s' <= '%s'\n", GetName(), "USE_NORMAL_MAPPING");
		return true;
	}

	return false;
}

bool GLCompileMacro_USE_REFLECTIVE_SPECULAR::MissesRequiredMacros( size_t permutation, const std::vector< GLCompileMacro * > &macros ) const
{
	bool foundUSE_NORMAL_MAPPING = false;

	for ( size_t i = 0; i < macros.size(); i++ )
	{
		GLCompileMacro *macro = macros[ i ];

		if ( ( permutation & macro->GetBit() ) != 0 && macro->GetType() == USE_NORMAL_MAPPING )
		{
			foundUSE_NORMAL_MAPPING = true;
		}
	}

	if ( !foundUSE_NORMAL_MAPPING )
	{
		//ri.Printf(PRINT_ALL, "missing macro! canceling '%s' <= '%s'\n", GetName(), "USE_NORMAL_MAPPING");
		return true;
	}

	return false;
}

bool GLShader::GetCompileMacrosString( size_t permutation, std::string &compileMacrosOut ) const
{
	compileMacrosOut = "";

	for ( size_t j = 0; j < _compileMacros.size(); j++ )
	{
		GLCompileMacro *macro = _compileMacros[ j ];

		if ( permutation & macro->GetBit() )
		{
			if ( macro->HasConflictingMacros( permutation, _compileMacros ) )
			{
				//ri.Printf(PRINT_ALL, "conflicting macro! canceling '%s'\n", macro->GetName());
				return false;
			}

			if ( macro->MissesRequiredMacros( permutation, _compileMacros ) )
			{
				return false;
			}

			compileMacrosOut += macro->GetName();
			compileMacrosOut += " ";
		}
	}

	return true;
}

int GLShader::SelectProgram()
{
	int    index = 0;

	size_t numMacros = _compileMacros.size();

	for ( size_t i = 0; i < numMacros; i++ )
	{
		if ( _activeMacros & BIT( i ) )
		{
			index += BIT( i );
		}
	}

	return index;
}

void GLShader::BindProgram()
{
	int index = SelectProgram();

	// program may not be loaded yet because the shader manager hasn't yet gotten to it
	// so try to load it now
	if ( index >= _shaderPrograms.size() || !_shaderPrograms[ index ].program )
	{
		_shaderManager->buildPermutation( this, index );
	}

	// program is still not loaded
	if ( index >= _shaderPrograms.size() || !_shaderPrograms[ index ].program )
	{
		std::string activeMacros = "";
		size_t      numMacros = _compileMacros.size();

		for ( size_t j = 0; j < numMacros; j++ )
		{
			GLCompileMacro *macro = _compileMacros[ j ];

			int           bit = macro->GetBit();

			if ( IsMacroSet( bit ) )
			{
				activeMacros += macro->GetName();
				activeMacros += " ";
			}
		}

		ri.Error( ERR_FATAL, "Invalid shader configuration: shader = '%s', macros = '%s'", _name.c_str(), activeMacros.c_str() );
	}

	_currentProgram = &_shaderPrograms[ index ];

	if ( r_logFile->integer )
	{
		std::string macros;

		this->GetCompileMacrosString( index, macros );

		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment( va( "--- GL_BindProgram( name = '%s', macros = '%s' ) ---\n", this->GetName().c_str(), macros.c_str() ) );
	}

	GL_BindProgram( _currentProgram );
}

void GLShader::SetRequiredVertexPointers()
{
	uint32_t macroVertexAttribs = 0;
	size_t   numMacros = _compileMacros.size();

	for ( size_t j = 0; j < numMacros; j++ )
	{
		GLCompileMacro *macro = _compileMacros[ j ];

		int           bit = macro->GetBit();

		if ( IsMacroSet( bit ) )
		{
			macroVertexAttribs |= macro->GetRequiredVertexAttributes();
		}
	}

	GL_VertexAttribsState( ( _vertexAttribsRequired | _vertexAttribs | macroVertexAttribs ) );  // & ~_vertexAttribsUnsupported);
}

GLShader_generic::GLShader_generic( GLShaderManager *manager ) :
	GLShader( "generic", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_ColorTextureMatrix( this ),
	u_ViewOrigin( this ),
	u_AlphaThreshold( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_TCGEN_ENVIRONMENT( this ),
	GLCompileMacro_USE_TCGEN_LIGHTMAP( this )
{
}

void GLShader_generic::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_generic::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_lightMapping::GLShader_lightMapping( GLShaderManager *manager ) :
	GLShader( "lightMapping", ATTR_POSITION | ATTR_TEXCOORD | ATTR_LIGHTCOORD | ATTR_NORMAL, manager ),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_AlphaThreshold( this ),
	u_ViewOrigin( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_DepthScale( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_lightMapping::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_lightMapping::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_lightMapping::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_lightMapping::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ),  2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_LightMap" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DeluxeMap" ), 4 );
}

GLShader_vertexLighting_DBS_entity::GLShader_vertexLighting_DBS_entity( GLShaderManager *manager ) :
	GLShader( "vertexLighting_DBS_entity", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_AlphaThreshold( this ),
	u_AmbientColor( this ),
	u_ViewOrigin( this ),
	u_LightDir( this ),
	u_LightColor( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_DepthScale( this ),
	u_EnvironmentInterpolation( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this ),
	GLCompileMacro_USE_REFLECTIVE_SPECULAR( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_vertexLighting_DBS_entity::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_vertexLighting_DBS_entity::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_vertexLighting_DBS_entity::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_vertexLighting_DBS_entity::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_EnvironmentMap0" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_EnvironmentMap1" ), 4 );
}

GLShader_vertexLighting_DBS_world::GLShader_vertexLighting_DBS_world( GLShaderManager *manager ) :
	GLShader( "vertexLighting_DBS_world",
	          ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL | ATTR_COLOR
	          | ATTR_AMBIENTLIGHT | ATTR_DIRECTEDLIGHT | ATTR_LIGHTDIRECTION, manager
	        ),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_AlphaThreshold( this ),
	u_ViewOrigin( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_DepthScale( this ),
	u_LightWrapAround( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_vertexLighting_DBS_world::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}
void GLShader_vertexLighting_DBS_world::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_vertexLighting_DBS_world::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_vertexLighting_DBS_world::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
}

GLShader_forwardLighting_omniXYZ::GLShader_forwardLighting_omniXYZ( GLShaderManager *manager ):
	GLShader("forwardLighting_omniXYZ", "forwardLighting", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_AlphaThreshold( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_ViewOrigin( this ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_DepthScale( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_forwardLighting_omniXYZ::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_forwardLighting_omniXYZ::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_forwardLighting_omniXYZ::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_forwardLighting_omniXYZ::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 4 );
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap" ), 5 );
	}
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_RandomMap" ), 6 );
}

GLShader_forwardLighting_projXYZ::GLShader_forwardLighting_projXYZ( GLShaderManager *manager ):
	GLShader("forwardLighting_projXYZ", "forwardLighting", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_AlphaThreshold( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_ViewOrigin( this ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ShadowMatrix( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_DepthScale( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_forwardLighting_projXYZ::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_forwardLighting_projXYZ::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_forwardLighting_projXYZ::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "LIGHT_PROJ ";
	compileMacros += "TWOSIDED ";
}

void GLShader_forwardLighting_projXYZ::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 4 );
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap0" ), 5 );
	}
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_RandomMap" ), 6 );
}

GLShader_forwardLighting_directionalSun::GLShader_forwardLighting_directionalSun( GLShaderManager *manager ):
	GLShader("forwardLighting_directionalSun", "forwardLighting", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_AlphaThreshold( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_ViewOrigin( this ),
	u_LightDir( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ShadowMatrix( this ),
	u_ShadowParallelSplitDistances( this ),
	u_ModelMatrix( this ),
	u_ViewMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_DepthScale( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_forwardLighting_directionalSun::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_forwardLighting_directionalSun::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_forwardLighting_directionalSun::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "LIGHT_DIRECTIONAL ";
	compileMacros += "TWOSIDED ";
}

void GLShader_forwardLighting_directionalSun::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	//glUniform1i(glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 3);
	//glUniform1i(glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 4);
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap0" ), 5 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap1" ), 6 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap2" ), 7 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap3" ), 8 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap4" ), 9 );
	}
}

GLShader_deferredLighting_omniXYZ::GLShader_deferredLighting_omniXYZ( GLShaderManager *manager ):
	GLShader("deferredLighting_omniXYZ", "deferredLighting", ATTR_POSITION, manager),
	u_ViewOrigin( this ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_LightFrustum( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_FRUSTUM_CLIPPING( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_deferredLighting_omniXYZ::BuildShaderCompileMacros( std::string& compileMacros )
{
	//compileMacros += "TWOSIDED ";
}

void GLShader_deferredLighting_omniXYZ::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 4 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 5 );
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap" ), 6 );
	}
}

GLShader_deferredLighting_projXYZ::GLShader_deferredLighting_projXYZ( GLShaderManager *manager ):
	GLShader("deferredLighting_projXYZ", "deferredLighting", ATTR_POSITION, manager),
	u_ViewOrigin( this ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_LightFrustum( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ShadowMatrix( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_FRUSTUM_CLIPPING( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_deferredLighting_projXYZ::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "LIGHT_PROJ ";
}

void GLShader_deferredLighting_projXYZ::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 4 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 5 );
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap0" ), 6 );
	}
}

GLShader_deferredLighting_directionalSun::GLShader_deferredLighting_directionalSun( GLShaderManager *manager ):
	GLShader("deferredLighting_directionalSun", "deferredLighting", ATTR_POSITION, manager),
	u_ViewOrigin( this ),
	u_LightDir( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightWrapAround( this ),
	u_LightAttenuationMatrix( this ),
	u_LightFrustum( this ),
	u_ShadowTexelSize( this ),
	u_ShadowBlur( this ),
	u_ShadowMatrix( this ),
	u_ShadowParallelSplitDistances( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_ViewMatrix( this ),
	u_UnprojectMatrix( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_FRUSTUM_CLIPPING( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_SHADOWING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_deferredLighting_directionalSun::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "LIGHT_DIRECTIONAL ";
}

void GLShader_deferredLighting_directionalSun::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 3 );
	//glUniform1i(glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 4);
	//glUniform1i(glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 5);
	//if(r_shadows->integer >= SHADOWING_ESM16)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap0" ), 6 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap1" ), 7 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap2" ), 8 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap3" ), 9 );
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap4" ), 10 );
	}
}

GLShader_geometricFill::GLShader_geometricFill( GLShaderManager *manager ):
	GLShader( "geometricFill", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_DiffuseTextureMatrix( this ),
	u_NormalTextureMatrix( this ),
	u_SpecularTextureMatrix( this ),
	u_AlphaThreshold( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_ViewOrigin( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_DepthScale( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this ),
	GLCompileMacro_USE_REFLECTIVE_SPECULAR( this )
{
}

void GLShader_geometricFill::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_geometricFill::BuildShaderFragmentLibNames( std::string& fragmentInlines )
{
	fragmentInlines += "reliefMapping";
}

void GLShader_geometricFill::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_geometricFill::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DiffuseMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_SpecularMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_EnvironmentMap0" ), 3 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_EnvironmentMap1" ), 4 );
}

GLShader_shadowFill::GLShader_shadowFill( GLShaderManager *manager ) :
	GLShader( "shadowFill", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_ColorTextureMatrix( this ),
	u_ViewOrigin( this ),
	u_AlphaThreshold( this ),
	u_LightOrigin( this ),
	u_LightRadius( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_Color( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_LIGHT_DIRECTIONAL( this )
{
}

void GLShader_shadowFill::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_shadowFill::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_reflection::GLShader_reflection( GLShaderManager *manager ):
	GLShader("reflection", "reflection_CB", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_NormalTextureMatrix( this ),
	u_ViewOrigin( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_USE_NORMAL_MAPPING( this )  //,
	//GLCompileMacro_TWOSIDED(this)
{
}

void GLShader_reflection::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_reflection::BuildShaderCompileMacros( std::string& compileMacros )
{
	compileMacros += "TWOSIDED ";
}

void GLShader_reflection::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 1 );
}

GLShader_skybox::GLShader_skybox( GLShaderManager *manager ) :
	GLShader( "skybox", ATTR_POSITION, manager ),
	u_ViewOrigin( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	GLDeformStage( this )
{
}

void GLShader_skybox::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_fogQuake3::GLShader_fogQuake3( GLShaderManager *manager ) :
	GLShader( "fogQuake3", ATTR_POSITION | ATTR_NORMAL, manager ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_Color( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	u_FogDistanceVector( this ),
	u_FogDepthVector( this ),
	u_FogEyeT( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this ),
	GLCompileMacro_EYE_OUTSIDE( this )
{
}

void GLShader_fogQuake3::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_fogQuake3::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_fogGlobal::GLShader_fogGlobal( GLShaderManager *manager ) :
	GLShader( "fogGlobal", ATTR_POSITION, manager ),
	u_ViewOrigin( this ),
	u_ViewMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	u_Color( this ),
	u_FogDistanceVector( this ),
	u_FogDepthVector( this )
{
}

void GLShader_fogGlobal::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 1 );
}

GLShader_heatHaze::GLShader_heatHaze( GLShaderManager *manager ) :
	GLShader( "heatHaze", ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL, manager ),
	u_NormalTextureMatrix( this ),
	u_ViewOrigin( this ),
	u_DeformMagnitude( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_ModelViewMatrixTranspose( this ),
	u_ProjectionMatrixTranspose( this ),
	u_ColorModulate( this ),
	u_Color( this ),
	u_BoneMatrix( this ),
	u_VertexInterpolation( this ),
	GLDeformStage( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this ),
	GLCompileMacro_USE_VERTEX_ANIMATION( this ),
	GLCompileMacro_USE_DEFORM_VERTEXES( this )
{
}

void GLShader_heatHaze::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning vertexAnimation ";

	if(glConfig.driverType == GLDRV_OPENGL3 && r_vboDeformVertexes->integer)
	{
		vertexInlines += "deformVertexes ";
	}
}

void GLShader_heatHaze::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 1 );

	if(r_heatHazeFix->integer && glConfig2.framebufferBlitAvailable && /*glConfig.hardwareType != GLHW_ATI && glConfig.hardwareType != GLHW_ATI_DX10 &&*/ glConfig.driverType != GLDRV_MESA)
	{
		glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ContrastMap" ), 2 );
	}
}

GLShader_screen::GLShader_screen( GLShaderManager *manager ) :
	GLShader( "screen", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this )
{
}

void GLShader_screen::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
}

GLShader_portal::GLShader_portal( GLShaderManager *manager ) :
	GLShader( "portal", ATTR_POSITION, manager ),
	u_ModelViewMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_PortalRange( this )
{
}

void GLShader_portal::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
}

GLShader_toneMapping::GLShader_toneMapping( GLShaderManager *manager ) :
	GLShader( "toneMapping", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this ),
	u_HDRKey( this ),
	u_HDRAverageLuminance( this ),
	u_HDRMaxLuminance( this ),
	GLCompileMacro_BRIGHTPASS_FILTER( this )
{
}

void GLShader_toneMapping::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
}

GLShader_contrast::GLShader_contrast( GLShaderManager *manager ) :
	GLShader( "contrast", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this )
{
}

void GLShader_contrast::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_cameraEffects::GLShader_cameraEffects( GLShaderManager *manager ) :
	GLShader( "cameraEffects", ATTR_POSITION | ATTR_TEXCOORD, manager ),
	u_ColorModulate( this ),
	u_ColorTextureMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_DeformMagnitude( this )
{
}

void GLShader_cameraEffects::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_GrainMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_VignetteMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 3 );
}

GLShader_blurX::GLShader_blurX( GLShaderManager *manager ) :
	GLShader( "blurX", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this ),
	u_DeformMagnitude( this )
{
}

void GLShader_blurX::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_blurY::GLShader_blurY( GLShaderManager *manager ) :
	GLShader( "blurY", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this ),
	u_DeformMagnitude( this )
{
}

void GLShader_blurY::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
}

GLShader_debugShadowMap::GLShader_debugShadowMap( GLShaderManager *manager ) :
	GLShader( "debugShadowMap", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this )
{
}

void GLShader_debugShadowMap::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
}

GLShader_depthToColor::GLShader_depthToColor( GLShaderManager *manager ) :
	GLShader( "depthToColor", ATTR_POSITION | ATTR_NORMAL, manager ),
	u_ModelViewProjectionMatrix( this ),
	u_BoneMatrix( this ),
	GLCompileMacro_USE_VERTEX_SKINNING( this )
{
}

void GLShader_depthToColor::BuildShaderVertexLibNames( std::string& vertexInlines )
{
	vertexInlines += "vertexSkinning ";
}

GLShader_lightVolume_omni::GLShader_lightVolume_omni( GLShaderManager *manager ) :
	GLShader( "lightVolume_omni", ATTR_POSITION, manager ),
	u_ViewOrigin( this ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightAttenuationMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	GLCompileMacro_USE_SHADOWING( this )
{
}

void GLShader_lightVolume_omni::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap" ), 3 );
}

GLShader_deferredShadowing_proj::GLShader_deferredShadowing_proj( GLShaderManager *manager ) :
	GLShader( "deferredShadowing_proj", ATTR_POSITION, manager ),
	u_LightOrigin( this ),
	u_LightColor( this ),
	u_LightRadius( this ),
	u_LightScale( this ),
	u_LightAttenuationMatrix( this ),
	u_ShadowMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	GLCompileMacro_USE_SHADOWING( this )
{
}

void GLShader_deferredShadowing_proj::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapXY" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_AttenuationMapZ" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ShadowMap" ), 3 );
}

GLShader_liquid::GLShader_liquid( GLShaderManager *manager ) :
	GLShader( "liquid", ATTR_POSITION | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL | ATTR_COLOR
#if !defined( COMPAT_Q3A ) && !defined( COMPAT_ET )
	                     | ATTR_LIGHTDIRECTION
#endif
		, manager ),
	u_NormalTextureMatrix( this ),
	u_ViewOrigin( this ),
	u_RefractionIndex( this ),
	u_ModelMatrix( this ),
	u_ModelViewProjectionMatrix( this ),
	u_UnprojectMatrix( this ),
	u_FresnelPower( this ),
	u_FresnelScale( this ),
	u_FresnelBias( this ),
	u_NormalScale( this ),
	u_FogDensity( this ),
	u_FogColor( this ),
	GLCompileMacro_USE_PARALLAX_MAPPING( this )
{
}

void GLShader_liquid::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_PortalMap" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 2 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_NormalMap" ), 3 );
}

GLShader_volumetricFog::GLShader_volumetricFog( GLShaderManager *manager ) :
	GLShader( "volumetricFog", ATTR_POSITION, manager ),
	u_ViewOrigin( this ),
	u_UnprojectMatrix( this ),
	u_ModelViewMatrix( this ),
	u_FogDensity( this ),
	u_FogColor( this )
{
}

void GLShader_volumetricFog::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMapBack" ), 1 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMapFront" ), 2 );
}

GLShader_screenSpaceAmbientOcclusion::GLShader_screenSpaceAmbientOcclusion( GLShaderManager *manager ) :
	GLShader( "screenSpaceAmbientOcclusion", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this )
{
}

void GLShader_screenSpaceAmbientOcclusion::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 1 );
}

GLShader_depthOfField::GLShader_depthOfField( GLShaderManager *manager ) :
	GLShader( "depthOfField", ATTR_POSITION, manager ),
	u_ModelViewProjectionMatrix( this )
{
}

void GLShader_depthOfField::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_CurrentMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 1 );
}

GLShader_motionblur::GLShader_motionblur( GLShaderManager *manager ) :
	GLShader( "motionblur", ATTR_POSITION, manager ),
	u_blurVec( this )
{
}

void GLShader_motionblur::SetShaderProgramUniforms( shaderProgram_t *shaderProgram )
{
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_ColorMap" ), 0 );
	glUniform1i( glGetUniformLocation( shaderProgram->program, "u_DepthMap" ), 1 );
}
