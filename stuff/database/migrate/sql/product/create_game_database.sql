-- Gideon_game_product �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_game_product ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_game_product')
	DROP DATABASE [Gideon_game_product]
GO

CREATE DATABASE [Gideon_game_product] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_game_product ����� �߰�
DROP LOGIN [Gideon_game_product]
GO

CREATE LOGIN [Gideon_game_product] WITH PASSWORD=N'Gideon_game_product',
	DEFAULT_DATABASE=[Gideon_game_product], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_game_product] ENABLE
GO

-- ========================================
-- Gideon_game_product DB�� �����ڷ� ����
USE [Gideon_game_product]
GO

-- database ����� ����
CREATE USER [Gideon_game_product] FOR LOGIN [Gideon_game_product] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_game_product'
GO

USE [master]
GO

