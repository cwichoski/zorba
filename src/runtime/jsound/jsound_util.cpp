/*
 * Copyright 2006-2013 The FLWOR Foundation.
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

#include "stdafx.h"

#include <zorba/config.h>
#include <zorba/internal/cxx_util.h>
#include <zorba/diagnostic_list.h>
#include <zorba/store_consts.h>

#include "diagnostics/assert.h"
#include "store/api/item.h"
#include "store/api/item_factory.h"
#include "store/api/iterator.h"
#include "system/globalenv.h"
#include "types/casting.h"
#include "types/root_typemanager.h"
#include "types/typeops.h"
#include "util/less.h"
#include "util/stl_util.h"
#include "util/string_util.h"
#include "util/xml_util.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/numconversions.h"

#include "jsound_util.h"

using namespace std;

namespace zorba {
namespace jsound {

///////////////////////////////////////////////////////////////////////////////

static facet_mask const facet_constraints      = 1 <<  0;
static facet_mask const facet_enumeration      = 1 <<  1;
static facet_mask const facet_length           = 1 <<  2;
static facet_mask const facet_maxExclusive     = 1 <<  3;
static facet_mask const facet_maxInclusive     = 1 <<  4;
static facet_mask const facet_maxLength        = 1 <<  5;
static facet_mask const facet_minExclusive     = 1 <<  6;
static facet_mask const facet_minInclusive     = 1 <<  7;
static facet_mask const facet_minLength        = 1 <<  8;
static facet_mask const facet_totalDigits      = 1 <<  9;
static facet_mask const facet_fractionDigits   = 1 << 10;
static facet_mask const facet_explicitTimezone = 1 << 11;
static facet_mask const facet_pattern          = 1 << 12;

#define FACET_EXCEPTION(ITEM,FACET)                                 \
  ZORBA_EXCEPTION(                                                  \
    jsd::FACET_VIOLATION, ERROR_PARAMS( (ITEM), #FACET, FACET##_ )  \
  )

#define HAS_FACET(F) \
  (facet_mask_ & facet_##F)

#define IS_SUBTYPE(T,U) \
  TypeOps::is_subtype( (T), store::U )

#define IS_ATOMIC_TYPE(ITEM,TYPE) \
  ( (ITEM)->isAtomic() && IS_SUBTYPE( (ITEM)->getTypeCode(), TYPE ) )

#define IS_KIND(ITEM,KIND) \
  ( (ITEM)->getKind() == store::Item::KIND )

static void assert_kind( store::Item_t const &item, char const *name,
                         store::Item::ItemKind kind ) {
  if ( item->getKind() != kind )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_TYPE,
      ERROR_PARAMS( item->getKind(), name, kind )
    );
}

inline void assert_kind( store::Item_t const &item, zstring const &name,
                         store::Item::ItemKind kind ) {
  assert_kind( item, name.c_str(), kind );
}

#define JSOUND_ASSERT_KIND(ITEM,KEY,KIND) \
  assert_kind( ITEM, KEY, store::Item::KIND )

static void assert_type( store::Item_t const &item, char const *name,
                         store::SchemaTypeCode type ) {
  if ( !item->isAtomic() )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_TYPE,
      ERROR_PARAMS( item->getKind(), name, store::Item::ATOMIC )
    );
  if ( !TypeOps::is_subtype( item->getTypeCode(), type ) )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_TYPE,
      ERROR_PARAMS( item->getTypeCode(), name, type )
    );
}

inline void assert_type( store::Item_t const &item, zstring const &name,
                         store::SchemaTypeCode type ) {
  assert_type( item, name.c_str(), type );
}

#define JSOUND_ASSERT_TYPE(ITEM,KEY,TYPE) \
  assert_type( ITEM, KEY, store::TYPE )

static store::SchemaTypeCode map_atomic_type( zstring const &type_name ) {
  if ( ZSTREQ( type_name, "string" ) )
    return store::XS_STRING;
  if ( ZSTREQ( type_name, "boolean" ) )
    return store::XS_BOOLEAN;
  if ( ZSTREQ( type_name, "integer" ) )
    return store::XS_INTEGER;
  if ( ZSTREQ( type_name, "decimal" ) )
    return store::XS_DECIMAL;
  if ( ZSTREQ( type_name, "double" ) )
    return store::XS_DOUBLE;
  if ( ZSTREQ( type_name, "float" ) )
    return store::XS_FLOAT;
  if ( ZSTREQ( type_name, "date" ) )
    return store::XS_DATE;
  if ( ZSTREQ( type_name, "dateTime" ) )
    return store::XS_DATETIME;
  if ( ZSTREQ( type_name, "dateTimeStamp" ) )
    return store::XS_DATETIME_STAMP;
  if ( ZSTREQ( type_name, "dayTimeDuration" ) )
    return store::XS_DT_DURATION;
  if ( ZSTREQ( type_name, "duration" ) )
    return store::XS_DURATION;
  if ( ZSTREQ( type_name, "time" ) )
    return store::XS_TIME;
  if ( ZSTREQ( type_name, "anyURI" ) )
    return store::XS_ANY_URI;
  if ( ZSTREQ( type_name, "base64Binary" ) )
    return store::XS_BASE64BINARY;
  if ( ZSTREQ( type_name, "byte" ) )
    return store::XS_BYTE;
  if ( ZSTREQ( type_name, "gDay" ) )
    return store::XS_GDAY;
  if ( ZSTREQ( type_name, "gMonth" ) )
    return store::XS_GMONTH;
  if ( ZSTREQ( type_name, "gMonthDay" ) )
    return store::XS_GMONTH_DAY;
  if ( ZSTREQ( type_name, "gYear" ) )
    return store::XS_GYEAR;
  if ( ZSTREQ( type_name, "gYearMonth" ) )
    return store::XS_GYEAR_MONTH;
  if ( ZSTREQ( type_name, "hexBinary" ) )
    return store::XS_HEXBINARY;
  if ( ZSTREQ( type_name, "long" ) )
    return store::XS_LONG;
  if ( ZSTREQ( type_name, "negativeInteger" ) )
    return store::XS_NEGATIVE_INTEGER;
  if ( ZSTREQ( type_name, "nonNegativeInteger" ) )
    return store::XS_NON_NEGATIVE_INTEGER;
  if ( ZSTREQ( type_name, "nonPositiveInteger" ) )
    return store::XS_NON_POSITIVE_INTEGER;
  if ( ZSTREQ( type_name, "positivInteger" ) )
    return store::XS_POSITIVE_INTEGER;
  if ( ZSTREQ( type_name, "short" ) )
    return store::XS_SHORT;
  if ( ZSTREQ( type_name, "unsignedByte" ) )
    return store::XS_UNSIGNED_BYTE;
  if ( ZSTREQ( type_name, "unsignedInt" ) )
    return store::XS_UNSIGNED_INT;
  if ( ZSTREQ( type_name, "unsignedLong" ) )
    return store::XS_UNSIGNED_LONG;
  if ( ZSTREQ( type_name, "unsignedShort" ) )
    return store::XS_UNSIGNED_SHORT;
  if ( ZSTREQ( type_name, "yearMonthDuration" ) )
    return store::XS_YM_DURATION;
  return store::XS_LAST;
}

static kind map_kind( store::Item::ItemKind k ) {
  switch ( k ) {
    case store::Item::ARRAY : return k_array ;
    case store::Item::ATOMIC: return k_atomic;
    case store::Item::OBJECT: return k_object;
    default                 : return k_none  ;
  }
}

static void assert_type_matches( store::Item_t const &item, type const *t,
                                 char const *name = "" ) {
  kind const k = map_kind( item->getKind() );
  if ( k != t->kind_ )
    throw ZORBA_EXCEPTION(
      jsd::TYPE_MISMATCH,
      ERROR_PARAMS( k, t->kind_, name )
    );
  if ( item->isAtomic() ) {
    atomic_type const *const at = static_cast<atomic_type const*>( t );
    store::SchemaTypeCode const stc = at->schemaTypeCode_;
    if ( !TypeOps::is_subtype( item->getTypeCode(), stc ) )
      throw ZORBA_EXCEPTION(
        jsd::TYPE_MISMATCH,
        ERROR_PARAMS( item->getTypeCode(), t->name_, name )
      );
  }
}

inline void assert_type_matches( store::Item_t const &item, type const *t,
                                 zstring const &name ) {
  assert_type_matches( item, t, name.c_str() );
}

static type const* find_builtin_atomic_type( zstring const &type_name,
                                             bool not_found_error = true ) {
  typedef map<char const*,atomic_type const*> map_type;
  static map_type m;

#define INIT_ATOMIC_TYPE(TYPE,CODE)               \
  static atomic_type instance_##TYPE;             \
  instance_##TYPE.name_ = #TYPE;                  \
  instance_##TYPE.schemaTypeCode_ = store::CODE;  \
  m[ #TYPE ] = &instance_##TYPE

  if ( m.empty() ) {
    INIT_ATOMIC_TYPE( anyURI, XS_ANY_URI );
    INIT_ATOMIC_TYPE( base64Binary, XS_BASE64BINARY );
    INIT_ATOMIC_TYPE( boolean, XS_BOOLEAN );
    INIT_ATOMIC_TYPE( byte, XS_BYTE );
    INIT_ATOMIC_TYPE( date, XS_DATE );
    INIT_ATOMIC_TYPE( dateTime, XS_DATETIME );
    INIT_ATOMIC_TYPE( dateTimeStamp, XS_DATETIME_STAMP );
    INIT_ATOMIC_TYPE( dayTimeDuration, XS_DT_DURATION );
    INIT_ATOMIC_TYPE( decimal, XS_DECIMAL );
    INIT_ATOMIC_TYPE( double, XS_DOUBLE );
    INIT_ATOMIC_TYPE( duration, XS_DURATION );
    INIT_ATOMIC_TYPE( float, XS_FLOAT );
    INIT_ATOMIC_TYPE( gDay, XS_GDAY );
    INIT_ATOMIC_TYPE( gMonth, XS_GMONTH );
    INIT_ATOMIC_TYPE( gMonthDay, XS_GMONTH_DAY );
    INIT_ATOMIC_TYPE( gYear, XS_GYEAR );
    INIT_ATOMIC_TYPE( gYearMonth, XS_GYEAR_MONTH );
    INIT_ATOMIC_TYPE( hexBinary, XS_HEXBINARY );
    INIT_ATOMIC_TYPE( integer, XS_INTEGER );
    INIT_ATOMIC_TYPE( long, XS_LONG );
    INIT_ATOMIC_TYPE( negativeInteger, XS_NEGATIVE_INTEGER );
    INIT_ATOMIC_TYPE( nonNegativeInteger, XS_NON_NEGATIVE_INTEGER );
    INIT_ATOMIC_TYPE( nonPositiveInteger, XS_NON_POSITIVE_INTEGER );
    INIT_ATOMIC_TYPE( positiveInteger, XS_POSITIVE_INTEGER );
    INIT_ATOMIC_TYPE( short, XS_SHORT );
    INIT_ATOMIC_TYPE( string, XS_STRING );
    INIT_ATOMIC_TYPE( time, XS_TIME );
    INIT_ATOMIC_TYPE( unsignedByte, XS_UNSIGNED_BYTE );
    INIT_ATOMIC_TYPE( unsignedInt, XS_UNSIGNED_INT );
    INIT_ATOMIC_TYPE( unsignedLong, XS_UNSIGNED_LONG );
    INIT_ATOMIC_TYPE( unsignedShort, XS_UNSIGNED_SHORT );
    INIT_ATOMIC_TYPE( yearMonthDuration, XS_YM_DURATION );

    GENV_ITEMFACTORY->createInteger(
      instance_byte.maxInclusive_, xs_integer( 255 )
    );
    GENV_ITEMFACTORY->createInteger(
      instance_byte.minInclusive_, xs_integer( -256 )
    );
    // TODO: add facets to other types
  }

  map_type::const_iterator const i( m.find( type_name.c_str() ) );
  if ( i != m.end() )
    return i->second;
  if ( not_found_error )
    throw ZORBA_EXCEPTION( jsd::UNKNOWN_TYPE, ERROR_PARAMS( type_name ) );
  return nullptr;
}

static kind find_kind( zstring const &name ) {
  if ( ZSTREQ( name, "atomic" ) )
    return k_atomic;
  if ( ZSTREQ( name, "array" ) )
    return k_array;
  if ( ZSTREQ( name, "object" ) )
    return k_object;
  if ( ZSTREQ( name, "union" ) )
    return k_union;
  return k_none;
}

inline int get_int( store::Item_t const &item ) {
  return to_xs_int( item->getIntegerValue() );
}

static store::Item_t get_value( store::Item_t const &jsd, char const *key ) {
  zstring s( key );
  store::Item_t key_item, value_item;
  GENV_ITEMFACTORY->createString( key_item, s );
  return jsd->getObjectValue( key_item );
}

static store::Item_t require_value( store::Item_t const &jsd,
                                    char const *key ) {
  store::Item_t value_item( get_value( jsd, key ) );
  if ( !value_item )
    throw ZORBA_EXCEPTION( jsd::MISSING_KEY, ERROR_PARAMS( key ) );
  return value_item;
}

///////////////////////////////////////////////////////////////////////////////

ostream& operator<<( ostream &os, kind k ) {
  switch ( k ) {
    case k_none  : return os << "none";
    case k_array : return os << "array";
    case k_atomic: return os << "atomic";
    case k_object: return os << "object";
    case k_union : return os << "union";
  }
}

///////////////////////////////////////////////////////////////////////////////

array_type::array_type() : min_max_type( k_array ) {
}

array_type::~array_type() {
  ztd::delete_ptr_seq( content_ );
}

void array_type::load_content( store::Item_t const &content_item,
                               validator const &v ) {
  JSOUND_ASSERT_KIND( content_item, "$content", ARRAY );
  if ( content_item->getArraySize() != numeric_consts<xs_integer>::one() )
    throw ZORBA_EXCEPTION( jsd::ILLEGAL_ARRAY_SIZE );
  store::Item_t const type_item(
    content_item->getArrayValue( numeric_consts<xs_integer>::one() )
  );
  if ( IS_ATOMIC_TYPE( type_item, XS_STRING ) ) {
    zstring fq_name_str( type_item->getStringValue() );
    type_ = v.fq_find_type( &fq_name_str );
  } else if ( IS_KIND( type_item, OBJECT ) ) {
    // TODO
  } else
    throw ZORBA_EXCEPTION( jsd::ILLEGAL_ARRAY_TYPE );
}

void array_type::load_type( store::Item_t const &type_item,
                            validator const &v ) {
  JSOUND_ASSERT_KIND( type_item, "$type", OBJECT );
  store::Iterator_t it( type_item->getObjectKeys() );
  store::Item_t item;
  it->open();
  while ( it->next( item ) ) {
    zstring const key( item->getStringValue() );
    store::Item_t const value_item( type_item->getObjectValue( item ) );
    if ( ZSTREQ( key, "$constraints" ) )
      load_constraints( value_item );
    else if ( ZSTREQ( key, "$content" ) )
      load_content( value_item, v );
    else if ( ZSTREQ( key, "$enumeration" ) )
      load_enumeration( value_item );
    else if ( ZSTREQ( key, "$maxLength" ) )
      load_maxLength( value_item );
    else if ( ZSTREQ( key, "$minLength" ) )
      load_minLength( value_item );
    else if ( ZSTREQ( key, "$kind" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$name" ) )
      load_name( value_item, v );
    else
      throw ZORBA_EXCEPTION( jsd::ILLEGAL_KEY, ERROR_PARAMS( key, "$type" ) );
  } // while
  it->close();
}

void array_type::validate( store::Item_t const &array_item ) const {
  JSOUND_ASSERT_KIND( array_item, name_, ARRAY );
  store::Iterator_t it( array_item->getArrayValues() );
  store::Item_t item;
  it->open();
  while ( it->next( item ) )
    assert_type_matches( item, type_ );
  it->close();
}

///////////////////////////////////////////////////////////////////////////////

atomic_type::atomic_type() : min_max_type( k_atomic ) {
  explicitTimezone_ = timezone::optional;  // TODO: correct?
}

void atomic_type::assert_min_max_facet( store::Item_t const &item,
                                        char const *facet_name ) const {
  switch ( schemaTypeCode_ ) {
    case store::XS_DATE:
    case store::XS_DATETIME:
    case store::XS_DOUBLE:
    case store::XS_DURATION:
    case store::XS_FLOAT:
    case store::XS_GDAY:
    case store::XS_GMONTH:
    case store::XS_GMONTH_DAY:
    case store::XS_GYEAR:
    case store::XS_GYEAR_MONTH:
    case store::XS_TIME:
      break;
    default:
      if ( !IS_SUBTYPE( schemaTypeCode_, XS_DECIMAL ) )
        throw ZORBA_EXCEPTION(
          jsd::ILLEGAL_FACET, ERROR_PARAMS( facet_name, schemaTypeCode_ )
        );
  }
  assert_type( item, facet_name, schemaTypeCode_ );
}

void atomic_type::load_baseType( store::Item_t const &baseType_item,
                                 validator const &v ) {
  if ( !baseType_item )
    throw ZORBA_EXCEPTION( jsd::MISSING_KEY, ERROR_PARAMS( "$baseType" ) );
  type::load_baseType( baseType_item, v );
  schemaTypeCode_ = map_atomic_type( baseType_item->getStringValue() );
}

void atomic_type::load_explicitTimezone( store::Item_t const &eTz_item ) {
  JSOUND_ASSERT_TYPE( eTz_item, "$explicitTimezone", XS_STRING );
  switch ( schemaTypeCode_ ) {
    case store::XS_DATE:
    case store::XS_DATETIME:
    case store::XS_TIME:
      break;
    default:
      throw ZORBA_EXCEPTION(
        jsd::ILLEGAL_FACET, ERROR_PARAMS( "$explicitTimezone", schemaTypeCode_ )
      );
  }
  zstring const eTz_str( eTz_item->getStringValue() );
  if ( ZSTREQ( eTz_str, "optional" ) )
    explicitTimezone_ = timezone::optional;
  else if ( ZSTREQ( eTz_str, "prohibited" ) )
    explicitTimezone_ = timezone::prohibited;
  else if ( ZSTREQ( eTz_str, "required" ) )
    explicitTimezone_ = timezone::required;
  else
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_EXPLICIT_TIMEZONE,
      ERROR_PARAMS( eTz_str )
    );
  facet_mask_ |= facet_explicitTimezone;
}

void atomic_type::load_fractionDigits( store::Item_t const &fDigits_item ) {
  JSOUND_ASSERT_TYPE( fDigits_item, "$fractionDigits", XS_INTEGER );
  if ( schemaTypeCode_ != store::XS_DECIMAL )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_FACET, ERROR_PARAMS( "$fractionDigits", schemaTypeCode_ )
    );
  fractionDigits_ = get_int( fDigits_item );
  // TODO: assert >= 0
  facet_mask_ |= facet_fractionDigits;
}

void atomic_type::load_length( store::Item_t const &length_item ) {
  JSOUND_ASSERT_TYPE( length_item, "$length", XS_INTEGER );
  switch ( schemaTypeCode_ ) {
    case store::XS_ANY_URI:
    case store::XS_BASE64BINARY:
    case store::XS_HEXBINARY:
    case store::XS_STRING:
      break;
    default:
      throw ZORBA_EXCEPTION(
        jsd::ILLEGAL_FACET, ERROR_PARAMS( "$length", schemaTypeCode_ )
      );
  }
  length_ = get_int( length_item );
  // TODO: assert >= 0
  facet_mask_ |= facet_length;
}

void atomic_type::load_maxExclusive( store::Item_t const &maxExclusive_item ) {
  assert_min_max_facet( maxExclusive_item, "$maxExclusive" );
  maxExclusive_ = maxExclusive_item;
  facet_mask_ |= facet_maxExclusive;
}

void atomic_type::load_maxInclusive( store::Item_t const &maxInclusive_item ) {
  assert_min_max_facet( maxInclusive_item, "$maxInclusive" );
  maxInclusive_ = maxInclusive_item;
  facet_mask_ |= facet_maxInclusive;
}

void atomic_type::load_minExclusive( store::Item_t const &minExclusive_item ) {
  assert_min_max_facet( minExclusive_item, "$minExclusive" );
  minExclusive_ = minExclusive_item;
  facet_mask_ |= facet_minExclusive;
}

void atomic_type::load_minInclusive( store::Item_t const &minInclusive_item ) {
  assert_min_max_facet( minInclusive_item, "$minInclusive" );
  minInclusive_ = minInclusive_item;
  facet_mask_ |= facet_minInclusive;
}

void atomic_type::load_pattern( store::Item_t const &pattern_item ) {
  JSOUND_ASSERT_TYPE( pattern_item, "$pattern", XS_STRING );
  pattern_ = pattern_item->getStringValue();
  // TODO: verify that the pattern is valid regex
  facet_mask_ |= facet_pattern;
}

void atomic_type::load_totalDigits( store::Item_t const &totalDigits_item ) {
  JSOUND_ASSERT_TYPE( totalDigits_item, "$totalDigits", XS_INTEGER );
  if ( !IS_SUBTYPE( schemaTypeCode_, XS_DECIMAL ) )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_FACET, ERROR_PARAMS( "$totalDigits", schemaTypeCode_ )
    );
  totalDigits_ = get_int( totalDigits_item );
  // TODO: assert >= 0
  facet_mask_ |= facet_totalDigits;
}

void atomic_type::load_type( store::Item_t const &type_item,
                             validator const &v ) {
  store::Iterator_t it( type_item->getObjectKeys() );
  store::Item_t item;
  it->open();
  while ( it->next( item ) ) {
    zstring const key( item->getStringValue() );
    store::Item_t const value_item( type_item->getObjectValue( item ) );
    if ( ZSTREQ( key, "$about" ) )
      load_about( value_item );
    else if ( ZSTREQ( key, "$baseType" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$constraints" ) )
      load_constraints( value_item );
    else if ( ZSTREQ( key, "$enumeration" ) )
      load_enumeration( value_item );
    else if ( ZSTREQ( key, "$explicitTimezone" ) )
      load_explicitTimezone( value_item );
    else if ( ZSTREQ( key, "$fractionDigits" ) )
      load_fractionDigits( value_item );
    else if ( ZSTREQ( key, "$kind" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$length" ) )
      load_length( value_item );
    else if ( ZSTREQ( key, "$maxExclusive" ) )
      load_maxExclusive( value_item );
    else if ( ZSTREQ( key, "$maxInclusive" ) )
      load_maxInclusive( value_item );
    else if ( ZSTREQ( key, "$maxLength" ) )
      load_maxLength( value_item );
    else if ( ZSTREQ( key, "$minExclusive" ) )
      load_minExclusive( value_item );
    else if ( ZSTREQ( key, "$minInclusive" ) )
      load_minInclusive( value_item );
    else if ( ZSTREQ( key, "$minLength" ) )
      load_minLength( value_item );
    else if ( ZSTREQ( key, "$name" ) )
      load_name( value_item, v );
    else if ( ZSTREQ( key, "$pattern" ) )
      load_pattern( value_item );
    else if ( ZSTREQ( key, "$totalDigits" ) )
      load_totalDigits( value_item );
    else
      throw ZORBA_EXCEPTION( jsd::ILLEGAL_KEY, ERROR_PARAMS( key, "$type" ) );
  } // while
  it->close();
}

void atomic_type::validate( store::Item_t const &item ) const {
  assert_type( item, name_, schemaTypeCode_ );

  if ( !(!HAS_FACET( length ) || item->getStringValue().length() == length_) )
    throw FACET_EXCEPTION( item, length );

  if ( !(!HAS_FACET( maxExclusive ) || item->compare( maxExclusive_ ) < 0) )
    throw FACET_EXCEPTION( item, maxInclusive );
  if ( !(!HAS_FACET( maxInclusive ) || item->compare( maxInclusive_ ) <= 0) )
    throw FACET_EXCEPTION( item, maxInclusive );
  if ( !(!HAS_FACET( minExclusive ) || item->compare( minExclusive_ ) > 0) )
    throw FACET_EXCEPTION( item, minInclusive );
  if ( !(!HAS_FACET( minInclusive ) || item->compare( minInclusive_ ) >= 0) )
    throw FACET_EXCEPTION( item, minInclusive );

  // TODO: totalDigits
  // TODO: fractionDigits
  // TODO: explicitTimezone
  // TODO: pattern
}

///////////////////////////////////////////////////////////////////////////////

min_max_type::min_max_type( kind k ) : type( k ) {
  facet_mask_ = 0;
}

void min_max_type::load_maxLength( store::Item_t const &maxLength_item ) {
  JSOUND_ASSERT_TYPE( maxLength_item, "$maxLength", XS_INTEGER );
  maxLength_ = get_int( maxLength_item );
  facet_mask_ |= facet_maxLength;
}

void min_max_type::load_minLength( store::Item_t const &minLength_item ) {
  JSOUND_ASSERT_TYPE( minLength_item, "$minLength", XS_INTEGER );
  minLength_ = get_int( minLength_item );
  facet_mask_ |= facet_minLength;
}

///////////////////////////////////////////////////////////////////////////////

object_type::field_descriptor::field_descriptor() {
  type_ = nullptr;
  optional_ = false;
}

void object_type::field_descriptor::
load_default( store::Item_t const &default_item ) {
  assert_type_matches( default_item, type_, "$default" );
  default_ = default_item;
}

void object_type::field_descriptor::
load_optional( store::Item_t const &optional_item ) {
  JSOUND_ASSERT_TYPE( optional_item, "$optional", XS_BOOLEAN );
  optional_ = optional_item->getBooleanValue();
}

void object_type::field_descriptor::
load_type( store::Item_t const &type_item, validator const &v ) {
  if ( IS_ATOMIC_TYPE( type_item, XS_STRING ) ) {
    zstring fq_name_str( type_item->getStringValue() );
    type_ = v.fq_find_type( &fq_name_str );
  } else if ( IS_KIND( type_item, OBJECT ) ) {
    // TODO
  } else
    throw ZORBA_EXCEPTION( jsd::ILLEGAL_OBJECT_TYPE );
}

object_type::object_type() : type( k_object ) {
  open_ = true;
}

void object_type::load_content( store::Item_t const &content_item,
                                validator const &v ) {
  JSOUND_ASSERT_KIND( content_item, "$content", OBJECT );
  store::Iterator_t it( content_item->getObjectKeys() );
  store::Item_t key_item;
  it->open();
  while ( it->next( key_item ) ) {
    // key_item is guaranteed to be a string by JSON syntax
    zstring const key_str( key_item->getStringValue() );
    // duplicate keys are checked for by JSON semantics
    field_descriptor &fd = content_[ key_str ];
    load_field_descriptor( content_item->getObjectValue( key_item ), v, &fd );
  }
  it->close();
}

void object_type::load_field_descriptor( store::Item_t const &field_item,
                                         validator const &v,
                                         field_descriptor *fd ) {
  JSOUND_ASSERT_KIND( field_item, "field descriptor", OBJECT );
  store::Item_t const type_item( require_value( field_item, "$type" ) );
  fd->load_type( type_item, v );
  store::Item_t const optional_item( get_value( field_item, "$optional" ) );
  if ( !!optional_item )
    fd->load_optional( optional_item );
  store::Item_t const default_item( get_value( field_item, "$default" ) );
  if ( !!default_item )
    fd->load_default( default_item );
}

void object_type::load_open( store::Item_t const &open_item ) {
  JSOUND_ASSERT_TYPE( open_item, "$open", XS_BOOLEAN );
  if ( baseType_ ) {
    object_type const *const bt = static_cast<object_type const*>( baseType_ );
    if ( !bt->open_ ) {
      //
      // JSound 5.4: If the $baseType's $open property is false, then $open
      // cannot be set back to true.
      //
      return;
    }
  }
  open_ = open_item->getBooleanValue();
}

void object_type::load_type( store::Item_t const &type_item,
                             validator const &v ) {
  store::Item_t const baseType_item( get_value( type_item, "$baseType" ) );
  if ( !!baseType_item )
    load_baseType( baseType_item, v );
  store::Item_t const content_item( require_value( type_item, "$content" ) );
  load_content( content_item, v );

  store::Iterator_t it( type_item->getObjectKeys() );
  store::Item_t item;
  it->open();
  while ( it->next( item ) ) {
    zstring const key( item->getStringValue() );
    store::Item_t const value_item( type_item->getObjectValue( item ) );
    if ( ZSTREQ( key, "$baseType" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$constraints" ) )
      load_constraints( value_item );
    else if ( ZSTREQ( key, "$content" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$enumeration" ) )
      load_enumeration( value_item );
    else if ( ZSTREQ( key, "$kind" ) )
      /* already handled */;
    else if ( ZSTREQ( key, "$name" ) )
      load_name( value_item, v );
    else if ( ZSTREQ( key, "$open" ) )
      load_open( value_item );
    else
      throw ZORBA_EXCEPTION( jsd::ILLEGAL_KEY, ERROR_PARAMS( key, "$type" ) );
  } // while
  it->close();
}

