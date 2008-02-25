/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: expr.cpp.cpp,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *  Copyright 2006-2007 FLWOR Foundation.
 *
 *  Author: Paul Pedersen
 *
 */
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "compiler/expression/expr.h"
#include "system/globalenv.h"
#include "context/namespace_context.h"
#include "functions/function.h"
#include "compiler/parser/parse_constants.h"
#include "compiler/parsetree/parsenodes.h"
#include "util/Assert.h"
#include "util/tracer.h"
#include "errors/error_factory.h"
#include "compiler/expression/expr_visitor.h"
#include "system/zorba.h"
#include "types/root_typemanager.h"
#include "store/api/store.h"
#include "store/api/item_factory.h"


using namespace std;
namespace xqp {
  
#define ITEM_FACTORY (GENV.getStore().getItemFactory())

  static yy::location null_loc;

  static expr_t dummy_expr;
  static expr_t *expr_iter_done = &dummy_expr;

#define BEGIN_EXPR_ITER() switch (v.state) { case 0:
#define BEGIN_EXPR_ITER2( type )                 \
  type &vv = dynamic_cast<type &> (v);           \
  BEGIN_EXPR_ITER()
#define END_EXPR_ITER()   v.i = expr_iter_done; }
#define ITER( m )                                           \
  do {                                                      \
    v.state = __LINE__; v.i = reinterpret_cast<expr_t *> (&(m)); \
    return;                                                 \
  case __LINE__:; } while (0)
#define ITER_FOR_EACH( iter, begin, end, expr )                      \
  for (vv.iter = (begin); vv.iter != (end); ++(vv.iter))  {          \
    ITER (expr);                                                     \
  }

// Other iterators

bool expr_iterator_data::done () const { return i == expr_iter_done; }

class fo_expr_iterator_data : public expr_iterator_data {
public:
  std::vector<expr_t>::iterator arg_iter;
  
public:
  fo_expr_iterator_data (expr *e_) : expr_iterator_data (e_) {}
};

class flwor_expr_iterator_data : public expr_iterator_data {
public:
  flwor_expr::clause_list_t::iterator clause_iter;
  flwor_expr::orderspec_list_t::iterator order_mod_iter;
  
public:
  flwor_expr_iterator_data (expr *e_) : expr_iterator_data (e_) {}
};

class typeswitch_expr_iterator_data : public expr_iterator_data {
public:
  std::vector<typeswitch_expr::clauseref_t>::const_iterator clause_iter;

public:
  typeswitch_expr_iterator_data (expr *e_) : expr_iterator_data (e_) {}
};

class relpath_expr_iterator_data : public expr_iterator_data {
public:
  std::vector<expr_t>::iterator step_iter;
  
public:
  relpath_expr_iterator_data (expr *e_) : expr_iterator_data (e_) {}
};

class axis_step_expr_iterator_data : public expr_iterator_data {
public:
  std::vector<expr_t>::iterator pred_iter;
  
public:
  axis_step_expr_iterator_data (expr *e_) : expr_iterator_data (e_) {}
};

  
#define DEF_ACCEPT( type )                         \
  void type::accept (expr_visitor &v) {            \
    if (v.begin_visit (*this))                     \
      accept_children (v);                         \
    v.end_visit (*this);                           \
  }

DEF_ACCEPT (expr)
DEF_ACCEPT (var_expr)
DEF_ACCEPT (flwor_expr)
DEF_ACCEPT (promote_expr)
DEF_ACCEPT (typeswitch_expr)
DEF_ACCEPT (if_expr)
DEF_ACCEPT (function_def_expr)
DEF_ACCEPT (fo_expr)
DEF_ACCEPT (ft_contains_expr)
DEF_ACCEPT (instanceof_expr)
DEF_ACCEPT (treat_expr)
DEF_ACCEPT (castable_expr)
DEF_ACCEPT (cast_expr)
DEF_ACCEPT (validate_expr)
DEF_ACCEPT (extension_expr)
DEF_ACCEPT (relpath_expr)
DEF_ACCEPT (axis_step_expr)
DEF_ACCEPT (match_expr)
DEF_ACCEPT (const_expr)
DEF_ACCEPT (order_expr)
DEF_ACCEPT (elem_expr)
DEF_ACCEPT (doc_expr)
DEF_ACCEPT (attr_expr)
DEF_ACCEPT (text_expr)
DEF_ACCEPT (pi_expr)

#undef DEF_ACCEPT

expr_iterator::expr_iterator (const expr_iterator &other) : iter (new expr_iterator_data (*other.iter)) {}
expr_iterator &expr_iterator::operator= (const expr_iterator &other) {
  if (this != &other) {
    delete iter;
    iter = new expr_iterator_data (*other.iter);
  }
  return *this;
}
expr_iterator &expr_iterator::operator++ () { iter->next (); return *this; }
expr_iterator expr_iterator::operator++ (int) { expr_iterator old; old = *this; ++*this; return old; }
expr_t &expr_iterator::operator* () { return *(iter->i); }
bool expr_iterator::done () const { return iter->done (); }
expr_iterator::~expr_iterator () { delete iter; }

  
expr::expr(
  yy::location const& _loc)
:
  loc(_loc)
{
}

expr::~expr() { }

