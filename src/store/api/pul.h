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
#ifndef ZORBA_STORE_PUL
#define ZORBA_STORE_PUL

#include <set>

#include <zorba/config.h>

#include "store/api/shared_types.h"
#include "store/api/update_consts.h"
#include "store/api/item.h"


namespace zorba 
{ 

class static_context;

typedef std::pair<const store::Item*, store::Index_t> IndexBinding;


namespace store 
{

class XmlNode;
class Iterator;
class IndexSpecification;


/*******************************************************************************

************************************** ******************************************/
class PUL : public Item
{
public:
  virtual ~PUL() { }

  bool isNode() const   { return false; }
  bool isAtomic() const { return false; }
  bool isPul() const    { return true; }
  bool isTuple() const  { return false; }
  bool isError() const  { return false; }

  virtual void addDelete(Item_t& node) = 0;

  virtual void addInsertInto(
        Item_t&              target,
        std::vector<Item_t>& children,
        const CopyMode&      copymode) = 0;

  virtual void addInsertFirst(
        Item_t&              target,
        std::vector<Item_t>& children,
        const CopyMode&      copymode) = 0;

  virtual void addInsertLast(
        Item_t&              target,
        std::vector<Item_t>& children,
        const CopyMode&      copymode) = 0;

  virtual void addInsertBefore(
        Item_t&              target,
        std::vector<Item_t>& siblings,
        const CopyMode&      copymode) = 0;

  virtual void addInsertAfter(
        Item_t&              target,
        std::vector<Item_t>& siblings,
        const CopyMode&      copymode) = 0;
  
  virtual void addInsertAttributes(
        Item_t&              target,
        std::vector<Item_t>& attrs,
        const CopyMode&      copymode) = 0;

  virtual void addReplaceNode(
        Item_t&              target,
        std::vector<Item_t>& replacementNodes,
        const CopyMode&      copymode) = 0;

  virtual void addReplaceContent(
        Item_t&              target,
        Item_t&              newTextChild,
        const CopyMode&      copymode) = 0;

  virtual void addReplaceValue(
        Item_t&              target,
        xqpStringStore_t&    newValue) = 0;

  virtual void addRename(
        Item_t&              target,
        Item_t&              newName) = 0;

  virtual void addSetElementType(
        Item_t&              target,
        Item_t&              typeName,
        Item_t&              typedValue,
        bool                 haveValue,
        bool                 haveEmptyValue,
        bool                 haveTypedValue,
        bool                 isInSubstitutionGroup) = 0;

  virtual void addSetElementType(
        Item_t&              target,
        Item_t&              typeName,
        std::vector<Item_t>& typedValue,
        bool                 haveValue,
        bool                 haveEmptyValue,
        bool                 haveTypedValue,
        bool                 isInSubstitutionGroup) = 0;

  virtual void addSetAttributeType(
        Item_t&              target,
        Item_t&              typeName,
        Item_t&              typedValue) = 0;

  virtual void addSetAttributeType(
        Item_t&              target,
        Item_t&              typeName,
        std::vector<Item_t>& typedValue) = 0;


  virtual void addPut(
        Item_t& target,
        Item_t& uri) = 0;

  // functions to add primitives for updating collection functions (e.g. create-collection)

  virtual void addCreateCollection(
        Item_t&                     name) = 0;

  virtual void addDropCollection(
        Item_t&                     name) = 0;

  virtual void addInsertIntoCollection(
        Item_t&                     name,
        Item_t&                     nodes) = 0;            

  virtual void addInsertFirstIntoCollection(
        Item_t&                     name,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addInsertLastIntoCollection(
        Item_t&                     name,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addInsertBeforeIntoCollection(
        Item_t&                     name,
        Item_t&                     target,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addInsertAfterIntoCollection(
        Item_t&                     name,
        Item_t&                     target,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addInsertAtIntoCollection(
        Item_t&                     name,
        ulong                       pos,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addRemoveFromCollection(
        Item_t&                     name,
        std::vector<store::Item_t>& nodes) = 0;

  virtual void addRemoveAtFromCollection(
        Item_t&                     name,
        ulong                       pos) = 0;

  virtual void addCreateIndex(
        const Item_t& qname,
        const IndexSpecification& spec,
        Iterator* sourceIter) = 0;

  virtual void addDropIndex(
        const Item_t& qname) = 0;

  virtual void addRefreshIndex(
        const Item_t& qname,
        Iterator* sourceIter) = 0;

  virtual void mergeUpdates(Item* other) = 0;

  virtual void applyUpdates(std::set<Item*>& validationNodes) = 0;

  virtual void checkTransformUpdates(const std::vector<Item*>& rootNodes) const = 0;

  virtual void getCreatedIndices(std::vector<IndexBinding>& indices) const = 0;

  virtual void getDropedIndices(std::vector<const store::Item*>& indices) const = 0;
};


}
}

#endif
