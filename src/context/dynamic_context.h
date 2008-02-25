/**
 *
 * @copyright
 * ========================================================================
 *	Copyright 2007 FLWOR Foundation
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *	
 *		http://www.apache.org/licenses/LICENSE-2.0
 *	
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 * ========================================================================
 *
 * @author John Cowan,Paul Pedersen (pcp071098@yahoo.com)
 * @file context/dynamic_context.h
 *
 */

#ifndef XQP_DYNAMIC_CONTEXT_H
#define XQP_DYNAMIC_CONTEXT_H

//#include "context/common.h"
//#include "context/context.h"
//#include "context/context_impl.h"
//#include "functions/library.h"
//#include "store/api/item.h"
#include <zorba/rchandle.h>
#include "util/hashmap.h"
#include <zorba/representations.h>
#include "common/shared_types.h"

namespace xqp {

//class itemstore;
class static_context;
class Iterator;
class Item;
typedef rchandle<Item>    Item_t;

class dynamic_context// : public context
{
protected:

	typedef union
	{
		Iterator* var_iterator;
	}
  dctx_value_t;

protected:
	static bool static_init;

	dynamic_context	      * parent;

	hashmap<dctx_value_t>   keymap;
	//struct ::tm	            execution_date_time;
  Item_t                  execution_date_time_item;
	long										execution_timezone_seconds;
	int			                implicit_timezone;
	xqp_string		          default_collection_uri;//default URI for fn:collection()

protected:
  xqp_string qname_internal_key (Item_t qname) const;
  xqp_string qname_internal_key (xqp_string default_ns, xqp_string prefix, xqp_string local) const;
  xqp_string qname_internal_key (xqp_string default_ns, xqp_string qname) const;

  Iterator* lookup_var_iter (xqp_string key) const;

	Item_t		ctxt_item;
	unsigned long		ctxt_position;
	//+context size is determined by fn:last() at runtime

public:
	static void init();

	dynamic_context(dynamic_context *parent=NULL);
	~dynamic_context();

  xqp_string expand_varname(static_context	*sctx, xqp_string qname) const;

public:
	Item_t context_item() const;
	unsigned long	context_position();

	xqtref_t context_item_type() const;

	void set_context_item(Item_t, unsigned long position);
	void set_context_item_type(xqtref_t );

  #if 0
  // return the value of a variable by QName
	PlanIter_t var_value(const Item*) const;

	const Item& default_element_type_namespace() const;
	void set_default_element_type_namespace(Item&);
	PlanIter_t namespaces() const;
	void add_namespace(Item&);
  #endif


//daniel: get the function directly from library object
//	const function* get_function(qnamekey_t key) { return lib->get(key); }

	void		set_execution_date_time(struct ::tm t, long tz_seconds);
	Item_t	get_execution_date_time();
	void		set_implicit_timezone(long tzone_seconds);
	long		get_implicit_timezone();

	void				add_variable(xqp_string varname, Iterator* var_iterator);
	Iterator_t	get_variable(xqp_string varname);

	xqp_string get_default_collection();
	void set_default_collection(xqp_string default_collection_uri);

};


}	/* namespace xqp */
#endif /*	XQP_DYNAMIC_CONTEXT_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
