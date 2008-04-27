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
#include "functions/Misc.h"

#include "system/globalenv.h"
#include "errors/error_manager.h"
#include "runtime/debug/debug_iterators.h"
#include "runtime/misc/MiscImpl.h"

using namespace std;

namespace zorba {

fn_trace_func::fn_trace_func(const signature& sig)
  : function(sig) { }

PlanIter_t fn_trace_func::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  return new FnTraceIterator ( loc, argv );
}



/*******************************************************************************
  3 The Error Function
********************************************************************************/

fn_error::fn_error (const signature &sig) : function (sig) {}


PlanIter_t fn_error::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  return new FnErrorIterator(loc, argv);
}


/*******************************************************************************
  8.1 fn:resolve-uri
********************************************************************************/

fn_resolve_uri::fn_resolve_uri(const signature& sig)
  : function(sig) { }

PlanIter_t fn_resolve_uri::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  return new FnResolveUriIterator ( loc, argv );
}



}
/* vim:set ts=2 sw=2: */
