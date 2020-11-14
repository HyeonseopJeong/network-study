# PA2
TCP와 UDP를 각 각 이용하여 client to server로 파일을 전송하는 프로그램 만들기.

## 파일 전송 프로그램 설명
#### client
- Client는 Server에 접속한 후, 자신의 파일을 Server에게 전송을 하는 프로그램이다.
- Client는 먼저 전송할 파일의 이름을 Server에게 알려준 후, 파일의 내용을 전송한다.
- 이 때 파일 이름을 전송한 다음 __지연 없이 즉시 파일 내용을 전송__ 하여야 한다.

#### server
- Server에서는 전송받은 파일 이름으로 해당 파일을 저장하여야 한다.
- 파일의 형태는 ASCII or binary 파일 모두를 지원 가능해야 한다. 
(하나의 프로그램 이용하여 수행하여야 하며, 일반적으로 바이너리 파일 전송이 가능하게 프로그램을 작성하면 ASCII인 경우도 동작한다.)
- 서버는 __iterative server__ 형태를 지니도록 한다.


### TCP

- Client로부터 파일 이름을 받았다는 ACK 등과 같은 것을 사용해서는 안된다.

**_프로그램 실행 예시_**

``` shell
$ TCPserver <port_number>
$ TCPclient <server_ip_address> <server_port_number> <file_name>
```


### UDP

- 주어진 파일 전송을 완료한 후에 종료는 정확하게 하려고 노력하여야 한다.
(발생할 수 있는 여러 가지 상황을 고려하여야 하고, reasonable한 조건에서 종료를 하여야 한다.)

- 파일 이름은 정확하게 전송되었는지 확인 해야하지만 파일 내용에서 발생한 에러는 프로그램에서 recovery하지 않는다.

**_프로그램 실행 예시_**
``` shell
$ UDPserver <port_number> 
$ UDPclient <server_ip_address> <server_port_number> <fie_name>
```


## 주의사항
- 프로그램 작성 시에 sleep() 등과 같이 시간을 지연시키는 code를 삽입해서는
안된다.
- 파일 전송을 마친 다음, server와 client에서 ```$ wc <file_name>``` 의 명령어를
이용하여 파일이 오류없이 전송되었는지 확인하여야 한다.
- 파일 크기는 1 MB 정도가 되는 binary 파일을 이용하면 된다.
