# p2p_Net
### 标签：p2p网络 tcp打洞、 Nat穿透、seed服务器、peer客户端

### 使用boost asio库实现了异步IO 多节点P2P通信

### 协议采用tcp协议

### 设置一个seed服务器，用来保存peer节点信息，对等节点建立连接后，不依靠seed服务器

p2pClient为peer对等客户端

seed_server为seed服务器，用来给无法互通的节点，建立链接

数据通信：仅实现了通信功能，具体数据内容，需要进一步编辑。

test_net 测试程序