void object_type::validate( store::Item_t const &item ) const {
  // TODO
}

///////////////////////////////////////////////////////////////////////////////

type::type( kind k ) : kind_( k ) {
  baseType_ = nullptr;
}

type::~type() {
  // out-of-line since it's virtual
}

void type::load_about( store::Item_t const &about_item ) {
  JSOUND_ASSERT_TYPE( about_item, "$about", XS_STRING );
  about_ = about_item->getStringValue();
}

void type::load_baseType( store::Item_t const &baseType_item,
                          validator const &v ) {
  if ( !baseType_item )
    return;
  JSOUND_ASSERT_TYPE( baseType_item, "$baseType", XS_STRING );
  zstring const baseType_str( baseType_item->getStringValue() );
  if ( kind const k = find_kind( baseType_str ) ) {
    //
    // The case where the kind of the base type of "array", "atomic", "object",
    // or "union" is specified explicitly ...
    //
    if ( k != kind_ ) {
      //
      // ... and does NOT match the kind of this type.
      //
      throw ZORBA_EXCEPTION(
        jsd::ILLEGAL_BASE_TYPE,
        ERROR_PARAMS( k, name_, kind_ )
      );
    }
    return;
  }

  zstring fq_baseType_str( baseType_str );
  type const *const bt = v.fq_find_type( &fq_baseType_str );

  if ( bt->kind_ != kind_ )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_BASE_TYPE,
      ERROR_PARAMS( fq_baseType_str, name_, kind_ )
    );
  baseType_ = bt;
}

