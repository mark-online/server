-- Gideon_game_product 용 스키마 (MS SQL Server 2005 이상)

-- ========================================
-- Gideon_game_product 생성
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_game_product')
	DROP DATABASE [Gideon_game_product]
GO

CREATE DATABASE [Gideon_game_product] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_game_product 사용자 추가
DROP LOGIN [Gideon_game_product]
GO

CREATE LOGIN [Gideon_game_product] WITH PASSWORD=N'Gideon_game_product',
	DEFAULT_DATABASE=[Gideon_game_product], DEFAULT_LANGUAGE=[한국어],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_game_product] ENABLE
GO

-- ========================================
-- Gideon_game_product DB의 소유자로 설정
USE [Gideon_game_product]
GO

-- database 사용자 생성
CREATE USER [Gideon_game_product] FOR LOGIN [Gideon_game_product] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- 역할 부여
EXEC sp_addrolemember N'db_owner', N'Gideon_game_product'
GO

USE [master]
GO

