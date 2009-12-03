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
#ifndef ZORBA_COMPILER_ANNOTATIONS_H
#define ZORBA_COMPILER_ANNOTATIONS_H

#include "zorbatypes/rchandle.h"

namespace zorba {

class AnnotationValue;

typedef rchandle<AnnotationValue> AnnotationValue_t;


class AnnotationValue : public SimpleRCObject 
{
public:
  virtual ~AnnotationValue() {}

  virtual bool equals(const AnnotationValue& other) { return this == &other; }
};


class IntAnnotationValue : public AnnotationValue 
{
public:
  int theValue;
  
  IntAnnotationValue(int n) : theValue(n) {}
};


inline bool operator==(const AnnotationValue_t& v1, const AnnotationValue_t& v2) 
{
  if (v2 == NULL || v1 == NULL)
    return v2 == NULL && v1 == NULL;

  return v2->equals(*v1.getp());
}

}

#endif /* ZORBA_ANNOTATIONS_H */
/* vim:set ts=2 sw=2: */
/*
 * Local variables:
 * mode: c++
 * End:
 */
