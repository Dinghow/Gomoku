@echo off
:main
echo ��ǰ�汾:  20161207
echo ��Ҫ��client��server�汾: 0.0.2
echo �����������bug������ϵ1652795 ��½��

:chooseServerMode
echo  ������    ��������   ��������    ��      ��   ��������   ��������
echo ��         ��         ��     ��   ��      ��   ��         ��     ��
echo ��         ��         ��     ��    ��    ��    ��         ��     ��
echo   ����     ��������   ��������      ��  ��     ��������   ��������
echo       ��   ��         ��  ��        ��  ��     ��         ��  ��
echo       ��   ��         ��   ��        ����      ��         ��   ��
echo  ������    ��������   ��    ����      ��       ��������   ��    ����
echo ��ѡ�����������ģʽ��
echo 1. Ĭ�ϵ�ַ���˿ں�
echo 2. �Զ���˿ں�
echo 3. ֱ���������˷��������û�ģʽ��
echo 4. ֱ���������˷�������AIģʽ��
echo 5. �˳�
set /p serverMode=�����룺
if %serverMode% == 1 goto defaultServer
if %serverMode% == 2 goto setServer
if %serverMode% == 3 goto connect2OthersDebug
if %serverMode% == 4 goto connect2OthersAI
if %serverMode% == 5 exit
cls
echo ѡ�����������ѡ��
goto main

:choosePlayMode
echo  ��������    ��         ������   ��������   ����      ��   ����������
echo ��      ��   ��           ��     ��         �� ��     ��       ��
echo ��           ��           ��     ��         ��  ��    ��       ��
echo ��           ��           ��     ��������   ��   ��   ��       ��
echo ��           ��           ��     ��         ��    ��  ��       ��
echo ��      ��   ��           ��     ��         ��     �� ��       ��
echo  ��������    ��������   ������   ��������   ��      ����       ��
echo ��ѡ���սģʽ��
echo 1. ��AI��ս
echo 2. һ��AIһ��Debug���û����֣�
echo 3. һ��AIһ��Debug��AI���֣�
echo 4. ����Debug
echo 5. ˫AI��ս
echo 6. ��AIģʽ���������������Լ���
echo 7. ��Debugģʽ���������������Լ���
echo 8. �����˳��ɡ�����
set /p playMode=������:
if %playMode% == 1 goto selfAiMode
if %playMode% == 2 goto singleAiModeUserFirst
if %playMode% == 3 goto singleAiModeAiFirst
if %playMode% == 4 goto doubleUserMode
if %playMode% == 5 goto doubleAiMode
if %playMode% == 6 goto singleAIConnect
if %playMode% == 7 goto singleUserConnect
if %playMode% == 8 exit
echo ѡ�����������ѡ��
goto choosePlayMode

:defaultServer
echo �Ὺ��һ����ɫ��һ���ġ���С���ġ�����������Ŷ~
set port=23333
start /min cmd /c "color F0 && cd win-server && cd src && server.exe"
goto choosePlayMode

:setServer
set /p port=������˿ںţ�
echo �Ὺ��һ����ɫ��һ���ġ���С���ġ�����������Ŷ~
start /min cmd /c "color F0 && cd win-server && cd src && server.exe -p %port%"
goto choosePlayMode

:connect2OthersDebug
echo ��ȷ����ǰ�ļ�������win-client\src\client.exe!
echo ��ģʽ���Ὺ���´��ڣ���Ϊֻ��һ�����ڣ���ʹ����Ϻ��ֶ��رռ��ɣ�
set /p connectAdd=������Է���IP��ַ��
set /p connectPort=������Է��Ķ˿ںţ�
set flag=0
if exist win-client\src\client.exe call win-client\src\client.exe -a %connectAdd% -p %connectPort%
exit

:connect2OthersAI
echo ��ģʽ���Ὺ���´��ڣ���Ϊֻ��һ�����ڣ���ʹ����Ϻ��ֶ��رռ��ɣ�
set /p connectAdd=������Է���IP��ַ��
set /p connectPort=������Է��Ķ˿ںţ�
set /p Ai=������Ai�����ļ��У�
cd %Ai%
call client.exe -a %connectAdd% -p %connectPort%
exit

:selfAiMode
set /p Ai=������Ai�����ļ��У�
start cmd /c "cd %Ai% && client.exe -p %port%"
start cmd /c "cd %Ai% && client.exe -p %port%"
goto endGame

:singleAiModeUserFirst
set /p Ai=������Ai�����ļ��У�
echo ��������ѡ��
start cmd /c "cd %Ai% && client.exe -D -p %port%"
echo ����ѡ�������ɹ�
echo ����AIѡ�֡����ڻᱻ��С����
ping 127.0.0.1 -n 2 > nul
start /min cmd /c "cd %Ai% && client.exe -p %port%"
echo AIѡ�������ɹ�
goto endGame

:singleAiModeAiFirst
set /p Ai=������Ai�����ļ��У�
echo ����AIѡ�֡����ڻᱻ��С����
start /min cmd /c "cd %Ai% && client.exe -p %port%"
echo AIѡ�������ɹ�
echo ��������ѡ��
ping 127.0.0.1 -n 2 > nul
start cmd /c "cd %Ai% && client.exe -D -p %port%"
echo ����ѡ�������ɹ�
goto endGame

:doubleUserMode
start cmd /c "cd win-client && cd src && client.exe -D -p %port%"
start cmd /c "cd win-client && cd src && client.exe -D -p %port%"
goto endGame

:doubleAiMode
set /p AiFirst=����������AI���ļ��У�
set /p AiSecond=���������AI���ļ��У�
echo ��������AI
start cmd /c "cd %AiFirst% && client.exe -p %port%"
echo ����AI�����ɹ�
echo ��������AIѡ��
ping 127.0.0.1 -n 2 > nul
start cmd /c "cd %AiSecond% && client.exe -p %port%"
echo ����AI�����ɹ�
goto endGame

:singleAIConnect
set /p Ai=������Ai�����ļ��У�
start cmd /c "cd %Ai% && client.exe -p %port%"
echo AIѡ�������ɹ�
goto endGame

:singleUserConnect
echo ��ȷ����ǰ�ļ�������win-client/src/client.exe��
start cmd /c "cd win-client\src && client.exe -D -p %port%"
goto endGame

:endGame
echo ��лʹ�ã�ף��������666��
echo ��������Զ��ر�����server��client��������������
pause > nul
taskkill /f /im server.exe
taskkill /f /im client.exe
exit