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

#ifndef ROCKETCIRCLEMENU_H
#define ROCKETCIRCLEMENU_H

#include <Rocket/Core.h>
#include <Rocket/Core/Element.h>
#include <Rocket/Controls.h>
#include <Rocket/Controls/DataSourceListener.h>
#include <Rocket/Controls/DataSource.h>
#include "client.h"
#include "rocket.h"

class RocketCircleMenu : public Rocket::Core::Element, public Rocket::Controls::DataSourceListener
{
public:
	RocketCircleMenu( const Rocket::Core::String &tag ) : Rocket::Core::Element( tag ), dirty_query( false ), dirty_layout( false ), init( false ), radius( 10 ), formatter( NULL ), data_source( NULL )
	{
	}

	void SetDataSource( const Rocket::Core::String &dsn )
	{
		ParseDataSource( data_source, data_table, dsn );
		data_source->AttachListener( this );
		dirty_query = true;
	}

	bool GetIntrinsicDimensions( Rocket::Core::Vector2f &dimensions )
	{
		const Rocket::Core::Property *property;

		dimensions.x = dimensions.y = -1;

		property = GetProperty( "width" );

		// Absolute unit. We can use it as is
		if ( property->unit & Rocket::Core::Property::ABSOLUTE_UNIT )
		{
			dimensions.x = property->value.Get<float>();
		}
		else
		{
			float base_size = 0;
			Rocket::Core::Element *parent = this;

			while ( parent = parent->GetParentNode() )
			{
				if ( ( base_size = parent->GetOffsetWidth() ) != 0 )
				{
					dimensions.x = ResolveProperty( "width", base_size );
					break;
				}
			}
		}

		property = GetProperty( "height" );
		if ( property->unit & Rocket::Core::Property::ABSOLUTE_UNIT )
		{
			dimensions.y = property->value.Get<float>();
		}
		else
		{
			float base_size = 0;
			Rocket::Core::Element *parent = this;

			while ( parent = parent->GetParentNode() )
			{
				if ( ( base_size = parent->GetOffsetHeight() ) != 0 )
				{
					dimensions.y = ResolveProperty( "height", base_size );
					break;
				}
			}
		}
		// Return the calculated dimensions. If this changes the size of the element, it will result in
		// a 'resize' event which is caught below and will regenerate the geometry.

		if ( this->dimensions != dimensions )
		{
			this->dimensions = dimensions;
			dirty_layout = true;
		}

		return true;
	}

	void OnAttributeChange( const Rocket::Core::AttributeNameList &changed_attributes )
	{
		if ( changed_attributes.find( "source" ) != changed_attributes.end() )
		{
			SetDataSource( GetAttribute<Rocket::Core::String>( "source", "" ) );
		}

		if ( changed_attributes.find( "fields" ) != changed_attributes.end() )
		{
			csvFields = GetAttribute<Rocket::Core::String>( "fields", "" );
			Rocket::Core::StringUtilities::ExpandString( fields, csvFields );
			dirty_query = true;
		}

		if ( changed_attributes.find( "formatter" ) != changed_attributes.end() )
		{
			formatter = Rocket::Controls::DataFormatter::GetDataFormatter( GetAttribute( "formatter" )->Get<Rocket::Core::String>() );
			dirty_query = true;
		}
	}

	void OnPropertyChange( const Rocket::Core::PropertyNameList &changed_properties )
	{
		if ( changed_properties.find( "width" ) != changed_properties.end() || changed_properties.find( "height" ) != changed_properties.end() )
		{
			dirty_layout = true;
		}

		if ( changed_properties.find( "radius" ) != changed_properties.end() )
		{
			radius = GetProperty( "radius" )->Get<int>();
		}

	}

	void OnRowAdd( Rocket::Controls::DataSource *data_source, const Rocket::Core::String &table, int first_row_added, int num_rows_added )
	{
		dirty_query = true;
	}

