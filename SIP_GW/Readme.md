# 2G转4G通信系统帮助文档
## 系统模块设计
本系统主要分为以下几个模块OpenBTS，SIP网关，RTP网关，UeGate网关，srsUE，srsENB以及4G模拟平台提供的MME与IMS。
- OpenBTS：使用无线电收发系统与2G手机交互，并将GSM信令转为SIP信令与RTP数据包。
- SIP网关：在不修改OpenBTS源码的情况下，修改来自OpenBTS的SIP信令中的部分条目，使其适用于IMS，并对来自IMS的信令也进行相同的修改。此外，还能从SDP/SIP中读取RTP连接信息，并通知RTP网关模块为建立相应的连接做准备。
- RTP网关：在SIP网关的控制下，将来自OpenBTS的GSM数据包编码为AMR-WB数据包，再将其发送给UeGate；将来自UeGate的AMR-WB数据包编码为GSM格式，并返回给OpenBTS。
- UeGate：由于srsUE所提供的IP地址为内网地址，而RTP网关与SIP网关均无法直接使用该IP地址给srsUE发送信息，所以需要UeGate网关将srsUE所处机器的实际IP地址的特定端口接收到的数据包转发给srsUE。
- srsUE：将2G手机的信息注册到MME中，申请网络服务地址，并提供一个外部发送数据给IMS的接口。
- srsENB：负责管理srsUE的虚拟基站设备，提供标准ENB-MME接口，和自定义的UE-ENB接口。
- 4G模拟平台：提供4G核心网服务与IMS服务。

## 工作流程
系统主要分为4G部分与2G部分，这两部分的断面为UeGate与SIP/RTP网关。
### 2G部分
运行OpenBTS的教程详见相关博客，SIP网关与RTP网关均在相应目录下使用
```
./sip
./rtp
```
运行即可，可以通过参数输入配置文件位置，默认位置为运行时目录。
### 4G部分
先让4G模拟平台工作，但是由于单台机器上不能同时启用多个ENB，所以只能将模拟平台提供的具有空口的4GENB运行在模拟平台上，而srsENB运行在其他计算机上。又由于MME提供给ENB建立SCTP连接的接口地址默认为本地地址，要使用外部平台的srsENB就需要4G模拟平台的MME使用平台实际地址而ENB和IMS使用其他的IP地址。修改地址的操作已写在MME文件夹下的init脚本中，每次运行模拟平台时都需要先运行init脚本，否则IMS无法绑定到制定IP。<br/>
启用MME后即可启用srsENB，其配置文件位于用户目录下的.srs文件夹中(CentOS为/root/.srs文件夹)，使用
```
sudo srsenb
```
运行即可。
之后使用srsUE通过srsENB向MME注册UE，其配置文件位于与srsENB相同。使用命令
```
sudo srsue
```
运行srsUE即可。
最后运行UeGate建立其对外的连接接口即可实现2G与4G间互通电话等功能。
## 模块间通信
```
               |------|  SIP  |------|
              /|SIP_GW|-------|      |
          SIP/ |------| UeCtrl|      |
| -------|/    |           |      |  UDP |-----|srsCtrl|------| S1AP|-----|
| OpenBTS|     |RTPCtrl    |UeGate|------|srsUE|-------|srsENB|-----| MME |
| -------|\    |           |      |  TCP |-----|       |------|     |-----|
          RTP\ |------|       |      |
              \|RTP_GW|-------|      |
               |------|       |------|
```
### 包结构
#### UeCtrl
UeCtrl基于UDP协议，主要有两个功能，一是UeGate上线后，向SIP_GW注册用户，让SIP_GW知晓应该将SIP包发送到什么位置。之后是RTP_GW准备好传输RTP数据后，通知UeGate开放相应的端口接收数据。其包结构定义在SIP_GW/srsuecontrolpacket.hpp头文件中

|身份标志(IMSI)| 事件类型(event)| 数据(data)|
|-------|-------|------|
|15Bytes| 1Bytes|8Bytes|

事件与数据解析

|事件名称|event值|数据解析流程|
|---|---|---|
|向SIP_GW注册用户|0x01|包中给出的数据为该用户在MME中注册得到的地址，再通过解析发送包的源地址即可知道该新用户的虚拟地址与真实地址|
|UeGate增加监听端口|0x02|数据包内传输4个Port(网络序)如下，RTP_GW的RTP端口，IMS的RTP端口，RTP_GW的RTCP端口，IMS的RTCP端口|
|UeGate停止监听端口|0x03|数据包内传输4个Port(网络序)如下，RTP_GW的RTP端口，IMS的RTP端口，RTP_GW的RTCP端口，IMS的RTCP端口|
#### RTPCtrl
RTPCtrl基于UDP协议，用于SIP_GW向RTP_GW添加通话信息。每次添加一个数据源的信息，通过callid辨别是否不同数据源是否位于同一会话中，并将同一会话中的用户放置于同一个slot中；删除会话时通过callid删除所有数据源的信息。其包结构定义在SIP_GW/rtpcontrolpacket.hpp头文件中

