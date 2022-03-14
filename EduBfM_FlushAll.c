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
 * Module: EduBfM_FlushAll.c
 *
 * Description :
 *  Flush dirty buffers holding trains.
 *
 * Exports:
 *  Four EduBfM_FlushAll(void)
 */

#include "EduBfM_Internal.h"
#include "EduBfM_common.h"

/*@================================
 * EduBfM_FlushAll()
 *================================*/
/*
 * Function: Four EduBfM_FlushAll(void)
 *
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Flush dirty buffers holding trains.
 *  A dirty buffer is one with the dirty bit set.
 *
 * Returns:
 *  error code
 */
Four EduBfM_FlushAll(void) {
  Four e;    /* error */
  Two i;     /* index */
  Four type; /* buffer type */

  // 각 bufferPool에 존재하는 page/train들 중 수정된 page/train들을 disk에
  // 기록함

  // 1. DIRTY bit가 1로 set 된 buffer element들에 저장된 각 page/train에 대해,
  // edubfm_FlushTrain()을 호출하여 해당 page/train을 disk에 기록함
  type = PAGE_BUF;
  for (i = 0; i < BI_NBUFS(type); ++i) {
    if ((BI_BITS(type, i) & DIRTY) != 0) {
      edubfm_FlushTrain(&(BI_KEY(type, i)), type);
      BI_BITS(type, i) &= ~DIRTY;
    }
  }
  type = LOT_LEAF_BUF;
  for (i = 0; i < BI_NBUFS(type); ++i) {
    if ((BI_BITS(type, i) & DIRTY) != 0) {
      edubfm_FlushTrain(&(BI_KEY(type, i)), type);
      BI_BITS(type, i) &= ~DIRTY;
    }
  }

  return (eNOERROR);

} /* EduBfM_FlushAll() */