  expr_iterator_data *expr::make_iter () {
    return new expr_iterator_data (this);
  }
  
  void expr::accept_children (expr_visitor &v) {
    for (expr_iterator i = expr_begin (); ! i.done (); ++i) {
      if (*i != NULL)
        (*i)->accept (v);
    }
  }

  expr_iterator expr::expr_begin() {
    expr_iterator_data *iter_data = make_iter ();
    iter_data->next ();
    return expr_iterator (iter_data);
  }

  void expr::next_iter (expr_iterator_data &v) {
    BEGIN_EXPR_ITER();
    ZORBA_ASSERT (false);
    END_EXPR_ITER();
  }

  expr_t expr::clone()
  {
      substitution_t s;
      return clone(s);
  }

  expr_t expr::clone(substitution_t& substitution)
  {
    Assert(false);
    return NULL; // Make the compiler happy
  }

/////////////////////////////////////////////////////////////////////////
//                                                                     //
//  XQuery 1.0 productions                                             //
//  [http://www.w3.org/TR/xquery/]                                     //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

// [29]
// [33a]

string var_expr::decode_var_kind(
  enum var_kind k)
{
  switch (k) {
  case for_var: return "FOR_VAR"; break;
  case let_var: return "LET_VAR"; break;
  case pos_var: return "POS_VAR"; break;
  case score_var: return "SCORE_VAR"; break;
  case quant_var: return "QUANT_VAR"; break;
  case context_var: return "$DOT"; break;
  default: return "???";
  }
}

var_expr::var_expr(yy::location const& loc, Item_t name) : expr (loc), varname_h (name), type (GENV_TYPESYSTEM.UNTYPED_TYPE) {}

var_expr::var_expr(yy::location const& loc, var_kind k, Item_t name) : expr (loc), kind (k), varname_h (name), type (GENV_TYPESYSTEM.UNTYPED_TYPE) {}  // TODO

Item_t var_expr::get_varname() const { return varname_h; }
xqtref_t var_expr::get_type() const { return type; }
void var_expr::set_type(xqtref_t t) { type = t; }

void var_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  END_EXPR_ITER();
}

expr::expr_t var_expr::clone(expr::substitution_t& substitution)
{
  expr::subst_iter_t i = substitution.find(this);
  if (i == substitution.end()) {
    return this;
  }
  return i->second->clone(substitution);
}

// [33] [http://www.w3.org/TR/xquery/#prod-xquery-FLWORExpr]


forlet_clause::forlet_clause(
  enum forlet_t _type,
  varref_t _var_h,
  varref_t _pos_var_h,
  varref_t _score_var_h,
  expr_t _expr_h)
:
  type(_type),
  var_h(_var_h),
  pos_var_h(_pos_var_h),
  score_var_h(_score_var_h),
  expr_h(_expr_h)
{
}


rchandle<forlet_clause> forlet_clause::clone(expr::substitution_t& substitution)
{
  expr_t expr_copy_h = expr_h->clone(substitution);

  varref_t var_copy_h(new var_expr(*var_h));
  substitution[var_h.getp()] = var_copy_h.getp();

  varref_t pos_var_copy_h;
  var_expr *pos_var_ptr = pos_var_h.getp();
  if (pos_var_ptr) {
    pos_var_copy_h = new var_expr(*pos_var_ptr);
    substitution[pos_var_ptr] = pos_var_copy_h.getp();
  }

  varref_t score_var_copy_h;
  var_expr *score_var_ptr = score_var_h.getp();
  if (score_var_ptr) {
    score_var_copy_h = new var_expr(*score_var_ptr);
    substitution[score_var_ptr] = score_var_copy_h.getp();
  }

  return new forlet_clause(type, var_copy_h, pos_var_copy_h, score_var_copy_h, expr_copy_h);
}


