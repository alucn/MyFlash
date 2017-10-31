### 1.How to build
该工具推荐用户在下载源码之后，进行动态编译链接安装

#### 动态编译链接
flashback利用MySQL的源码，提供了远程回滚功能。然而相关API并没有在原生的limysqlclient中导出。因此如果要顺利编译和运行，需要到该地址进行编译安装(https://github.com/mark-neil-wang/mysql57-flashback-related) 之后（安装方法参考MySQL文档http://dev.mysql.com/doc/internals/en/cmake-howto-quick-debug-configuration.html）
再运行如下命令安装flashback软件

```
sh build.sh
```

