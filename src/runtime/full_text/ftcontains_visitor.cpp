/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>                        /* for fabs(3) */
#include <limits>
#include <memory>

#include "compiler/expression/ft_expr.h"
#include "compiler/expression/ftnode.h"
#include "compiler/parser/query_loc.h"
#include "util/indent.h"
#include "util/stl_util.h"
#include "zorbaerrors/error_manager.h"
#include "zorbatypes/numconversions.h"

#ifndef NDEBUG
#include "system/properties.h"
#define DOUT            Properties::instance()->debug_out()
#define TRACE_FULL_TEXT Properties::instance()->traceFulltext()
#endif /* NDEBUG */

#include "ft_util.h"
#include "ftcontains_visitor.h"

using namespace std;

namespace zorba {

#ifdef WIN32
// Windows annoyingly defines these as macros.
#undef max
#undef min
#endif

////////// Local functions ////////////////////////////////////////////////////

#define GET_OPTION(O) get_##O##_option()

#define REPLACE_OPTION(O)                                                     \
  if ( newer_options->GET_OPTION(O) || !older_options->GET_OPTION(O) ) ; else \
    newer_options->set_##O##_option( older_options->GET_OPTION(O) )

static void replace_match_options( ftmatch_options const *older_options,
                                   ftmatch_options *newer_options ) {
  REPLACE_OPTION( case );
  REPLACE_OPTION( diacritics );
  REPLACE_OPTION( extension );
  REPLACE_OPTION( language );
  REPLACE_OPTION( stem );
  REPLACE_OPTION( stop_word );
  REPLACE_OPTION( thesaurus );
  REPLACE_OPTION( wild_card );
}

inline void set_error_query_loc( error::ZorbaError &e, QueryLoc const &loc ) {
  e.setQueryLocation(
    loc.getLineBegin(), loc.getColumnBegin(), loc.getFilename()
  );
}

inline double to_double( xs_double const &d ) {
  return d.getNumber();
}

inline ft_int to_ft_int( xs_integer const &i ) {
  ft_int result;
  if ( !NumConversions::integerToUInt( i, result ) ) {
    // TODO
  }
  return result;
}

////////// PUSH/POP macros ////////////////////////////////////////////////////

#ifndef NDEBUG

inline void pop_helper( char const *what, int line ) {
  if ( TRACE_FULL_TEXT )
    DOUT << indent << "(pop " << what << " @ line " << line << ")\n";
}

#define PUSH(WHAT,OBJ)                                                  \
  do {                                                                  \
    if ( TRACE_FULL_TEXT )                                              \
      DOUT << indent << "(push " #WHAT " @ line " << __LINE__ << ")\n"; \
    WHAT##_stack_.push( OBJ );                                          \
  } while (0)

#define POP(WHAT) \
  ( pop_helper( #WHAT, __LINE__ ), pop_stack( WHAT##_stack_ )

#else /* NDEBUG */

#define PUSH(WHAT,OBJ)  WHAT##_stack_.push( OBJ )
#define POP(WHAT)       pop_stack( WHAT##_stack_ )

#endif /* NDEBUG */

#define TOP(WHAT)       WHAT##_stack_.top()

////////// ftcontains_visitor non-inline member functions /////////////////////

ftcontains_visitor::ftcontains_visitor( FTTokenIterator_t &search_ctx,
                                        static_context const &static_ctx,
                                        store::Item const *ignore_item,
                                        PlanState &state ) :
  search_ctx_( search_ctx ),
  static_ctx_( static_ctx ),
  ignore_item_( ignore_item ),
  plan_state_( state )
{
  // do nothing
}

ftcontains_visitor::~ftcontains_visitor() {
  while ( !matches_stack_.empty() )
    delete POP( matches );
}

void ftcontains_visitor::eval_ftrange( ftrange const &range,
                                       ft_int *at_least, ft_int *at_most ) {
  *at_least = 0;
  *at_most = numeric_limits<ft_int>::max();
  ft_int const int1 = get_int( range.get_plan_iter1() );

  switch ( range.get_mode() ) {
    case ft_range_mode::at_least:
      *at_least = int1;
      break;
    case ft_range_mode::at_most:
      *at_most = int1;
      break;
    case ft_range_mode::exactly:
      *at_least = *at_most = int1;
      break;
    case ft_range_mode::from_to:
      *at_least = int1;
      *at_most = get_int( range.get_plan_iter2() );
      break;
  }
}

bool ftcontains_visitor::ftcontains() const {
  if ( matches_stack_.empty() )
    return false;
  ft_all_matches const &am = *TOP( matches );
  if ( am.empty() )
    return false;
  //
  // See spec section 4.3.
  //
  FOR_EACH( ft_all_matches, m, am )
    if ( m->excludes.empty() )
      return true;
  return false;
}

expr_visitor* ftcontains_visitor::get_expr_visitor() {
  return NULL;
}

double ftcontains_visitor::get_double( PlanIter_t const &iter ) {
  store::Item_t item;
  iter->reset( plan_state_ );
	bool const got_item = PlanIterator::consumeNext( item, iter, plan_state_ );
  ZORBA_ASSERT( got_item );
  return to_double( item->getDoubleValue() );
}

ft_int ftcontains_visitor::get_int( PlanIter_t const &iter ) {
  store::Item_t item;
  iter->reset( plan_state_ );
	bool const got_item = PlanIterator::consumeNext( item, iter, plan_state_ );
  ZORBA_ASSERT( got_item );
  return to_ft_int( item->getIntegerValue() );
}

////////// Visit macros ///////////////////////////////////////////////////////

#ifndef NDEBUG

#define BEGIN_VISIT(LABEL)                                \
  if ( !TRACE_FULL_TEXT ); else                           \
  cout << indent << "BEGIN " #LABEL << endl << inc_indent

#define END_VISIT(LABEL)                                \
  if ( !TRACE_FULL_TEXT ); else                         \
  cout << dec_indent << indent << "END " #LABEL << endl

#else /* NDEBUG */

#define BEGIN_VISIT(LABEL)  /* nothing */
#define END_VISIT(LABEL)    /* nothing */

#endif /* NDEBUG */

#define V ftcontains_visitor

///////////////////////////////////////////////////////////////////////////////

//
// We define these first (before redefining DEF_FTNODE_VISITOR_BEGIN_VISIT and
// DEF_FTNODE_VISITOR_END_VISIT) so they don't print debugging information.  We
// don't care about printing debugging information for these since they're all
// empty.
//
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftcase_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftdiacritics_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftextension_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftlanguage_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftmatch_options )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftrange )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftstem_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftstop_word_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftstop_words )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftthesaurus_id )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftthesaurus_option )
DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftwild_card_option )

