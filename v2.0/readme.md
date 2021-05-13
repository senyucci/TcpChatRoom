&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;在继之前完成windows间的TCP通信后，想要实现远程TCP通信，若将本机作为服务器，除非进行内网穿透则无法实现，
倒不如直接部署一台远程服务器进行文本的接收与转发，在购入一台轻量级linux服务器后进行linux服务器上的服务端编写，在编写过程中发现linux上的TCP通信与windows不尽相同，
于是在查询相关api后进行服务端的编写。
