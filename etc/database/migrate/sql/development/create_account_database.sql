-- Gideon_account_development �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_account_development ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_account_development')
	DROP DATABASE [Gideon_account_development]
GO

CREATE DATABASE [Gideon_account_development] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_account_development ����� �߰�
DROP LOGIN [Gideon_account_development]
GO

CREATE LOGIN [Gideon_account_development] WITH PASSWORD=N'Gideon_account_development',
	DEFAULT_DATABASE=[Gideon_account_development], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_account_development] ENABLE
GO

-- ========================================
-- Gideon_account_development DB�� �����ڷ� ����
USE [Gideon_account_development]
GO

-- database ����� ����
CREATE USER [Gideon_account_development] FOR LOGIN [Gideon_account_development] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_account_development'
GO
