# NET Project

* 각자 맡은 부분의 실행 환경(캡처까지 필요 없고 mysql 썻는지와 이외 뭔가 추가로 해야 할 것 있는지 (ws2_32.lib는 뻔하니 빼죠.)) 적어주기.
* 각자 코드를 테스트 할 때 썼던 데이터 셋을 적어주기 (sql insert 문이나 아님 말로 설명하셔도 됩니다.) 
* 자신이 담당한 부분, 서버, 클라이언트 구분 되도록 파일 명을 바꿔주세요. (git pull 하면 파일명만으로 구분 가능합니다.)



## Rider

#### 실행 환경

```
서버 : ws2_32.lib 추가(+강의 노트), libmysql.lib 추가(+ include, lib 폴더에 추가)
클라이언트 : ws2_32.lib 추가(+강의 노트)
```

#### 테스트 데이터 셋

```sql
# 광고 데이터 베이스
insert into advertisement(storename, ridername) values ('wow', 'gugu');
insert into advertisement(storename, ridername) values ('gg', 'gg');
insert into advertisement(storename, ridername) values ('about', NULL);
insert into advertisement(storename, ridername) values ('starbus', NULL);

# 주문 데이터 베이스
insert into ordering(storename, menuname, ridername, storestatus, riderstatus) values ('about', 'smoothe', NULL, 0, 0);
insert into ordering(storename, menuname, ridername, storestatus, riderstatus) values ('about', 'maxime', NULL, 0, 0);
insert into ordering(storename, menuname, ridername, storestatus, riderstatus) values ('wow','pizza', 'gugu', 2, 1);
```

#### 코드 볼 시 가이드

1. 저는 select 문을 피하기 위해서 udp로 처리하기로 하였습니다. 공용 구조체로 요청을 받아 어떤 요청인지 확인하고 해당 요청에 따라 switch를 통해 분기처리 하였습니다. 이 방식을 사용하면 server에 블락될 요소가 없기 때문에 thread를 쓸 필요가 없다고 생각합니다. **아직 update를 안해봐서 확실하지 않습니다.**

2. client는 로그인 후 작업이라는 흐름을 따라가고, 서버는 요청이 오면 처리하고 다시 요청 받는 식입니다. 분기 처리가 많은데 제가 정의한 구조체를 먼저 읽고 흐름을 따라가시기 바랍니다.

#### 주의 사항

1. 상점의 픽업 대기를 status가 2라고 가정하였습니다. 
2.  배달원과 매칭 전 riderstatus는 0, 배달원이 선택 후 riderstatus는 1, 배달원이 배달 완료 클릭 후 riderstatus는 2로 할 생각입니다.

