# Lisp Interpretor
### 前言
虽然作为一名准安全人员，还号称自己研究过二进制，但是几乎没有纯用c开发过东西，这在面试滴滴安全的时候被深深鄙视了。c和c++作为程序员的一个基础，因此需要在实战中磨练。由此，在硕士毕业尾端，无事可做，便想到写个解析器，一来打发无聊，二来弥补一些自己的遗憾
### 参考
http://www.buildyourownlisp.com
### 编译
cc parsing.c mpc.c -lreadline -std=c99 -o parsing