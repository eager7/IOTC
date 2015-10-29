操作手册

…or create a new repository on the command line
echo # test >> README.md

git init
新建目录后，需要进入到这个目录下，然后使用上述命令将此目录加入项目版本控制（后续还需要add等动作）

git add README.md
添加文件到项目管理中

git commit -m "first commit"
上传文件

git remote add origin https://github.com/eager7/IOTC.git
添加文件到指定地址的远程仓库

git push -u origin master
本地版本合成到远程

…or push an existing repository from the command line

git remote add origin https://github.com/eager7/IOTC.git

git push -u origin master
