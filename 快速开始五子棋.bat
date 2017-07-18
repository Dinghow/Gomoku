@echo off
:main
echo 当前版本:  20161207
echo 需要的client、server版本: 0.0.2
echo 如果您发现了bug，请联系1652795 王陆洋

:chooseServerMode
echo  ■■■    ■■■■   ■■■■    ■      ■   ■■■■   ■■■■
echo ■         ■         ■     ■   ■      ■   ■         ■     ■
echo ■         ■         ■     ■    ■    ■    ■         ■     ■
echo   ■■     ■■■■   ■■■■      ■  ■     ■■■■   ■■■■
echo       ■   ■         ■  ■        ■  ■     ■         ■  ■
echo       ■   ■         ■   ■        ■■      ■         ■   ■
echo  ■■■    ■■■■   ■    ■■      ■       ■■■■   ■    ■■
echo 请选择服务器运行模式：
echo 1. 默认地址及端口号
echo 2. 自定义端口号
echo 3. 直接连接他人服务器（用户模式）
echo 4. 直接连接他人服务器（AI模式）
echo 5. 退出
set /p serverMode=请输入：
if %serverMode% == 1 goto defaultServer
if %serverMode% == 2 goto setServer
if %serverMode% == 3 goto connect2OthersDebug
if %serverMode% == 4 goto connect2OthersAI
if %serverMode% == 5 exit
cls
echo 选择错误！请重新选择
goto main

:choosePlayMode
echo  ■■■■    ■         ■■■   ■■■■   ■■      ■   ■■■■■
echo ■      ■   ■           ■     ■         ■ ■     ■       ■
echo ■           ■           ■     ■         ■  ■    ■       ■
echo ■           ■           ■     ■■■■   ■   ■   ■       ■
echo ■           ■           ■     ■         ■    ■  ■       ■
echo ■      ■   ■           ■     ■         ■     ■ ■       ■
echo  ■■■■    ■■■■   ■■■   ■■■■   ■      ■■       ■
echo 请选择对战模式：
echo 1. 单AI自战
echo 2. 一个AI一个Debug（用户先手）
echo 3. 一个AI一个Debug（AI先手）
echo 4. 两个Debug
echo 5. 双AI对战
echo 6. 单AI模式（用于他人连接自己）
echo 7. 单Debug模式（用于他人连接自己）
echo 8. 还是退出吧。。。
set /p playMode=请输入:
if %playMode% == 1 goto selfAiMode
if %playMode% == 2 goto singleAiModeUserFirst
if %playMode% == 3 goto singleAiModeAiFirst
if %playMode% == 4 goto doubleUserMode
if %playMode% == 5 goto doubleAiMode
if %playMode% == 6 goto singleAIConnect
if %playMode% == 7 goto singleUserConnect
if %playMode% == 8 exit
echo 选择错误！请重新选择！
goto choosePlayMode

:defaultServer
echo 会开启一个颜色不一样的【最小化的】服务器窗口哦~
set port=23333
start /min cmd /c "color F0 && cd win-server && cd src && server.exe"
goto choosePlayMode

:setServer
set /p port=请输入端口号：
echo 会开启一个颜色不一样的【最小化的】服务器窗口哦~
start /min cmd /c "color F0 && cd win-server && cd src && server.exe -p %port%"
goto choosePlayMode

:connect2OthersDebug
echo 请确保当前文件夹下有win-client\src\client.exe!
echo 此模式不会开启新窗口（因为只有一个窗口），使用完毕后手动关闭即可！
set /p connectAdd=请输入对方的IP地址：
set /p connectPort=请输入对方的端口号：
set flag=0
if exist win-client\src\client.exe call win-client\src\client.exe -a %connectAdd% -p %connectPort%
exit

:connect2OthersAI
echo 此模式不会开启新窗口（因为只有一个窗口），使用完毕后手动关闭即可！
set /p connectAdd=请输入对方的IP地址：
set /p connectPort=请输入对方的端口号：
set /p Ai=请拖入Ai所在文件夹：
cd %Ai%
call client.exe -a %connectAdd% -p %connectPort%
exit

:selfAiMode
set /p Ai=请拖入Ai所在文件夹：
start cmd /c "cd %Ai% && client.exe -p %port%"
start cmd /c "cd %Ai% && client.exe -p %port%"
goto endGame

:singleAiModeUserFirst
set /p Ai=请拖入Ai所在文件夹：
echo 启动人类选手
start cmd /c "cd %Ai% && client.exe -D -p %port%"
echo 人类选手启动成功
echo 启动AI选手【窗口会被最小化】
ping 127.0.0.1 -n 2 > nul
start /min cmd /c "cd %Ai% && client.exe -p %port%"
echo AI选手启动成功
goto endGame

:singleAiModeAiFirst
set /p Ai=请拖入Ai所在文件夹：
echo 启动AI选手【窗口会被最小化】
start /min cmd /c "cd %Ai% && client.exe -p %port%"
echo AI选手启动成功
echo 启动人类选手
ping 127.0.0.1 -n 2 > nul
start cmd /c "cd %Ai% && client.exe -D -p %port%"
echo 人类选手启动成功
goto endGame

:doubleUserMode
start cmd /c "cd win-client && cd src && client.exe -D -p %port%"
start cmd /c "cd win-client && cd src && client.exe -D -p %port%"
goto endGame

:doubleAiMode
set /p AiFirst=请拖入先手AI的文件夹：
set /p AiSecond=请拖入后手AI的文件夹：
echo 启动先手AI
start cmd /c "cd %AiFirst% && client.exe -p %port%"
echo 先手AI启动成功
echo 启动后手AI选手
ping 127.0.0.1 -n 2 > nul
start cmd /c "cd %AiSecond% && client.exe -p %port%"
echo 后手AI启动成功
goto endGame

:singleAIConnect
set /p Ai=请拖入Ai所在文件夹：
start cmd /c "cd %Ai% && client.exe -p %port%"
echo AI选手启动成功
goto endGame

:singleUserConnect
echo 请确保当前文件夹下有win-client/src/client.exe！
start cmd /c "cd win-client\src && client.exe -D -p %port%"
goto endGame

:endGame
echo 感谢使用，祝您五子棋666！
echo 按任意键自动关闭所有server和client并结束本次运行
pause > nul
taskkill /f /im server.exe
taskkill /f /im client.exe
exit