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

#include <zorba/exception.h>

#include "zorbaerrors/error_manager.h"

#include "store/naive/store_defs.h"
#include "store/naive/simple_store.h"
#include "store/naive/simple_pul.h"
#include "store/naive/node_items.h"
#include "store/naive/atomic_items.h"
#include "store/naive/simple_collection.h"

#include "store/api/collection.h"
#include "store/api/iterator.h"
#include "store/api/item_factory.h"


#include "context/internal_uri_resolvers.h"
#include "system/globalenv.h"


namespace zorba { namespace simplestore {


/*******************************************************************************

********************************************************************************/
NodeToUpdatesMap::~NodeToUpdatesMap()
{
  NodeToUpdatesMap::iterator ite = theMap.begin();
  NodeToUpdatesMap::iterator end = theMap.end();

  for (; ite != end; ++ite)
  {
    delete (*ite).second;
  }
}


/*******************************************************************************

********************************************************************************/
inline void cleanList(std::vector<UpdatePrimitive*> aVector)
{
  for ( std::vector<UpdatePrimitive*>::iterator lIter = aVector.begin();
        lIter != aVector.end();
        ++lIter ) 
  {
    delete (*lIter);
  }
}


/*******************************************************************************

********************************************************************************/
PULImpl::~PULImpl()
{
  cleanList(theDoFirstList);
  cleanList(theInsertList);
  cleanList(theReplaceNodeList);
  cleanList(theReplaceContentList);
  cleanList(theDeleteList);
  cleanList(thePutList);
  cleanList(theValidationList);
  cleanList(theCreateCollectionList);
  cleanList(theInsertIntoCollectionList);
  cleanList(theDeleteFromCollectionList);
  cleanList(theDropCollectionList);
}


/*******************************************************************************
  Create a delete primitive in "this" pul for the given node, if another delete
  for the same node does not exist already.
********************************************************************************/
void PULImpl::addDelete(store::Item_t& target)
{
  XmlNode* n = BASE_NODE(target);

  NodeUpdates* updates = NULL;
  bool found = theNodeToUpdatesMap.get(n, updates);

  if (!found)
  {
    UpdDelete* upd = new UpdDelete(this, target);
    theDeleteList.push_back(upd);

    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    ulong numUpdates = updates->size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      if ((*updates)[i]->getKind() == store::UpdateConsts::UP_DELETE)
        return;
    }

    UpdDelete* upd = new UpdDelete(this, target);
    theDeleteList.push_back(upd);
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addInsertInto(
    store::Item_t&              target,
    std::vector<store::Item_t>& children,
    const store::CopyMode&      copymode)
{
  store::Item_t sibling;

  addInsertChildren(store::UpdateConsts::UP_INSERT_INTO,
                    target, sibling, children, copymode);
}


void PULImpl::addInsertFirst(
    store::Item_t&              target,
    std::vector<store::Item_t>& children,
    const store::CopyMode&      copymode)
{
  store::Item_t sibling;

  addInsertChildren(store::UpdateConsts::UP_INSERT_INTO_FIRST,
                    target, sibling, children, copymode);
}


void PULImpl::addInsertLast(
    store::Item_t&              target,
    std::vector<store::Item_t>& children,
    const store::CopyMode&      copymode)
{
  store::Item_t sibling;

  addInsertChildren(store::UpdateConsts::UP_INSERT_INTO_LAST,
                    target, sibling, children, copymode);
}


void PULImpl::addInsertBefore(
    store::Item_t&                   target,
    std::vector<store::Item_t>&      siblings,
    const store::CopyMode&           copymode)
{
  store::Item_t parent = target->getParent();

  addInsertChildren(store::UpdateConsts::UP_INSERT_BEFORE,
                    parent, target, siblings, copymode);
}


void PULImpl::addInsertAfter(
    store::Item_t&                   target,
    std::vector<store::Item_t>&      siblings,
    const store::CopyMode&           copymode)
{
  store::Item_t parent = target->getParent();

  addInsertChildren(store::UpdateConsts::UP_INSERT_AFTER,
                    parent, target, siblings, copymode);
}


void PULImpl::addInsertChildren(
    store::UpdateConsts::UpdPrimKind kind,
    store::Item_t&                   target,
    store::Item_t&                   sibling,
    std::vector<store::Item_t>&      children,
    const store::CopyMode&           copymode)
{
  XmlNode* n = BASE_NODE(target);

  NodeUpdates* updates = 0;
  bool found = theNodeToUpdatesMap.get(n, updates);

  UpdInsertChildren* upd = new UpdInsertChildren(this, kind,
                                                 target, sibling, children,
                                                 copymode);

  if (kind == store::UpdateConsts::UP_INSERT_INTO)
    theDoFirstList.push_back(upd);
  else
    theInsertList.push_back(upd);

  if (!found)
  {
    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addInsertAttributes(
    store::Item_t&              target,
    std::vector<store::Item_t>& attrs,
    const store::CopyMode&      copymode)
{
  ElementNode* n = ELEM_NODE(target);

  ulong numAttrs = attrs.size();
  for (ulong i = 0; i < numAttrs; i++)
  {
    n->checkNamespaceConflict(attrs[i]->getNodeName(), XUDY0023);
  }

  NodeUpdates* updates = 0;
  bool found = theNodeToUpdatesMap.get(n, updates);

  UpdInsertAttributes* upd = new UpdInsertAttributes(this, target, attrs, copymode);
  theDoFirstList.push_back(upd);

  if (!found)
  {
    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    XmlNode* tmp = reinterpret_cast<XmlNode*>(n);
    theNodeToUpdatesMap.insert(tmp, updates);
  }
  else
  {
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addReplaceNode(
    store::Item_t&              target,
    std::vector<store::Item_t>& newNodes,
    const store::CopyMode&      copymode)
{
  XmlNode* n = BASE_NODE(target);

  store::Item_t parent = target->getParent();

  NodeUpdates* updates = 0;
  bool found = theNodeToUpdatesMap.get(n, updates);

  UpdatePrimitive* upd;
  store::UpdateConsts::UpdPrimKind kind;

  if (target->getNodeKind() == store::StoreConsts::attributeNode)
  {
    ElementNode* elemParent = reinterpret_cast<ElementNode*>(n->theParent);

    if (elemParent != NULL)
    {
      ulong numNewAttrs = newNodes.size();
      for (ulong i = 0; i < numNewAttrs; ++i)
      {
        elemParent->checkNamespaceConflict(newNodes[i]->getNodeName(), XUDY0023); 
      }
    }

    upd = new UpdReplaceAttribute(this, parent, target, newNodes, copymode);
    kind = store::UpdateConsts::UP_REPLACE_ATTRIBUTE;
  }
  else
  {
    upd = new UpdReplaceChild(this, parent, target, newNodes, copymode);
    kind = store::UpdateConsts::UP_REPLACE_CHILD;
  }

  if (!found)
  {
    theReplaceNodeList.push_back(upd);

    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    ulong numUpdates = updates->size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      if ((*updates)[i]->getKind() == kind)
      {
        delete upd;
        ZORBA_ERROR(XUDY0016);
      }
    }

    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addReplaceContent(
    store::Item_t&         target,
    store::Item_t&         newChild,
    const store::CopyMode& copymode)
{
  XmlNode* n = BASE_NODE(target);

  NodeUpdates* updates = NULL;
  bool found = theNodeToUpdatesMap.get(n, updates);

  if (!found)
  {
    UpdatePrimitive* upd = new UpdReplaceElemContent(this, target, newChild, copymode);
    theReplaceContentList.push_back(upd);

    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    ulong numUpdates = updates->size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      if ((*updates)[i]->getKind() == store::UpdateConsts::UP_REPLACE_CONTENT)
        ZORBA_ERROR(XUDY0017);
    }

    UpdatePrimitive* upd = new UpdReplaceElemContent(this, target, newChild, copymode);
    theReplaceContentList.push_back(upd);
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addReplaceValue(store::Item_t& target, xqpStringStore_t& newValue)
{
  XmlNode* n = BASE_NODE(target);
  store::StoreConsts::NodeKind targetKind = n->getNodeKind();

  NodeUpdates* updates = NULL;
  bool found = theNodeToUpdatesMap.get(n, updates);

  UpdatePrimitive* upd;
  switch (targetKind)
  {
  case store::StoreConsts::attributeNode:
    upd = new UpdReplaceAttrValue(this, target, newValue);
    break;
  case store::StoreConsts::textNode:
    upd = new UpdReplaceTextValue(this, target, newValue);
    break;
  case store::StoreConsts::piNode:
    upd = new UpdReplacePiValue(this, target, newValue);
    break;
  case store::StoreConsts::commentNode:
    upd = new UpdReplaceCommentValue(this, target, newValue);
    break;
  default:
    ZORBA_FATAL(0, "");
  }

  if (!found)
  {
    theDoFirstList.push_back(upd);

    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    ulong numUpdates = updates->size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      if (store::UpdateConsts::isReplaceValue((*updates)[i]->getKind()))
      {
        delete upd;
        ZORBA_ERROR(XUDY0017);
      }
    }

    theDoFirstList.push_back(upd);
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addRename(store::Item_t& target, store::Item_t& newName)
{
  XmlNode* n = BASE_NODE(target);
  store::StoreConsts::NodeKind targetKind = n->getNodeKind();

  NodeUpdates* updates = NULL;
  bool found = theNodeToUpdatesMap.get(n, updates);

  UpdatePrimitive* upd;
  switch (targetKind)
  {
  case store::StoreConsts::elementNode:
  {
    ElementNode* elemTarget = ELEM_NODE(target);
    elemTarget->checkNamespaceConflict(newName.getp(), XUDY0023);

    upd = new UpdRenameElem(this, target, newName);
    break;
  }
  case store::StoreConsts::attributeNode:
  {
    ElementNode* elemParent = reinterpret_cast<ElementNode*>(n->theParent);

    if (elemParent != NULL)
      elemParent->checkNamespaceConflict(newName.getp(), XUDY0023);

    upd = new UpdRenameAttr(this, target, newName);
    break;
  }
  case store::StoreConsts::piNode:
  {
    xqpStringStore_t tmp = newName->getStringValue();
    upd = new UpdRenamePi(this, target, tmp);
    break;
  }
  default:
    ZORBA_FATAL(0, "");
  }

  if (!found)
  {
    theDoFirstList.push_back(upd);

    updates = new NodeUpdates(1);
    (*updates)[0] = upd;
    theNodeToUpdatesMap.insert(n, updates);
  }
  else
  {
    ulong numUpdates = updates->size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      if (store::UpdateConsts::isRename((*updates)[i]->getKind()))
      {
        delete upd;
        ZORBA_ERROR(XUDY0015);
      }
    }

    theDoFirstList.push_back(upd);
    updates->push_back(upd);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addSetElementType(
    store::Item_t&              target,
    store::Item_t&              typeName,
    store::Item_t&              value,
    bool                        haveValue,
    bool                        haveEmptyValue,
    bool                        haveTypedValue,
    bool                        isInSubstitutionGroup)
{
  UpdatePrimitive* upd = new UpdSetElementType(this, target,
                                               typeName, value,
                                               haveValue, haveEmptyValue,
                                               haveTypedValue,
                                               false,
                                               isInSubstitutionGroup);
  theValidationList.push_back(upd);
}


void PULImpl::addSetElementType(
    store::Item_t&              target,
    store::Item_t&              typeName,
    std::vector<store::Item_t>& valueV,
    bool                        haveValue,
    bool                        haveEmptyValue,
    bool                        haveTypedValue,
    bool                        isInSubstitutionGroup)
{
  store::Item_t typedValue = new ItemVector(valueV);

  UpdatePrimitive* upd = new UpdSetElementType(this, target,
                                               typeName, typedValue,
                                               haveValue, haveEmptyValue,
                                               haveTypedValue,
                                               true,
                                               isInSubstitutionGroup);
  theValidationList.push_back(upd);
}


void PULImpl::addSetAttributeType(
    store::Item_t&              target,
    store::Item_t&              typeName,
    store::Item_t&              typedValue)
{
  UpdatePrimitive* upd = new UpdSetAttributeType(this, target,
                                                 typeName, typedValue, false);
  theValidationList.push_back(upd);
}


void PULImpl::addSetAttributeType(
    store::Item_t&              target,
    store::Item_t&              typeName,
    std::vector<store::Item_t>& typedValueV)
{
  store::Item_t typedValue = new ItemVector(typedValueV);

  UpdatePrimitive* upd = new UpdSetAttributeType(this, target,
                                                 typeName, typedValue, true);
  theValidationList.push_back(upd);
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addPut(store::Item_t& target, store::Item_t& uri)
{
  ulong numPuts = thePutList.size();

  for (ulong i = 0; i < numPuts; ++i)
  {
    UpdPut* upd = static_cast<UpdPut*>(thePutList[i]);

    if (upd->theTargetUri == uri)
    {
      ZORBA_ERROR(XUDY0031);
    }
  }

  if (target->getNodeKind() != store::StoreConsts::documentNode)
  {
    assert(target->getNodeKind() == store::StoreConsts::elementNode);

    ElementNode* elem =  static_cast<ElementNode*>(target.getp());

    DocumentNode* doc = new DocumentNode();
    doc->setTree(elem->getTree());
    doc->setOrdPath(NULL, 1, store::StoreConsts::documentNode);

    doc->insertChild(elem, 0);

    store::Item_t docItem(doc);

    target = docItem;
  }

  UpdatePrimitive* upd = new UpdPut(this, target, uri);

  thePutList.push_back(upd);
}


/*******************************************************************************
 collection functions
********************************************************************************/
void PULImpl::addCreateCollection(
    store::Item_t&    name)
{
  theCreateCollectionList.push_back(
    new UpdCreateCollection(this, name)
  );
}


void PULImpl::addDropCollection(
    store::Item_t&              name)
{
  theDropCollectionList.push_back(
    new UpdDropCollection(this, name)
  );
}


void PULImpl::addInsertIntoCollection(
    store::Item_t&         name,
    store::Item_t&         node)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertIntoCollection(this, name, node)
  );
} 


void PULImpl::addInsertFirstIntoCollection(
    store::Item_t&              name,
    std::vector<store::Item_t>& nodes)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertFirstIntoCollection(this, name, nodes)
  );
}


void PULImpl::addInsertLastIntoCollection(
    store::Item_t&              name,
    std::vector<store::Item_t>& nodes)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertLastIntoCollection(this, name, nodes)
  );
}


void PULImpl::addInsertBeforeIntoCollection(
    store::Item_t&              name,
    store::Item_t&              target,
    std::vector<store::Item_t>& nodes)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertBeforeIntoCollection(this, name, target, nodes)
  );
}


void PULImpl::addInsertAfterIntoCollection(
    store::Item_t&              name,
    store::Item_t&              target,
    std::vector<store::Item_t>& nodes)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertAfterIntoCollection(this, name, target, nodes)
  );
}


void PULImpl::addInsertAtIntoCollection(
    store::Item_t&              name,
    ulong                       pos,
    std::vector<store::Item_t>& nodes)
{
  theInsertIntoCollectionList.push_back(
    new UpdInsertAtIntoCollection(this, name, pos, nodes)
  );
}


void PULImpl::addRemoveFromCollection(
    store::Item_t&            name,
    std::vector<store::Item_t>& nodes)
{
  theDeleteFromCollectionList.push_back(
    new UpdRemoveNodesFromCollection(this, name, nodes)
  );
}


void PULImpl::addRemoveAtFromCollection(
    store::Item_t&            name,
    ulong              pos)
{
  theDeleteFromCollectionList.push_back(
    new UpdRemoveNodeAtFromCollection(this, name, pos)
  );
}


/*******************************************************************************

********************************************************************************/
void PULImpl::addCreateIndex(
    const store::Item_t& qname,
    const store::IndexSpecification& spec,
    store::Iterator* sourceIter)
{
  UpdatePrimitive* upd = new UpdCreateIndex(this, qname, spec, sourceIter);
  theCreateIndexList.push_back(upd);
}


void PULImpl::addDropIndex(const store::Item_t& qname)
{
  UpdatePrimitive* upd = new UpdDropIndex(this, qname);
  theDropIndexList.push_back(upd);
}


void PULImpl::addRefreshIndex(
    const store::Item_t& qname,
    store::Iterator* sourceIter)
{
  UpdatePrimitive* upd = new UpdRefreshIndex(this, qname, sourceIter);
  theRefreshIndexList.push_back(upd);
}


/*******************************************************************************

********************************************************************************/
void PULImpl::mergeUpdates(store::Item* other)
{
  PULImpl* otherp = reinterpret_cast<PULImpl*>(other);

  mergeUpdateList(theDoFirstList, otherp->theDoFirstList, UP_LIST_DO_FIRST);

  mergeUpdateList(theInsertList, otherp->theInsertList, UP_LIST_NONE);

  mergeUpdateList(theReplaceNodeList,
                  otherp->theReplaceNodeList,
                  UP_LIST_REPLACE_NODE);

  mergeUpdateList(theReplaceContentList,
                  otherp->theReplaceContentList,
                  UP_LIST_REPLACE_CONTENT);

  mergeUpdateList(theDeleteList, otherp->theDeleteList, UP_LIST_DELETE);

  mergeUpdateList(thePutList, otherp->thePutList, UP_LIST_PUT);

  // merge collection functions
  mergeUpdateList(theCreateCollectionList,
                  otherp->theCreateCollectionList,
                  UP_LIST_CREATE_COLLECTION);

  mergeUpdateList(theInsertIntoCollectionList,
                  otherp->theInsertIntoCollectionList,
                  UP_LIST_NONE);

  mergeUpdateList(theDeleteFromCollectionList,
                  otherp->theDeleteFromCollectionList,
                  UP_LIST_NONE);

  mergeUpdateList(theDropCollectionList,
                  otherp->theDropCollectionList,
                  UP_LIST_NONE);

  mergeUpdateList(theCreateIndexList, otherp->theCreateIndexList, UP_LIST_NONE);

  mergeUpdateList(theDropIndexList, otherp->theDropIndexList, UP_LIST_NONE);

  mergeUpdateList(theRefreshIndexList, otherp->theRefreshIndexList, UP_LIST_NONE);
}


void PULImpl::mergeUpdateList(
    std::vector<UpdatePrimitive*>&  myList,
    std::vector<UpdatePrimitive*>&  otherList,
    UpdListKind                     listKind)
{
  ulong numUpdates;
  ulong numOtherUpdates;

  numUpdates = myList.size();
  numOtherUpdates = otherList.size();

  for (ulong i = 0; i < numOtherUpdates; i++)
  {
    if (listKind == UP_LIST_PUT)
    {
      UpdPut* otherUpd = static_cast<UpdPut*>(otherList[i]);

      ulong numPuts = thePutList.size();

      for (ulong j = 0; j < numPuts; ++j)
      {
        UpdPut* upd = static_cast<UpdPut*>(thePutList[j]);

        if (upd->theTargetUri->equals(otherUpd->theTargetUri))
        {
          ZORBA_ERROR(XUDY0031);
        }
      }

      thePutList.push_back(otherUpd);
      otherList[i] = NULL;

      continue;
    }

    if (listKind == UP_LIST_CREATE_COLLECTION) {
      UpdCreateCollection* otherUpd = static_cast<UpdCreateCollection*>(otherList[i]);
      for (size_t j = 0; j < theCreateCollectionList.size(); ++j) {
        if (myList[j]->getKind() == store::UpdateConsts::UP_CREATE_COLLECTION) {
          UpdCreateCollection* upd = static_cast<UpdCreateCollection*>(theCreateCollectionList[j]);
          if (upd->getCollectionName()->equals(otherUpd->getCollectionName())) {
            ZORBA_ERROR(XDXX0001);
          }
        }
      }
    }

    UpdatePrimitive* otherUpd = otherList[i];
    otherUpd->thePul = this;

    store::UpdateConsts::UpdPrimKind updKind = otherUpd->getKind();
    XmlNode* target;

    if (updKind == store::UpdateConsts::UP_REPLACE_CHILD)
      target = BASE_NODE(reinterpret_cast<UpdReplaceChild*>(otherUpd)->theChild);
    else if (updKind == store::UpdateConsts::UP_REPLACE_ATTRIBUTE)
      target = BASE_NODE(reinterpret_cast<UpdReplaceAttribute*>(otherUpd)->theAttr);
    else
      target = BASE_NODE(otherUpd->theTarget);

    NodeUpdates* targetUpdates = NULL;
    bool found = (target == NULL ?
                  false : 
                  theNodeToUpdatesMap.get(target, targetUpdates));

    if (!found)
    {
      myList.push_back(otherUpd);
      otherList[i] = NULL;

      if (target)
      {
        targetUpdates = new NodeUpdates(1);
        (*targetUpdates)[0] = otherUpd;
        theNodeToUpdatesMap.insert(target, targetUpdates);
      }
    }
    else
    {
      switch (listKind)
      {
      case UP_LIST_DO_FIRST:
      {
        if (store::UpdateConsts::isRename(updKind))
        {
          ulong numTargetUpdates = targetUpdates->size();
          for (ulong j = 0; j < numTargetUpdates; j++)
          {
            if (store::UpdateConsts::isRename((*targetUpdates)[j]->getKind()))
              ZORBA_ERROR(XUDY0015);
          }
        }
        else if (store::UpdateConsts::isReplaceValue(updKind))
        {
          ulong numTargetUpdates = targetUpdates->size();
          for (ulong j = 0; j < numTargetUpdates; j++)
          {
            if (store::UpdateConsts::isReplaceValue((*targetUpdates)[j]->getKind()))
              ZORBA_ERROR(XUDY0017);
          }
        }
        break;
      }
      case UP_LIST_REPLACE_NODE:
      {
        if (store::UpdateConsts::isReplaceNode(updKind))
        {
          ulong numTargetUpdates = targetUpdates->size();
          for (ulong j = 0; j < numTargetUpdates; ++j)
          {
            if (store::UpdateConsts::isReplaceNode((*targetUpdates)[j]->getKind()))
              ZORBA_ERROR(XUDY0016);
          }
        }
        break;
      }
      case UP_LIST_REPLACE_CONTENT:
      {
        if (updKind == store::UpdateConsts::UP_REPLACE_CONTENT)
        {
          ulong numTargetUpdates = targetUpdates->size();
          for (ulong j = 0; j < numTargetUpdates; ++j)
          {
            if ((*targetUpdates)[j]->getKind() == store::UpdateConsts::UP_REPLACE_CONTENT)
              ZORBA_ERROR(XUDY0017);
          }
        }
        break;
      }
      case UP_LIST_DELETE:
      {
        if (updKind == store::UpdateConsts::UP_DELETE)
        {
          ulong numTargetUpdates = targetUpdates->size();
          ulong j;
          for (j = 0; j < numTargetUpdates; ++j)
          {
            if ((*targetUpdates)[j]->getKind() == store::UpdateConsts::UP_DELETE)
              break;
          }

          if (j < numTargetUpdates)
            continue;
        }
        break;
      }
      default:
        break;
      }

      myList.push_back(otherUpd);
      otherList[i] = NULL;
      targetUpdates->push_back(otherUpd);
    }
  }

  otherList.clear();
}


/*******************************************************************************
  Check that each target node of this pul is inside one of the trees rooted at
  the given root nodes (the root nodes are the copies of the nodes produced by
  the source expr of a transform expr).
********************************************************************************/
void PULImpl::checkTransformUpdates(const std::vector<store::Item*>& rootNodes) const
{
  ulong numRoots = rootNodes.size();

  NodeToUpdatesMap::iterator it = theNodeToUpdatesMap.begin();
  NodeToUpdatesMap::iterator end = theNodeToUpdatesMap.end();

  for (; it != end; ++it)
  {
    const XmlNode* targetNode = (*it).first;

    bool found = false;

    for (ulong i = 0; i < numRoots; i++)
    {
      XmlNode* rootNode = reinterpret_cast<XmlNode*>(rootNodes[i]);

      if (targetNode->getTree() == rootNode->getTree())
      {
        found = true;
        break;
      }
    }

    if (!found)
      ZORBA_ERROR(XUDY0014);
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::getCreatedIndices(std::vector<IndexBinding>& indices) const
{
  ulong n = theCreateIndexList.size();
  indices.resize(n);

  for (ulong i = 0; i < n; ++i)
  {
    UpdCreateIndex* upd = static_cast<UpdCreateIndex*>(theCreateIndexList[i]);
    indices[i].first = upd->theQName.getp();
    indices[i].second = upd->theIndex;
  }
}


/*******************************************************************************

********************************************************************************/
void PULImpl::getDropedIndices(std::vector<const store::Item*>& indices) const
{
  ulong n = theDropIndexList.size();
  indices.resize(n);

  for (ulong i = 0; i < n; ++i)
  {
    UpdDropIndex* upd = static_cast<UpdDropIndex*>(theDropIndexList[i]);
    indices[i] = upd->theQName.getp();
  }
}


/*******************************************************************************

********************************************************************************/
inline void applyList(std::vector<UpdatePrimitive*> aVector)
{
  for (std::vector<UpdatePrimitive*>::iterator lIter = aVector.begin();
       lIter != aVector.end();
       ++lIter) 
  {
    (*lIter)->apply();
  }
}

void PULImpl::applyUpdates(std::set<zorba::store::Item*>& validationNodes)
{
  try
  {
    theValidationNodes = &validationNodes;

    applyList(theDoFirstList);
    applyList(theInsertList);
    applyList(theReplaceNodeList);
    applyList(theReplaceContentList);
    applyList(theDeleteList);
    applyList(theValidationList);

    ulong numToRecheck = thePrimitivesToRecheck.size();
    for (ulong i = 0; i < numToRecheck; ++i)
      thePrimitivesToRecheck[i]->check();

    applyList(thePutList);

    applyList(theCreateCollectionList);
    applyList(theInsertIntoCollectionList);
    applyList(theDeleteFromCollectionList);

    applyList(theRefreshIndexList);

    applyList(theCreateIndexList);
    applyList(theDropIndexList);

    applyList(theDropCollectionList);
  }
  catch (error::ZorbaError& e)
  {
#ifndef NDEBUG
    std::cerr << "Exception thrown during pul::applyUpdates: "
              << std::endl <<  e.theDescription << std::endl;
#endif

    try
    {
      undoUpdates();
    }
    catch (...)
    {
      ZORBA_FATAL(0, "Error during pul::undoUpdates()");
    }

    throw e;
  }
  catch(...)
  {
#ifndef NDEBUG
    std::cerr << "Unknown exception thrown during pul::applyUpdates " << std::endl;
#endif

    try
    {
      undoUpdates();
    }
    catch (...)
    {
      ZORBA_FATAL(0, "Error during pul::undoUpdates()");
    }

    throw;
  }

  try
  {
    ulong numUpdates = theReplaceNodeList.size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      UpdatePrimitive* upd = theReplaceNodeList[i];

      XmlNode* node = BASE_NODE(
                      upd->getKind() == store::UpdateConsts::UP_REPLACE_CHILD ?
                      static_cast<UpdReplaceChild*>(upd)->theChild :
                      static_cast<UpdReplaceAttribute*>(upd)->theAttr);

      // To make the detach() method work properly, we must set the node's
      // parent back to what it used to be.
      node->theParent = INTERNAL_NODE(upd->theTarget);
      node->detach();
    }

    numUpdates = theReplaceContentList.size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      UpdReplaceElemContent* upd;
      upd = static_cast<UpdReplaceElemContent*>(theReplaceContentList[i]);

      ulong numChildren = upd->theOldChildren.size();
      for (ulong j = 0; j < numChildren; j++)
      {
        XmlNode* node = upd->theOldChildren.get(j);
        node->theParent = INTERNAL_NODE(upd->theTarget);
        node->detach();
      }
    }

    numUpdates = theDeleteList.size();
    for (ulong i = 0; i < numUpdates; i++)
    {
      UpdDelete* upd = static_cast<UpdDelete*>(theDeleteList[i]);

      if (upd->theParent != NULL)
      {
        XmlNode* target = BASE_NODE(upd->theTarget);
        target->theParent = upd->theParent;
        target->detach();
      }
    }
  }
  catch (...)
  {
    ZORBA_FATAL(0, "Unexpected error during pul apply");
  }
}


/*******************************************************************************

********************************************************************************/
inline void undoList(std::vector<UpdatePrimitive*> aVector)
{
  for ( std::vector<UpdatePrimitive*>::reverse_iterator lIter = aVector.rbegin();
        lIter != aVector.rend();
        ++lIter ) 
  {
    if ((*lIter)->isApplied())
      (*lIter)->undo();
  }
}

void PULImpl::undoUpdates()
{
  try
  {
    undoList(theDropCollectionList);

    undoList(theDropIndexList);
    undoList(theCreateIndexList);

    undoList(theRefreshIndexList);

    undoList(theDeleteFromCollectionList);
    undoList(theInsertIntoCollectionList);
    undoList(theCreateCollectionList);

    undoList(thePutList);

    undoList(theDeleteList);
    undoList(theReplaceContentList);
    undoList(theReplaceNodeList);
    undoList(theInsertList);
    undoList(theDoFirstList);
  }
  catch (...)
  {
    ZORBA_FATAL(0, "Unexpected error during pul undo");
  }
}


/*******************************************************************************
  For now, just disconnect the current target from its parent (if any). The
  actual deletion of the target node and its subtree is done after all update
  primitives have been applied without errors (see PULImpl::applyUpdates()
  method). This way, to undo a delete, we just need to reconnect the target node
  at its original position under its original parent.
********************************************************************************/
void UpdDelete::apply()
{
  XmlNode* target = BASE_NODE(theTarget);

  theParent = target->theParent;

  if (theParent != NULL)
  {
    theParent->deleteChild(*this);
  }
}


void UpdDelete::undo()
{
  if (theParent != NULL)
  {
    theParent->restoreChild(*this);
  }
}


/*******************************************************************************

********************************************************************************/
UpdInsertChildren::UpdInsertChildren(
    PULImpl*                         pul,
    store::UpdateConsts::UpdPrimKind kind,
    store::Item_t&                   target,
    store::Item_t&                   sibling,
    std::vector<store::Item_t>&      children,
    const store::CopyMode&           copymode)
  :
  UpdatePrimitive(pul, target),
  theKind(kind),
  theCopyMode(copymode),
  theNumApplied(0)
{
  theSibling.transfer(sibling);

  ulong numNewChildren = 0;
  ulong numChildren = children.size();
  theNewChildren.resize(numChildren);

  for (ulong i = 0; i < numChildren; i++)
  {
    if (i > 0 &&
        children[i]->getNodeKind() == store::StoreConsts::textNode &&
        theNewChildren[i-1]->getNodeKind() == store::StoreConsts::textNode)
    {
      TextNode* node1 = reinterpret_cast<TextNode*>(theNewChildren[i-1].getp());
      TextNode* node2 = reinterpret_cast<TextNode*>(children[i].getp());

      xqpStringStore_t newText = node1->getText()->append(node2->getText());
      node1->setText(newText);
    }
    else
    {
      theNewChildren[i].transfer(children[i]);
      ++numNewChildren;
    }

    if (theRemoveType == false)
    {
      store::StoreConsts::NodeKind childKind = theNewChildren[i]->getNodeKind();
      if (childKind == store::StoreConsts::elementNode ||
          childKind == store::StoreConsts::textNode)
        theRemoveType = true;
    }
  }

  theNewChildren.resize(numNewChildren);
}


void UpdInsertChildren::apply()
{
  theIsApplied = true;

  switch (theKind)
  {
  case store::UpdateConsts::UP_INSERT_INTO:
  {
    InternalNode* target = INTERNAL_NODE(theTarget);
    target->insertChildren(*this, target->numChildren());
    break;
  }
  case store::UpdateConsts::UP_INSERT_INTO_FIRST:
  {
    INTERNAL_NODE(theTarget)->insertChildren(*this, 0);
    break;
  }
  case store::UpdateConsts::UP_INSERT_INTO_LAST:
  {
    InternalNode* target = INTERNAL_NODE(theTarget);
    target->insertChildren(*this, target->numChildren());
    break;
  }
  case store::UpdateConsts::UP_INSERT_BEFORE:
  {
    BASE_NODE(theSibling)->insertSiblingsBefore(*this);
    break;
  }
  case store::UpdateConsts::UP_INSERT_AFTER:
  {
    BASE_NODE(theSibling)->insertSiblingsAfter(*this);
    break;
  }
  default:
    ZORBA_FATAL(0, "");
  }
}


void UpdInsertChildren::undo()
{
  if (theKind == store::UpdateConsts::UP_INSERT_BEFORE ||
      theKind == store::UpdateConsts::UP_INSERT_AFTER)
  {
    reinterpret_cast<InternalNode*>(theSibling->getParent())->
    undoInsertChildren(*this);
  }
  else
  {
    INTERNAL_NODE(theTarget)->undoInsertChildren(*this);
  }
}


/*******************************************************************************

********************************************************************************/
UpdInsertAttributes::UpdInsertAttributes(
    PULImpl*                     pul,
    store::Item_t&               target,
    std::vector<store::Item_t>&  attrs,
    const store::CopyMode&       copymode)
  :
  UpdatePrimitive(pul, target),
  theCopyMode(copymode),
  theNumApplied(0)
{
  ulong numAttrs = attrs.size();
  theNewAttrs.resize(numAttrs);
  for (ulong i = 0; i < numAttrs; i++)
    theNewAttrs[i].transfer(attrs[i]);
}


void UpdInsertAttributes::apply()
{
  theIsApplied = true;
  ELEM_NODE(theTarget)->insertAttributes(*this);
}


void UpdInsertAttributes::undo()
{
  ELEM_NODE(theTarget)->undoInsertAttributes(*this);
}


void UpdInsertAttributes::check()
{
  ElementNode* target = ELEM_NODE(theTarget);
  target->checkUniqueAttrs();
}


/*******************************************************************************

********************************************************************************/
UpdReplaceAttribute::UpdReplaceAttribute(
    PULImpl*                    pul,
    store::Item_t&              target,
    store::Item_t&              attr,
    std::vector<store::Item_t>& newAttrs,
    const store::CopyMode&      copymode)
  :
  UpdatePrimitive(pul, target),
  theCopyMode(copymode),
  theNumApplied(0)
{
  theAttr.transfer(attr);

  ulong numAttrs = newAttrs.size();
  theNewAttrs.resize(numAttrs);
  for (ulong i = 0; i < numAttrs; i++)
      theNewAttrs[i].transfer(newAttrs[i]);
}


void UpdReplaceAttribute::apply()
{
  theIsApplied = true;
  ELEM_NODE(theTarget)->replaceAttribute(*this);
}


void UpdReplaceAttribute::undo()
{
  ELEM_NODE(theTarget)->restoreAttribute(*this);
}


void UpdReplaceAttribute::check()
{
  ElementNode* target = ELEM_NODE(theTarget);
  target->checkUniqueAttrs();
}


/*******************************************************************************

********************************************************************************/
UpdReplaceChild::UpdReplaceChild(
    PULImpl*                    pul,
    store::Item_t&              target,
    store::Item_t&              child,
    std::vector<store::Item_t>& newChildren,
    const store::CopyMode&      copymode)
  :
  UpdatePrimitive(pul, target),
  theCopyMode(copymode),
  theNumApplied(0),
  theIsTyped(false)
{
  theChild.transfer(child);

  store::StoreConsts::NodeKind targetKind = theTarget->getNodeKind();

  store::StoreConsts::NodeKind childKind = theChild->getNodeKind();
  if (targetKind == store::StoreConsts::elementNode &&
      (childKind == store::StoreConsts::elementNode ||
       childKind == store::StoreConsts::textNode))
    theRemoveType = true;

  ulong numChildren = newChildren.size();
  theNewChildren.resize(numChildren);
  for (ulong i = 0; i < numChildren; i++)
  {
    theNewChildren[i].transfer(newChildren[i]);

    if (theRemoveType == false)
    {
      store::StoreConsts::NodeKind childKind = theNewChildren[i]->getNodeKind();
      if (targetKind == store::StoreConsts::elementNode &&
          (childKind == store::StoreConsts::elementNode ||
           childKind == store::StoreConsts::textNode))
        theRemoveType = true;
    }
  }
}


void UpdReplaceChild::apply()
{
  theIsApplied = true;
  INTERNAL_NODE(theTarget)->replaceChild(*this);
}


void UpdReplaceChild::undo()
{
  INTERNAL_NODE(theTarget)->restoreChild(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdReplaceElemContent::apply()
{
  ELEM_NODE(theTarget)->replaceContent(*this);
  theIsApplied = true;
}


void UpdReplaceElemContent::undo()
{
  ELEM_NODE(theTarget)->restoreContent(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdRenameElem::apply()
{
  ELEM_NODE(theTarget)->replaceName(*this);
  theIsApplied = true;
}


void UpdRenameElem::undo()
{
  ELEM_NODE(theTarget)->restoreName(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdSetElementType::apply()
{
  ElementNode* target = ELEM_NODE(theTarget);

  target->theTypeName.transfer(theTypeName);

  if(target->haveTypedTypedValue())
  {
    TextNode* textChild = reinterpret_cast<TextNode*>(target->getChild(0));

    xqpStringStore_t textValue;
    textChild->getStringValue(textValue);
    textChild->setValue(NULL);

    textChild->theFlags &= ~XmlNode::IsTyped;
    textChild->setText(textValue);
  }

  if (theHaveValue)
  {
    target->setHaveValue();

    if (theHaveEmptyValue)
      target->setHaveEmptyValue();

    if (theHaveTypedValue)
    {
      ZORBA_FATAL(target->numChildren() == 1, "");
      ZORBA_FATAL(target->getChild(0)->getNodeKind() == store::StoreConsts::textNode, "");

      TextNode* textChild = reinterpret_cast<TextNode*>(target->getChild(0));

      textChild->setTyped(theTypedValue);
      if (theHaveListValue)
        textChild->setHaveListValue();
    }
  }
  else
  {
    target->resetHaveValue();
  }

  if (theIsInSubstitutionGroup)
    target->setInSubstGroup();
}


/*******************************************************************************

********************************************************************************/
void UpdReplaceAttrValue::apply()
{
  ATTR_NODE(theTarget)->replaceValue(*this);
  theIsApplied = true;
}


void UpdReplaceAttrValue::undo()
{
  ATTR_NODE(theTarget)->restoreValue(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdRenameAttr::apply()
{
  ATTR_NODE(theTarget)->replaceName(*this);
  theIsApplied = true;
}


void UpdRenameAttr::undo()
{
  ATTR_NODE(theTarget)->restoreName(*this);
}


void UpdRenameAttr::check()
{
  AttributeNode* attr = ATTR_NODE(theTarget);
  if (attr->getParent() != NULL)
  {
    ElementNode* parent = reinterpret_cast<ElementNode*>(attr->getParent());
    parent->checkUniqueAttrs();
  }
}


/*******************************************************************************

********************************************************************************/
void UpdSetAttributeType::apply()
{
  AttributeNode* target = ATTR_NODE(theTarget);

  target->theTypeName.transfer(theTypeName);
  target->theTypedValue.transfer(theTypedValue);

  if (theHaveListValue)
    target->setHaveListValue();
}


/*******************************************************************************

********************************************************************************/
UpdReplaceTextValue::~UpdReplaceTextValue()
{
  if (theIsTyped)
    theOldContent.setValue(NULL);
  else
    theOldContent.setText(NULL);
}


void UpdReplaceTextValue::apply()
{
  TEXT_NODE(theTarget)->replaceValue(*this);
  theIsApplied = true;
}


void UpdReplaceTextValue::undo()
{
  TEXT_NODE(theTarget)->restoreValue(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdReplacePiValue::apply()
{
  PI_NODE(theTarget)->replaceValue(*this);
  theIsApplied = true;
}


void UpdReplacePiValue::undo()
{
  PI_NODE(theTarget)->restoreValue(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdRenamePi::apply()
{
  PI_NODE(theTarget)->replaceName(*this);
  theIsApplied = true;
}


void UpdRenamePi::undo()
{
  PI_NODE(theTarget)->restoreName(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdReplaceCommentValue::apply()
{
  COMMENT_NODE(theTarget)->replaceValue(*this);
  theIsApplied = true;
}


void UpdReplaceCommentValue::undo()
{
  COMMENT_NODE(theTarget)->restoreValue(*this);
}


/*******************************************************************************

********************************************************************************/
void UpdPut::apply()
{
  SimpleStore* store = SimpleStoreManager::getStore();

  try
  {
    store->addNode(theTargetUri->getStringValue(), theTarget);
  }
  catch(error::ZorbaError& e)
  {
    if (e.theErrorCode == API0020_DOCUMENT_ALREADY_EXISTS)
    {
      theOldDocument = store->getDocument(theTargetUri->getStringValue());
      store->deleteDocument(theTargetUri->getStringValue());
      store->addNode(theTargetUri->getStringValue(), theTarget);
    }
    else
    {
      throw;
    }
  }

  theIsApplied = true;
}


void UpdPut::undo()
{
  SimpleStore* store = SimpleStoreManager::getStore();

  store->deleteDocument(theTargetUri->getStringValue());
  store->addNode(theTargetUri->getStringValue(), theOldDocument);
}


/*******************************************************************************
  UpdatePrimitives for collection functions
********************************************************************************/


/*******************************************************************************
  UpdCreateCollection
********************************************************************************/
void UpdCreateCollection::apply()
{
  GET_STORE().createCollection(theCollectionName);
  theIsApplied = true;
}


void UpdCreateCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  if (lColl) 
  {
    GET_STORE().deleteCollection(theCollectionName);
  }
}


/*******************************************************************************
  UpdDropCollection
********************************************************************************/
void UpdDropCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  // save nodes for potential undo
  store::Item_t lTmp = NULL;
  store::Iterator_t lIter = lColl->getIterator(true);
  assert(lIter);

  lIter->open();
  while (lIter->next(lTmp))
    theSavedItems.push_back(lTmp);
  lIter->close();

  GET_STORE().deleteCollection(theCollectionName);
  theIsApplied = true;
}


void UpdDropCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  if (!lColl) {
    GET_STORE().createCollection(theCollectionName); 
#ifndef NDEBUG
    lColl = GENV_STORE.getCollection(theCollectionName);
    assert(lColl);
#endif
  }


  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theSavedItems.begin();
       lIter != theSavedItems.end(); ++lIter) {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) {
#ifndef NDEBUG
      dynamic_cast<SimpleCollection*>(lColl.getp())->addNode(lIter->getp());
#else
      static_cast<SimpleCollection*>(lColl.getp())->addNode(lIter->getp());
#endif
    }
  }
}


/*******************************************************************************
  UpdInsertIntoCollection
********************************************************************************/
void UpdInsertIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  lColl->addNode(theNode);

  theIsApplied = true;
}


void UpdInsertIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  // remove the node if it exists
  long lIndex;
  if ( (lIndex = lColl->indexOf(theNode.getp())) != -1 ) 
  {
    lColl->removeNode(lIndex);
  }
}


/*******************************************************************************
  UpdInsertFirstIntoCollection
********************************************************************************/
void UpdInsertFirstIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  for (std::vector<store::Item_t>::reverse_iterator lIter = theNodes.rbegin();
       lIter != theNodes.rend();
       ++lIter) 
  {
    lColl->addNode(*lIter, 1);
  }
}

void UpdInsertFirstIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter) 
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
      lColl->removeNode(lIndex);
    }
  }
}


/*******************************************************************************
  UpdInsertLastIntoCollection
********************************************************************************/
void UpdInsertLastIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter) 
  {
    lColl->addNode(*lIter, -1);
  }
}

void UpdInsertLastIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter) 
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
      lColl->removeNode(lIndex);
    }
  }
}