expr_iterator_data *flwor_expr::make_iter () {
  return new flwor_expr_iterator_data (this);
}

void flwor_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER2 (flwor_expr_iterator_data);

  ITER_FOR_EACH (clause_iter, clause_v.begin (), clause_v.end (),
                 (*vv.clause_iter)->expr_h);
  
  ITER (where_h);

  ITER_FOR_EACH (order_mod_iter, orderspec_begin (), orderspec_end (),
                 (*vv.order_mod_iter).first);

  ITER (retval_h);
  
  END_EXPR_ITER(); 
}

expr::expr_t flwor_expr::clone(expr::substitution_t& substitution)
{
  expr_t flwor_copy = new flwor_expr(get_loc());
  flwor_expr *flwor_copy_ptr = static_cast<flwor_expr *>(flwor_copy.getp());

  for(clause_list_t::iterator i = clause_v.begin(); i != clause_v.end(); ++i) {
    flwor_copy_ptr->add((*i)->clone(substitution));
  }

  for(orderspec_list_t::iterator i = orderspec_v.begin(); i != orderspec_v.end(); ++i) {
    expr_t e_copy = i->first->clone(substitution);
    flwor_copy_ptr->add(orderspec_t(e_copy, i->second));
  }

  flwor_copy_ptr->set_order_stable(order_stable);

  if (where_h.getp()) {
    flwor_copy_ptr->set_where(where_h->clone(substitution));
  }

  flwor_copy_ptr->set_retval(retval_h->clone(substitution));

  return flwor_copy;
}

case_clause::case_clause() : var_h(NULL), case_expr_h(NULL), type(GENV_TYPESYSTEM.UNTYPED_TYPE) { }

promote_expr::promote_expr(yy::location const& loc) : expr(loc) { }

promote_expr::promote_expr(yy::location const& loc, expr_t input, xqtref_t type)
  : expr(loc),
  input_expr_h(input),
  target_type(type) { }

void promote_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (input_expr_h);
  END_EXPR_ITER ();
}

xqtref_t promote_expr::get_target_type() { return target_type; }
void promote_expr::set_target_type(xqtref_t target) { target_type = target; }

// [42] [http://www.w3.org/TR/xquery/#prod-xquery-QuantifiedExpr]

// [43] [http://www.w3.org/TR/xquery/#prod-xquery-TypeswitchExpr]

typeswitch_expr::typeswitch_expr(
  yy::location const& loc)
:
  expr(loc)
{
}

expr_iterator_data *typeswitch_expr::make_iter () {
  return new typeswitch_expr_iterator_data (this);
}

void typeswitch_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER2(typeswitch_expr_iterator_data);
  ITER (switch_expr_h);
  for (vv.clause_iter = begin (); vv.clause_iter != end (); ++(vv.clause_iter)) {
    ITER ((*vv.clause_iter)->var_h);
    ITER ((*vv.clause_iter)->case_expr_h);
  }
  ITER (default_var_h);
  ITER (default_clause_h);
  END_EXPR_ITER ();
}


// [45] [http://www.w3.org/TR/xquery/#prod-xquery-IfExpr]

if_expr::if_expr(
  yy::location const& loc,
  rchandle<expr> _cond_expr_h,
  rchandle<expr> _then_expr_h,
  rchandle<expr> _else_expr_h)
:
  expr(loc),
  cond_expr_h(_cond_expr_h),
  then_expr_h(_then_expr_h),
  else_expr_h(_else_expr_h)
{
}

if_expr::if_expr(
  yy::location const& loc)
:
  expr(loc)
{
}


void if_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER (cond_expr_h);
  ITER (then_expr_h);
  ITER (else_expr_h);
  END_EXPR_ITER();
}


