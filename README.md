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

1. 配置使用默认密码
	echo "https://eager7:pct1197639@github.com" .git-credentials
	git config --global credential.helper store

	可以看到~/.gitconfig文件，会多了一项：
	[credential]
    		helper = store

2. 使用vim作为默认的编辑器
	git config --global core.editor vim

3. 基本使用命令
	查看本地版本的状态以及差异：
		pct@ubuntu-x86:~/ubuntu-x86/IOTC$ git status
		On branch master
		Your branch is up-to-date with 'origin/master'.
		Changes to be committed:
		  (use "git reset HEAD <file>..." to unstage)
			new file:   Makefile
			new file:   main.c
			new file:   utils.h
	增加到本地库：
		pct@ubuntu-x86:~/ubuntu-x86/IOTC$ git add .
	
	提交到本地：
		pct@ubuntu-x86:~/ubuntu-x86/IOTC$ git commit -a
		[master 36cad9b] Add Makefile & main.c
		 3 files changed, 153 insertions(+)
		 create mode 100755 Makefile
		 create mode 100644 main.c
		 create mode 100755 utils.h
		这样提交会弹出一个vim的文本编辑界面，写入你的注释，第一行简略描述，空一行，然后详细描述。
	
	提交到远程仓库，即网络上：
		pct@ubuntu-x86:~/ubuntu-x86/IOTC$ git push -u origin master 
		Counting objects: 6, done.
		Delta compression using up to 2 threads.
		Compressing objects: 100% (5/5), done.
		Writing objects: 100% (5/5), 1.79 KiB | 0 bytes/s, done.
		Total 5 (delta 0), reused 0 (delta 0)
		To https://github.com/eager7/IOTC
		   6c85c21..36cad9b  master -> master
		Branch master set up to track remote branch master from origin.

	下载一个分支
		git clone https://github.com/eager7/socket_comm -b epoll

