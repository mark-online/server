-- Gideon_game_test �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_game_test ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_game_test')
	DROP DATABASE [Gideon_game_test]
GO

CREATE DATABASE [Gideon_game_test] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_game_test ����� �߰�
DROP LOGIN [Gideon_game_test]
GO

CREATE LOGIN [Gideon_game_test] WITH PASSWORD=N'Gideon_game_test',
	DEFAULT_DATABASE=[Gideon_game_test], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_game_test] ENABLE
GO

-- ========================================
-- Gideon_game_test DB�� �����ڷ� ����
USE [Gideon_game_test]
GO

-- database ����� ����
CREATE USER [Gideon_game_test] FOR LOGIN [Gideon_game_test] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_game_test'
GO

USE [master]
GO