void type::load_constraints( store::Item_t const &constraints_item ) {
  // TODO
  facet_mask_ |= facet_constraints;
}

void type::load_enumeration( store::Item_t const &enumeration_item ) {
  JSOUND_ASSERT_KIND( enumeration_item, "$enumeration", ARRAY );
  store::Iterator_t it( enumeration_item->getArrayValues() );
  store::Item_t item;
  it->open();
  while ( it->next( item ) ) {
    assert_type_matches( item, this );
    enumeration_.values_.push_back( item );
  }
  it->close();
  facet_mask_ |= facet_enumeration;
}

void type::load_name( store::Item_t const &name_item, validator const &v ) {
  JSOUND_ASSERT_TYPE( name_item, "$name", XS_STRING );
  zstring fq_name_str( name_item->getStringValue() );
  zstring uri;
  v.fq_type_name( &fq_name_str, &uri );
  if ( !uri.empty() && uri != v.get_namespace() )
    throw ZORBA_EXCEPTION(
      jsd::ILLEGAL_NAMESPACE,
      ERROR_PARAMS( fq_name_str, v.get_namespace() )
    );
  if ( v.find_type( fq_name_str, false ) )
    throw ZORBA_EXCEPTION( jsd::DUPLICATE_TYPE, ERROR_PARAMS( fq_name_str ) );
  name_ = fq_name_str;
}

