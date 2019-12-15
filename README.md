Gideon projects
===============

## Directories

 - /bin : exe, dll, pdb
 - /build : 빌드 스크립트
 - /doc : 문서
 - /include : 공유되는 헤더 파일
 - /init.d : 서버 구동 스크립트 파일
 - /lib : import lib, static lib
 - /src : 소스 파일
 - /test : 유닛 테스트

## Environment Variables
 - SNE_DIR : SNE의 경로
 - GIDEON_CS_DIR : GideonClientServer 프로젝트의 경로
 - GIDEON_SERVER_DIR : Gideon server 프로젝트의 경로
 - GIDEON_SERVER_HOME : Gideon server 구동 홈 경로
 - PATH
   - x64: %SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64
   - Win32: %SNE_DIR%\bin\Win32;%GIDEON_CS_DIR%\bin\Win32;%GIDEON_SERVER_DIR%\bin\Win32