////////////////////////////////
//  first-order expressions
////////////////////////////////
// includes:
// [46] [http://www.w3.org/TR/xquery/#prod-xquery-OrExpr]
// [47] [http://www.w3.org/TR/xquery/#prod-xquery-AndExpr]
// [48] [http://www.w3.org/TR/xquery/#prod-xquery-ComparisonExpr]
// [49] [http://www.w3.org/TR/xquery/#prod-xquery-RangeExpr]
// [50] [http://www.w3.org/TR/xquery/#prod-xquery-AdditiveExpr]
// [51] [http://www.w3.org/TR/xquery/#prod-xquery-MultiplicativeExpr]
// [52] [http://www.w3.org/TR/xquery/#prod-xquery-UnionExpr]
// [53] [http://www.w3.org/TR/xquery/#prod-xquery-IntersectExceptExpr]

expr_iterator_data *fo_expr::make_iter () { return new fo_expr_iterator_data (this); }

void fo_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER2(fo_expr_iterator_data);
  ITER_FOR_EACH (arg_iter, begin (), end (), *vv.arg_iter);
  END_EXPR_ITER ();
}

const signature &fo_expr::get_signature () const {
  return func->get_signature ();
}

Item_t fo_expr::get_fname () const
{ return func->get_fname (); }


// [48a] [http://www.w3.org/TR/xquery-full-text/#prod-xquery-FTContainsExpr]

ft_contains_expr::ft_contains_expr(
  yy::location const& loc,
  rchandle<expr> _range_h,
  rchandle<expr> _ft_select_h,
  rchandle<expr> _ft_ignore_h)
:
  expr(loc),
  range_h(_range_h),
  ft_select_h(_ft_select_h),
  ft_ignore_h(_ft_ignore_h)
{
}


void ft_contains_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (range_h);
  ITER (ft_select_h);
  ITER (ft_ignore_h);
  END_EXPR_ITER ();
}


// [54] [http://www.w3.org/TR/xquery/#prod-xquery-InstanceofExpr]

instanceof_expr::instanceof_expr(yy::location const& loc,
                                 rchandle<expr> _expr_h,
                                 xqtref_t _type)
:
  expr(loc), expr_h(_expr_h), type (_type)
{
}

xqtref_t instanceof_expr::get_type() const { return type; }

void instanceof_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER (expr_h);
  END_EXPR_ITER();  
}


// [55] [http://www.w3.org/TR/xquery/#prod-xquery-TreatExpr]

treat_expr::treat_expr(
  yy::location const& loc,
  rchandle<expr> _expr_h,
  xqtref_t _type,
  enum ZorbaError::ErrorCodes err_)
:
  expr(loc),
  expr_h(_expr_h),
  type(_type),
  err (err_)
{
}

xqtref_t treat_expr::get_type() const { return type; }

void treat_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


// [56] [http://www.w3.org/TR/xquery/#prod-xquery-CastableExpr]

castable_expr::castable_expr(
  yy::location const& loc,
  rchandle<expr> _expr_h,
  xqtref_t _type)
:
  expr(loc),
  expr_h(_expr_h),
  type(_type)
{
}

bool castable_expr::is_optional() const { return GENV_TYPESYSTEM.quantifier(*type) == TypeConstants::QUANT_QUESTION; }

xqtref_t castable_expr::get_type() const { return type; }

void castable_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


// [57] [http://www.w3.org/TR/xquery/#prod-xquery-CastExpr]

cast_expr::cast_expr(
  yy::location const& loc,
  rchandle<expr> _expr_h,
  xqtref_t _type)
:
  expr(loc),
  expr_h(_expr_h),
  type(_type)
{
}

bool cast_expr::is_optional() const { return GENV_TYPESYSTEM.quantifier(*type) == TypeConstants::QUANT_QUESTION; }

xqtref_t cast_expr::get_type() const { return type; }

void cast_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


// [63] [http://www.w3.org/TR/xquery/#prod-xquery-ValidateExpr]

validate_expr::validate_expr(
  yy::location const& loc,
  enum validation_mode_t _valmode,
  rchandle<expr> _expr_h)
:
  expr(loc),
  valmode(_valmode),
  expr_h(_expr_h)
{
}


void validate_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


// [65] [http://www.w3.org/TR/xquery/#prod-xquery-ExtensionExpr]

extension_expr::extension_expr(
  yy::location const& loc)
:
  expr(loc)
{
}

extension_expr::extension_expr(
  yy::location const& loc,
  expr_t _expr_h)
:
  expr(loc),
  expr_h(_expr_h)
{
}


void extension_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


/*******************************************************************************

 [69] [http://www.w3.org/TR/xquery/#prod-xquery-RelativePathExpr]

  RelativPathExpr ::= "/" | ("/" | "//")?  StepExpr (("/" | "//") StepExpr)*

********************************************************************************/
relpath_expr::relpath_expr(yy::location const& loc)
  :
  expr(loc)
{
}

