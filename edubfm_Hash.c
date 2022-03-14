/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubfm_Hash.c
 *
 * Description:
 *  Some functions are provided to support buffer manager.
 *  Each BfMHashKey is mapping to one table entry in a hash table(hTable),
 *  and each entry has an index which indicates a buffer in a buffer pool.
 *  An ordinary hashing method is used and linear probing strategy is
 *  used if collision has occurred.
 *
 * Exports:
 *  Four edubfm_LookUp(BfMHashKey *, Four)
 *  Four edubfm_Insert(BfMHaskKey *, Two, Four)
 *  Four edubfm_Delete(BfMHashKey *, Four)
 *  Four edubfm_DeleteAll(void)
 */

#include <stdlib.h> /* for malloc & free */

#include "EduBfM_Internal.h"
#include "EduBfM_common.h"

/*@
 * macro definitions
 */

/* Macro: BFM_HASH(k,type)
 * Description: return the hash value of the key given as a parameter
 * Parameters:
 *  BfMHashKey *k   : pointer to the key
 *  Four type       : buffer type
 * Returns: (Two) hash value
 */
#define BFM_HASH(k, type) (((k)->volNo + (k)->pageNo) % HASHTABLESIZE(type))

/*@================================
 * edubfm_Insert()
 *================================*/
/*
 * Function: Four edubfm_Insert(BfMHashKey *, Two, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Insert a new entry into the hash table.
 *  If collision occurs, then use the linear probing method.
 *
 * Returns:
 *  error code
 *    eBADBUFINDEX_BFM - bad index value for buffer table
 */
Four edubfm_Insert(BfMHashKey *key, /* IN a hash key in Buffer Manager */
                   Two index,       /* IN an index used in the buffer pool */
                   Four type)       /* IN buffer type */
{
  Four i;
  Two hashValue;

  CHECKKEY(key); /*@ check validity of key */

  if ((index < 0) || (index > BI_NBUFS(type))) {
    ERR(eBADBUFINDEX_BFM);
    return eBADBUFINDEX_BFM;
  }

  // 1. 해당 buffer element에 저장된 page/train의 hash key value를 이용하여,
  // hashTable에서 해당 array index를 삽입할 위치를 결정함
  hashValue = BFM_HASH(key, type);
  Two hashEntry = BI_HASHTABLEENTRY(type, hashValue);

  if (hashEntry == -1) {
    // 2. Collision이 발생하지 않은 경우, 해당 array index를 결정된 위치에
    // 삽입함
    BI_HASHTABLEENTRY(type, hashValue) = index;
  } else {
    // 3. Collision이 발생한 경우, chaining 방법을 사용하여 이를 처리함
    // 1) 해당 buffer element에 대한 nextHashEntry 변수에 기존 hashTable entry
    // (array index) 를 저장함
    BI_NEXTHASHENTRY(type, index) = hashEntry;

    // 2) 새로운 array index를 결정된 위치에 삽입함
    BI_HASHTABLEENTRY(type, hashValue) = index;
  }

  return (eNOERROR);
} /* edubfm_Insert */

/*@================================
 * edubfm_Delete()
 *================================*/
/*
 * Function: Four edubfm_Delete(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the entry which corresponds to `key' and
 *  Delete the entry from the hash table.
 *
 * Returns:
 *  error code
 *    eNOTFOUND_BFM - The key isn't in the hash table.
 */
Four edubfm_Delete(BfMHashKey *key, /* IN a hash key in buffer manager */
                   Four type)       /* IN buffer type */
{
  Two i, prev;
  Two hashValue;

  CHECKKEY(key); /*@ check validity of key */

  // 1. 해당 buffer element에 저장된 page/train의 hash key value를 이용하여,
  // 삭제할 buffer element의 array index를 hashTable에서 검색함
  i = edubfm_LookUp(key, type);

  if (i != -1) {
    // 2. 검색된 entry (array index) 를 hashTable에서 삭제함
    // 동일한 hash key value를 갖는 page/train들이 저장된 buffer element들의
    // array index들간의 linked list 구조가 유지되도록 해당 array index를 삭제함

    Two prev = BI_NEXTHASHENTRY(type, i);
    hashValue = BFM_HASH(key, type);
    BI_HASHTABLEENTRY(type, hashValue) = prev;

    return (eNOERROR);
  } else {
    ERR(eNOTFOUND_BFM);
    return eNOTFOUND_BFM;
  }

} /* edubfm_Delete */

/*@================================
 * edubfm_LookUp()
 *================================*/
/*
 * Function: Four edubfm_LookUp(BfMHashKey *, Four)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Look up the given key in the hash table and return its
 *  corressponding index to the buffer table.
 *
 * Retruns:
 *  index on buffer table entry holding the train specified by 'key'
 *  (NOTFOUND_IN_HTABLE - The key don't exist in the hash table.)
 */
Four edubfm_LookUp(BfMHashKey *key, /* IN a hash key in Buffer Manager */
                   Four type)       /* IN buffer type */
{
  Two i, j; /* indices */
  Two hashValue;

  CHECKKEY(key); /*@ check validity of key */

  // 1. 현재 page/train의 hash key value 구하기
  hashValue = BFM_HASH(key, type);

  // 2. hashTableEntry에 해당하는 buffer element idx 구하기 (후보)
  i = BI_HASHTABLEENTRY(type, hashValue);

  // 3. 정확히 pageNo와 volNo가 일치하는 page/train을 찾는다
  while (i != NOTFOUND_IN_HTABLE) {
    BfMHashKey *key2 = &BI_KEY(type, i);

    if (EQUALKEY(key, key2)) {
      break;
    } else {
      i = BI_NEXTHASHENTRY(type, i);
    }
  }

  return i;
} /* edubfm_LookUp */

/*@================================
 * edubfm_DeleteAll()
 *================================*/
/*
 * Function: Four edubfm_DeleteAll(void)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Delete all hash entries.
 *
 * Returns:
 *  error code
 */
Four edubfm_DeleteAll(void) {
  Two i;
  Two tableSize;
  Four type;

  // 각 hashTable에서 모든 entry (buffer element의 array index) 들을 삭제함

  type = PAGE_BUF;
  tableSize = HASHTABLESIZE(type);
  for (int i = 0; i < tableSize; ++i) {
    BI_HASHTABLEENTRY(type, i) = -1;
  }
  type = LOT_LEAF_BUF;
  tableSize = HASHTABLESIZE(type);
  for (int i = 0; i < tableSize; ++i) {
    BI_HASHTABLEENTRY(type, i) = -1;
  }

  return (eNOERROR);

} /* edubfm_DeleteAll() */