|命令|PayloadType|编码格式|slot中的位置|数据源地址|callid|
|---|---|---|---|---|---|
|1Bytes|1Bytes|1Bytes|1Bytes|sizeof(sockaddr_in)|不定|

一个slot共有4个位置，分别表示
- OpenBTS为主叫方
- IMS为被叫方
- OpenBTS为被叫方
- IMS为主叫方

#### srsCtrl
srsCtrl是基于UDP的，用于代替UE与ENB之间的RRC层所设计的传输协议，其结构如下。
##### srsue->srsenb
|type|ip|port|imsi|lcid|rrc establishment cause|data|
|---|---|---|---|---|---|---|
|1Bytes|4Bytes|2Bytes|15Bytes|2Bytes|sizeof(LIBLTE_S1AP_RRC_ESTABLISHMENT_CAUSE_ENUM)|不定|

##### srsenb->srsue
|type|imsi|lcid|data|
|---|---|---|---|
|1Bytes|15Bytes|2Bytes|不定|

## 源码
### SIP_GW
SIP网关中共有两个线程，SIPControlThread用于接收UeGate发来的控制信息及时更新用户列表，SIPThread则用于处理SIP信令。
#### SIPControlThread
接收到UeGate发来的控制包，进行解析后添加用户信息到用户记录中。
#### SIPThread
对于从OpenBTS发出的SIP信令，需要进行以下操作
- 从信令中获取发送方的IMSI
- 修改SIP信令中的Via信息为虚拟UE的信息
- 修改SIP信令中的域信息为与IMS相匹配的信息
- 去除OpenBTS所发出的url中的IMSI四个字母
- 将SIP信令中的Contact信息修改为适用于IMS的信息
- 将SIP信令中的Route代理信息删除

对于从IMS发出的SIP信令，需要进行以下操作
- 修改Via信息
- 将url中的tel信息依据用户列表修改为SIP信息
- 将域的信息改为OpenBTS

对于具有SDP的信令还需要进行以下操作
- 从SIP中取出callid
- 从SDP中获取PayloadType
- 从SDP中获取编码格式
- 确定该数据源在slot中的位置
- 构建控制包，并将其发送给RTP_GW
- 修改SDP中的Media，Connection信息
- 替换SDP中不需要的信息
- 将SIP中的SDP信息更新为新的SDP

### RTP_GW
RTP网关中共有三个线程，RTPControlThread，RTPThread和RTCPThread。
#### RTPControlThread
接收到SIP_GW发来的控制包，进行解析后更新用户信息记录表。
#### RTPThread
接收到RTP包后进行以下操作
- 通过包的源地址获取源信息
- 进行编解码操作
- 重新打包得到新的RTP包
- 再将新的RTP包发送到slot中的对应地址

##### 编解码操作
AMR-WB->GSM的操作如下
- 将非对齐的AMR-WB数据重新排列为对齐的AMR-WB数据
- 将对齐的AMR-WB编解码为GSM格式数据

GSM->AMR-WB的操作如下
- 将GSM格式的数据编解码为对齐的AMR-WB数据
- 将对齐的AMR-WB数据重新排列为非对齐的AMR-WB数据

#### RTCPThread
获取RTCP包的源地址，从用户信息列表中获取接收方地址，并将该RTCP包直接转送即可。

### srsENB
srsENB的Vir-RRC层一共有以下几类函数
- S1AP接口：该接口主要负责处理MME与ENB之间的信令处理
- GTPU接口：该接口主要负责处理MME与ENB之间的数据传递
- Handle函数：该接口主要处理block_queue中的SDU

#### S1AP接口
- write_dl_info：将来自MME的NAS信令放入到消息队列中
- release_complete：将ue的信息从用户数据列表中删除
- setup_ue_ctxt：将默认承载的信息更新到用户信息列表中，理论上来说应该在此时通知UE有新的信道可以使用，但由于我们默认每一个信道都总是可用的，所以此处不做任何处理
- setup_ue_erabs：同上
- release_erabs：同上，只是将建立承载过程换为消除承载
- add_paging_id：寻呼过程，但由于目前认为每一个用户都能一直在线，所以并不存在寻乎过程

#### GTPU接口
- write_sdu：将从MME收到的数据包加入到消息队列里

#### Handle函数
- handle_normal:通过S1AP用于传递NAS信令给MME。
- handle_data：通过GTPU通过传输数据给MME。
- handle_attach：在设备初次连接时使用attach过程。
- send_normal：将来自MME的NAS信令传递给UE。
- send_paging：将来自MME的Paging消息传递给UE，但实际上并没有写相关的逻辑。
- append_head：为sdu包添加头部结构信息。
- receive_uplink：将来自UE的数据去掉自定义的头部信息，再将其编排为srsENB内部的结构，之后调用不同的的handle函数处理信令包与数据包。
- send_downlink：读取来自消息队列中的消息，调用不同的handle函数，最后将其发送给UE。
