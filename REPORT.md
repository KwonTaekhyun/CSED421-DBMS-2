# EduBfM Report

Name: 권택현

Student id: 20180522

# Problem Analysis

본 과제는 객체 관계형 DBMS인 오디세우스의 저장 시스템인 오디세우스/COSMOS 중 Buffer manager에 대한 연산들을 구현하는 것으로
구체적으로는 Disk상의 page또는 train을 main memory 상에 유지하는 buffer manager에 대한 데이터 구조 및 연산들을 구현하는 것입니다.

# Design For Problem Solving

## High Level

### < Buffer manager API 구현 >

1. EduBfM_GetTrain()
    Page/train을 bufferPool에 fix 하고, page/train이 저장된 buffer element에 대한 포인터를 반환함
    
    1. Fix 할 page/train의 hash key value를 이용하여, 해당 page/train이 저장된 buffer element의 array index를 hashTable에서 검색함
    2. Fix 할 page/train이 bufferPool에 존재하지 않는 경우,
        1) bufferPool에서 page/train을 저장할 buffer element 한 개를 할당 받음
        2) Page/train을 disk로부터 읽어와서 할당 받은 buffer element에 저장함
        3) 할당 받은 buffer element에 대응하는 bufTable element를 갱신함
        4) 할당 받은 buffer element의 array index를 hashTable에 삽입함
        5) 할당 받은 buffer element에 대한 포인터를 반환함
    3. Fix 할 page/train이 bufferPool에 존재하는 경우,
        1) 해당 page/train이 저장된 buffer element에 대응하는 bufTable element를 갱신함
        2) 해당 buffer element에 대한 포인터를 반환함

2. EduBfM_FreeTrain()
    Page/train을 bufferPool에서 unfix 함

    1. Unfix 할 page/train의 hash key value를 이용하여, 해당 page/train이 저장된 buffer element의 array index를 hashTable에서 검색함
    2. 해당 buffer element에 대한 fixed 변수 값을 1 감소시킴 (0미만이 되지 않도록 한다.)

3. EduBfM_SetDirty()
    bufferPool에 저장된 page/train이 수정되었음을 표시하기 위해 DIRTY bit를 1로 set함 

    1. 수정된 page/train의 hash key value를 이용하여, 해당 해당 page/train이 저장된 buffer element의 array index를 hashTable에서 검색함
    2. 해당 buffer element에 대한 DIRTY bit를 1로 set함

4. EduBfM_FlushAll()
    각 bufferPool에 존재하는 page/train들 중 수정된 page/train들을 disk에 기록함

    1. DIRTY bit가 1로 set 된 buffer element들에 저장된 각 page/train에 대해, edubfm_FlushTrain()을 호출하여 해당 page/train을 disk에 기록함

5. EduBfM_DiscardAll()
    각 bufferPool에 존재하는 page/train들을 disk에 기록하지 않고 bufferPool에서 삭제함

    1. 각 bufTable의 모든 element들을 초기화함
    2. 각 hashTable에 저장된 모든 entry (즉, array index) 들을 삭제함


## Low Level

### < Buffer manager internal function 구현 >

1. edubfm_ReadTrain()
    Page/train을 disk로부터 읽어와서 buffer element에 저장하고, 해당 buffer element에 대한 포인터를 반환함

2. edubfm_AllocTrain()
    bufferPool에서 page/train을 저장하기 위한 buffer element를 한 개 할당 받고, 해당 buffer element의 array index를 반환함

    1. Second chance buffer replacement algorithm을 사용하여, 할당 받을 buffer element를 선정함
        1) 할당 대상 선정을 위해 대응하는 fixed 변수 값이 0인 buffer element들을 순차적으로 방문함
        2) 각 buffer element 방문시 REFER bit를 검사하여 동일한 buffer element를 2회째 방문한 경우 (REFER bit == 0), 해당 buffer element를 할당 대상으로 선정하고, 아닌 경우 (REFER bit == 1), REFER bit를 0으로 설정함
    2. 선정된 buffer element와 관련된 데이터 구조를 초기화함
        1) 선정된 buffer element에 저장되어 있던 page/train이 수정된 경우, 기존 buffer element의 내용을 disk로 flush함
        2) 선정된 buffer element에 대응하는 bufTable element를 초기화함
        3) 선정된 buffer element의 array index (hashTable entry) 를 hashTable에서 삭제함
    3. 선정된 buffer element의 array index를 반환함

