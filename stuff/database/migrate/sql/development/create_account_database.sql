-- Gideon_account_development 용 스키마 (MS SQL Server 2005 이상)

-- ========================================
-- Gideon_account_development 생성
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_account_development')
	DROP DATABASE [Gideon_account_development]
GO

CREATE DATABASE [Gideon_account_development] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_account_development 사용자 추가
DROP LOGIN [Gideon_account_development]
GO

CREATE LOGIN [Gideon_account_development] WITH PASSWORD=N'Gideon_account_development',
	DEFAULT_DATABASE=[Gideon_account_development], DEFAULT_LANGUAGE=[한국어],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_account_development] ENABLE
GO

-- ========================================
-- Gideon_account_development DB의 소유자로 설정
USE [Gideon_account_development]
GO

-- database 사용자 생성
CREATE USER [Gideon_account_development] FOR LOGIN [Gideon_account_development] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- 역할 부여
EXEC sp_addrolemember N'db_owner', N'Gideon_account_development'
GO