	void OnRowChange( Rocket::Controls::DataSource *data_source, const Rocket::Core::String &table, int first_row_changed, int num_rows_changed )
	{
		dirty_query = true;
	}

	void OnRowChange( Rocket::Controls::DataSource *data_source, const Rocket::Core::String &table )
	{
		dirty_query = true;
	}

	void OnRowRemove( Rocket::Controls::DataSource *data_source, const Rocket::Core::String &table, int first_row_removed, int num_rows_removed )
	{
		dirty_query = true;
	}



	void OnUpdate( void )
	{
		if ( !init )
		{
			AddCancelbutton();
		}

		if ( dirty_query )
		{
			dirty_query = false;

			RemoveAllChildren();
			AddCancelbutton();

			Rocket::Controls::DataQuery query( data_source, data_table, csvFields, 0, -1 );
			int index = 0;

			while ( query.NextRow() )
			{
				Rocket::Core::StringList raw_data;
				Rocket::Core::String out;
				for ( size_t i = 0; i < fields.size(); ++i )
				{
					raw_data.push_back( query.Get<Rocket::Core::String>( fields[ i ], "" ) );
				}

				raw_data.push_back( va( "%d", index++ ) );

				if ( formatter )
				{
					formatter->FormatData( out, raw_data );
				}
				else
				{
					for ( size_t i = 0; i < raw_data.size(); ++i )
					{
						if ( i > 0 )
						{
							out.Append( "," );
						}

						out.Append( raw_data[ i ] );
					}
				}

				Rocket::Core::Factory::InstanceElementText( this, out );
			}

			LayoutChildren();
		}
	}
protected:
	void LayoutChildren( void )
	{
		dirty_layout = false;

		int numChildren = 0;
		float width, height;
		Rocket::Core::Element *child;
		Rocket::Core::Vector2f offset = GetRelativeOffset();

		// First child is the cancel button. It should go in the center.
		child = GetFirstChild();
		width = child->GetOffsetWidth();
		height = child->GetOffsetHeight();
		child->SetProperty( "position", "absolute" );
		child->SetProperty( "top", va( "%fpx", offset.y + ( dimensions.y / 2 ) - ( height / 2 ) ) );
		child->SetProperty( "left", va( "%fpx", offset.x + ( dimensions.x / 2 ) - ( width / 2 ) ) );

		// No other children
		if ( ( numChildren = GetNumChildren() ) <= 1 )
		{
			return;
		}

		int angle = 360 /  ( numChildren - 1 );

		// Rest are the circular buttons
		for ( int i = 1; i < numChildren; ++i )
		{
			child = GetChild( i );
			width = child->GetOffsetWidth();
			height = child->GetOffsetHeight();
			float y = sin( angle * ( i - 1 ) * ( M_PI / 180.0f ) ) * radius;
			float x = cos( angle * ( i - 1 ) * ( M_PI / 180.0f ) ) * radius;

			child->SetProperty( "position", "absolute" );
			child->SetProperty( "left", va( "%fpx", ( dimensions.x / 2 ) - ( width / 2 ) + offset.x + x  ) );
			child->SetProperty( "top", va( "%fpx", ( dimensions.y / 2 ) - ( height / 2 ) + offset.y + y ) );
		}
	}

private:

	void AddCancelbutton( void )
	{
		init = true;
		Rocket::Core::Factory::InstanceElementText( this, va( "<button onClick=\"hide %s\">Cancel</button>", GetOwnerDocument()->GetId().CString() ) );
		GetFirstChild()->SetClass( "cancelButton", true );
	}

	bool dirty_query;
	bool dirty_layout;
	bool init;
	int radius;
	Rocket::Controls::DataFormatter *formatter;
	Rocket::Controls::DataSource *data_source;

	Rocket::Core::String data_table;
	Rocket::Core::String csvFields;
	Rocket::Core::StringList fields;
	Rocket::Core::Vector2f dimensions;
};

#endif
