-- Gideon_account_product �� ��Ű�� (MS SQL Server 2005 �̻�)

-- ========================================
-- Gideon_account_product ����
USE [master]
GO

IF  EXISTS (SELECT name FROM sys.databases WHERE name = N'Gideon_account_product')
	DROP DATABASE [Gideon_account_product]
GO

CREATE DATABASE [Gideon_account_product] COLLATE Korean_Wansung_CI_AS
GO

-- ========================================
-- Gideon_account_product ����� �߰�
DROP LOGIN [Gideon_account_product]
GO

CREATE LOGIN [Gideon_account_product] WITH PASSWORD=N'Gideon_account_product',
	DEFAULT_DATABASE=[Gideon_account_product], DEFAULT_LANGUAGE=[�ѱ���],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF
GO

ALTER LOGIN [Gideon_account_product] ENABLE
GO

-- ========================================
-- Gideon_account_product DB�� �����ڷ� ����
USE [Gideon_account_product]
GO

-- database ����� ����
CREATE USER [Gideon_account_product] FOR LOGIN [Gideon_account_product] --WITH DEFAULT_SCHEMA=[db_owner]
GO

-- ���� �ο�
EXEC sp_addrolemember N'db_owner', N'Gideon_account_product'
GO
