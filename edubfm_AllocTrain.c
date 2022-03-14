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
 * Module: edubfm_AllocTrain.c
 *
 * Description :
 *  Allocate a new buffer from the buffer pool.
 *
 * Exports:
 *  Four edubfm_AllocTrain(Four)
 */

#include <errno.h>

#include "EduBfM_Internal.h"
#include "EduBfM_common.h"

extern CfgParams_T sm_cfgParams;

/*@================================
 * edubfm_AllocTrain()
 *================================*/
/*
 * Function: Four edubfm_AllocTrain(Four)
 *
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Allocate a new buffer from the buffer pool.
 *  The used buffer pool is specified by the parameter 'type'.
 *  This routine uses the second chance buffer replacement algorithm
 *  to select a victim.  That is, if the reference bit of current checking
 *  entry (indicated by BI_NEXTVICTIM(type), macro for
 *  bufInfo[type].nextVictim) is set, then simply clear
 *  the bit for the second chance and proceed to the next entry, otherwise
 *  the current buffer indicated by BI_NEXTVICTIM(type) is selected to be
 *  returned.
 *  Before return the buffer, if the dirty bit of the victim is set, it
 *  must be force out to the disk.
 *
 * Returns;
 *  1) An index of a new buffer from the buffer pool
 *  2) Error codes: Negative value means error code.
 *     eNOUNFIXEDBUF_BFM - There is no unfixed buffer.
 *     some errors caused by fuction calls
 */
Four edubfm_AllocTrain(Four type) /* IN type of buffer (PAGE or TRAIN) */
{
  Four e = 0;       /* for error */
  Four victim = -1; /* return value */
  Four i;

  /* Error check whether using not supported functionality by EduBfM */
  if (sm_cfgParams.useBulkFlush) ERR(eNOTSUPPORTED_EDUBFM);

  // 1. Second chance buffer replacement algorithm을 사용하여, 할당 받을
  // buffer element를 선정함
  // 1) 할당 대상 선정을 위해 대응하는 fixed 변수 값이 0인 buffer element들을
  // 순차적으로 방문함
  // 각 buffer element 방문시 REFER bit를 검사하여 동일한 buffer element를 2회째
  // 방문한 경우 (REFER bit == 0), 해당 buffer element를 할당 대상으로 선정하고,
  // 아닌 경우 (REFER bit == 1), REFER bit를 0으로 설정함
  Four fixedNum = 0;

  i = BI_NEXTVICTIM(type);
  if (BI_FIXED(type, i) != 0) {
    ++fixedNum;
  } else if ((BI_BITS(type, i) & REFER) == 0) {
    victim = i;
    BI_BITS(type, i) |= REFER;
  } else {
    BI_BITS(type, i) &= ~REFER;
  }

  if (victim == -1) {
    for (i = BI_NEXTVICTIM(type) + 1; i != BI_NEXTVICTIM(type);
         i = (i + 1) % BI_NBUFS(type)) {
      if (BI_FIXED(type, i) != 0) {
        ++fixedNum;
        continue;
      } else if ((BI_BITS(type, i) & REFER) == 0) {
        victim = i;
        BI_BITS(type, i) |= REFER;
        break;
      } else {
        BI_BITS(type, i) &= ~REFER;
      }
    }
  }

  if (fixedNum == BI_NBUFS(type)) e = eNOUNFIXEDBUF_BFM;

  if (victim == -1) {
    i = BI_NEXTVICTIM(type);
    if (BI_FIXED(type, i) != 0) {
    } else if ((BI_BITS(type, i) & REFER) == 0) {
      victim = i;
      BI_BITS(type, i) |= REFER;
    } else {
      BI_BITS(type, i) &= ~REFER;
    }

    if (victim == -1) {
      for (i = BI_NEXTVICTIM(type) + 1; i != BI_NEXTVICTIM(type);
           i = (i + 1) % BI_NBUFS(type)) {
        if (BI_FIXED(type, i) != 0) {
          continue;
        } else if ((BI_BITS(type, i) & REFER) == 0) {
          victim = i;
          BI_BITS(type, i) |= REFER;
          break;
        } else {
          BI_BITS(type, i) &= ~REFER;
        }
      }
    }
  }

  if (victim != -1) {
    // 2) 선정된 buffer element와 관련된 데이터 구조를 초기화함
    // 2-1) 선정된 buffer element에 저장되어 있던 page/train이 수정된 경우, 기존
    // buffer element의 내용을 disk로 flush함
    edubfm_FlushTrain(&BI_KEY(type, victim), type);

    // 2-2) 선정된 buffer element에 대응하는 bufTable element를 초기화함
    BI_FIXED(type, victim) = 0;
    BI_BITS(type, victim) = 0x4;

    for (int i = 0; i < BI_NBUFS(type); ++i) {
      if (BI_NEXTHASHENTRY(type, i) == victim) {
        BI_NEXTHASHENTRY(type, i) = BI_NEXTHASHENTRY(type, victim);
      }
    }

    // 2-3) 선정된 buffer element의 array index (hashTable entry) 를
    // hashTable에서 삭제함
    edubfm_Delete(&BI_KEY(type, victim), type);

    // 제거하려는 elem과 연결된 elem을 hash table에 연결한 후 제거
    BI_NEXTHASHENTRY(type, victim) = -1;

    // 3) 선정된 buffer element의 array index를 반환함
    return victim;
  } else {
    return e;
  }
} /* edubfm_AllocTrain */