///////////////////////////////////////////////////////////////////////////////

union_type::union_type() : type( k_union ) {
}

void union_type::load_type( store::Item_t const &type, validator const &v ) {
  // TODO
}

void union_type::validate( store::Item_t const &item ) const {
  // TODO
}

///////////////////////////////////////////////////////////////////////////////

validator::validator( store::Item_t const &jsd_item ) {
  JSOUND_ASSERT_KIND( jsd_item, "JSound", OBJECT );
  load_namespace( require_value( jsd_item, "$namespace" ) );
  store::Item_t item( get_value( jsd_item, "$imports" ) );
  if ( !!item )
    load_imports( item );
  load_types( require_value( jsd_item, "$types" ) );
}

validator::~validator() {
  MUTATE_EACH( typename_map, i, types_ )
    delete i->second;
}

type const* validator::find_type( zstring const &type_name,
                                  bool not_found_error ) const {
  type const *t = find_builtin_atomic_type( type_name, false );
  if ( t )
    return t;
  if ( type_name.compare( 0, 2, "Q{" ) == 0 ) {
    typename_map::const_iterator const i( types_.find( type_name ) );
    if ( i != types_.end() )
      return i->second;
  }
  if ( not_found_error )
    throw ZORBA_EXCEPTION( jsd::UNKNOWN_TYPE, ERROR_PARAMS( type_name ) );
  return nullptr;
}

