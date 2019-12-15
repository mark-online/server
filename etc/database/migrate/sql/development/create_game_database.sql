-- Gideon_game_development �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_game_development ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_game_development')
	DROP DATABASE [Gideon_game_development]
GO

CREATE DATABASE [Gideon_game_development] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_game_development ����� �߰�
DROP LOGIN [Gideon_game_development]
GO

CREATE LOGIN [Gideon_game_development] WITH PASSWORD=N'Gideon_game_development',
	DEFAULT_DATABASE=[Gideon_game_development], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_game_development] ENABLE
GO

-- ========================================
-- Gideon_game_development DB�� �����ڷ� ����
USE [Gideon_game_development]
GO

-- database ����� ����
CREATE USER [Gideon_game_development] FOR LOGIN [Gideon_game_development] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_game_development'
GO

USE [master]
GO

