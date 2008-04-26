/**
 * @copyright
 * ========================================================================
 *  Copyright FLWOR Foundation
 * ========================================================================
 *
 * @author Sorin Nasoi (sorin.nasoi@ipdevel.ro)
 * @author Nicolae Brinza (nicolae.brinza@ipdevel.ro)
 * @author Dan Muresan (dan.muresan@ipdevel.ro)
 * @file util/utf8/xqpString.cpp
 *
 */



#include <unicode/regex.h>
#include "zorbatypes/xqpstring.h"

#ifndef ZORBA_NO_UNICODE
#include "zorbatypes/Unicode_util.h"
#include "zorbatypes/utf8.h"
#include "zorbatypes/numconversions.h"
#include "zorbatypes/collation_manager.h"


using namespace std;


namespace zorba
{


/*******************************************************************************
  whitespace = " \t\r\n" meaning (#x20) (#x9) (#xD) (#xA)
********************************************************************************/
bool xqpStringStore::is_whitespace(uint32_t cp)
{
  bool res = (cp == 0x20 || cp == 0x9 || cp == 0xD || cp == 0xA) ? true : false;

  return res;
}


/*******************************************************************************
  Returns true is cp reprezents "unreserved" as defined by rfc3986.
********************************************************************************/
bool xqpStringStore::is_unreservedCP(uint32_t cp)
{
  bool ret = false;
  if((0x30 <= cp && cp <= 0x39)|| //0-9
     (0x41 <= cp && cp <= 0x5A)|| //A-Z
     (0x61 <= cp && cp <= 0x7A)|| //a-z
     cp == 0x2D || //-
     cp == 0x2E || //.
     cp == 0x5F || //_
     cp == 0x7E)   //~
  {  
    ret = true;
  }
  return ret;
}


/*******************************************************************************
  Returns true is cp reprezents "ucschar" as defined by rfc3987
********************************************************************************/
bool xqpStringStore::is_ucscharCP(uint32_t cp)
{
  bool ret = false;
  if((0xA0 <= cp && cp <=0xD7FF) ||
     (0xF900 <= cp && cp <=0xFDCF) ||
     (0xFDF0 <= cp && cp <=0xFFEF) ||
     (0x10000 <= cp && cp <=0x1FFFD) ||
     (0x20000 <= cp && cp <=0x2FFFD) ||
     (0x30000 <= cp && cp <=0x3FFFD) ||
     (0x40000 <= cp && cp <=0x4FFFD) ||
     (0x50000 <= cp && cp <=0x5FFFD) ||
     (0x60000 <= cp && cp <=0x6FFFD) ||
     (0x70000 <= cp && cp <=0x7FFFD) ||
     (0x80000 <= cp && cp <=0x8FFFD) ||
     (0x90000 <= cp && cp <=0x9FFFD) ||
     (0xA0000 <= cp && cp <=0xAFFFD) ||
     (0xB0000 <= cp && cp <=0xBFFFD) ||
     (0xC0000 <= cp && cp <=0xCFFFD) ||
     (0xD0000 <= cp && cp <=0xDFFFD) ||
     (0xE0000 <= cp && cp <=0xEFFFD))
  {
    ret = true;
  }
  return ret;
}


/*******************************************************************************
  Return true is cp reprezents "iprivate" as defined by rfc3987
********************************************************************************/
bool xqpStringStore::is_iprivateCP( uint32_t cp ) const
{
  bool ret = false;
  if((0xE000 <= cp && cp <=0xF8FF) ||
     (0xF0000 <= cp && cp <=0xFFFFD) ||
     (0x100000 <= cp && cp <= 0x10FFFD)
     )
  {
    ret = true;
  }
  return ret;
}


/*******************************************************************************
  Return true if cp is a printable characters of the US-ASCII coded character
  set meaning octets ranging from 32 to 126 (decimal).
********************************************************************************/
bool xqpStringStore::is_printableASCII(uint32_t cp) const
{
  bool ret = false;
  if(0x20 <= cp && cp <=0x7E)//32 to 126 (decimal)
  {
    ret = true;
  }
  return ret;
}


/*******************************************************************************
  Return true for the following printable ASCII characters that are invalid in
  an IRI: "<", ">", " " " (double quote), space, "{", "}", "|", "\", "^", and "`".
********************************************************************************/
bool xqpStringStore::is_Invalid_in_IRI(uint32_t cp) const
{
  bool ret = false;
  if(0x3C == cp || 0x3E == cp || 0x22 == cp ||
     0x7B == cp || 0x7D == cp || 0x7C == cp || 
     0x5C == cp || 0X5E == cp || 0x60 == cp ||
     0x20 == cp)
  {
    ret = true;
  }
  return ret;
}


/*******************************************************************************

********************************************************************************/
uint32_t xqpStringStore::hash(const char* str)
{
  uint32_t hash = 5381;
  int c;
  while ((c = *str++))
  {
    hash = ((hash << 5) + hash) + c; // hash*33 + c
  }
  return hash;
}


/*******************************************************************************

********************************************************************************/
uint32_t xqpStringStore::hash(XQPCollator* coll) const
{
    if(!coll) {
      return hash(c_str());
    }

    CollationKey collKey;
    UErrorCode status = U_ZERO_ERROR;
    
    coll->theCollator->getCollationKey(this->getUnicodeString(), collKey, status);

    if(U_FAILURE(status))
    {
      assert(false);
    }
    
    return collKey.hashCode();
}


/*******************************************************************************

********************************************************************************/
void xqpStringStore::clear()
{
  theString.erase();
}


/*******************************************************************************

********************************************************************************/
std::string::size_type xqpStringStore::numChars() const
{
  const char* c = c_str();
  return UTF8Distance(c, c + bytes());
}


/*******************************************************************************

********************************************************************************/
bool xqpStringStore::byteEqual(const xqpStringStore& src) const
{
  if (this == &src)
    return true;

  uint32_t len = bytes();

  if(len == src.bytes() && memcmp(c_str(), src.c_str(), len) == 0)
    return true;

  return false;
}


bool xqpStringStore::byteEqual(const char* src, uint32_t srclen) const
{
  if(bytes() == srclen && memcmp(c_str(), src, srclen) == 0)
    return true;
  
  return false;
}


int xqpStringStore::compare(const xqpStringStore* src, XQPCollator* coll) const
{
  if ( ! coll )
    return theString.compare(src->theString);

  Collator::EComparisonResult result = ::Collator::EQUAL;

  result = coll->theCollator->compare(this->getUnicodeString(),
                                      src->getUnicodeString());

  return result;
}


/*******************************************************************************
  Locate in "this" the first occurrence of the "pattern" substring. Return the
  offset into this of the start of "pattern", or -1 if not found.
********************************************************************************/
int32_t xqpStringStore::indexOf(const char* pattern) const
{
  if (empty())
    return -1;

  size_t lRes = theString.find(pattern);
  return (lRes == std::string::npos) ? -1 : lRes;
}


/*******************************************************************************
  Locate in "this" the first occurrence of the "pattern" substring, using the
  given collation. Return the offset into this of the start of "pattern", or
  -1 if not found.
********************************************************************************/
int32_t xqpStringStore::indexOf(const xqpStringStore* pattern, XQPCollator* coll) const
{
  if (empty())
    return -1;

  if ( ! coll ) 
  {
    size_t lRes = theString.find(pattern->c_str());
    return (lRes == std::string::npos) ? -1 : lRes;
  }

  UErrorCode status = U_ZERO_ERROR;

  StringSearch search(pattern->getUnicodeString(),
                      getUnicodeString(), 
                      (RuleBasedCollator*)coll->theCollator, NULL, status);

  if(U_FAILURE(status))
  {
    assert(false);
    return -1;
  }

  for(int16_t pos = search.first(status);
      U_SUCCESS(status) && pos != USEARCH_DONE;
      pos = search.next(status))
  {
    return pos;
  }

  if (U_FAILURE(status))
  {
    assert(false);
    return -1;
  }
  return -1;
}


/*******************************************************************************

********************************************************************************/
int32_t xqpStringStore::lastIndexOf(const char* pattern) const
{
  size_t lRes = theString.rfind(pattern);
  return (lRes == std::string::npos) ? -1 : lRes;
}


/*******************************************************************************

********************************************************************************/
int32_t xqpStringStore::lastIndexOf(const xqpStringStore* pattern, XQPCollator* coll) const
{
  if ( ! coll )
  {
    size_t lRes = theString.rfind(pattern->c_str());
    return (lRes == std::string::npos) ? -1 : lRes;
  }

  UErrorCode status = U_ZERO_ERROR;

  StringSearch search(pattern->getUnicodeString(),
                      getUnicodeString(), 
                      (RuleBasedCollator *)coll->theCollator, NULL, status);

  if(U_FAILURE(status))
  {
    assert(false);
    return -1;
  }

  int32_t pos = search.last(status);
  if (U_FAILURE(status))
  {
    assert(false);
    return -1;
  }

  if(U_SUCCESS(status) && pos != USEARCH_DONE)
  {
    //TODO check if this condition is enough
    return pos;
  }

  return -1;
}


/*******************************************************************************
  Determine if "pattern" is a suffix of "this"
********************************************************************************/
bool xqpStringStore::endsWith(const char* pattern) const
{
  //TODO check if this condition is enough
  return (lastIndexOf(pattern) + strlen(pattern) == bytes());
}


/*******************************************************************************
  Determine if "pattern" is a suffix of "this"
********************************************************************************/
bool xqpStringStore::endsWith(const xqpStringStore* pattern, XQPCollator* coll) const
{
  //TODO check if this condition is enough
  return (lastIndexOf(pattern, coll) + pattern->numChars() == numChars());
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::append(const xqpStringStore_t& suffix) const
{
  return new xqpStringStore(theString + suffix->str());
}


xqpStringStore_t xqpStringStore::append(const std::string& suffix) const
{
  return new xqpStringStore(theString + suffix);
}


xqpStringStore_t xqpStringStore::append(const char* suffix) const
{
  return new xqpStringStore(theString + suffix);
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::uppercase() const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  uint32_t cp;
  char seq[5];

  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  for(i = 0; i < len; ++i)
  {
    cp = toUpper(UTF8Decode(c));
    memset(seq, 0, sizeof(seq));
    UTF8Encode(cp, seq);
    newStr->theString += seq;
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::lowercase() const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  uint32_t cp;
  char seq[5];

  //create the new xqpStringStore
  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  for(i = 0; i < len; ++i)
  {
    cp = toLower(UTF8Decode(c));
    memset(seq, 0, sizeof(seq));
    UTF8Encode(cp, seq);
    newStr->theString += seq;
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************
  Returns a new xqpString by stripping leading and trailing whitespace and
  replacing sequences of one or more than one whitespace character with a
  single space, #x20.
********************************************************************************/
xqpStringStore_t xqpStringStore::normalizeSpace() const
{
  //create the new xqpStringStore
  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  uint32_t len = numChars();
  const char* c = c_str();
  uint32_t cp, cpPrev;
  char seq[5];

  cpPrev = 0x20;
  while(len > 0)
  {
    cp = UTF8Decode(c);
    if( !is_whitespace(cp) ||
        (is_whitespace(cp) && !is_whitespace(cpPrev))
        )
    {
      if(is_whitespace(cp))
        cp = 0x20;
        
      memset(seq, 0, sizeof(seq));
      UTF8Encode(cp, seq);
      newStr->theString += seq;
    }
    cpPrev = cp;
    --len;
  }
  newStr->theString += "\0";

  return newStr->trimR();
}


/*******************************************************************************
  Create a new xqpStringStore obj that is a suffix of "this". The suffix is
  defined by removing from "this" all of its leading chars that belong to a
  given set S of chars. S is defined as the 1st "len" chars in the "start"
  string. 
********************************************************************************/
xqpStringStore_t xqpStringStore::trimL(const char* start, uint16_t len) const
{
  if(empty() || 0 == len)
    return new xqpStringStore(*this);

  //create the new xqpStringStore
  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  uint32_t StrLen = numChars();
  const char* c = c_str();

  uint32_t* trimCP;//[len];
  trimCP = new uint32_t[len];
  for(uint16_t i = 0; i < len; i++)
    trimCP[i] = UTF8Decode(start);

  bool found = false; 
  bool firstCp = true;
  
  while(StrLen > 0 && !found)
  {
    uint32_t cp = UTF8Decode(c);

    for(uint16_t i = 0; i < len; i++)
    {
      if(trimCP[i] == cp)
      {
        firstCp = false;
        break;
      }
    }

    if(firstCp)
    {
      char seq[5];
      memset(seq, 0, sizeof(seq));
      UTF8Encode(cp, seq);
      newStr->theString += seq;
      newStr->theString += c;
      found = true;
    }

    --StrLen;
    firstCp = true;
  }

  delete[] trimCP;
  return newStr.release();
}
  

/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::trimL() const
{
  char seq = ' ';
  return trimL( &seq, 1 );
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::trimR(const char* start, uint16_t len) const
{
  if(empty() || 0 == len )
    return new xqpStringStore(*this);
    
  uint32_t StrLen = numChars();

  uint32_t* trimCP;//[len];
  trimCP = new uint32_t[len];
  for(uint16_t i = 0; i < len; i++)
    trimCP[i] = UTF8Decode(start);

  //create the new xqpStringStore
  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  uint32_t pos = 0;
  uint32_t cp = 0;
  const char* end = c_str();
  const char* c = c_str();
  char seq[5];

  zorba::advance(end, StrLen, end + bytes());

  bool firstCp = true;

  while(StrLen > 0)
  {
    cp = UTF8DecodePrev(end);

    for(uint16_t i=0; i<len; i++)
    {
      if(trimCP[i] == cp)
      {
        firstCp = false;
        break;
      }
    }
      
    if( firstCp )
    {
      pos = zorba::UTF8Distance(c, end);
      break;
    }
    --StrLen;
    firstCp = true;
  }
    
  ++pos;

  while(pos > 0)
  {
    cp = UTF8Decode(c);
      
    memset(seq, 0, sizeof(seq));
    UTF8Encode(cp, seq);
    newStr->theString += seq;
    
    --pos;
  }

  delete[] trimCP;
  return newStr.release();
}
  

/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::trimR() const
{
  char seq = ' ';
  return trimR( &seq, 1 );
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::trim(const char* start, uint16_t len) const
{
  if(empty() || 0 == len)
    return new xqpStringStore(*this);

  xqpStringStore_t tmp = trimL(start, len);
  return tmp->trimR(start, len);
}


/*******************************************************************************
  Removes the leading and trailing whitespaces (#x20).
********************************************************************************/
xqpStringStore_t xqpStringStore::trim() const
{
  if(empty())
    return new xqpStringStore("");
  
  char seq = ' ';
  xqpStringStore_t tmp = trimL(&seq, 1);
  return tmp->trimR(&seq,1);
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::formatAsXML(const char* src) const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  uint32_t cp;
  char seq[5];

  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));

  for(i = 0; i < len; ++i)
  {
    cp = UTF8Decode(c);
    if(cp < 128)
    {
      memset(seq, 0, sizeof(seq));
      UTF8Encode(cp, seq);
      newStr->theString += seq;
    }
    else
    {
      newStr->theString += '&';
      newStr->theString += '#';
      newStr->theString +=  Integer::parseInt(cp).toString();
      newStr->theString += ';';
    }
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************
  Escape all characters except printable characters of the US-ASCII coded
  character set, specifically the octets ranging from 32 to 126 (decimal)
********************************************************************************/
xqpStringStore_t xqpStringStore::escapeHtmlUri() const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  unsigned int cp;
  char seq[5];
  const char* prev = c_str();
  distance_type length;

  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));
    
  for(i = 0; i < len; ++i)
  {
    prev = c;
    cp = UTF8Decode(c);
    memset(seq, 0, sizeof(seq));
    if(is_printableASCII(cp))
    {
      UTF8Encode(cp, seq);
      newStr->theString += seq;
    }
    else
    {
      //codepoint has to be escaped
      length = sequence_length(prev);
      if(length != 1)
      {
        for(int j = 0; j < length;++j)
        {
          cp = mask8(*prev);
          sprintf(seq, "%%%X", cp);
          newStr->theString += seq;
          prev++;
        }
      }
    }
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************
  Converts an xs:string containing an IRI into a URI according to the rules
  spelled out in Section 3.1 of http://www.ietf.org/rfc/rfc3987.txt
********************************************************************************/
xqpStringStore_t xqpStringStore::iriToUri() const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  unsigned int cp;
  char seq[5];
  const char* prev = c_str();
  distance_type length;

  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));
    
  for(i = 0; i < len; ++i)
  {
    prev = c;
    cp = UTF8Decode(c);
    memset(seq, 0, sizeof(seq));
    if(!is_ucscharCP(cp) && !is_iprivateCP(cp) && !is_Invalid_in_IRI(cp))
    {
      UTF8Encode(cp, seq);
      newStr->theString += seq;
    }
    else
    {
      //codepoint has to be escaped
      length = sequence_length(prev);
      for(int j = 0; j < length;++j)
      {
        cp = mask8(*prev);
        sprintf(seq, "%%%X", cp);
        newStr->theString += seq;
        prev++;
        }
    }
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************

********************************************************************************/
xqpStringStore_t xqpStringStore::encodeForUri() const
{
  uint32_t i;
  uint32_t len = numChars();
  const char* c = c_str();
  unsigned int cp;
  char seq[5];
  const char* prev = c_str();
  distance_type length;
  
  std::auto_ptr<xqpStringStore> newStr(new xqpStringStore(""));
    
  for(i = 0; i < len; ++i)
  {
    prev = c;
    cp = UTF8Decode(c);
    memset(seq, 0, sizeof(seq));
    if(is_unreservedCP(cp))
    {
      UTF8Encode(cp, seq);
      newStr->theString += seq;
    }
    else
    {
      //codepoint has to be escaped
      length = sequence_length(prev);
      for(int j = 0; j < length; ++j)
      {
        cp = mask8(*prev);
        sprintf(seq, "%%%X", cp);
        newStr->theString += seq;
        prev++;
      }
    }
  }
  newStr->theString += "\0";

  return newStr.release();
}


/*******************************************************************************
  Return an UnicodeString (UTF-16 encoded) version of the string.
********************************************************************************/
UnicodeString xqpStringStore::getUnicodeString() const
{
  UnicodeString ret;
  UErrorCode status = U_ZERO_ERROR;
  int32_t len = bytes();
  UChar* buffer = ret.getBuffer(len);

  u_strFromUTF8(buffer, ret.getCapacity(), &len, c_str(), len, &status);

  if(U_FAILURE(status))
  {
    assert(false);
  }

  ret.releaseBuffer(U_SUCCESS(status) ? len : 0);

  return ret;
}


/*******************************************************************************

********************************************************************************/
checked_vector<uint32_t> xqpStringStore::getCodepoints() const
{
  checked_vector<uint32_t> tt;
  uint16_t vLength;
  
  vLength = numChars() + 1;
  const char* c = c_str();
  while( --vLength > 0 )
  {
    tt.push_back(UTF8Decode(c));
  }
  return tt;
}


std::ostream& operator<<(std::ostream& os, const xqpStringStore& src)
{
  //TODO is there a need to perform charset conversion to/from the current locale ?!?!
  os << src.theString;
  return os;
}


  xqpString::xqpString()
  {
    theStrStore = new xqpStringStore("");
  }

  xqpString::xqpString(const std::string& src)
  {
    theStrStore = new xqpStringStore(src);
  }

  xqpString::xqpString(const char* src)
  {
    theStrStore = new xqpStringStore(src);
  }


  xqpString& xqpString::operator=(const std::string& src)
  {
    theStrStore = new xqpStringStore(src);
    return *this;
  }

  xqpString& xqpString::operator=(const char* src)
  {
    theStrStore = new xqpStringStore(src);
    return *this;
  }

  xqpString& xqpString::operator=(uint32_t cp)
  {
    theStrStore->theString.reserve(5);
    char seq[5] = {0,0,0,0,0};
    UTF8Encode(cp, seq);
    theStrStore = new xqpStringStore(seq);
    return *this;
  }

  xqpString& xqpString::operator=(char c)
  {
    theStrStore = new xqpStringStore(&c);
    return *this;
  }

  //xqpString::operator+=()
  xqpString& xqpString::operator+=(xqpString src)
  {
    xqpStringStore_t temp = new xqpStringStore(*theStrStore);
    temp->theString += src;
    theStrStore = temp;
    return *this;
  }

  xqpString& xqpString::operator+=(const char* src)
  {
    xqpStringStore_t temp = new xqpStringStore(*theStrStore);
    temp->theString += src;
    theStrStore = temp;
    return *this;
  }

  xqpString& xqpString::operator+=(uint32_t cp)
  {
    theStrStore->theString.reserve(5);
    char seq[5] = {0,0,0,0,0};
    UTF8Encode(cp, seq);
    theStrStore = new xqpStringStore(theStrStore->theString + seq);

    return *this;
  }

  xqpString& xqpString::operator+=(char c)
  {
    theStrStore = new xqpStringStore(theStrStore->theString + c);
    return *this;
  }

  //xqpString::stream I/O operators
  std::istream& operator>>(std::istream& is, xqpString& utf8_src)
  {
    std::string buffer;
    is >> buffer;
    //TODO is there a need to perform charset conversion to/from the current locale ?!?!
    utf8_src = buffer;
    return is;
  }

  std::ostream& operator<<(std::ostream& os, xqpStringStore_t str)
  {
    //TODO is there a need to perform charset conversion to/from the current locale ?!?!
    os << *str;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, xqpString utf8_src)
  {
    //TODO is there a need to perform charset conversion to/from the current locale ?!?!
    os << *utf8_src.theStrStore;
    return os;
  }

  int xqpString::compare(const char* src, XQPCollator* coll) const
  {
    //TODO optimize the code here
    xqpString tmp(src);
    return compare(tmp, coll);
  }

  uint32_t xqpString::hash(XQPCollator* coll) const
  {
    return theStrStore->hash(coll);
  }

  void xqpString::reserve(xqpString::size_type size)
  {
    theStrStore->theString.reserve(size);
  }

  //xqpString::Clear
  void xqpString::clear()
  {
    theStrStore->theString.erase();
  }


  xqpString xqpString::substr(xqpString::size_type index, xqpString::size_type length) const
  {
    char* target;
    int32_t size =  length*4 + 1;
    target = new char[size]; //will hold UTF-8 encoded characters
    UnicodeString str = theStrStore->getUnicodeString();

    int32_t targetsize = str.extract(index, length, target, size, "UTF-8");
    target[targetsize] = 0; /* NULL termination */

    xqpString ret(&target[0]);

    delete [] target;
    return ret;
  }


xqpString xqpString::substr(xqpStringStore::distance_type index) const
  {
    if(index >= (int32_t)length())
    {
      index = length();
    }
    else if(index < 0)
    {
      xqpString ret(theStrStore->theString);
      return ret;
    }

    const char * d = theStrStore->c_str();
    advance(d, index, d+length());

    xqpString ret(d);

    return ret;
  }

  const char* xqpString::c_str() const
  {
    return theStrStore->c_str();
  }
  
  //normalize
  xqpString xqpString::normalize(xqpString normMode)
  {
    UnicodeString result;
    UErrorCode status = U_ZERO_ERROR;

    if (normMode.empty ())
      return *this;
    else if(normMode == "NFC")
    {
      Normalizer::normalize(theStrStore->getUnicodeString(), UNORM_NFC , 0, result, status);
    }
    else if(normMode == "NFKC")
    {
      Normalizer::normalize(theStrStore->getUnicodeString(), UNORM_NFKC , 0, result, status);
    }
    else if(normMode == "NFD")
    {
      Normalizer::normalize(theStrStore->getUnicodeString(), UNORM_NFD , 0, result, status);
    }
    else if(normMode == "NFKD")
    {
      Normalizer::normalize(theStrStore->getUnicodeString(), UNORM_NFKD , 0, result, status);
    }

    if(U_FAILURE(status))
    {
      assert(false);
    }
    
    return getXqpString( result ); 
  }


  std::map<uint32_t,uint32_t> xqpString::createMapArray(xqpString mapString, xqpString transString) const
  {
    uint16_t      mapLen    = mapString.length();
    uint16_t      transLen  = transString.length();
    const char*   mapPtr    = mapString.theStrStore->c_str();
    const char*   transPtr  = transString.theStrStore->c_str();
    uint32_t      tmp0, tmp1;
    
    std::map<uint32_t,uint32_t> mapArray;
    std::map<uint32_t,uint32_t>::iterator it;
    
    if(mapLen >0)
    {
      while((mapLen > 0) && (transLen > 0))
      {
        tmp0 = UTF8Decode(mapPtr);
        tmp1 = UTF8Decode(transPtr);
        mapArray.insert(std::pair<uint32_t,uint32_t>(tmp0, tmp1));
        --mapLen;
        --transLen;
      }
      
      while(mapLen > 0)
      {
        tmp0 = UTF8Decode(mapPtr);
        mapArray.insert(std::pair<uint32_t,uint32_t>(tmp0, 0xFFFFFFFF)); //0xFFFFFFFF = unsigned long_MAX
        --mapLen;
      }
    }
    return mapArray;
  }
  
  xqpString xqpString::translate(xqpString mapString, xqpString transString) const
  {
    std::map<uint32_t,uint32_t>           myMap;
    std::map<uint32_t,uint32_t>::iterator it;

    //create the map
    myMap = createMapArray(mapString, transString);

    //create the new xqpStringStore
    std::string tmp = "";
    uint32_t len = length();
    const char* c = c_str();
    uint32_t cp, i;
    char seq[5];

    for(i=0; i<len; ++i)
    {
      cp = UTF8Decode(c);

      it= myMap.find(cp);
      if( it != myMap.end() )
      {
        cp = (*it).second;
      }

      if(cp != 0xFFFFFFFF) //0xFFFFFFFF = unsigned long_MAX
      {
        memset(seq, 0, sizeof(seq));
        UTF8Encode(cp, seq);
        tmp += seq;
      }
    }
    tmp += "\0";

    xqpString res(tmp.c_str());
    return res;
  }


  uint32_t parse_regex_flags (const char *flag_cstr) {
    uint32_t flags = 0;
    for (const char *p = flag_cstr; *p != '\0'; p++) {
      switch (*p) {
      case 'i': flags |= UREGEX_CASE_INSENSITIVE; break;
      case 's': flags |= UREGEX_DOTALL; break;
      case 'm': flags |= UREGEX_MULTILINE; break;
      case 'x': flags |= UREGEX_COMMENTS; break;
      default:
        throw zorbatypesException("", ZorbatypesError::FORX0001);
        break;
      }
    }
    return flags;
  }

  bool
    xqpString::matches(xqpString pattern, xqpString flags)
  {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString uspattern = pattern.getUnicodeString (),
      us = getUnicodeString ();

    RegexMatcher matcher (uspattern, parse_regex_flags (flags.c_str ()), status);
    if (U_FAILURE(status)) {
      throw zorbatypesException("", ZorbatypesError::FORX0002);
      return false;
    }

    matcher.reset (us);
    return matcher.find ();
  }


  xqpString
  xqpString::replace(xqpString pattern, xqpString replacement, xqpString flags) {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString uspattern = pattern.getUnicodeString (),
      us = getUnicodeString ();

    RegexMatcher matcher (uspattern, us, parse_regex_flags (flags.c_str ()), status);
    if (U_FAILURE(status)) {
      throw zorbatypesException("", ZorbatypesError::FORX0002);
      return "";
    }

    if((replacement.indexOf("$") != -1 && !replacement.matches("\\$[0-9]",""))||
        (replacement.indexOf("\\") != -1 && !replacement.matches("\\$[0-9]","")))
    {
      throw zorbatypesException("", ZorbatypesError::FORX0004);
      return "";
    }
    
    UnicodeString result = matcher.replaceAll (replacement.getUnicodeString(), status);
    if (U_FAILURE(status)) {
      return "";
      // TODO: error
    }
    return getXqpString (result);
  }

  xqpString
  xqpString::tokenize(xqpString pattern, xqpString flags, xqpString *remaining)
  {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString uspattern = pattern.getUnicodeString (),
      us = getUnicodeString ();
    RegexMatcher m (uspattern, us, parse_regex_flags (flags.c_str ()), status);
    if (U_FAILURE(status)) {
      throw zorbatypesException("", ZorbatypesError::FORX0002);
      return "";
    }
    if (m.find ()) {
      int32_t start = m.start (status), end = m.end (status);
      *remaining = substr (end, length () - end);
      return substr (0, start);
    } else {
      *remaining = xqpString ();
      return substr (0, length ());
    }
  }


  xqpString xqpString::fromUTF16(const UChar* src, int32_t len)
  {
    char* target;
    int32_t targetLen = len*4 + 1;
    target = new char[targetLen];
    UErrorCode status = U_ZERO_ERROR;

    //open a convertor to UTF-8
    UConverter *conv = ucnv_open("utf-8", &status);

    if(U_FAILURE(status))
    {
      assert(false);

      delete[] target;
      return "";
    }

    //Convert from UTF-16 to UTF-8
    ucnv_fromUChars (conv, target, targetLen, (const UChar*)src, len, &status);
    //close the converter
    ucnv_close(conv);

    if(U_FAILURE(status))
    {
      assert(false);

      delete[] target;
      return "";
    }

    xqpString ret(&target[0]);
    delete[] target;
    return ret;
  }

  // Private methods
  xqpString xqpString::getXqpString(UnicodeString source)
  {
    char* target;
    int32_t targetLen = source.getCapacity()*4 + 1;
    target = new char[targetLen];
    UErrorCode status = U_ZERO_ERROR;

    //open a convertor to UTF-8
    UConverter *conv = ucnv_open("utf-8", &status);

    if(U_FAILURE(status))
    {
      assert(false);

      delete[] target;
      return "";
    }

    //Convert from UTF-16 to UTF-8
    ucnv_fromUChars (conv, target, targetLen, source.getBuffer( source.length() ), source.length(), &status);
    //close the converter
    ucnv_close(conv);

    if(U_FAILURE(status))
    {
      assert(false);

      delete[] target;
      return "";
    }

    xqpString ret(&target[0]);
    delete[] target;
    return ret;
}

  wchar_t * xqpString::getWCS(xqpString source) const
  {
    int32_t destCapacity =  source.length()*2 + 1;
    wchar_t* destWCS;
    destWCS = new wchar_t[destCapacity];
    int32_t destLen;

    UnicodeString unicodeStr = source.theStrStore->getUnicodeString();
    int32_t srcLen = unicodeStr.length();
    UChar* srcBuf = unicodeStr.getBuffer(srcLen);
    UErrorCode status = U_ZERO_ERROR;

    wchar_t* ret =  u_strToWCS(destWCS, destCapacity, &destLen, srcBuf, srcLen, &status);

    if(U_FAILURE(status))
    {
      assert(false);
    }

    return ret;
  }
}/* namespace zorba */

#endif//#ifndef ZORBA_NO_UNICODE