////////// The "main" FT nodes ////////////////////////////////////////////////

#ifndef NDEBUG

#undef DEF_FTNODE_VISITOR_BEGIN_VISIT
#define DEF_FTNODE_VISITOR_BEGIN_VISIT(V,C)     \
  ft_visit_result::type V::begin_visit( C& ) {  \
    BEGIN_VISIT(C);                             \
    return ft_visit_result::proceed;            \
  }

#undef DEF_FTNODE_VISITOR_END_VISIT
#define DEF_FTNODE_VISITOR_END_VISIT(V,C)   \
  void V::end_visit( C& ) {                 \
    END_VISIT(C);                           \
  }

#endif /* NDEBUG */

ft_visit_result::type V::begin_visit( ftand& ) {
  BEGIN_VISIT( ftand );
  PUSH( matches, NULL ); // sentinel
  return ft_visit_result::proceed;
}
void V::end_visit( ftand& ) {
  while ( true ) {
    // the popping order is significant
    auto_ptr<ft_all_matches> am_right( POP( matches ) );
    auto_ptr<ft_all_matches> am_left( POP( matches ) );
    if ( !am_left.get() ) {
      PUSH( matches, am_right.release() );
      break;
    }
    auto_ptr<ft_all_matches> result( new ft_all_matches );
    apply_ftand( *am_left, *am_right, *result );
    PUSH( matches, result.release() );
  }
  END_VISIT( ftand );
}

