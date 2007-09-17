/*
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: David Graf (david.graf@28msec.com)
 *
 */

#include "util/rchandle.h"

#include "simple_temp_seq.h"

namespace xqp
{
	typedef rchandle<BasicIterator> Iterator_t;

	SimpleTempSeq::SimpleTempSeq ( Iterator_t iterator )
	{
		this->loc = iterator->loc;
		Item_t curItem = iterator->next();
		while ( curItem != NULL )
		{
			this->items.push_back ( curItem );
			curItem = iterator->next();
		}
	}
	SimpleTempSeq::~SimpleTempSeq()
	{	}

	Iterator_t SimpleTempSeq::getIterator()
	{
		return new SimpleTempSeq::Iterator ( this->loc, &this->items );
	}
	Iterator_t SimpleTempSeq::getIterator ( int32_t startPos, int32_t endPos, bool streaming )
	{
		return rchandle<BasicIterator> ( NULL );
	}
	Iterator_t SimpleTempSeq::getIterator ( int32_t startPos, Iterator_t function, const std::vector<var_iterator>& vars, bool streaming )
	{
		return rchandle<BasicIterator> ( NULL );
	}
	Iterator_t SimpleTempSeq::getIterator ( const std::vector<int32_t>& positions, bool streaming )
	{
		return rchandle<BasicIterator> ( NULL );
	}
	Iterator_t SimpleTempSeq::getIterator ( Iterator_t positions, bool streaming )
	{
		return rchandle<BasicIterator> ( NULL );
	}
	Item_t SimpleTempSeq::getItem ( int32_t position )
	{
		if ( int32_t ( this->items.size() ) > position )
		{
			return this->items[position];
		}
		else
		{
			return NULL;
		}
	}
	void SimpleTempSeq::purge()
	{

	}
	void SimpleTempSeq::purgeUpTo ( int32_t upTo )
	{

	}
	void SimpleTempSeq::purgeItem ( const std::vector<int32_t>& positions )
	{

	}
	void SimpleTempSeq::purgeItem ( int32_t position )
	{

	}
	bool SimpleTempSeq::empty()
	{
		return this->items.size() == 0;
	}

	Item_t SimpleTempSeq::Iterator::nextImpl()
	{
		this->curPos++;
		switch ( this->borderType )
		{
			case none:
				if ( this->curPos < int32_t ( this->items->size() ) )
					return ( *this->items ) [this->curPos];
				break;
			case startEnd:
				if ( this->curPos < this->endPos )
					return ( *this->items ) [this->curPos];
				break;
			case specificPositions:
				if ( this->curPos < int32_t ( this->positions.size() ) )
					return ( *this->items ) [this->positions[this->curPos]];
				break;
		}
		return NULL;
	}

	void SimpleTempSeq::Iterator::resetImpl()
	{
		switch ( this->borderType )
		{
			case startEnd:
				this->curPos = this->startPos - 2;
				break;
			case none:
			case specificPositions:
				this->curPos = -1;
				break;
		}
	}

	void SimpleTempSeq::Iterator::releaseResourcesImpl()
	{
	}

} /* namespace xqp */
