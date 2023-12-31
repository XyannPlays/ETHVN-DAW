/*
 * TempoSyncKnobModel.cpp - adds bpm to ms conversion for knob class
 *
 * Copyright (c) 2005-2007 Danny McRae <khjklujn/at/yahoo.com>
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include <QAction>
#include <QDomElement>

#include "TempoSyncKnobModel.h"
#include "Engine.h"
#include "Song.h"


namespace lmms
{


TempoSyncKnobModel::TempoSyncKnobModel( const float _val, const float _min,
				const float _max, const float _step,
				const float _scale, Model * _parent,
				const QString & _display_name ) :
	FloatModel( _val, _min, _max, _step, _parent, _display_name ),
	m_tempoSyncMode( SyncMode::None ),
	m_tempoLastSyncMode( SyncMode::None ),
	m_scale( _scale ),
	m_custom( _parent )
{
	connect( Engine::getSong(), SIGNAL(tempoChanged(lmms::bpm_t)),
			this, SLOT(calculateTempoSyncTime(lmms::bpm_t)),
			Qt::DirectConnection );
}




void TempoSyncKnobModel::setTempoSync( QAction * _item )
{
	setTempoSync( static_cast<SyncMode>(_item->data().toInt()) );
}




void TempoSyncKnobModel::setTempoSync( SyncMode _note_type )
{
	setSyncMode( _note_type );
	Engine::getSong()->setModified();
}




void TempoSyncKnobModel::calculateTempoSyncTime( bpm_t _bpm )
{
	float conversionFactor = 1.0;
	
	if( m_tempoSyncMode != SyncMode::None )
	{
		switch( m_tempoSyncMode )
		{
			case SyncMode::Custom:
				conversionFactor = 
			static_cast<float>( m_custom.getDenominator() ) /
			static_cast<float>( m_custom.getNumerator() );
				break;
			case SyncMode::DoubleWholeNote:
				conversionFactor = 0.125;
				break;
			case SyncMode::WholeNote:
				conversionFactor = 0.25;
				break;
			case SyncMode::HalfNote:
				conversionFactor = 0.5;
				break;
			case SyncMode::QuarterNote:
				conversionFactor = 1.0;
				break;
			case SyncMode::EighthNote:
				conversionFactor = 2.0;
				break;
			case SyncMode::SixteenthNote:
				conversionFactor = 4.0;
				break;
			case SyncMode::ThirtysecondNote:
				conversionFactor = 8.0;
				break;
			default: ;
		}
		bool journalling = testAndSetJournalling( false );
		float oneUnit = 60000.0 / ( _bpm * conversionFactor * m_scale );
		setValue( oneUnit * maxValue() );
		setJournalling( journalling );
	}

	if( m_tempoSyncMode != m_tempoLastSyncMode )
	{
		emit syncModeChanged( m_tempoSyncMode );
		m_tempoLastSyncMode = m_tempoSyncMode;
	}
	else if (m_tempoSyncMode == SyncMode::Custom)
	{
		emit syncModeChanged(m_tempoSyncMode);
	}
}




void TempoSyncKnobModel::saveSettings( QDomDocument & _doc, QDomElement & _this,
							const QString & _name )
{
	_this.setAttribute( _name + "_syncmode", (int) syncMode() );
	m_custom.saveSettings( _doc, _this, _name );
	FloatModel::saveSettings( _doc, _this, _name );
}




void TempoSyncKnobModel::loadSettings( const QDomElement & _this,
							const QString & _name )
{
	FloatModel::loadSettings( _this, _name );
	m_custom.loadSettings( _this, _name );
	setSyncMode( ( SyncMode ) _this.attribute( _name + "_syncmode" ).toInt() );
}




void TempoSyncKnobModel::setSyncMode( SyncMode _new_mode )
{
	if( m_tempoSyncMode != _new_mode )
	{
		m_tempoSyncMode = _new_mode;
		if( _new_mode == SyncMode::Custom )
		{
			connect( &m_custom, SIGNAL(dataChanged()),
					this, SLOT(updateCustom()),
					Qt::DirectConnection );
		}
	}
	calculateTempoSyncTime( Engine::getSong()->getTempo() );
}




void TempoSyncKnobModel::setScale( float _new_scale )
{
	m_scale = _new_scale;
	calculateTempoSyncTime( Engine::getSong()->getTempo() );
	emit scaleChanged( _new_scale );
}




void TempoSyncKnobModel::updateCustom()
{
	setSyncMode( SyncMode::Custom );
}


} // namespace lmms
