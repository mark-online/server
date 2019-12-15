USE [master]

IF NOT EXISTS (SELECT name FROM sys.server_principals WHERE name = 'Gideon_game_product')
BEGIN
  CREATE LOGIN [Gideon_game_product] WITH PASSWORD=N'Gideon_game_product',
	DEFAULT_DATABASE=[Gideon_account_product], DEFAULT_LANGUAGE=[ÇÑ±¹¾î],
	CHECK_EXPIRATION=OFF, CHECK_POLICY=OFF		
END

USE [Gideon_account_product]
CREATE USER [Gideon_game_product] FOR LOGIN [Gideon_game_product] --WITH DEFAULT_SCHEMA=[db_owner]
EXEC sp_addrolemember N'db_owner', N'Gideon_game_product'
