/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: ft_values.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: John Cowan,Paul Pedersen
 *
 */
 

#ifndef XQP_FTVALUES_H
#define XQP_FTVALUES_H

#include "item.h"
#include "ft_options.h"
#include <zorba/rchandle.h>

#include <iostream>
#include <string>
#include <vector>

namespace xqp {


/*______________________________________________________________________
|  
|	'ft_value' encapsulates an xquery full-text search constraint.
|	Refer to "XQuery 1.0 and XPath 2.0 Full-Text"
|	[http://www.w3.org/TR/xquery-full-text/]
|_______________________________________________________________________*/

class ft_value : virtual public SimpleRCObject { };


class ft_or_value : public ft_value
{
protected:
	std::vector<ft_value const*> pv;

public:
	ft_or_value() {}
	~ft_or_value() {}

public:
	void add(ft_value const* ft_p) { pv.push_back(ft_p); }
	uint32_t count() const { return pv.size(); }
	std::vector<ft_value const*>::const_iterator begin()
		{ return pv.begin(); }
	std::vector<ft_value const*>::const_iterator end()
		{ return pv.end(); }

public:
	friend std::ostream& operator<<(std::ostream& os, ft_or_value const&);

};


class ft_and_value : public ft_value
{
protected:
	std::vector<ft_value const*> pv;

public:
	ft_and_value() {}
	~ft_and_value() {}

public:
	void add(ft_value const* ft_p) { pv.push_back(ft_p); }
	uint32_t count() const { return pv.size(); }
	std::vector<ft_value const*>::const_iterator begin()
		{ return pv.begin(); }
	std::vector<ft_value const*>::const_iterator end()
		{ return pv.end(); }

public:
	friend std::ostream& operator<<(std::ostream& os, ft_and_value const&);

};


class ft_mildnot_value : public ft_value
{
protected:
	ft_value const* pos_p;
	std::vector<ft_value const*> neg_pv;

public:
	ft_mildnot_value(
		ft_value const* pos_p,
		ft_value const* neg_p);
	ft_mildnot_value() {}
	~ft_mildnot_value() {}

public:
	void add_negval(ft_value const* ft_p) { neg_pv.push_back(ft_p); }
	uint32_t negval_count() const { return neg_pv.size(); }
	std::vector<ft_value const*>::const_iterator negval_begin()
		{ return neg_pv.begin(); }
	std::vector<ft_value const*>::const_iterator negval_end()
		{ return neg_pv.end(); }

public:
	friend std::ostream& operator<<(std::ostream& os, ft_mildnot_value const&);

};


class ft_unarynot_value : public ft_value
{
protected:
	ft_value const* qphrase_p;

public:
	ft_unarynot_value(ft_value const* ft_p) : qphrase_p(ft_p) {}
	ft_unarynot_value() {}
	~ft_unarynot_value() {}

public:
	ft_value const* get_qphrase() const { return qphrase_p; };
	
public:
	friend std::ostream& operator<<(std::ostream& os, ft_unarynot_value const&);

};


// qphrase = 'qualified phrase'
class ft_qphrase_value : public ft_value
{
public:
	enum ft_anyall_option {
		ft_any,
		ft_any_word,
		ft_all,
		ft_all_words,
		ft_phrase
	};

protected:
	std::vector<std::string> word_v;
	enum ft_anyall_option opt;

public:
	ft_qphrase_value() {}
	~ft_qphrase_value() {}

public:
	enum ft_anyall_option get_opt() const { return opt; }
	void set_opt(enum ft_anyall_option _opt) { opt = _opt; }
	void add(std::string const& s) { word_v.push_back(s); }
	uint32_t count() const { return word_v.size(); }
	std::vector<std::string>::const_iterator begin()
		{ return word_v.begin(); }
	std::vector<std::string>::const_iterator end()
		{ return word_v.end(); }

public:
	friend std::ostream& operator<<(std::ostream& os, ft_qphrase_value const&);

};


} /* namespace xqp */
#endif /* XQP_FTVALUES_H */

