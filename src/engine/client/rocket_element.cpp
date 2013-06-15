	/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 2012 Unvanquished Developers

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

#include "rocketElement.h"
#include <Rocket/Core/Core.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/ElementInstancer.h>
#include <Rocket/Core/ElementInstancerGeneric.h>
extern "C"
{
#include "client.h"
}

extern Rocket::Core::Element *activeElement;

// File contains support code for custom rocket elements

void Rocket_GetElementTag( char *tag, int length )
{
	if ( activeElement )
	{
		Q_strncpyz( tag, activeElement->GetTagName().CString(), length );
	}
}

void Rocket_SetElementDimensions( float x, float y )
{
	if ( activeElement )
	{
		static_cast<RocketElement*>(activeElement)->SetDimensions( x, y );
	}
}

void Rocket_RegisterElement( const char *tag )
{
	Rocket::Core::Factory::RegisterElementInstancer( tag, new Rocket::Core::ElementInstancerGeneric< RocketElement >() )->RemoveReference();
}

void Rocket_SetInnerRML( const char *name, const char *id, const char *RML )
{
	if ( ( !*name || !*id ) && activeElement )
	{
		if ( activeElement->GetInnerRML() != RML )
		{
			activeElement->SetInnerRML( RML );
		}
	}
	else
	{
		Rocket::Core::ElementDocument *document = menuContext->GetDocument( name );
		if ( document )
		{
			Rocket::Core::Element *e = document->GetElementById( id );

			if ( e )
			{
				if ( e->GetInnerRML() != RML )
				{
					e->SetInnerRML( RML );
				}
			}
		}
	}
}


void Rocket_GetAttribute( const char *name, const char *id, const char *attribute, char *out, int length )
{
	if ( ( !*name || !*id ) && activeElement )
	{
		Q_strncpyz( out, activeElement->GetAttribute< Rocket::Core::String >( attribute, "" ).CString(), length );
	}
	else
	{
		Rocket::Core::ElementDocument *document = menuContext->GetDocument( name );

		if ( document )
		{
			Q_strncpyz( out, document->GetElementById( id )->GetAttribute< Rocket::Core::String >( attribute, "" ).CString(), length );
		}
	}
}

void Rocket_SetAttribute( const char *name, const char *id, const char *attribute, const char *value )
{
	if ( ( !*name && !*id ) && activeElement )
	{
		activeElement->SetAttribute( attribute, value );
	}
	else
	{
		Rocket::Core::ElementDocument *document = name[0] ? menuContext->GetDocument( name ) : menuContext->GetFocusElement()->GetOwnerDocument();

		if ( document )
		{
			Rocket::Core::Element *element = document->GetElementById( id );

			if ( element )
			{
				element->SetAttribute( attribute, value );
			}
		}
	}
}

void Rocket_GetElementAbsoluteOffset( float *x, float *y )
{
	if ( activeElement )
	{
		Rocket::Core::Vector2f position = activeElement->GetAbsoluteOffset();
		*x = position.x;
		*y = position.y;
	}
	else
	{
		*x = -1;
		*y = -1;
	}
}

void Rocket_GetProperty( const char *name, void *out, int len, rocketVarType_t type )
{
	if ( activeElement )
	{
		switch ( type )
		{
			case ROCKET_STRING:
			{
				char *string = ( char * ) out;
				Q_strncpyz( string, activeElement->GetProperty<Rocket::Core::String>( name ).CString(), len );
				return;
			}

			case ROCKET_FLOAT:
			{
				float *f = ( float * ) out;

				if ( len != sizeof( float ) )
				{
					return;
				}

				*f = activeElement->GetProperty<float>( name );
				return;
			}

			case ROCKET_INT:
			{
				int *i = ( int * ) out;

				if ( len != sizeof( int ) )
				{
					return;
				}

				*i = activeElement->GetProperty<int>( name );
				return;
			}

			case ROCKET_COLOR:
			{
				vec_t *outColor = ( vec_t * ) out;

				if ( len != sizeof ( vec4_t ) )
				{
					return;
				}

				Rocket::Core::Colourb color = activeElement->GetProperty<Rocket::Core::Colourb>( name );
				outColor[ 0 ] = color.red, outColor[ 1 ] = color.green, outColor[ 2 ] = color.blue, outColor[ 3 ] = color.alpha;
				return;
			}
		}
	}
}

void Rocket_SetClass( const char *in, qboolean activate )
{
	activeElement->SetClass( in, static_cast<bool>( activate ) );
}