/*******************************************************************************
  UpdInsertBeforeIntoCollection
********************************************************************************/
void UpdInsertBeforeIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter)
  {
    lColl->addNode(*lIter, theTarget, true);
  }
}

void UpdInsertBeforeIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter)
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
      lColl->removeNode(lIndex);
    }
  }
}


/*******************************************************************************
  UpdInsertAfterIntoCollection
********************************************************************************/
void UpdInsertAfterIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  for (std::vector<store::Item_t>::reverse_iterator lIter = theNodes.rbegin();
       lIter != theNodes.rend();
       ++lIter) 
  {
    lColl->addNode(*lIter, theTarget, false);
  }
}


void UpdInsertAfterIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter)
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
      lColl->removeNode(lIndex);
    }
  }
}


/*******************************************************************************
  UpdInsertAtIntoCollection
********************************************************************************/
void UpdInsertAtIntoCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  ulong lPos = thePos;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter) 
  {
    lColl->addNode(*lIter, lPos++);
  }
}

void UpdInsertAtIntoCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodes.begin();
       lIter != theNodes.end();
       ++lIter) 
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
      lColl->removeNode(lIndex);
    }
  }
}


/*******************************************************************************
  UpdRemoveNodesFromCollection
********************************************************************************/
void UpdRemoveNodesFromCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  for (std::vector<store::Item_t>::iterator lIter = theNodesToDelete.begin();
       lIter != theNodesToDelete.end();
       ++lIter) 
  {
    lColl->removeNode(lIter->getp());
  }
}

void UpdRemoveNodesFromCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  long lIndex;
  for (std::vector<store::Item_t>::iterator lIter = theNodesToDelete.begin();
       lIter != theNodesToDelete.end();
       ++lIter) 
  {
    if ( ( lIndex = lColl->indexOf(lIter->getp())) != -1) 
    {
#ifndef NDEBUG
      dynamic_cast<SimpleCollection*>(lColl.getp())->addNode(lIter->getp());
#else
      static_cast<SimpleCollection*>(lColl.getp())->addNode(lIter->getp());
#endif
    }
  }
}


/*******************************************************************************
  UpdRemoveNodeAtFromCollection
********************************************************************************/
void UpdRemoveNodeAtFromCollection::apply()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

  theNode = lColl->nodeAt(thePos);
  assert(theNode);
  lColl->removeNode(thePos);
}

void UpdRemoveNodeAtFromCollection::undo()
{
  store::Collection_t lColl = GENV_STORE.getCollection(theCollectionName);
  assert(lColl);

#ifndef NDEBUG
  dynamic_cast<SimpleCollection*>(lColl.getp())->addNode(theNode);
#else
  static_cast<SimpleCollection*>(lColl.getp())->addNode(theNode);
#endif
}


/*******************************************************************************

********************************************************************************/
void UpdCreateIndex::apply()
{
  SimpleStore* store = SimpleStoreManager::getStore();

  theIndex = store->createIndex(theQName, theSpec, theSourceIter);

  theIsApplied = true;
}


