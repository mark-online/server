-- Gideon_account_test �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_account_test ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_account_test')
	DROP DATABASE [Gideon_account_test]
GO

CREATE DATABASE [Gideon_account_test] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_account_test ����� �߰�
DROP LOGIN [Gideon_account_test]
GO

CREATE LOGIN [Gideon_account_test] WITH PASSWORD=N'Gideon_account_test',
	DEFAULT_DATABASE=[Gideon_account_test], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_account_test] ENABLE
GO

-- ========================================
-- Gideon_account_test DB�� �����ڷ� ����
USE [Gideon_account_test]
GO

-- database ����� ����
CREATE USER [Gideon_account_test] FOR LOGIN [Gideon_account_test] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_account_test'
GO

USE [master]
GO

