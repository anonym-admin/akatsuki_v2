# AKATSUKI ENGINE
이 코드는 이전 작성했던 hmk_demo 코드를 기반으로 하여 작성했습니다. DirectX12 API 를 사용한 Renderer 를 구현해 사용하고 있습니다. 해당 Renderer 는 언리얼 PBR 로 Shading 을 진행하고 있으며, 멀티쓰레드 랜더링을 지원합니다.

그림자 랜더링의 경우 Cascade Shadow Map 기술을 활용해 전역조명이 비추는것 처럼 그림자 랜더링이 가능합니다. 그림자의 경우 카메라의 이동에 따른 특정 시야각에서 짤리는 현상이 발생합니다. 뷰프러스텀에 의한 현상으로 추측되는데, 이는 현재 오류를 잡기위해 노력중입니다.

실행파일의 링크는 아래에 따로 공유합니다.

binary 파일 링크: https://github.com/anonym-admin/binary

실행파일 경로: bin/Release/akatsuki_client_x64.exe

./binary-main/binary-main/ 경로에 아래 드라이브 링크에 공유된 asset 폴더와 shader 폴더를 넣어서 사용하시면 됩니다.

에셋과 쉐이더 파일 링크: https://drive.google.com/drive/folders/1QgTWX35QcKyK2Pt71QIrIGiePa74fpGP

F5 버튼을 누르면 에디터를 선택할 수 있는 화면으로 전환되고 console 창에 진입하고 싶은 에디터의 번호를 입력하면 됩니다.

0. 모델 에디터
1. 레벨 에디터
2. 파티클 에디터

hmk_demo: https://github.com/anonym-admin/hmk_demo

기술데모영상: https://www.youtube.com/@%ED%99%8D%EB%AA%85%EA%B5%AD-n8e/videos 

## Controls
- F1:		NONE
- F2:		Debug Mode (Render Collider, wire frame)
- F5:		Change Editor Mode
- F6:		Change Scene
- F10:	    On/Off VSync
- O:		Press F2 and press (PostEffect, PostProcess Controller Flag)