expr_iterator_data *relpath_expr::make_iter () { return new relpath_expr_iterator_data (this); }

void relpath_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER2(relpath_expr_iterator_data);

  ITER_FOR_EACH (step_iter, begin (), end (), *vv.step_iter);

  END_EXPR_ITER();
}


/*******************************************************************************

  [71] [http://www.w3.org/TR/xquery/#prod-xquery-AxisStep]

  AxisStep ::= Axis NodeTest Predicate*

********************************************************************************/
axis_step_expr::axis_step_expr(yy::location const& loc)
  :
  expr(loc)
{
}

expr_iterator_data *axis_step_expr::make_iter () {
  return new axis_step_expr_iterator_data (this);
}

void axis_step_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER2(axis_step_expr_iterator_data);

  ITER_FOR_EACH (pred_iter, thePreds.begin (), thePreds.end (), *vv.pred_iter);

  ITER (theNodeTest);

  END_EXPR_ITER();
}


/*******************************************************************************

  [78] NodeTest ::= KindTest | NameTest

  [79] NameTest ::= QName | Wildcard
  [80] Wildcard ::= "*" | (NCName ":" "*") | ("*" ":" NCName)

  [123] KindTest ::= DocumentTest | ElementTest | AttributeTest |
                     SchemaElementTest | SchemaAttributeTest |
                     PITest | CommentTest | TextTest | AnyKindTest

********************************************************************************/
match_expr::match_expr(yy::location const& loc)
  :
  expr(loc),
  theWildKind(match_no_wild),
  theQName(NULL),
  theTypeName(NULL),
  theNilledAllowed(false)
{
}


void match_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  END_EXPR_ITER();
}


StoreConsts::NodeKind match_expr::getNodeKind() const
{
  switch (theTestKind)
  {
  case match_name_test:
    return StoreConsts::elementNode;
  case match_doc_test:
    return StoreConsts::elementNode;
  case match_elem_test:
    return StoreConsts::elementNode;
  case match_attr_test:
    return StoreConsts::attributeNode;
  case match_xs_elem_test:
    return StoreConsts::elementNode;
  case match_xs_attr_test:
    return StoreConsts::attributeNode;
  case match_pi_test:
    return StoreConsts::piNode;
  case match_text_test:
    return StoreConsts::textNode;
  case match_comment_test:
    return StoreConsts::commentNode;
  case match_anykind_test:
    return StoreConsts::anyNode;
  default:
    ZORBA_ASSERT (false && "Unknown node test kind");
  }
  return StoreConsts::anyNode;
}


// [84] [http://www.w3.org/TR/xquery/#prod-xquery-PrimaryExpr]


// [85] [http://www.w3.org/TR/xquery/#prod-xquery-PrimaryExpr]

const_expr::const_expr(yy::location const& loc,
                           xqpString v)
  :
  expr(loc),
  val (ITEM_FACTORY.createString (v))
{
}

const_expr::const_expr(yy::location const& loc,
                           xqp_integer v)
:
  expr(loc),
  val (ITEM_FACTORY.createInteger (v))
{
}

const_expr::const_expr(
  yy::location const& loc,
  xqp_decimal v)
:
  expr(loc),
  val (ITEM_FACTORY.createDecimal (v))
{
}

const_expr::const_expr(
  yy::location const& loc,
  xqp_double v)
:
  expr(loc),
  val (ITEM_FACTORY.createDouble (v))
{
}

const_expr::const_expr(
  yy::location const& loc,
  xqp_boolean v)
:
  expr(loc),
  val (ITEM_FACTORY.createBoolean (v))
{
}

const_expr::const_expr(
  yy::location const& loc,
  Item_t v)
:
  expr(loc),
  val(v)
{
}

const_expr::const_expr(
  yy::location const& aLoc, 
  const char* aNamespace,
  const char* aPrefix, 
  const char* aLocal)
:
  expr(aLoc),
  val ((Item*)&*ITEM_FACTORY.createQName(aNamespace, aPrefix, aLocal))
{
}


void const_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  END_EXPR_ITER();
}


// [91] [http://www.w3.org/TR/xquery/#prod-xquery-OrderedExpr]

order_expr::order_expr(
  yy::location const& loc,
  order_type_t _type,
  rchandle<expr> _expr_h)