type const* validator::fq_find_type( zstring *type_name,
                                     bool not_found_error ) const {
  fq_type_name( type_name );
  return find_type( *type_name, not_found_error );
}

void validator::fq_type_name( zstring *type_name, zstring *uri ) const {
  zstring prefix, local, func_local_uri;
  if ( !uri )
    uri = &func_local_uri;

  if ( xml::split_qname( *type_name, &prefix, &local ) && !prefix.empty() ) {
    prefix_namespace_map::const_iterator const i( prefix_ns_.find( prefix ) );
    if ( i == prefix_ns_.end() )
      throw ZORBA_EXCEPTION( jsd::UNKNOWN_PREFIX, ERROR_PARAMS( prefix ) );
    *type_name = "Q{" + i->second + '}' + local;
  } else if ( xml::split_uri_name( *type_name, uri, &local ) ) {
    // Do nothing since type_name is already a URIQualifiedName.
  } else {
    if ( !find_builtin_atomic_type( *type_name, false ) )
      *type_name = "Q{" + namespace_ + '}' + *type_name;
  }
}

void validator::load_import( store::Item_t const &import_item ) {
  JSOUND_ASSERT_KIND( import_item, "$imports array element", OBJECT );
  store::Item_t const ns_item( require_value( import_item, "$namespace" ) );
  JSOUND_ASSERT_TYPE( ns_item, "$namespace", XS_STRING );
  store::Item_t const prefix_item( require_value( import_item, "$prefix" ) );
  JSOUND_ASSERT_TYPE( prefix_item, "$prefix", XS_STRING );
  // TODO: add optional $location
  zstring const ns_str( ns_item->getStringValue() );
  zstring const prefix_str( prefix_item->getStringValue() );
  if ( prefix_str.find( ':' ) != zstring::npos )
    throw ZORBA_EXCEPTION( jsd::ILLEGAL_PREFIX, ERROR_PARAMS( prefix_str ) );
  if ( ztd::contains( prefix_ns_, prefix_str ) )
    throw ZORBA_EXCEPTION( jsd::DUPLICATE_PREFIX, ERROR_PARAMS( prefix_str ) );
  prefix_ns_[ prefix_str ] = ns_str;
}