DEF_FTNODE_VISITOR_VISIT_MEM_FNS( V, ftextension_selection )

ft_visit_result::type V::begin_visit( ftmild_not& ) {
  BEGIN_VISIT( ftmild_not );
  PUSH( matches, NULL ); // sentinel
  return ft_visit_result::proceed;
}
void V::end_visit( ftmild_not &mn ) {
  while ( true ) {
    // the popping order is significant
    auto_ptr<ft_all_matches> am_right( POP( matches ) );
    auto_ptr<ft_all_matches> am_left( POP( matches ) );
    if ( !am_left.get() ) {
      PUSH( matches, am_right.release() );
      break;
    }
    try {
      auto_ptr<ft_all_matches> result( new ft_all_matches );
      apply_ftmild_not( *am_left, *am_right, *result );
      PUSH( matches, result.release() );
    }
    catch ( error::ZorbaError &e ) {
      set_error_query_loc( e, mn.get_loc() );
      throw;
    }
  }
  END_VISIT( ftmild_not );
}

ft_visit_result::type V::begin_visit( ftor& ) {
  BEGIN_VISIT( ftor );
  PUSH( matches, NULL ); // sentinel
  return ft_visit_result::proceed;
}
void V::end_visit( ftor& ) {
  while ( true ) {
    // the popping order is significant
    auto_ptr<ft_all_matches> am_right( POP( matches ) );
    auto_ptr<ft_all_matches> am_left( POP( matches ) );
    if ( !am_left.get() ) {
      PUSH( matches, am_right.release() );
      break;
    }
    auto_ptr<ft_all_matches> result( new ft_all_matches );
    apply_ftor( *am_left, *am_right, *result );
    PUSH( matches, result.release() );
  }
  END_VISIT( ftor );
}

ft_visit_result::type V::begin_visit( ftprimary_with_options &pwo ) {
  BEGIN_VISIT( ftprimary_with_options );

  ftmatch_options const *const older_options = options_stack_.empty() ?
    static_ctx_.get_match_options() : TOP( options );
  ftmatch_options *const newer_options =
    new ftmatch_options( *pwo.get_match_options() );

  if ( older_options )
    replace_match_options( older_options, newer_options );
  newer_options->set_missing_defaults();

  PUSH( options, newer_options );
  return ft_visit_result::proceed;
}
void V::end_visit( ftprimary_with_options &pwo ) {
  delete POP( options );
  END_VISIT( ftprimary_with_options );
}

ft_visit_result::type V::begin_visit( ftweight &w ) {
  double const weight = get_double( w.get_weight_iter() );
  if ( fabs( weight ) > 1000.0 )
    ZORBA_ERROR_LOC( FTDY0016, w.get_weight_expr()->get_loc() );
  // TODO: do something with weight
  return ft_visit_result::proceed;
}
DEF_FTNODE_VISITOR_END_VISIT( V, ftweight )

ft_visit_result::type V::begin_visit( ftselection& ) {
  BEGIN_VISIT( ftselection );
  query_pos_ = 0;
  return ft_visit_result::proceed;
}
DEF_FTNODE_VISITOR_END_VISIT( V, ftselection )

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftunary_not )
void V::end_visit( ftunary_not& ) {
  apply_ftunary_not( *TOP( matches ) );
  END_VISIT( ftunary_not );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftwords )
void V::end_visit( ftwords &w ) {
  ftmatch_options const &options = *TOP( options );
  locale::iso639_1::type const lang = get_lang_from( &options );
  bool const wildcards = get_wildcards_from( &options );

  PlanIter_t plan_iter = w.get_value_iter();
  plan_iter->reset( plan_state_ );
  FTQueryItemSeq query_items;
  store::Item_t item;

  while ( PlanIterator::consumeNext( item, plan_iter, plan_state_ ) ) {
    try {
      FTQueryItem const qi( item->getQueryTokens( lang, wildcards ) );
      if ( qi->hasNext() )
        query_items.push_back( qi );
    }
    catch ( error::ZorbaError &e ) {
      set_error_query_loc( e, w.get_value_expr()->get_loc() );
      throw;
    }
  }

  if ( !query_items.empty() ) {
    auto_ptr<ft_all_matches> result( new ft_all_matches );
    try {
      apply_ftwords(
        query_items, ++query_pos_, ignore_item_, w.get_mode(), options, *result
      );
    }
    catch ( error::ZorbaError &e ) {
      set_error_query_loc( e, w.get_value_expr()->get_loc() );
      throw;
    }
    PUSH( matches, result.release() );
  }
  END_VISIT( ftwords );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftwords_times )
void V::end_visit( ftwords_times &wt ) {
  if ( ftrange const *const range = wt.get_times() ) {
    try {
      ft_int at_least, at_most;
      eval_ftrange( *range, &at_least, &at_most );
      auto_ptr<ft_all_matches> const am( POP( matches ) );
      auto_ptr<ft_all_matches> result( new ft_all_matches );
      apply_fttimes( *am, range->get_mode(), at_least, at_most, *result );
      PUSH( matches, result.release() );
    }
    catch ( error::ZorbaError &e ) {
      set_error_query_loc( e, wt.get_loc() );
      throw;
    }
  }
  END_VISIT( ftwords_times );
}

////////// FTPosFilters ///////////////////////////////////////////////////////

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftcontent_filter )
void V::end_visit( ftcontent_filter &f ) {
  apply_ftcontent(
    *TOP( matches ), f.get_mode(), search_ctx_->begin(), search_ctx_->end() - 1
  );
  END_VISIT( ftcontent_filter );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftdistance_filter )
void V::end_visit( ftdistance_filter &f ) {
  ft_int at_least, at_most;
  eval_ftrange( *f.get_range(), &at_least, &at_most );
  auto_ptr<ft_all_matches> const am( POP( matches ) );
  auto_ptr<ft_all_matches> result( new ft_all_matches );
  apply_ftdistance( *am, at_least, at_most, f.get_unit(), *result );
  PUSH( matches, result.release() );
  END_VISIT( ftdistance_filter );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftorder_filter )
void V::end_visit( ftorder_filter& ) {
  auto_ptr<ft_all_matches> const am( POP( matches ) );
  auto_ptr<ft_all_matches> result( new ft_all_matches );
  apply_ftorder( *am, *result );
  PUSH( matches, result.release() );
  END_VISIT( ftorder_filter );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftscope_filter )
void V::end_visit( ftscope_filter &f ) {
  auto_ptr<ft_all_matches> const am( POP( matches ) );
  auto_ptr<ft_all_matches> result( new ft_all_matches );
  apply_ftscope( *am, f.get_scope(), f.get_unit(), *result );
  PUSH( matches, result.release() );
  END_VISIT( ftscope_filter );
}

DEF_FTNODE_VISITOR_BEGIN_VISIT( V, ftwindow_filter )
void V::end_visit( ftwindow_filter &f ) {
  auto_ptr<ft_all_matches> const am( POP( matches ) );
  auto_ptr<ft_all_matches> result( new ft_all_matches );
  apply_ftwindow( *am, get_int( f.get_window_iter() ), f.get_unit(), *result );
  PUSH( matches, result.release() );
  END_VISIT( ftwindow_filter );
}

#undef V

} // namespace zorba
/* vim:set et sw=2 ts=2: */
