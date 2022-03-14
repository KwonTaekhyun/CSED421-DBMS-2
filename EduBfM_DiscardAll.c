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
 * Module: EduBfM_DiscardAll.c
 *
 * Description :
 *  Discard all buffers.
 *
 * Exports:
 *  Four EduBfM_DiscardAll(void)
 */

#include "EduBfM_Internal.h"
#include "EduBfM_common.h"

/*@================================
 * EduBfM_DiscardAll()
 *================================*/
/*
 * Function: Four EduBfM_DiscardAll(void)
 *
 * Description :
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Discard all buffers.
 *
 * Returns:
 *  error code
 */
Four EduBfM_DiscardAll(void) {
  Four e;    /* error */
  Two i;     /* index */
  Four type; /* buffer type */

  // 각 bufferPool에 존재하는 page/train들을 disk에 기록하지 않고
  // bufferPool에서 삭제함

  // 1. 각 bufTable의 모든 element들을 초기화함
  type = PAGE_BUF;
  for (i = 0; i < BI_NBUFS(type); ++i) {
    BI_KEY(type, i).volNo = 1000;
    BI_KEY(type, i).pageNo = -1;
    BI_FIXED(type, i) = 0;
    BI_BITS(type, i) = 0x0;
  }

  type = LOT_LEAF_BUF;
  for (i = 0; i < BI_NBUFS(type); ++i) {
    BI_KEY(type, i).volNo = 1000;
    BI_KEY(type, i).pageNo = -1;
    BI_FIXED(type, i) = 0;
    BI_BITS(type, i) = 0x0;
  }

  // 2. 각 hashTable에 저장된 모든 entry (즉, array index) 들을 삭제함
  edubfm_DeleteAll();

  return (eNOERROR);

} /* EduBfM_DiscardAll() */
