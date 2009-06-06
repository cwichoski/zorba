#include "debugger_communication.h"
#include "synchronous_logger.h"
#include "utils.h"
#include "message_factory.h"

#include <zorbaerrors/Assert.h>
#include <memory>

#ifdef WIN32
#define sleep(s) Sleep(1000*s)
#endif

zorba::DebuggerCommunicator::DebuggerCommunicator( unsigned short aCommandPort, unsigned short aEventPort )
:
m_eventSocket(0), m_commandSocket(0)
{
	TCPServerSocket* srvSocket = new TCPServerSocket(aCommandPort);
	m_commandSocket = srvSocket->accept();
	ZORBA_ASSERT(m_commandSocket);
#ifndef NDEBUG
	synchronous_logger::clog << "[Server Thread] Client connected\n";
#endif
	for ( unsigned int i = 0; i < 3 && ! m_eventSocket; i++ ) {
		try {
			//Wait one second before trying to reconnect
			sleep(1);
			//Connect the client to the event server
			m_eventSocket = new TCPSocket( m_commandSocket->getForeignAddress(), aEventPort );
			break;
		} catch ( DebuggerSocketException &e )  {
			if ( i < 2 ) continue;
			synchronous_logger::cerr << "[Server Thread] Couldn't connect to the debugger server event: \n";
			synchronous_logger::cerr << "[Server Thread] " <<  e.what() << "\n";
			return;
		}
	}
}

void zorba::DebuggerCommunicator::handshake()
{
	ZorbaArrayAutoPointer<Byte> msg(new Byte[11]);
	try
	{
		m_commandSocket->recv( msg.get(), 11 );
		m_commandSocket->send( msg.get(), 11 );
	} catch( DebuggerSocketException &e ) {
		synchronous_logger::clog << "[Server Thread] handshake failed\n";
		synchronous_logger::clog << "[Server Thread]" << e.what() << "\n";
	}
}

zorba::AbstractCommandMessage* zorba::DebuggerCommunicator::handleTCPClient()
{
	ZorbaArrayAutoPointer<Byte> lByteMessage;
	AbstractMessage* lMessage;
	ReplyMessage* lReplyMessage;
	Length length;
	try
	{
		lMessage = MessageFactory::buildMessage( m_commandSocket );
		AbstractCommandMessage* lCommandMessage(dynamic_cast< AbstractCommandMessage * >(lMessage));
		if ( lCommandMessage )
		{
			return lCommandMessage;
		} else {
#ifndef NDEBUG
			synchronous_logger::clog << "[Server Thread] Received something wrong\n";
#endif
			//If something goes wrong, buildMessage() receive a Reply Message containing the error description
			//Send it back to the client right away
			lReplyMessage = dynamic_cast<ReplyMessage *>(lMessage);
			if( lReplyMessage != 0 ){
				lByteMessage.reset(lReplyMessage->serialize( length ));
				m_commandSocket->send( lByteMessage.get(), length );
			} else {
				synchronous_logger::cerr << "[Server Thread] Internal error occured. Couldn't send the error message.\n";
			}
			return NULL;
		}
	} catch ( std::exception &e ) {
#ifndef NDEBUG
		synchronous_logger::clog << "[Server Thread] The connection with the client is closed\n";
		synchronous_logger::clog <<  e.what() << "\n";
#endif
	} catch(...) {
		synchronous_logger::clog << "[Server Thread] unknown exception\n";
	}
	return NULL;
}

void zorba::DebuggerCommunicator::sendEvent( AbstractCommandMessage* aMessage )
{
	Length length;
	ZorbaArrayAutoPointer<Byte> lMessage(aMessage->serialize(length));
	try
	{
#ifndef NDEBUG
		synchronous_logger::clog << "[Server Thread] send an event: ";
		switch ( aMessage->getCommand() )
		{
		case STARTED:
			synchronous_logger::cerr << "started\n";
			break;
		case TERMINATED:
			synchronous_logger::cerr << "terminated\n";
			break;
		case SUSPENDED:
			synchronous_logger::cerr << "suspended\n";
			break;
		case RESUMED:
			synchronous_logger::cerr << "resumed\n";
			break;
		case EVALUATED:
			synchronous_logger::cerr << "evaluated\n";
			break;
		}
#endif
		m_eventSocket->send( lMessage.get(), length );
#ifndef NDEBUG
		synchronous_logger::clog << "[Server Thread] event sent\n";
#endif
	} catch( DebuggerSocketException &e ) {
		synchronous_logger::cerr << e.what() << "\n";
	}
}

zorba::DebuggerCommunicator::~DebuggerCommunicator()
{
	delete m_eventSocket;
	delete m_commandSocket;
}

void zorba::DebuggerCommunicator::sendReplyMessage( AbstractCommandMessage* aMessage )
{
	Length length;
	ZorbaArrayAutoPointer<Byte> lByteMessage;
	std::auto_ptr<ReplyMessage> lReply(aMessage->getReplyMessage());
	lByteMessage.reset(lReply->serialize( length ));
	m_commandSocket->send( lByteMessage.get(), length );
}