void UpdCreateIndex::undo()
{
  if (theIsApplied)
  {
    SimpleStore* store = SimpleStoreManager::getStore();

    store->deleteIndex(theQName);
  }
}


/*******************************************************************************

********************************************************************************/
void UpdDropIndex::apply()
{
  SimpleStore* store = SimpleStoreManager::getStore();

  if ((theIndex = store->getIndex(theQName)) == NULL)
  {
    ZORBA_ERROR_PARAM(STR0002_INDEX_DOES_NOT_EXIST,
                      theQName->getStringValue()->c_str(), "");
  }

  store->deleteIndex(theQName);

  theIsApplied = true;
}


void UpdDropIndex::undo()
{
  if (theIsApplied)
  {
    SimpleStore* store = SimpleStoreManager::getStore();

    store->addIndex(theIndex);
  }
}


/*******************************************************************************

********************************************************************************/
void UpdRefreshIndex::apply()
{
  SimpleStore* store = SimpleStoreManager::getStore();

  if ((theIndex = store->getIndex(theQName)) == NULL)
  {
    ZORBA_ERROR_PARAM(STR0002_INDEX_DOES_NOT_EXIST,
                      theQName->getStringValue()->c_str(), "");
  }

  store->deleteIndex(theQName);

  try
  {
    store->createIndex(theQName, theIndex->getSpecification(), theSourceIter);
  }
  catch (...)
  {
    store->addIndex(theIndex);
    throw;
  }

  theIsApplied = true;
}


void UpdRefreshIndex::undo()
{
  if (theIsApplied)
  {
    SimpleStore* store = SimpleStoreManager::getStore();
    store->deleteIndex(theQName);
    store->addIndex(theIndex);
  }
}


}
}
