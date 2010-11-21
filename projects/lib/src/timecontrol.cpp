/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "timecontrol.h"
#include <QStringList>


TimeControl::TimeControl()
	: m_movesPerTc(0),
	  m_timePerTc(0),
	  m_timePerMove(0),
	  m_increment(0),
	  m_timeLeft(0),
	  m_movesLeft(0),
	  m_maxDepth(0),
	  m_nodeLimit(0),
	  m_lastMoveTime(0),
	  m_expiryMargin(0),
	  m_expired(false)
{
}

TimeControl::TimeControl(const QString& str)
	: m_movesPerTc(0),
	  m_timePerTc(0),
	  m_timePerMove(0),
	  m_increment(0),
	  m_timeLeft(0),
	  m_movesLeft(0),
	  m_maxDepth(0),
	  m_nodeLimit(0),
	  m_lastMoveTime(0),
	  m_expiryMargin(0),
	  m_expired(false)
{
	QStringList list = str.split('+');

	// increment
	if (list.size() == 2)
	{
		int inc = (int)(list[1].toDouble() * 1000);
		if (inc >= 0)
			setIncrement(inc);
	}

	list = list[0].split('/');
	QString strTime;

	// moves per tc
	if (list.size() == 2)
	{
		int nmoves = list[0].toInt();
		if (nmoves >= 0)
			setMovesPerTc(nmoves);
		strTime = list[1];
	}
	else
		strTime = list[0];

	// time per tc
	int ms = 0;
	list = strTime.split(':');
	if (list.size() == 2)
		ms = (int)(list[0].toDouble() * 60000 + list[1].toDouble() * 1000);
	else
		ms = (int)(list[0].toDouble() * 1000);

	if (ms > 0)
		setTimePerTc(ms);
}

bool TimeControl::operator==(const TimeControl& other) const
{
	if (m_movesPerTc == other.m_movesPerTc
	&&  m_timePerTc == other.m_timePerTc
	&&  m_timePerMove == other.m_timePerMove
	&&  m_increment == other.m_increment
	&&  m_maxDepth == other.m_maxDepth
	&&  m_nodeLimit == other.m_nodeLimit)
		return true;
	return false;
}

bool TimeControl::isValid() const
{
	if (m_movesPerTc < 0
	||  m_timePerTc < 0
	||  m_timePerMove < 0
	||  m_increment < 0
	||  m_maxDepth < 0
	||  m_nodeLimit < 0
	||  m_expiryMargin < 0
	||  (m_timePerTc == m_timePerMove))
		return false;
	return true;
}

QString TimeControl::toString() const
{
	if (!isValid())
		return QString();

	if (m_timePerMove != 0)
		return QString("%1/move").arg((double)m_timePerMove / 1000);

	QString str;
	if (m_movesPerTc > 0)
		str += QString::number(m_movesPerTc) + "/";
	str += QString::number((double)m_timePerTc / 1000);

	if (m_increment > 0)
		str += QString("+") + QString::number((double)m_increment / 1000);
	return str;
}

void TimeControl::initialize()
{
	m_expired = false;
	m_lastMoveTime = 0;

	if (m_timePerTc != 0)
	{
		m_timeLeft = m_timePerTc;
		m_movesLeft = m_movesPerTc;
	}
	else if (m_timePerMove != 0)
		m_timeLeft = m_timePerMove;
}

int TimeControl::timePerTc() const
{
	return m_timePerTc;
}

int TimeControl::movesPerTc() const
{
	return m_movesPerTc;
}

int TimeControl::timeIncrement() const
{
	return m_increment;
}

int TimeControl::timePerMove() const
{
	return m_timePerMove;
}

int TimeControl::timeLeft() const
{
	return m_timeLeft;
}

int TimeControl::movesLeft() const
{
	return m_movesLeft;
}

int TimeControl::maxDepth() const
{
	return m_maxDepth;
}

int TimeControl::nodeLimit() const
{
	return m_nodeLimit;
}

int TimeControl::expiryMargin() const
{
	return m_expiryMargin;
}

void TimeControl::setTimePerTc(int timePerTc)
{
	Q_ASSERT(timePerTc >= 0);
	m_timePerTc = timePerTc;
}

void TimeControl::setMovesPerTc(int movesPerTc)
{
	Q_ASSERT(movesPerTc >= 0);
	m_movesPerTc = movesPerTc;
}

void TimeControl::setIncrement(int increment)
{
	Q_ASSERT(increment >= 0);
	m_increment = increment;
}

void TimeControl::setTimePerMove(int timePerMove)
{
	Q_ASSERT(timePerMove >= 0);
	m_timePerMove = timePerMove;
}

void TimeControl::setTimeLeft(int timeLeft)
{
	m_timeLeft = timeLeft;
}

void TimeControl::setMovesLeft(int movesLeft)
{
	Q_ASSERT(movesLeft >= 0);
	m_movesLeft = movesLeft;
}

void TimeControl::setMaxDepth(int maxDepth)
{
	Q_ASSERT(maxDepth >= 0);
	m_maxDepth = maxDepth;
}

void TimeControl::setNodeLimit(int limit)
{
	Q_ASSERT(limit >= 0);
	m_nodeLimit = limit;
}

void TimeControl::setExpiryMargin(int expiryMargin)
{
	Q_ASSERT(expiryMargin >= 0);
	m_expiryMargin = expiryMargin;
}

void TimeControl::startTimer()
{
	m_time.start();
}

void TimeControl::update()
{
	m_lastMoveTime = m_time.elapsed();
	if (m_lastMoveTime > m_timeLeft + m_expiryMargin)
		m_expired = true;

	if (m_timePerMove != 0)
		setTimeLeft(m_timePerMove);
	else
	{
		setTimeLeft(m_timeLeft + m_increment - m_lastMoveTime);
		
		if (m_movesPerTc > 0)
		{
			setMovesLeft(m_movesLeft - 1);
			
			// Restart the time control
			if (m_movesLeft == 0)
			{
				setMovesLeft(m_movesPerTc);
				setTimeLeft(m_timePerTc + m_timeLeft);
			}
		}
	}
}

int TimeControl::lastMoveTime() const
{
	return m_lastMoveTime;
}

bool TimeControl::expired() const
{
	return m_expired;
}