void validator::load_imports( store::Item_t const &imports_item ) {
  JSOUND_ASSERT_KIND( imports_item, "$imports", ARRAY );
  store::Iterator_t it( imports_item->getArrayValues() );
  store::Item_t import_item;
  it->open();
  while ( it->next( import_item ) )
    load_import( import_item );
  it->close();
}

unique_ptr<type> validator::load_kind( store::Item_t const &kind_item ) {
  JSOUND_ASSERT_TYPE( kind_item, "$kind", XS_STRING );
  zstring const kind_str( kind_item->getStringValue() );
  switch ( find_kind( kind_str ) ) {
    case k_atomic:
      return unique_ptr<type>( new atomic_type );
    case k_array:
      return unique_ptr<type>( new array_type );
    case k_object:
      return unique_ptr<type>( new object_type );
    case k_union:
      return unique_ptr<type>( new union_type );
    default:
      throw ZORBA_EXCEPTION(
        jsd::ILLEGAL_VALUE, ERROR_PARAMS( kind_str, "$kind" )
      );
  } // switch
}

void validator::load_namespace( store::Item_t const &namespace_item ) {
  JSOUND_ASSERT_TYPE( namespace_item, "$namespace", XS_STRING );
  namespace_ = namespace_item->getStringValue();
}

void validator::load_type_top( store::Item_t const &type_item ) {
  store::Item_t const kind_item( require_value( type_item, "$kind" ) );
  store::Item_t const name_item( require_value( type_item, "$name" ) );
  zstring fq_name_str( name_item->getStringValue() );
  fq_type_name( &fq_name_str );
  unique_ptr<type> t( load_kind( kind_item ) );
  t->load_baseType( get_value( type_item, "$baseType" ), *this );
  t->load_type( type_item, *this );
  types_[ fq_name_str ] = t.get();
  t.release();
}

void validator::load_types( store::Item_t const &types_item ) {
  JSOUND_ASSERT_KIND( types_item, "$types", ARRAY );
  store::Iterator_t it( types_item->getArrayValues() );
  store::Item_t type_item;
  it->open();
  while ( it->next( type_item ) )
    load_type_top( type_item );
  it->close();
}

void validator::validate( store::Item_t const &json, zstring const &type_name,
                          store::Item_t *result ) const {
  zstring fq_name_str( type_name );
  fq_find_type( &fq_name_str )->validate( json );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jsound
} // namespace zorba

/* vim:set et sw=2 ts=2: */