:
  expr(loc),
  type(_type),
  expr_h(_expr_h)
{
}


void order_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (expr_h);
  END_EXPR_ITER ();
}


// [93] [http://www.w3.org/TR/xquery/#prod-xquery-FunctionCall]

// [96] [http://www.w3.org/TR/xquery/#doc-exquery-DirElemConstructor]
elem_expr::elem_expr (
    yy::location const& aLoc,
    expr_t aQNameExpr,
    expr_t aAttrs,
    expr_t aContent,
    rchandle<namespace_context> aNSCtx)
    :
    constructor_expr ( aLoc ),
    theQNameExpr ( aQNameExpr ),
    theAttrs ( aAttrs ),
    theContent ( aContent ),
    theNSCtx(aNSCtx)
{
}

elem_expr::elem_expr (
    yy::location const& aLoc,
    expr_t aQNameExpr,
    expr_t aContent,
    rchandle<namespace_context> aNSCtx)
    :
    constructor_expr ( aLoc ),
    theQNameExpr ( aQNameExpr ),
    theAttrs ( 0 ),
    theContent ( aContent ),
    theNSCtx(aNSCtx)
{
}
  

rchandle<namespace_context> elem_expr::getNSCtx() { return theNSCtx; }

void elem_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER (theQNameExpr);
  ITER (theAttrs);
  ITER (theContent);
  END_EXPR_ITER();
}


// [110] [http://www.w3.org/TR/xquery/#prod-xquery-CompDocConstructor]

doc_expr::doc_expr(
  yy::location const& loc,
  rchandle<expr> aContent)
:
  constructor_expr(loc),
  theContent(aContent)
{
}


void doc_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER(theContent);
  END_EXPR_ITER();
}


// [111] [http://www.w3.org/TR/xquery/#prod-xquery-CompElemConstructor]


// [113] [http://www.w3.org/TR/xquery/#prod-xquery-CompAttrConstructor]

attr_expr::attr_expr(
  yy::location const& loc,
  expr_t aQNameExpr,
  expr_t aValueExpr)
:
  constructor_expr(loc),
  theQNameExpr(aQNameExpr),
  theValueExpr(aValueExpr)
{
}


Item* attr_expr::getQName() const
{
  const_expr* qnExpr =  dynamic_cast<const_expr*>(theQNameExpr.getp());
  if (qnExpr != 0)
    return qnExpr->get_val().getp();

  return 0;
}


void attr_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER (theQNameExpr);
  ITER (theValueExpr);
  END_EXPR_ITER();
}


// [114] [http://www.w3.org/TR/xquery/#prod-xquery-CompTextConstructor]

text_expr::text_expr(
  yy::location const& loc,
  text_constructor_type type_arg,
  expr_t text_arg)
:
  constructor_expr(loc),
  type (type_arg),
  text(text_arg)
{
}


void text_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER();
  ITER (text);
  END_EXPR_ITER();
}


// [115] [http://www.w3.org/TR/xquery/#prod-xquery-CompCommentConstructor]


// [114] [http://www.w3.org/TR/xquery/#prod-xquery-CompPIConstructor]

pi_expr::pi_expr(
  yy::location const& loc,
  rchandle<expr> _target_expr_h,
  rchandle<expr> _content_expr_h)
:
  text_expr(loc, text_expr::pi_constructor, _content_expr_h),
  target_expr_h(_target_expr_h)
{
}


void pi_expr::next_iter (expr_iterator_data& v) {
  BEGIN_EXPR_ITER ();
  ITER (target_expr_h);
  ITER (text);
  END_EXPR_ITER ();
}

void function_def_expr::next_iter (expr_iterator_data& v) {
}

function_def_expr::function_def_expr (yy::location const& loc, Item_t name_, std::vector<rchandle<var_expr> > &params_, xqtref_t return_type)
  : expr (loc), name (name_)
{
  assert (return_type != NULL);
  params.swap (params_);
  vector<xqtref_t> args;
  // TODO: copy param types into sig
  for (unsigned i = 0; i < param_size (); i++)
    args.push_back (GENV_TYPESYSTEM.ITEM_TYPE_STAR);
  sig = auto_ptr<signature> (new signature (get_name (), args, GENV_TYPESYSTEM.ITEM_TYPE_STAR));
}


} /* namespace xqp */
/* vim:set ts=2 sw=2: */