3. edubfm_Insert()
    hashTable에 buffer element의 array index를 삽입함

    1. 해당 buffer element에 저장된 page/train의 hash key value를 이용하여, hashTable에서 해당 array index를 삽입할 위치를 결정함
        1) Hash key value가 n 인 경우, hashTable의 n 번째 entry에 삽입함
    2. Collision이 발생하지 않은 경우, 해당 array index를 결정된 위치에 삽입함
    3. Collision이 발생한 경우, chaining 방법을 사용하여 이를 처리함
        1) 해당 buffer element에 대한 nextHashEntry 변수에 기존 hashTable entry (array index) 를 저장함
        2) 새로운 array index를 결정된 위치에 삽입함 (동일한 hash key value를 갖는 page/train들이 저장된 buffer element들의 array index들이 linked list 형태로 연결되어 유지됨)

4. edubfm_Delete()
    hashTable에서 buffer element의 array index를 삭제함

    1. 해당 buffer element에 저장된 page/train의 hash key value를 이용하여, 삭제할 buffer element의 array index를 hashTable에서 검색함
    2. 검색된 entry (array index) 를 hashTable에서 삭제함
        1) 동일한 hash key value를 갖는 page/train들이 저장된 buffer element들의 array index들간의 linked list 구조가 유지되도록 해당 array index를 삭제함

5. edubfm_DeleteAll()
    각 hashTable에서 모든 entry (buffer element의 array index) 들을 삭제함

6. edubfm_LookUp()
    hashTable에서 파라미터로 주어진 hash key (BfMHashKey) 에 대응하는 buffer element의 array index를 검색하여 반환함

    1. 해당 hash key를 갖는 page/train이 저장된 buffer element의 array index를 hashTable에서 검색함
    2. 검색된 array index를 반환함

7. edubfm_FlushTrain()
    수정된 page/train을 disk에 기록함

    1. Flush 할 page/train의 hash key value를 이용하여, 해당 page/train이 저장된 buffer element의 array index를 hashTable에서 검색함
    2. 해당 buffer element에 대한 DIRTY bit가 1로 set 된 경우, 해당 page/train을 disk에 기록함
    3. 해당 DIRTY bit를 unset 함

# Mapping Between Implementation And the Design

### < Buffer manager API 구현 >

1. EduBfM_GetTrain()
    ![getReain](https://user-images.githubusercontent.com/57590123/158126758-8f5487b0-88eb-4a7f-982e-8fc0bc0ff62f.png)

2. EduBfM_FreeTrain()
    ![freeTrain](https://user-images.githubusercontent.com/57590123/158126986-5502e787-87ad-45c8-8d2d-f07ef498fd2e.png)

3. EduBfM_SetDirty()
    ![setDirty](https://user-images.githubusercontent.com/57590123/158127042-0ee9fcda-4e5a-4eef-b5e8-cf47fb4e52d1.png)

4. EduBfM_FlushAll()
    ![flushAll](https://user-images.githubusercontent.com/57590123/158127101-d818d62b-ddd9-4821-9eea-a9100ec46fa2.png)

5. EduBfM_DiscardAll()
    ![discardAll](https://user-images.githubusercontent.com/57590123/158127150-f70f6704-8cd9-4d48-8e03-9020c361b3a2.png)

### < Buffer manager internal function 구현 >

1. edubfm_ReadTrain()
    ![readTrain](https://user-images.githubusercontent.com/57590123/158127195-2504b9bc-e4d1-4e5f-9ff8-3d3c362adb13.png)

2. edubfm_AllocTrain()
    ![allocTrain](https://user-images.githubusercontent.com/57590123/158127258-2e3ddf68-53be-4f23-8e41-e0b8c35de669.png)

3. edubfm_Insert()
    ![insert](https://user-images.githubusercontent.com/57590123/158127322-318b402d-5c08-4bee-9fbe-782b8b535765.png)
8-2e3ddf68-53be-4f23-8e41-e0b8c35de669.png)

4. edubfm_Delete()
    ![delete](https://user-images.githubusercontent.com/57590123/158127383-57c314f8-eacc-4252-a49c-cea9c1b3a3c8.png)

5. edubfm_DeleteAll()
    ![deleteAll](https://user-images.githubusercontent.com/57590123/158127542-10006ae9-d0c0-42ca-a4f1-8136cc371625.png)

6. edubfm_LookUp()
    ![lookUp](https://user-images.githubusercontent.com/57590123/158127604-a7686dc4-d2d9-40a2-b17b-3ffac8629f0c.png)

7. edubfm_FlushTrain()
    ![flushTrain](https://user-images.githubusercontent.com/57590123/158127654-5c42032e-52f4-4251-98e0-3f6367a4b5c5.png)
