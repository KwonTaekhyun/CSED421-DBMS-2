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
#ifndef _EDUBFM_H_
#define _EDUBFM_H_

#include "EduBfM_basictypes.h"
#include "EduBfM_common.h"

/*@
 * Function Prototypes
 */
/* Interface Function Prototypes */
Four EduBfM_FreeTrain(TrainID *, Four);
Four EduBfM_GetTrain(TrainID *, char **, Four);
Four EduBfM_SetDirty(TrainID *, Four);
Four EduBfM_DiscardAll(void);
Four EduBfM_FlushAll(void);

#endif /* _EDUBFM_H_ */
