-- Gideon_game_test 용 스키마 (MS SQL Server 2005 이상)

-- ========================================
-- Gideon_game_test 생성
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_game_test')
	DROP DATABASE [Gideon_game_test]
GO

CREATE DATABASE [Gideon_game_test] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_game_test 사용자 추가
DROP LOGIN [Gideon_game_test]
GO

CREATE LOGIN [Gideon_game_test] WITH PASSWORD=N'Gideon_game_test',
	DEFAULT_DATABASE=[Gideon_game_test], DEFAULT_LANGUAGE=[한국어],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_game_test] ENABLE
GO

-- ========================================
-- Gideon_game_test DB의 소유자로 설정
USE [Gideon_game_test]
GO

-- database 사용자 생성
CREATE USER [Gideon_game_test] FOR LOGIN [Gideon_game_test] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- 역할 부여
EXEC sp_addrolemember N'db_owner', N'Gideon_game_test'
GO

USE [master]
GO

