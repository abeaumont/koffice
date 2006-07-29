/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <qdom.h>
#include <kdebug.h>

#include "events.h"

namespace KFormDesigner {

Connection::Connection(const QString &sender, const QString &signal,
	 const QString &receiver, const QString &slot)
{
	m_sender = sender;
	m_signal = signal;
	m_receiver = receiver;
	m_slot = slot;
}

 ///////////////////////////////////////

ConnectionBuffer::ConnectionBuffer()
{
	setAutoDelete(true);
}

void
ConnectionBuffer::fixName(const QString &oldName, const QString &newName)
{
	for(Connection *c = first(); c; c = next())
	{
		if(c->sender() == oldName)
			c->setSender(newName);
		if(c->receiver() == oldName)
			c->setReceiver(newName);
	}
}

ConnectionBuffer*
ConnectionBuffer::allConnectionsForWidget(const QString &widget)
{
	ConnectionBuffer *list = new ConnectionBuffer();
	list->setAutoDelete(false); // or it will delete all our connections
	for(Connection *c = first(); c; c = next())
	{
		if((c->sender() == widget) || (c->receiver() == widget))
			list->append(c);
	}

	return list;
}

void
ConnectionBuffer::save(QDomNode &parentNode)
{
	if(isEmpty())
		return;

	QDomDocument domDoc = parentNode.ownerDocument();
	QDomElement connections;
	if(!parentNode.namedItem("connections").isNull())
		connections = parentNode.namedItem("connections").toElement();
	else
		connections = domDoc.createElement("connections");
	parentNode.appendChild(connections);

	for(Connection *c = first(); c; c = next())
	{
		QDomElement connection = domDoc.createElement("connection");
		connection.setAttribute("language", "C++");
		connections.appendChild(connection);

		QDomElement sender = domDoc.createElement("sender");
		connection.appendChild(sender);
		QDomText senderText = domDoc.createTextNode(c->sender());
		sender.appendChild(senderText);

		QDomElement signal = domDoc.createElement("signal");
		connection.appendChild(signal);
		QDomText signalText = domDoc.createTextNode(c->signal());
		signal.appendChild(signalText);

		QDomElement receiver = domDoc.createElement("receiver");
		connection.appendChild(receiver);
		QDomText receiverText = domDoc.createTextNode(c->receiver());
		receiver.appendChild(receiverText);

		QDomElement slot = domDoc.createElement("slot");
		connection.appendChild(slot);
		QDomText slotText = domDoc.createTextNode(c->slot());
		slot.appendChild(slotText);
	}
}

void
ConnectionBuffer::saveAllConnectionsForWidget(const QString &widget, QDomNode parentNode)
{
	ConnectionBuffer *buff = allConnectionsForWidget(widget);
	buff->save(parentNode);
	delete buff;
}

void
ConnectionBuffer::load(QDomNode node)
{
	for(QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		Connection *conn = new Connection();
		conn->setSender(n.namedItem("sender").toElement().text());
		conn->setSignal(n.namedItem("signal").toElement().text());
		conn->setReceiver(n.namedItem("receiver").toElement().text());
		conn->setSlot(n.namedItem("slot").toElement().text());
		append(conn);
	}
}

void
ConnectionBuffer::removeAllConnectionsForWidget(const QString &widget)
{
	for(Connection *c = first(); c; c = next())
	{
		if((c->sender() == widget) || (c->receiver() == widget))
			removeRef(c);
	}
}

}

//#include "events.moc"
